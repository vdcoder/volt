#include "Tags.hpp"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "StableKeyManager.hpp"
#include "VNodeHandle.hpp"

namespace volt {

namespace tag {

// Helper to convert Tag enum to string
inline const char* tagToString(ETag a_nTag) {
    switch (a_nTag) {
        case ETag::_TEXT: return "#text";
        case ETag::_FRAGMENT: return "#fragment";
        case ETag::DIV: return "div";
        case ETag::SPAN: return "span";
        case ETag::H1: return "h1";
        case ETag::H2: return "h2";
        case ETag::H3: return "h3";
        case ETag::H4: return "h4";
        case ETag::H5: return "h5";
        case ETag::H6: return "h6";
        case ETag::P: return "p";
        case ETag::A: return "a";
        case ETag::BUTTON: return "button";
        case ETag::INPUT: return "input";
        case ETag::TEXTAREA: return "textarea";
        case ETag::SELECT: return "select";
        case ETag::OPTION: return "option";
        case ETag::UL: return "ul";
        case ETag::OL: return "ol";
        case ETag::LI: return "li";
        case ETag::TABLE: return "table";
        case ETag::THEAD: return "thead";
        case ETag::TBODY: return "tbody";
        case ETag::TR: return "tr";
        case ETag::TD: return "td";
        case ETag::TH: return "th";
        case ETag::FORM: return "form";
        case ETag::LABEL: return "label";
        case ETag::IMG: return "img";
        case ETag::BR: return "br";
        case ETag::HR: return "hr";
        default: return "div";
    }
}

// ============================================================================
// HTML Element Helper Functions
// ============================================================================

// Macro to generate overloads for a tag using variadic templates
// This allows natural syntax: div({props}, child1, child2, "text", ...)
#define VNODE_TAG_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), {}, {}); \
    } \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey, std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), std::move(props), {}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey, Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), {}, {std::forward<Children>(children)...}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey, std::vector<std::pair<short, PropValueType>> props, Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), std::move(props), {std::forward<Children>(children)...}); \
    }

// Macro for self-closing tags (no children): (props), ()
#define VNODE_SELFCLOSING_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey, std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), std::move(props), {}); \
    } \
    inline VNodeHandle tagName(StableKeyManager::StableKey a_stableKey) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(a_stableKey), {}, {}); \
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

} // namespace tag

} // namespace volt