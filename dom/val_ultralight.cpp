// ============================================================================
// volt::Val – Ultralight / JavaScriptCore backend
//
// Wraps JavaScriptCore JSValueRef / JSObjectRef for native desktop debugging.
//
// NOTE: This file is the skeleton for the Ultralight backend. It will be
// fleshed out when the Ultralight SDK is integrated and the CMake native
// toolchain is ready. For now it exists to validate the interface.
// ============================================================================

#include "val.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

// For now, the Ultralight backend is stubbed. These will be replaced with
// real JavaScriptCore calls once the SDK is linked.
//
// The key mapping for each operation:
//
//   emscripten::val                JSValueRef / JSObjectRef
//   ─────────────────────────────  ─────────────────────────────────
//   val::global("document")        JSContextGetGlobalObject(ctx) → "document"
//   val::module_property("fn")     <Ultralight-specific: View::EvaluateScript>
//   val("hello")                   JSValueMakeString(ctx, JSStringCreateWithUTF8CString("hello"))
//   val(42)                        JSValueMakeNumber(ctx, 42)
//   val.call("method", args)       JSObjectCallAsFunction(ctx, obj, thisObj, argc, argv, &exc)
//   val["prop"]                    JSObjectGetProperty(ctx, obj, propName, &exc)
//   val.set("prop", v)             JSObjectSetProperty(ctx, obj, propName, v, kJSPropertyAttributeNone, &exc)
//   val.as<std::string>()          JSValueToStringCopy(ctx, val, &exc) → JSStringGetUTF8CStringPtr()
//   val.as<int>()                  (int)JSValueToNumber(ctx, val, &exc)
//   val.isUndefined()              JSValueIsUndefined(ctx, val)
//   val.isNull()                   JSValueIsNull(ctx, val)
//   val.hasOwnProperty("p")        JSObjectHasProperty(ctx, obj, propName)

namespace volt {

// ---- Val constructors --------------------------------------------------

Val::Val() : m_impl(nullptr) {}
Val::Val(int) : m_impl(nullptr) {}
Val::Val(unsigned) : m_impl(nullptr) {}
Val::Val(long) : m_impl(nullptr) {}
Val::Val(unsigned long) : m_impl(nullptr) {}
Val::Val(float) : m_impl(nullptr) {}
Val::Val(double) : m_impl(nullptr) {}
Val::Val(const char*) : m_impl(nullptr) {}
Val::Val(const std::string&) : m_impl(nullptr) {}
Val::Val(std::nullptr_t) : m_impl(nullptr) {}

Val::Val(const Val&) = default;
Val& Val::operator=(const Val&) = default;
Val::Val(Val&&) = default;
Val& Val::operator=(Val&&) = default;
Val::~Val() = default;

// ---- Factories (stubs) -------------------------------------------------

Val Val::undefined() { return Val(); }
Val Val::null()      { return Val(nullptr); }
Val Val::global(const char*) { return Val(); }
Val Val::module_property(const char*) { return Val(); }

// ---- Stubs for remaining methods ---------------------------------------

Val Val::operator[](const char*) const { return Val(); }
void Val::set(const char*, const Val&) {}
void Val::set(const char*, const std::string&) {}
void Val::set(const char*, int) {}
void Val::set(const char*, std::nullptr_t) {}
intptr_t Val::as_intptr() const { return 0; }
bool Val::isUndefined() const { return true; }
bool Val::isNull() const { return true; }
bool Val::hasOwnProperty(const char*) const { return false; }
bool Val::isTruthy() const { return false; }
void Val::delete_(const char*) {}
uintptr_t Val::handle() const { return 0; }
bool Val::operator==(const Val&) const { return false; }

std::size_t ValHash::operator()(const Val&) const noexcept { return 0; }

// ---- DOM helpers (stubs) ------------------------------------------------

namespace dom {

void setAttribute(Val, const std::string&, const std::string&) {}
void setAttribute(Val, const std::string&, Val) {}
void removeAttribute(Val, const std::string&) {}
Val createElement(const std::string&) { return Val(); }
Val createTextNode(const std::string&) { return Val(); }
Val getElementById(const std::string&) { return Val(); }
void insertBefore(Val, Val, Val) {}
void appendChild(Val, Val) {}
void removeChild(Val, Val) {}
void replaceChild(Val, Val, Val) {}
Val getChildAt(Val, int) { return Val(); }
int getChildCount(Val) { return 0; }

} // namespace dom

} // namespace volt
