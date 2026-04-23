#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <cstdint>
#include <span>
#include <type_traits>

namespace volt::emscripten_remote {

#ifdef EM_REMOTE  // native + remote browser mode

// Distinguish null and undefined for people who care
struct JSNull {};
struct JSUndefined {};

using JSPrimitive = std::variant<
    JSNull,
    JSUndefined,
    bool,
    double,
    std::string
>;

// Forward decl for transport; you can define this elsewhere.
struct Transport;

// This val models something like emscripten::val in spirit.
class val {
public:
    using handle_type = std::intptr_t;

    // Constructors
    val() = default;                         // default = undefined
    explicit val(handle_type handle);        // internal: from handle
    val(const char* s);                      // "..." literal
    val(std::string_view s);
    val(bool b);
    val(double d);
    static val null();
    static val undefined();

    // Factory helpers
    static val global(std::string_view name);   // equivalent of emscripten::val::global
    static val object();                        // {}
    static val array();                         // []

    // Property access
    val get(std::string_view key) const;
    void set(std::string_view key, const val& value);
    void set(std::string_view key, JSPrimitive primitive);
    void delete_(std::string_view key);

    // Convenient operator[]
    val operator[](std::string_view key) const;
    val operator[](const char* key) const { return (*this)[std::string_view{key}]; }

    // Function calls on this val (treated as JS function or object with method)
    template<typename T = val, typename... Args>
    T call(std::string_view funcName, Args&&... args) const;

    // Direct operator() call if this val is a function
    template<typename T = val, typename... Args>
    T operator()(Args&&... args) const {
        return call<T>("", std::forward<Args>(args)...);
    }

    // Coercion
    template<typename T>
    T as() const;

    // Introspection (optional)
    bool is_null() const;
    bool is_undefined() const;

    // Internal handle access (for advanced users / bridging)
    handle_type handle() const { return m_handle; }

private:
    handle_type m_handle = 0;            // 0 could mean "undefined" or "uninitialized"
};

// ===============================
// Implementation sketch (REMOTE)
// ===============================

namespace detail {

    // global transport; you can later make this more configurable
    Transport& transport();

    // Convert C++ args into JSPrimitive/val for serialization
    inline JSPrimitive to_js_primitive(const val& v) = delete; // disallow: val is handled separately

    inline JSPrimitive to_js_primitive(bool b)    { return JSPrimitive{b}; }
    inline JSPrimitive to_js_primitive(double d)  { return JSPrimitive{d}; }
    inline JSPrimitive to_js_primitive(int i)     { return JSPrimitive{static_cast<double>(i)}; }
    inline JSPrimitive to_js_primitive(std::string s) { return JSPrimitive{std::move(s)}; }
    inline JSPrimitive to_js_primitive(std::string_view sv) { return JSPrimitive{std::string(sv)}; }
    inline JSPrimitive to_js_primitive(const char* s) { return JSPrimitive{std::string(s)}; }

    template<typename T>
    concept JsReturnable = std::is_same_v<T, void> ||
                           std::is_same_v<T, bool> ||
                           std::is_same_v<T, double> ||
                           std::is_same_v<T, std::string> ||
                           std::is_same_v<T, val>;

    // Pack args into something serializable: e.g. vector of (isVal, primitiveOrHandle)
    struct ArgPayload {
        bool is_val;
        JSPrimitive primitive;
        val::handle_type handle{};
    };

    template<typename T>
    ArgPayload make_arg_payload(T&& t) {
        if constexpr (std::is_same_v<std::decay_t<T>, val>) {
            return ArgPayload{true, JSPrimitive{}, t.handle()};
        } else {
            return ArgPayload{false, to_js_primitive(std::forward<T>(t)), 0};
        }
    }

    template<typename... Args>
    std::vector<ArgPayload> pack_args(Args&&... args) {
        std::vector<ArgPayload> result;
        result.reserve(sizeof...(Args));
        (result.push_back(make_arg_payload(std::forward<Args>(args))), ...);
        return result;
    }

} // namespace detail

// call<T> implementation
template<typename T, typename... Args>
T val::call(std::string_view funcName, Args&&... args) const {
    static_assert(detail::JsReturnable<T>, "Unsupported return type for emscripten_remote::val::call");

    auto argPayload = detail::pack_args(std::forward<Args>(args)...);

    if constexpr (std::is_same_v<T, void>) {
        detail::transport().send_val_call_no_result(m_handle, funcName, std::move(argPayload));
    } else if constexpr (std::is_same_v<T, bool>) {
        return detail::transport().send_val_call_get_bool(m_handle, funcName, std::move(argPayload));
    } else if constexpr (std::is_same_v<T, double>) {
        return detail::transport().send_val_call_get_double(m_handle, funcName, std::move(argPayload));
    } else if constexpr (std::is_same_v<T, std::string>) {
        return detail::transport().send_val_call_get_string(m_handle, funcName, std::move(argPayload));
    } else if constexpr (std::is_same_v<T, val>) {
        auto newHandle = detail::transport().send_val_call_get_val(m_handle, funcName, std::move(argPayload));
        return val{newHandle};
    }
}

// as<T> implementation
template<typename T>
T val::as() const {
    static_assert(detail::JsReturnable<T> && !std::is_same_v<T, void>,
                  "val::as<T> only supports non-void JsReturnable types");

    if constexpr (std::is_same_v<T, bool>) {
        return detail::transport().send_val_coerce_bool(m_handle);
    } else if constexpr (std::is_same_v<T, double>) {
        return detail::transport().send_val_coerce_double(m_handle);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return detail::transport().send_val_coerce_string(m_handle);
    } else if constexpr (std::is_same_v<T, val>) {
        // "as<val>" might just return *this or a cloned handle; up to you
        return *this;
    }
}

#else  // !EM_REMOTE  -> WASM build, use real emscripten::val

// In this mode, you just alias directly:
using val = emscripten::val;

// You can still add helper functions here if you like, as inline wrappers.

#endif // EM_REMOTE

} // namespace volt::emscripten_remote