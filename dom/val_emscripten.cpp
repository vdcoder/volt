// ============================================================================
// volt::Val – Emscripten backend
//
// Most methods are inline in val.hpp. This file provides the dom:: helpers
// and any out-of-line definitions needed for the Emscripten target.
// ============================================================================

#include "val.hpp"

namespace volt {
namespace dom {

void setAttribute(Val element, const std::string& key, const std::string& value) {
    element.call<void>("setAttribute", key, value);
}

void setAttribute(Val element, const std::string& key, Val value) {
    element.call<void>("setAttribute", key, value);
}

void removeAttribute(Val element, const std::string& key) {
    element.call<void>("removeAttribute", key);
}

Val createElement(const std::string& tagName) {
    return Val::global("document").call<Val>("createElement", tagName);
}

Val createTextNode(const std::string& text) {
    return Val::global("document").call<Val>("createTextNode", text);
}

Val getElementById(const std::string& id) {
    return Val::global("document").call<Val>("getElementById", id);
}

void insertBefore(Val parent, Val newChild, Val referenceNode) {
    parent.call<void>("insertBefore", newChild, referenceNode);
}

void appendChild(Val parent, Val child) {
    parent.call<void>("appendChild", child);
}

void removeChild(Val parent, Val child) {
    parent.call<void>("removeChild", child);
}

void replaceChild(Val parent, Val newChild, Val oldChild) {
    parent.call<void>("replaceChild", newChild, oldChild);
}

Val getChildAt(Val parent, int index) {
    return parent.call<Val>("getChildAt", index);
}

int getChildCount(Val parent) {
    return parent.call<int>("getChildCount");
}

} // namespace dom
} // namespace volt
