#pragma once

#include <iostream>
#include <emscripten/val.h>
#include <emscripten/emscripten.h>

using emscripten::EM_VAL;
using emscripten::val;

EM_JS(size_t, js_string_length, (EM_VAL a_hS), {
    return Emval.toValue(a_hS).length;
});

EM_JS(EM_VAL, js_string_concat, (EM_VAL a_hS1, EM_VAL a_hS2), {
    const js_val1 = Emval.toValue(a_hS1);
    const js_val2 = Emval.toValue(a_hS2);
    const result_string = js_val1.concat(js_val2);
    return Emval.toHandle(result_string);
});

EM_JS(EM_VAL, create_js_string, (const char* a_cStr), {
    const js_string = UTF8ToString(Number(a_cStr));
    return Emval.toHandle(js_string);
});

EM_JS(EM_VAL, i32_to_js_string, (int a_n), {
    return Emval.toHandle(a_n.toString());
});

EM_JS(bool, js_string_equals, (EM_VAL a_hS1, EM_VAL a_hS2), {
    const js_val1 = Emval.toValue(a_hS1);
    const js_val2 = Emval.toValue(a_hS2);
    return js_val1 === js_val2;
});

class String {
public:
    String() {
        m_handle = create_js_string("");
    }

    // construct from existing EM_VAL handle, assumes ownership
    String(EM_VAL a_hStr) {
        m_handle = a_hStr;
    }

    // shares ownership, emscripten::val can safely go out of scope
    String(emscripten::val a_evalStr) {
        m_handle = a_evalStr.as_handle();
        emscripten::internal::_emval_incref(m_handle);
    }

    String(const char* a_cStr) {
        m_handle = create_js_string(a_cStr);
    }
    
    String(const String& other) {
        m_handle = other.m_handle;
        emscripten::internal::_emval_incref(m_handle);
    }
    
    String(String&& other) noexcept
        : m_handle(other.m_handle) {
        other.m_handle = 0;
    }

    ~String() {
        emscripten::internal::_emval_decref(m_handle);
    }

    String& operator=(const String& other) {
        if (this != &other) {
            emscripten::internal::_emval_decref(m_handle);
            m_handle = other.m_handle;
            emscripten::internal::_emval_incref(m_handle);
        }
        return *this;
    }

    String& operator=(String&& other) noexcept {
        if (this != &other) {
            emscripten::internal::_emval_decref(m_handle);
            m_handle = other.m_handle;
            other.m_handle = 0;
        }
        return *this;
    }

    std::string std_str() const {
        emscripten::internal::_emval_incref(m_handle);
        return emscripten::val::take_ownership(m_handle).as<std::string>();
    }

    size_t length() const {
        return js_string_length(m_handle);
    }

    String operator+(const String& other) const {
        return String(js_string_concat(m_handle, other.m_handle));
    }

    bool operator==(const String& other) const {
        return js_string_equals(m_handle, other.m_handle);
    }

    bool operator!=(const String& other) const {
        return !js_string_equals(m_handle, other.m_handle);
    }

private:
    EM_VAL m_handle;
};

#define S(text) String(text)

inline String str(int n) {
    return String(i32_to_js_string(n));
}