#pragma once

// ============================================================================
// volt::Val - Abstract JS Value type
//
// Platform-agnostic wrapper for JavaScript values, abstracting over
// emscripten::val (Emscripten/WASM) and JSValueRef (Ultralight/native).
//
// Usage:
//   volt::Val doc = volt::Val::global("document");
//   volt::Val el  = doc.call<volt::Val>("createElement", "div");
//   el.set("innerHTML", volt::Val("Hello"));
//   std::string html = el["innerHTML"].as<std::string>();
//
// Targets selected at compile time:
//   -D VOLT_TARGET_EMSCRIPTEN   →  wraps emscripten::val
//   -D VOLT_TARGET_ULTRALIGHT   →  wraps JSValueRef (JSC)
// ============================================================================

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>
#include <type_traits>

#if defined(VOLT_TARGET_EMSCRIPTEN)
#  include <emscripten/val.h>
#elif defined(VOLT_TARGET_ULTRALIGHT)
// Ultralight backend headers included in .cpp
#else
#  error "Define VOLT_TARGET_EMSCRIPTEN or VOLT_TARGET_ULTRALIGHT"
#endif

namespace volt {

// ============================================================================
// volt::Val - Primary JS Value Type
// ============================================================================

#if defined(VOLT_TARGET_EMSCRIPTEN)

// ---- Emscripten backend -----------------------------------------------

class Val {
public:
    // Construction from C++ primitives
    Val()                            : m_v(emscripten::val()) {}
    Val(int v)                       : m_v(emscripten::val(v)) {}
    Val(unsigned v)                  : m_v(emscripten::val(v)) {}
    Val(long v)                      : m_v(emscripten::val(v)) {}
    Val(unsigned long v)             : m_v(emscripten::val(v)) {}
    Val(float v)                     : m_v(emscripten::val(v)) {}
    Val(double v)                    : m_v(emscripten::val(v)) {}
    Val(const char* v)               : m_v(emscripten::val(v)) {}
    Val(const std::string& v)        : m_v(emscripten::val(v)) {}
    Val(std::nullptr_t)              : m_v(emscripten::val::null()) {}

    // Factories
    static Val undefined()           { return Val(emscripten::val::undefined()); }
    static Val null()                { return Val(emscripten::val::null()); }
    static Val global(const char* n) { return Val(emscripten::val::global(n)); }
    static Val module_property(const char* n) {
        return Val(emscripten::val::module_property(n));
    }

    // Raw handle access (low-level interop)
    const emscripten::val& raw() const { return m_v; }
    emscripten::val& raw()             { return m_v; }

    // JS method call
    template<typename T = Val, typename... A>
    T call(const char* n, A&&... a) const {
        if constexpr (std::is_same_v<T, Val>)
            return Val(m_v.call<emscripten::val>(n, std::forward<A>(a)...));
        else if constexpr (std::is_same_v<T, std::string>)
            return m_v.call<std::string>(n, std::forward<A>(a)...);
        else if constexpr (std::is_same_v<T, int>)
            return m_v.call<int>(n, std::forward<A>(a)...);
        else if constexpr (std::is_same_v<T, double>)
            return m_v.call<double>(n, std::forward<A>(a)...);
        else if constexpr (std::is_same_v<T, bool>)
            return m_v.call<bool>(n, std::forward<A>(a)...);
        else
            return m_v.call<T>(n, std::forward<A>(a)...);
    }

    template<typename... A>
    void call(const char* n, A&&... a) const {
        m_v.call<void>(n, std::forward<A>(a)...);
    }

    // Property access
    Val operator[](const char* k) const { return Val(m_v[k]); }
    void set(const char* k, const Val& v)       { m_v.set(k, v.m_v); }
    void set(const char* k, const std::string& v) { m_v.set(k, emscripten::val(v)); }
    void set(const char* k, int v)              { m_v.set(k, emscripten::val(v)); }
    void set(const char* k, std::nullptr_t)     { m_v.set(k, emscripten::val::null()); }

