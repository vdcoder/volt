#pragma once

#include "VNode.hpp"
#include <vector>
#include <string>
#include <map>
#include <optional>

// ============================================================================
// Diff Operations
// ============================================================================
enum class DiffOp {
    NONE,           // No changes
    REPLACE,        // Replace entire subtree (different tags or text content changed)
    UPDATE          // Update existing element (check flags for what changed)
};

// Update flags - bit-based for flexibility
enum UpdateFlags : uint8_t {
    UPDATE_PROPS    = 1 << 0,  // Props changed
    UPDATE_CHILDREN = 1 << 1   // Children changed (additions, removals, or updates)
};

// ============================================================================
// PropDiff - Describes changes to props
// ============================================================================
struct PropDiff {
    std::vector<std::pair<short, std::string>> added;  // New props or changed values
    std::vector<short> removed;                         // Props that were removed (just IDs)
    
    bool isEmpty() const {
        return added.empty() && removed.empty();
    }
};

// ============================================================================
// DiffNode - Represents a change in the tree
// ============================================================================
struct DiffNode {
    DiffOp op = DiffOp::NONE;
    uint8_t updateFlags = 0;  // Bit flags for UPDATE operations
    
    // For REPLACE: the new VNode to replace with
    std::optional<VNode> newNode;
    
    // For UPDATE with UPDATE_PROPS flag: the prop changes
    std::optional<PropDiff> propDiff;
    
    // For UPDATE with UPDATE_CHILDREN flag: map of child index -> DiffNode
    std::map<size_t, DiffNode> childrenDiff;
    
    // Track if children were added at the end
    std::vector<VNode> addedChildren;
    
    // Track removed children indices (from end, for safe removal)
    std::vector<size_t> removedChildIndices;
    
    bool hasChanges() const {
        return op != DiffOp::NONE;
    }
    
    bool hasPropsChanged() const {
        return op == DiffOp::UPDATE && (updateFlags & UPDATE_PROPS);
    }
    
    bool hasChildrenChanged() const {
        return op == DiffOp::UPDATE && (updateFlags & UPDATE_CHILDREN);
    }
};

// ============================================================================
// Diff Algorithm
// ============================================================================

// Compare props of two nodes (both vectors are already sorted!)
inline PropDiff diffProps(const std::vector<std::pair<short, std::string>>& oldProps, 
                   const std::vector<std::pair<short, std::string>>& newProps) {
    PropDiff result;
    
    size_t oldIdx = 0;
    size_t newIdx = 0;
    
    // Walk both sorted vectors in parallel
    while (oldIdx < oldProps.size() || newIdx < newProps.size()) {
        if (oldIdx >= oldProps.size()) {
            // Remaining items in new are additions
            result.added.push_back(newProps[newIdx]);
            ++newIdx;
        } else if (newIdx >= newProps.size()) {
            // Remaining items in old are removals
            result.removed.push_back(oldProps[oldIdx].first);
            ++oldIdx;
        } else if (oldProps[oldIdx].first < newProps[newIdx].first) {
            // Old key not in new = removal
            result.removed.push_back(oldProps[oldIdx].first);
            ++oldIdx;
        } else if (newProps[newIdx].first < oldProps[oldIdx].first) {
            // New key not in old = addition
            result.added.push_back(newProps[newIdx]);
            ++newIdx;
        } else {
            // Same key, check if value changed
            if (oldProps[oldIdx].second != newProps[newIdx].second) {
                result.added.push_back(newProps[newIdx]);
            }
            ++oldIdx;
            ++newIdx;
        }
    }
    
    return result;
}

// Forward declaration
inline DiffNode diffNodes(const VNode& oldNode, const VNode& newNode);

// Diff children recursively
inline std::map<size_t, DiffNode> diffChildren(const std::vector<VNode>& oldChildren,
                                        const std::vector<VNode>& newChildren,
                                        std::vector<VNode>& addedChildren,
                                        std::vector<size_t>& removedIndices) {
    std::map<size_t, DiffNode> result;
    
    size_t minSize = std::min(oldChildren.size(), newChildren.size());
    
    // Compare existing children at same positions
    for (size_t i = 0; i < minSize; ++i) {
        DiffNode childDiff = diffNodes(oldChildren[i], newChildren[i]);
        if (childDiff.hasChanges()) {
            result[i] = std::move(childDiff);
        }
    }
    
    // Handle additions (new children beyond old length)
    if (newChildren.size() > oldChildren.size()) {
        for (size_t i = oldChildren.size(); i < newChildren.size(); ++i) {
            addedChildren.push_back(newChildren[i]);
        }
    }
    
    // Handle removals (old children beyond new length)
    if (oldChildren.size() > newChildren.size()) {
        for (size_t i = newChildren.size(); i < oldChildren.size(); ++i) {
            removedIndices.push_back(i);
        }
    }
    
    return result;
}

// Main diff function
inline DiffNode diffNodes(const VNode& oldNode, const VNode& newNode) {
    DiffNode diff;
    
    // Case 1: Different tags -> REPLACE entire subtree
    if (oldNode.tag != newNode.tag) {
        diff.op = DiffOp::REPLACE;
        diff.newNode = newNode;
        return diff;
    }
    
    // Case 2: Text nodes -> Check if content changed
    if (oldNode.isText() && newNode.isText()) {
        if (oldNode.getText() != newNode.getText()) {
            diff.op = DiffOp::REPLACE;
            diff.newNode = newNode;
        }
        // If text is same, no changes - return diff with no hasChanges()
        return diff;
    }
    
    // Case 3: Same tag (element node) -> Compare props and children
    PropDiff propDiff = diffProps(oldNode.props, newNode.props);
    bool propsChanged = !propDiff.isEmpty();
    
    std::vector<VNode> addedChildren;
    std::vector<size_t> removedIndices;
    std::map<size_t, DiffNode> childrenDiff = diffChildren(
        oldNode.children, 
        newNode.children,
        addedChildren,
        removedIndices
    );
    
    bool childrenChanged = !childrenDiff.empty() || !addedChildren.empty() || !removedIndices.empty();
    
    // Determine the operation
    if (propsChanged || childrenChanged) {
        diff.op = DiffOp::UPDATE;
        
        if (propsChanged) {
            diff.updateFlags |= UPDATE_PROPS;
            diff.propDiff = std::move(propDiff);
        }
        
        if (childrenChanged) {
            diff.updateFlags |= UPDATE_CHILDREN;
            diff.childrenDiff = std::move(childrenDiff);
            diff.addedChildren = std::move(addedChildren);
            diff.removedChildIndices = std::move(removedIndices);
        }
    }
    // else: op remains NONE (no changes)
    
    return diff;
}

// Entry point for diffing two trees
inline DiffNode diff(const VNode& oldRoot, const VNode& newRoot) {
    return diffNodes(oldRoot, newRoot);
}
