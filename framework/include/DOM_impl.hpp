#include "DOM.hpp"

namespace volt {

namespace dom {

void setAttribute(emscripten::val a_element, std::string a_sKey, std::string a_sValue) {
    //a_element.set(a_sKey, a_sValue);
    a_element.call<void>("setAttribute", a_sKey, a_sValue);
}

void removeAttribute(emscripten::val a_element, std::string a_sKey) {
    //a_element.delete_(a_sKey);
    a_element.call<void>("removeAttribute", a_sKey);
}

emscripten::val createElement(std::string a_sTagName) {
    return emscripten::val::global("document").call<emscripten::val>("createElement", a_sTagName);
}

emscripten::val createTextNode(std::string a_sText) {
    return emscripten::val::global("document").call<emscripten::val>("createTextNode", a_sText);
}

emscripten::val getElementById(std::string a_sId) {
    return emscripten::val::global("document").call<emscripten::val>("getElementById", a_sId);
}

void appendChild(emscripten::val a_parent, emscripten::val a_child) {
    a_parent.call<void>("appendChild", a_child);
}

void removeChild(emscripten::val a_parent, emscripten::val a_child) {
    a_parent.call<void>("removeChild", a_child);
}

void replaceChild(emscripten::val a_parent, emscripten::val a_newChild, emscripten::val a_oldChild) {
    a_parent.call<void>("replaceChild", a_newChild, a_oldChild);
}

emscripten::val getChildAt(emscripten::val a_parent, int a_nIndex) {
    return a_parent.call<emscripten::val>("getChildAt", a_nIndex);
}

int getChildCount(emscripten::val a_parent) {
    return a_parent.call<int>("getChildCount");
}

} // namespace dom

}