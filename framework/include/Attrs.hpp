#pragma once

#include <string>
#include <utility>
#include <functional>

// Forward declaration
namespace volt { class VoltRuntime; }

// ============================================================================
// Attribute ID Constants
// ============================================================================
// Using short (2 bytes) instead of strings for memory efficiency

// Special internal IDs (negative range to avoid conflicts)
constexpr short ATTR_TEXT_CONTENT = -1;  // Internal: text node content storage

// Events (0-99)
constexpr short ATTR_ONCLICK    = 0;
constexpr short ATTR_ONINPUT    = 1;
constexpr short ATTR_ONCHANGE   = 2;
constexpr short ATTR_ONSUBMIT   = 3;
constexpr short ATTR_ONKEYDOWN  = 4;
constexpr short ATTR_ONKEYUP    = 5;
constexpr short ATTR_ONKEYPRESS = 6;
constexpr short ATTR_ONFOCUS    = 7;
constexpr short ATTR_ONBLUR     = 8;
constexpr short ATTR_ONMOUSEDOWN = 9;
constexpr short ATTR_ONMOUSEUP  = 10;
constexpr short ATTR_ONMOUSEMOVE = 11;
constexpr short ATTR_ONMOUSEENTER = 12;
constexpr short ATTR_ONMOUSELEAVE = 13;

// Style & Class (100-109)
constexpr short ATTR_STYLE      = 100;
constexpr short ATTR_CLASS      = 101;

// Common attributes (110-199)
constexpr short ATTR_ID         = 110;
constexpr short ATTR_TYPE       = 111;
constexpr short ATTR_VALUE      = 112;
constexpr short ATTR_PLACEHOLDER = 113;
constexpr short ATTR_HREF       = 114;
constexpr short ATTR_SRC        = 115;
constexpr short ATTR_ALT        = 116;
constexpr short ATTR_TITLE      = 117;
constexpr short ATTR_NAME       = 118;
constexpr short ATTR_DISABLED   = 119;
constexpr short ATTR_CHECKED    = 120;
constexpr short ATTR_SELECTED   = 121;
constexpr short ATTR_READONLY   = 122;
constexpr short ATTR_REQUIRED   = 123;
constexpr short ATTR_AUTOCOMPLETE = 124;
constexpr short ATTR_AUTOFOCUS  = 125;
constexpr short ATTR_TABINDEX   = 126;
constexpr short ATTR_FOR        = 127;
constexpr short ATTR_WIDTH      = 128;
constexpr short ATTR_HEIGHT     = 129;
constexpr short ATTR_MIN        = 130;
constexpr short ATTR_MAX        = 131;
constexpr short ATTR_STEP       = 132;
constexpr short ATTR_PATTERN    = 133;
constexpr short ATTR_MAXLENGTH  = 134;
constexpr short ATTR_ROWS       = 135;
constexpr short ATTR_COLS       = 136;

// Custom attributes start at 10000
constexpr short ATTR_CUSTOM_START = 10000;

