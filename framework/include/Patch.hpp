#pragma once

#include "VNode.hpp"
#include "Diff.hpp"
#include <emscripten/val.h>
#include <emscripten/emscripten.h>

using emscripten::EM_VAL;

// ============================================================================
// DOM Manipulation - Low-level EM_JS functions
// ============================================================================

// Create a DOM element by tag name
EM_JS(EM_VAL, dom_createElement, (const char* tagName), {
    const element = document.createElement(UTF8ToString(tagName));
    return Emval.toHandle(element);
});

// Create a text node
EM_JS(EM_VAL, dom_createTextNode, (const char* text), {
    const textNode = document.createTextNode(UTF8ToString(text));
    return Emval.toHandle(textNode);
});

// Set an attribute on an element
EM_JS(void, dom_setAttribute, (EM_VAL elementHandle, const char* key, const char* value), {
    const element = Emval.toValue(elementHandle);
    element.setAttribute(UTF8ToString(key), UTF8ToString(value));
});

// Remove an attribute from an element
EM_JS(void, dom_removeAttribute, (EM_VAL elementHandle, const char* key), {
    const element = Emval.toValue(elementHandle);
    element.removeAttribute(UTF8ToString(key));
});

// Append a child to a parent element
EM_JS(void, dom_appendChild, (EM_VAL parentHandle, EM_VAL childHandle), {
    const parent = Emval.toValue(parentHandle);
    const child = Emval.toValue(childHandle);
    parent.appendChild(child);
});

// Remove a child from a parent element
EM_JS(void, dom_removeChild, (EM_VAL parentHandle, EM_VAL childHandle), {
    const parent = Emval.toValue(parentHandle);
    const child = Emval.toValue(childHandle);
    parent.removeChild(child);
});

// Replace an old child with a new child
EM_JS(void, dom_replaceChild, (EM_VAL parentHandle, EM_VAL newChildHandle, EM_VAL oldChildHandle), {
    const parent = Emval.toValue(parentHandle);
    const newChild = Emval.toValue(newChildHandle);
    const oldChild = Emval.toValue(oldChildHandle);
    parent.replaceChild(newChild, oldChild);
});

// Get a child at a specific index
EM_JS(EM_VAL, dom_getChildAt, (EM_VAL parentHandle, int index), {
    const parent = Emval.toValue(parentHandle);
    const child = parent.childNodes[index];
    return child ? Emval.toHandle(child) : 0;
});

// Get the number of children
EM_JS(int, dom_getChildCount, (EM_VAL parentHandle), {
    const parent = Emval.toValue(parentHandle);
    return parent.childNodes.length;
});

// Set text content of a text node
EM_JS(void, dom_setTextContent, (EM_VAL nodeHandle, const char* text), {
    const node = Emval.toValue(nodeHandle);
    node.textContent = UTF8ToString(text);
});

// ============================================================================
// VNode Rendering - Create DOM from VNode
// ============================================================================

// Forward declaration
EM_VAL renderVNode(const VNode& vnode);

// Render children and append to parent
void renderChildren(EM_VAL parentHandle, const std::vector<VNode>& children) {
    for (const auto& child : children) {
        // Flatten fragments - render their children directly
        if (child.isFragment()) {
            renderChildren(parentHandle, child.children);
        } else {
            EM_VAL childHandle = renderVNode(child);
            dom_appendChild(parentHandle, childHandle);
            emscripten::internal::_emval_decref(childHandle);
        }
    }
}

// Render a VNode to a DOM element
// Render a VNode into a new DOM element
inline EM_VAL renderVNode(const VNode& vnode) {
    EM_VAL elementHandle;
    
    if (vnode.isText()) {
        // Create text node
        elementHandle = dom_createTextNode(vnode.getText().c_str());
    } else {
        // Create element
        const char* tagName = tagToString(vnode.tag);
        elementHandle = dom_createElement(tagName);
        
        // Set attributes (convert attr ID to name, skip internal IDs)
        for (const auto& [attrId, value] : vnode.props) {
            if (attrId >= 0) {  // Skip negative IDs (internal use only)
                const char* attrName = attrIdToName(attrId);
                dom_setAttribute(elementHandle, attrName, value.c_str());
            }
        }
        
        // Render and append children (fragments are flattened in renderChildren)
        renderChildren(elementHandle, vnode.children);
    }
    
    return elementHandle;
}

