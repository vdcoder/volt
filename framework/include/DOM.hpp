#pragma once

// ============================================================================
// DOM Manipulation
// ============================================================================

#include <emscripten/val.h>

namespace volt {

namespace dom {

void setAttribute(emscripten::val a_element, std::string a_sKey, std::string a_sValue);

void removeAttribute(emscripten::val a_element, std::string a_sKey);

emscripten::val createElement(std::string a_sTagName);

emscripten::val createTextNode(std::string a_sText);

emscripten::val getElementById(std::string a_sId);

void insertBefore(emscripten::val a_parent, emscripten::val a_newChild, emscripten::val a_referenceNode);

void appendChild(emscripten::val a_parent, emscripten::val a_child);

void removeChild(emscripten::val a_parent, emscripten::val a_child);

void replaceChild(emscripten::val a_parent, emscripten::val a_newChild, emscripten::val a_oldChild);

emscripten::val getChildAt(emscripten::val a_parent, int a_nIndex);

int getChildCount(emscripten::val a_parent);

} // namespace dom

}