    // Type conversion
    template<typename T> T as() const            { return m_v.as<T>(); }

    // intptr_t helpers (for __cpp_ptr storage)
    intptr_t as_intptr() const                   { return m_v.as<intptr_t>(); }

    // State checks
    bool isUndefined() const                     { return m_v.isUndefined(); }
    bool isNull() const                          { return m_v.isNull(); }
    bool hasOwnProperty(const char* k) const     { return m_v.hasOwnProperty(k); }
    bool isTruthy() const                        { return m_v.as<bool>(); }

    // Property deletion
    void delete_(const char* k)                  { m_v.delete_(k); }

    // Hash / handle
    uintptr_t handle() const                     { return m_v.as_handle(); }

    // Equality (strict JS equality via handle comparison)
    bool operator==(const Val& o) const { return handle() == o.handle(); }
    bool operator!=(const Val& o) const { return !(*this == o); }

    // Copy / move (emscripten::val has shared semantics via GC)
    Val(const Val&) = default;
    Val& operator=(const Val&) = default;
    Val(Val&&) = default;
    Val& operator=(Val&&) = default;

private:
    explicit Val(const emscripten::val& v) : m_v(v) {}
    emscripten::val m_v;
};

struct ValHash {
    std::size_t operator()(const Val& v) const noexcept {
        return std::hash<uintptr_t>{}(v.handle());
    }
};

#elif defined(VOLT_TARGET_ULTRALIGHT)

// ---- Ultralight / JavaScriptCore backend ------------------------------

// Opaque JSC types (forward declared — actual includes in .cpp)
using JSContextRef = const struct OpaqueJSContext*;
using JSObjectRef  = struct OpaqueJSValue*;
using JSValueRef   = const struct OpaqueJSValue*;

class Val {
public:
    Val();
    Val(int v);
    Val(unsigned v);
    Val(long v);
    Val(unsigned long v);
    Val(float v);
    Val(double v);
    Val(const char* v);
    Val(const std::string& v);
    Val(std::nullptr_t);

    Val(const Val&);
    Val& operator=(const Val&);
    Val(Val&&);
    Val& operator=(Val&&);
    ~Val();

    static Val undefined();
    static Val null();
    static Val global(const char* name);
    static Val module_property(const char* name);

    template<typename T = Val, typename... A>
    T call(const char* name, A&&... a) const;

    template<typename... A>
    void call(const char* name, A&&... a) const;

    Val operator[](const char* key) const;
    void set(const char* key, const Val& value);
    void set(const char* key, const std::string& value);
    void set(const char* key, int value);
    void set(const char* key, std::nullptr_t);

    // Type conversions
    template<typename T> T as() const;
    intptr_t as_intptr() const;

    bool isUndefined() const;
    bool isNull() const;
    bool hasOwnProperty(const char* key) const;
    bool isTruthy() const;

    void delete_(const char* key);

    uintptr_t handle() const;

    bool operator==(const Val& other) const;
    bool operator!=(const Val& other) const { return !(*this == other); }

private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

struct ValHash {
    std::size_t operator()(const Val& v) const noexcept;
};

#endif // platform

// ============================================================================
// DOM helpers (abstract version of DOM.hpp)
// ============================================================================

namespace dom {

void setAttribute(Val element, const std::string& key, const std::string& value);
void setAttribute(Val element, const std::string& key, Val value);
void removeAttribute(Val element, const std::string& key);
Val  createElement(const std::string& tagName);
Val  createTextNode(const std::string& text);
Val  getElementById(const std::string& id);
void insertBefore(Val parent, Val newChild, Val referenceNode);
void appendChild(Val parent, Val child);
void removeChild(Val parent, Val child);
void replaceChild(Val parent, Val newChild, Val oldChild);
Val  getChildAt(Val parent, int index);
int  getChildCount(Val parent);

} // namespace dom

} // namespace volt
