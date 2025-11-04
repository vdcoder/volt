#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "String.hpp"
#include "Attrs.hpp"

// ============================================================================
// HTML Tag Enum - For performance
// ============================================================================
enum class Tag {
    TEXT,       // Special tag for text nodes
    FRAGMENT,   // Special tag for fragment containers (invisible wrapper)
    DIV,
    SPAN,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    P,
    A,
    BUTTON,
    INPUT,
    TEXTAREA,
    SELECT,
    OPTION,
    UL,
    OL,
    LI,
    TABLE,
    THEAD,
    TBODY,
    TR,
    TD,
    TH,
    FORM,
    LABEL,
    IMG,
    BR,
    HR,
    // Add more as needed
};

// Helper to convert Tag enum to string
inline const char* tagToString(Tag tag) {
    switch (tag) {
        case Tag::TEXT: return "#text";
        case Tag::FRAGMENT: return "#fragment";
        case Tag::DIV: return "div";
        case Tag::SPAN: return "span";
        case Tag::H1: return "h1";
        case Tag::H2: return "h2";
        case Tag::H3: return "h3";
        case Tag::H4: return "h4";
        case Tag::H5: return "h5";
        case Tag::H6: return "h6";
        case Tag::P: return "p";
        case Tag::A: return "a";
        case Tag::BUTTON: return "button";
        case Tag::INPUT: return "input";
        case Tag::TEXTAREA: return "textarea";
        case Tag::SELECT: return "select";
        case Tag::OPTION: return "option";
        case Tag::UL: return "ul";
        case Tag::OL: return "ol";
        case Tag::LI: return "li";
        case Tag::TABLE: return "table";
        case Tag::THEAD: return "thead";
        case Tag::TBODY: return "tbody";
        case Tag::TR: return "tr";
        case Tag::TD: return "td";
        case Tag::TH: return "th";
        case Tag::FORM: return "form";
        case Tag::LABEL: return "label";
        case Tag::IMG: return "img";
        case Tag::BR: return "br";
        case Tag::HR: return "hr";
        default: return "div";
    }
}

// ============================================================================
// VNode - Virtual DOM Node
// ============================================================================
class VNode {
public:
    Tag tag;
    std::vector<std::pair<short, std::string>> props;  // Changed from map to vector!
    std::vector<VNode> children;

    // Constructor for element nodes
    VNode(Tag t, std::vector<std::pair<short, std::string>> p = {}, std::vector<VNode> c = {})
        : tag(t), props(std::move(p)), children(std::move(c)) {
        // Sort props by attribute ID for efficient diffing
        std::sort(props.begin(), props.end(), 
            [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    
    // Implicit conversion from std::string to text node
    VNode(const std::string& textContent)
        : tag(Tag::TEXT), props({{ATTR_TEXT_CONTENT, textContent}}), children({}) {}
    
    // Implicit conversion from const char* to text node
    VNode(const char* textContent)
        : tag(Tag::TEXT), props({{ATTR_TEXT_CONTENT, textContent}}), children({}) {}

    // Check if this is a text node
    bool isText() const {
        return tag == Tag::TEXT;
    }
    
    // Check if this is a fragment node
    bool isFragment() const {
        return tag == Tag::FRAGMENT;
    }

    // Get text content (only valid for TEXT nodes)
    std::string getText() const {
        if (isText() && !props.empty() && props[0].first == ATTR_TEXT_CONTENT) {
            return props[0].second;
        }
        return "";
    }
};

// ============================================================================
// Helper Functions - Create text nodes
// ============================================================================
inline VNode text(const std::string& content) {
    // Text nodes use a special prop to store content
    return VNode(Tag::TEXT, {{ATTR_TEXT_CONTENT, content}});
}

inline VNode text(const String& content) {
    return VNode(Tag::TEXT, {{ATTR_TEXT_CONTENT, content.std_str()}});
}

inline VNode text(const char* content) {
    return VNode(Tag::TEXT, {{ATTR_TEXT_CONTENT, content}});
}

// ============================================================================
// Fragment Helper - Transparent container for grouping nodes
// ============================================================================
inline VNode fragment(std::vector<VNode> children) {
    return VNode(Tag::FRAGMENT, {}, std::move(children));
}

// Map helper - Creates a fragment with mapped children
template<typename Container, typename Func>
inline VNode map(const Container& container, Func mapper) {
    std::vector<VNode> children;
    children.reserve(container.size());
    for (const auto& item : container) {
        children.push_back(mapper(item));
    }
    return fragment(std::move(children));
}

// ============================================================================
// HTML Element Helper Functions (with elegant overloads)
// ============================================================================

// Macro to generate overloads for a tag using variadic templates
// This allows natural syntax: div({props}, child1, child2, "text", ...)
#define VNODE_TAG_HELPER(tagName, tagEnum) \
    inline VNode tagName() { \
        return VNode(Tag::tagEnum, {}, {}); \
    } \
    inline VNode tagName(std::vector<std::pair<short, std::string>> props) { \
        return VNode(Tag::tagEnum, std::move(props), {}); \
    } \
    template<typename... Children> \
    inline VNode tagName(Children&&... children) { \
        return VNode(Tag::tagEnum, {}, {std::forward<Children>(children)...}); \
    } \
    template<typename... Children> \
    inline VNode tagName(std::vector<std::pair<short, std::string>> props, Children&&... children) { \
        return VNode(Tag::tagEnum, std::move(props), {std::forward<Children>(children)...}); \
    }

// Macro for self-closing tags (no children): (props), ()
#define VNODE_SELFCLOSING_HELPER(tagName, tagEnum) \
    inline VNode tagName(std::vector<std::pair<short, std::string>> props) { \
        return VNode(Tag::tagEnum, std::move(props), {}); \
    } \
    inline VNode tagName() { \
        return VNode(Tag::tagEnum, {}, {}); \
    }

// Common HTML elements
VNODE_TAG_HELPER(div, DIV)
VNODE_TAG_HELPER(span, SPAN)
VNODE_TAG_HELPER(h1, H1)
VNODE_TAG_HELPER(h2, H2)
VNODE_TAG_HELPER(h3, H3)
VNODE_TAG_HELPER(h4, H4)
VNODE_TAG_HELPER(h5, H5)
VNODE_TAG_HELPER(h6, H6)
VNODE_TAG_HELPER(p, P)
VNODE_TAG_HELPER(a, A)
VNODE_TAG_HELPER(button, BUTTON)
VNODE_TAG_HELPER(textarea, TEXTAREA)
VNODE_TAG_HELPER(select, SELECT)
VNODE_TAG_HELPER(option, OPTION)
VNODE_TAG_HELPER(ul, UL)
VNODE_TAG_HELPER(ol, OL)
VNODE_TAG_HELPER(li, LI)
VNODE_TAG_HELPER(table, TABLE)
VNODE_TAG_HELPER(thead, THEAD)
VNODE_TAG_HELPER(tbody, TBODY)
VNODE_TAG_HELPER(tr, TR)
VNODE_TAG_HELPER(td, TD)
VNODE_TAG_HELPER(th, TH)
VNODE_TAG_HELPER(form, FORM)
VNODE_TAG_HELPER(label, LABEL)

// Self-closing elements
VNODE_SELFCLOSING_HELPER(input, INPUT)
VNODE_SELFCLOSING_HELPER(img, IMG)
VNODE_SELFCLOSING_HELPER(br, BR)
VNODE_SELFCLOSING_HELPER(hr, HR)