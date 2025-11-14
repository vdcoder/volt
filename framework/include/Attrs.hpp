#pragma once

#include <string>
#include <utility>
#include <functional>

namespace volt {

namespace attr {

// ============================================================================
// Attribute ID Constants
// ============================================================================
// Using short (2 bytes) instead of strings for memory efficiency

// Bubble Events (0-99)
constexpr short ATTR_EVT_CLICK    = 0;
constexpr short ATTR_EVT_INPUT    = 1;
constexpr short ATTR_EVT_CHANGE   = 2;
constexpr short ATTR_EVT_SUBMIT   = 3;
constexpr short ATTR_EVT_KEYDOWN  = 4;
constexpr short ATTR_EVT_KEYUP    = 5;
constexpr short ATTR_EVT_KEYPRESS = 6;
constexpr short ATTR_EVT_MOUSEDOWN = 7;
constexpr short ATTR_EVT_MOUSEUP  = 8;
constexpr short ATTR_EVT_MOUSEMOVE = 9;
constexpr short ATTR_EVT_FOCUSIN  = 10;
constexpr short ATTR_EVT_FOCUSOUT = 11;

// Non-Bubble Events (100-199)
constexpr short ATTR_EVT_NON_BUBBLE_START = 100;
constexpr short ATTR_EVT_NON_BUBBLE_END = 200;
constexpr short ATTR_EVT_FOCUS    = 100;
constexpr short ATTR_EVT_BLUR     = 101;
constexpr short ATTR_EVT_MOUSEENTER = 102;
constexpr short ATTR_EVT_MOUSELEAVE = 103;
constexpr short ATTR_EVT_SCROLL   = 104;
constexpr short ATTR_EVT_RESIZE   = 105;
constexpr short ATTR_EVT_LOAD     = 106;
constexpr short ATTR_EVT_UNLOAD   = 107;
constexpr short ATTR_EVT_BEFOREUNLOAD   = 108;
constexpr short ATTR_EVT_ERROR    = 109;
constexpr short ATTR_EVT_ABORT    = 110;


// Style & Class (200-209)
constexpr short ATTR_STYLE      = 200;
constexpr short ATTR_CLASS      = 201;

// Common attributes (210-299)
constexpr short ATTR_TEXTCONTENT = 210;
constexpr short ATTR_ID         = 211;
constexpr short ATTR_TYPE       = 212;
constexpr short ATTR_VALUE      = 213;
constexpr short ATTR_PLACEHOLDER = 214;
constexpr short ATTR_HREF       = 215;
constexpr short ATTR_SRC        = 216;
constexpr short ATTR_ALT        = 217;
constexpr short ATTR_TITLE      = 218;
constexpr short ATTR_NAME       = 219;
constexpr short ATTR_DISABLED   = 220;
constexpr short ATTR_CHECKED    = 221;
constexpr short ATTR_SELECTED   = 222;
constexpr short ATTR_READONLY   = 223;
constexpr short ATTR_REQUIRED   = 224;
constexpr short ATTR_AUTOCOMPLETE = 225;
constexpr short ATTR_AUTOFOCUS  = 226;
constexpr short ATTR_TABINDEX   = 227;
constexpr short ATTR_FOR        = 228;
constexpr short ATTR_WIDTH      = 229;
constexpr short ATTR_HEIGHT     = 230;
constexpr short ATTR_MIN        = 231;
constexpr short ATTR_MAX        = 232;
constexpr short ATTR_STEP       = 233;
constexpr short ATTR_PATTERN    = 234;
constexpr short ATTR_MAXLENGTH  = 235;
constexpr short ATTR_ROWS       = 236;
constexpr short ATTR_COLS       = 237;

// Custom attributes start at 10000
constexpr short ATTR_CUSTOM_START = 10000;

// ============================================================================
// Attribute Name Lookup Table
// ============================================================================
inline const char* attrIdToName(short id) {
    switch (id) {
        // Bubble Events
        case ATTR_EVT_CLICK: return "click";
        case ATTR_EVT_INPUT: return "input";
        case ATTR_EVT_CHANGE: return "change";
        case ATTR_EVT_SUBMIT: return "submit";
        case ATTR_EVT_KEYDOWN: return "keydown";
        case ATTR_EVT_KEYUP: return "keyup";
        case ATTR_EVT_KEYPRESS: return "keypress";
        case ATTR_EVT_MOUSEDOWN: return "mousedown";
        case ATTR_EVT_MOUSEUP: return "mouseup";
        case ATTR_EVT_MOUSEMOVE: return "mousemove";
        case ATTR_EVT_FOCUSIN: return "focusin";
        case ATTR_EVT_FOCUSOUT: return "focusout";

        // Non-Bubble Events
        case ATTR_EVT_FOCUS: return "focus";
        case ATTR_EVT_BLUR: return "blur";
        case ATTR_EVT_MOUSEENTER: return "mouseenter";
        case ATTR_EVT_MOUSELEAVE: return "mouseleave";
        case ATTR_EVT_SCROLL: return "scroll";
        case ATTR_EVT_RESIZE: return "resize";
        case ATTR_EVT_LOAD: return "load";
        case ATTR_EVT_UNLOAD: return "unload";
        case ATTR_EVT_BEFOREUNLOAD: return "beforeunload";
        case ATTR_EVT_ERROR: return "error";
        case ATTR_EVT_ABORT: return "abort";

        // Style & Class
        case ATTR_STYLE: return "style";
        case ATTR_CLASS: return "class";
        
        // Common attributes
        case ATTR_TEXTCONTENT: return "textContent";
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
// Common Attributes
// ============================================================================

#define DEFINE_ATTR_HELPER(funcName, attrId) \
    inline std::pair<short, std::string> funcName(std::string a_sValue) { \
        return {attrId, a_sValue}; \
    }

DEFINE_ATTR_HELPER(style, ATTR_STYLE)
DEFINE_ATTR_HELPER(className, ATTR_CLASS)
DEFINE_ATTR_HELPER(id, ATTR_ID)
DEFINE_ATTR_HELPER(type, ATTR_TYPE)
DEFINE_ATTR_HELPER(value, ATTR_VALUE)
DEFINE_ATTR_HELPER(placeholder, ATTR_PLACEHOLDER)
DEFINE_ATTR_HELPER(href, ATTR_HREF)
DEFINE_ATTR_HELPER(src, ATTR_SRC)
DEFINE_ATTR_HELPER(alt, ATTR_ALT)
DEFINE_ATTR_HELPER(title, ATTR_TITLE)
DEFINE_ATTR_HELPER(name, ATTR_NAME)
DEFINE_ATTR_HELPER(disabled, ATTR_DISABLED)
DEFINE_ATTR_HELPER(checked, ATTR_CHECKED)
DEFINE_ATTR_HELPER(selected, ATTR_SELECTED)
DEFINE_ATTR_HELPER(readonly, ATTR_READONLY)
DEFINE_ATTR_HELPER(required, ATTR_REQUIRED)
DEFINE_ATTR_HELPER(autocomplete, ATTR_AUTOCOMPLETE)
DEFINE_ATTR_HELPER(autofocus, ATTR_AUTOFOCUS)
DEFINE_ATTR_HELPER(tabindex, ATTR_TABINDEX)
DEFINE_ATTR_HELPER(forAttr, ATTR_FOR)
DEFINE_ATTR_HELPER(width, ATTR_WIDTH)
DEFINE_ATTR_HELPER(height, ATTR_HEIGHT)
DEFINE_ATTR_HELPER(min, ATTR_MIN)
DEFINE_ATTR_HELPER(max, ATTR_MAX)
DEFINE_ATTR_HELPER(step, ATTR_STEP)
DEFINE_ATTR_HELPER(pattern, ATTR_PATTERN)
DEFINE_ATTR_HELPER(maxlength, ATTR_MAXLENGTH)
DEFINE_ATTR_HELPER(rows, ATTR_ROWS)
DEFINE_ATTR_HELPER(cols, ATTR_COLS)

// ============================================================================
// Events 
// ============================================================================

#define DECLARE_EVENT_HELPER(funcName, eventId) \
    inline std::pair<short, std::function<void(emscripten::val)>> funcName(std::function<void(emscripten::val)> a_fnCallback) { \
        return {eventId, a_fnCallback}; \
    }

// Bubble Events
DECLARE_EVENT_HELPER(onClick, ATTR_EVT_CLICK);
DECLARE_EVENT_HELPER(onInput, ATTR_EVT_INPUT);
DECLARE_EVENT_HELPER(onChange, ATTR_EVT_CHANGE);
DECLARE_EVENT_HELPER(onSubmit, ATTR_EVT_SUBMIT);
DECLARE_EVENT_HELPER(onKeyDown, ATTR_EVT_KEYDOWN);
DECLARE_EVENT_HELPER(onKeyUp, ATTR_EVT_KEYUP);
DECLARE_EVENT_HELPER(onKeyPress, ATTR_EVT_KEYPRESS);
DECLARE_EVENT_HELPER(onMouseDown, ATTR_EVT_MOUSEDOWN);
DECLARE_EVENT_HELPER(onMouseUp, ATTR_EVT_MOUSEUP);
DECLARE_EVENT_HELPER(onMouseMove, ATTR_EVT_MOUSEMOVE);
DECLARE_EVENT_HELPER(onFocusIn, ATTR_EVT_FOCUSIN);
DECLARE_EVENT_HELPER(onFocusOut, ATTR_EVT_FOCUSOUT);

// Non-Bubble Events
DECLARE_EVENT_HELPER(onFocus, ATTR_EVT_FOCUS);
DECLARE_EVENT_HELPER(onBlur, ATTR_EVT_BLUR);
DECLARE_EVENT_HELPER(onMouseEnter, ATTR_EVT_MOUSEENTER);
DECLARE_EVENT_HELPER(onMouseLeave, ATTR_EVT_MOUSELEAVE);
DECLARE_EVENT_HELPER(onScroll, ATTR_EVT_SCROLL);
DECLARE_EVENT_HELPER(onResize, ATTR_EVT_RESIZE);
DECLARE_EVENT_HELPER(onLoad, ATTR_EVT_LOAD);
DECLARE_EVENT_HELPER(onUnload, ATTR_EVT_UNLOAD);
DECLARE_EVENT_HELPER(onBeforeUnload, ATTR_EVT_BEFOREUNLOAD);
DECLARE_EVENT_HELPER(onError, ATTR_EVT_ERROR);
DECLARE_EVENT_HELPER(onAbort, ATTR_EVT_ABORT);

} // namespace attrs

} // namespace volt