// ============================================================================
// Patching - Apply DiffNode to existing DOM
// ============================================================================

// Forward declaration
void patchNode(EM_VAL domElement, const DiffNode& diff);

// Patch props on an element
void patchProps(EM_VAL domElement, const PropDiff& propDiff) {
    // Add or update props (convert attr ID to name, skip internal IDs)
    for (const auto& [attrId, value] : propDiff.added) {
        if (attrId >= 0) {  // Skip negative IDs (internal use only)
            const char* attrName = attrIdToName(attrId);
            dom_setAttribute(domElement, attrName, value.c_str());
        }
    }
    
    // Remove props (convert attr ID to name, skip internal IDs)
    for (const auto& attrId : propDiff.removed) {
        if (attrId >= 0) {  // Skip negative IDs (internal use only)
            const char* attrName = attrIdToName(attrId);
            dom_removeAttribute(domElement, attrName);
        }
    }
}

// Patch children recursively
inline void patchChildren(EM_VAL domElement, 
                   const std::map<size_t, DiffNode>& childrenDiff,
                   const std::vector<VNode>& addedChildren,
                   const std::vector<size_t>& removedIndices) {
    // 1. Patch existing children that have diffs
    for (const auto& [index, childDiff] : childrenDiff) {
        EM_VAL childElement = dom_getChildAt(domElement, index);
        if (childElement) {
            patchNode(childElement, childDiff);
            emscripten::internal::_emval_decref(childElement);
        }
    }
    
    // 2. Remove children (iterate backwards to maintain indices)
    for (auto it = removedIndices.rbegin(); it != removedIndices.rend(); ++it) {
        EM_VAL childToRemove = dom_getChildAt(domElement, *it);
        if (childToRemove) {
            dom_removeChild(domElement, childToRemove);
            emscripten::internal::_emval_decref(childToRemove);
        }
    }
    
    // 3. Append new children at the end
    for (const auto& newChild : addedChildren) {
        EM_VAL newChildElement = renderVNode(newChild);
        if (newChildElement) {
            dom_appendChild(domElement, newChildElement);
            emscripten::internal::_emval_decref(newChildElement);
        }
    }
}

// Main patch function - recursively apply diff to DOM
// Patch a single node based on diff operation
inline void patchNode(EM_VAL domElement, const DiffNode& diff) {
    if (diff.op == DiffOp::NONE) {
        return; // No changes
    }
    
    if (diff.op == DiffOp::REPLACE) {
        // Replace entire subtree
        if (diff.newNode) {
            EM_VAL newElement = renderVNode(*diff.newNode);
            
            // Get parent to do replacement
            emscripten::val domVal = emscripten::val::take_ownership(domElement);
            emscripten::internal::_emval_incref(domElement); // Keep alive
            
            emscripten::val parent = domVal["parentNode"];
            if (!parent.isNull() && !parent.isUndefined()) {
                EM_VAL parentHandle = parent.as_handle();
                dom_replaceChild(parentHandle, newElement, domElement);
            }
            
            emscripten::internal::_emval_decref(newElement);
        }
        return;
    }
    
    if (diff.op == DiffOp::UPDATE) {
        // Update props if needed
        if (diff.hasPropsChanged() && diff.propDiff) {
            patchProps(domElement, *diff.propDiff);
        }
        
        // Update children if needed
        if (diff.hasChildrenChanged()) {
            patchChildren(
                domElement,
                diff.childrenDiff,
                diff.addedChildren,
                diff.removedChildIndices
            );
        }
    }
}

// Entry point - patch a root element
// Entry point for patching the DOM with a diff
inline void patch(EM_VAL rootElement, const DiffNode& diff) {
    patchNode(rootElement, diff);
}