// ============================================================================
// Attribute Name Lookup Table
// ============================================================================
inline const char* attrIdToName(short id) {
    switch (id) {
        // Events
        case ATTR_ONCLICK: return "onclick";
        case ATTR_ONINPUT: return "oninput";
        case ATTR_ONCHANGE: return "onchange";
        case ATTR_ONSUBMIT: return "onsubmit";
        case ATTR_ONKEYDOWN: return "onkeydown";
        case ATTR_ONKEYUP: return "onkeyup";
        case ATTR_ONKEYPRESS: return "onkeypress";
        case ATTR_ONFOCUS: return "onfocus";
        case ATTR_ONBLUR: return "onblur";
        case ATTR_ONMOUSEDOWN: return "onmousedown";
        case ATTR_ONMOUSEUP: return "onmouseup";
        case ATTR_ONMOUSEMOVE: return "onmousemove";
        case ATTR_ONMOUSEENTER: return "onmouseenter";
        case ATTR_ONMOUSELEAVE: return "onmouseleave";
        
        // Style & Class
        case ATTR_STYLE: return "style";
        case ATTR_CLASS: return "class";
        
        // Common attributes
        case ATTR_ID: return "id";
        case ATTR_TYPE: return "type";
        case ATTR_VALUE: return "value";
        case ATTR_PLACEHOLDER: return "placeholder";
        case ATTR_HREF: return "href";
        case ATTR_SRC: return "src";
        case ATTR_ALT: return "alt";
        case ATTR_TITLE: return "title";
        case ATTR_NAME: return "name";
        case ATTR_DISABLED: return "disabled";
        case ATTR_CHECKED: return "checked";
        case ATTR_SELECTED: return "selected";
        case ATTR_READONLY: return "readonly";
        case ATTR_REQUIRED: return "required";
        case ATTR_AUTOCOMPLETE: return "autocomplete";
        case ATTR_AUTOFOCUS: return "autofocus";
        case ATTR_TABINDEX: return "tabindex";
        case ATTR_FOR: return "for";
        case ATTR_WIDTH: return "width";
        case ATTR_HEIGHT: return "height";
        case ATTR_MIN: return "min";
        case ATTR_MAX: return "max";
        case ATTR_STEP: return "step";
        case ATTR_PATTERN: return "pattern";
        case ATTR_MAXLENGTH: return "maxlength";
        case ATTR_ROWS: return "rows";
        case ATTR_COLS: return "cols";
        
        default: return "unknown";
    }
}

// ============================================================================
// Runtime Context - Set during rendering
// ============================================================================
namespace volt {
    // This is set by VoltRuntime before calling app->render()
    // Allows event helpers to register callbacks with the correct runtime
    extern thread_local VoltRuntime* currentRuntime;
}

// ============================================================================
// Attribute Helper Functions - Return std::pair<short, std::string>
// ============================================================================

// Style & Class
inline std::pair<short, std::string> style(const std::string& value) {
    return {ATTR_STYLE, value};
}

inline std::pair<short, std::string> className(const std::string& value) {
    return {ATTR_CLASS, value};
}

// Common attributes
inline std::pair<short, std::string> id(const std::string& value) {
    return {ATTR_ID, value};
}

inline std::pair<short, std::string> type(const std::string& value) {
    return {ATTR_TYPE, value};
}

inline std::pair<short, std::string> value(const std::string& value) {
    return {ATTR_VALUE, value};
}

inline std::pair<short, std::string> placeholder(const std::string& value) {
    return {ATTR_PLACEHOLDER, value};
}

inline std::pair<short, std::string> href(const std::string& value) {
    return {ATTR_HREF, value};
}

inline std::pair<short, std::string> src(const std::string& value) {
    return {ATTR_SRC, value};
}

inline std::pair<short, std::string> alt(const std::string& value) {
    return {ATTR_ALT, value};
}

inline std::pair<short, std::string> title(const std::string& value) {
    return {ATTR_TITLE, value};
}

inline std::pair<short, std::string> name(const std::string& value) {
    return {ATTR_NAME, value};
}

inline std::pair<short, std::string> disabled(const std::string& value = "") {
    return {ATTR_DISABLED, value};
}

inline std::pair<short, std::string> checked(const std::string& value = "") {
    return {ATTR_CHECKED, value};
}

inline std::pair<short, std::string> selected(const std::string& value = "") {
    return {ATTR_SELECTED, value};
}

inline std::pair<short, std::string> readonly(const std::string& value = "") {
    return {ATTR_READONLY, value};
}

inline std::pair<short, std::string> required(const std::string& value = "") {
    return {ATTR_REQUIRED, value};
}

// ============================================================================
// Event Helper Functions - Defined in VoltEvents.hpp
// (Needs full VoltRuntime definition, moved to separate file)
// ============================================================================

std::pair<short, std::string> onClick(std::function<void()> callback);
std::pair<short, std::string> onInput(std::function<void(const std::string&)> callback);
std::pair<short, std::string> onChange(std::function<void(const std::string&)> callback);
std::pair<short, std::string> onSubmit(std::function<void()> callback);
std::pair<short, std::string> onKeyDown(std::function<void()> callback);
std::pair<short, std::string> onKeyUp(std::function<void()> callback);
std::pair<short, std::string> onFocus(std::function<void()> callback);
std::pair<short, std::string> onBlur(std::function<void()> callback);
