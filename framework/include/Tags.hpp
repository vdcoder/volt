#pragma once
#include "ETags.hpp"
#include "VNodeHandle.hpp"

namespace volt {

namespace tag {


// Helper to convert Tag enum to string
inline const char* tagToString(ETag tag);


// ============================================================================
// HTML Element Helper Functions
// ============================================================================

// Macro to generate overloads for a tag using variadic templates
// This allows natural syntax: div({props}, child1, child2, "text", ...)
#define VNODE_TAG_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName() { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {}); \
    } \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {std::forward<Children>(children)...}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props, Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {std::forward<Children>(children)...}); \
    }

// Macro for self-closing tags (no children): (props), ()
#define VNODE_SELFCLOSING_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {}); \
    } \
    inline VNodeHandle tagName() { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {}); \
    }

// Fragment
VNODE_TAG_HELPER(_fragment, _FRAGMENT)

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

} // namespace tag

} // namespace volt