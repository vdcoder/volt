#pragma once

#include <string>
#include <emscripten.h>

// Bridge from C++ to JS volt console
EM_JS(void, volt_js_log,
      (int level, const char* category, int indent, const char* msg),
{
    if (typeof window !== 'undefined' &&
        window.volt &&
        typeof window.volt._shouldPrint === 'function' &&
        typeof window.volt._print === 'function') {

        const cat = UTF8ToString(Number(category));
        const text = UTF8ToString(Number(msg));

        if (window.volt._shouldPrint(level, cat)) {
            window.volt._print(level, cat, indent, text);
        }
    }
});


namespace volt {

    enum class LogLevel : int {
        TRACE_L = 0,
        DEBUG_L = 1,
        INFO_L  = 2,
        WARN_L  = 3,
        ERROR_L = 4,
    };

#ifdef VOLT_ENABLE_LOG

    // Per-thread (per-runtime) indentation level for logs
    inline thread_local int g_logIndentLevel = 0;

    inline void logIndentPush() noexcept {
        ++g_logIndentLevel;
    }

    inline void logIndentPop() noexcept {
        --g_logIndentLevel;
        if (g_logIndentLevel < 0) {
            g_logIndentLevel = 0;
        }
    }

    inline int getLogIndentLevel() noexcept {
        return g_logIndentLevel;
    }

    inline const char* toString(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::TRACE_L: return "TRACE";
            case LogLevel::DEBUG_L: return "DEBUG";
            case LogLevel::INFO_L:  return "INFO";
            case LogLevel::WARN_L:  return "WARN";
            case LogLevel::ERROR_L: return "ERROR";
        }
        return "INFO";
    }

    // Core logging function used by macros.
    inline void volt_log(LogLevel level,
                    const char* category,
                    const std::string& message)
    {
        // Do *not* pre-indent in the string anymore.
        // We send the indent level as a separate int to JS.
        int indent = getLogIndentLevel();

        volt_js_log(
            static_cast<int>(level),  // 0..4 (TRACE_L..ERROR_L)
            category,
            indent,
            message.c_str()
        );
    }

    inline void log(const std::string& a_sMessage) {
        emscripten::val console = emscripten::val::global("console");
        console.call<void>("log", a_sMessage);
    }

#else  // !VOLT_ENABLE_LOG

    // When logging is disabled, we still declare the functions so the macros
    // compile, but they do nothing. (Most compilers will fully optimize them away.)

    inline void logIndentPush() noexcept {}
    inline void logIndentPop() noexcept {}
    inline int  getLogIndentLevel() noexcept { return 0; }

    inline const char* toString(LogLevel) {
        return "";
    }

    inline void volt_log(LogLevel,
                    const char*,
                    const std::string&) {}

    inline void log(const std::string& a_sMessage) {}

#endif // VOLT_ENABLE_LOG

} // namespace volt

// ---- Public macros --------------------------------------------------------

#ifdef VOLT_ENABLE_LOG

    #define VOLT_LOG(level, category, msg_expr) \
        do { ::volt::volt_log((level), (category), (msg_expr)); } while (0)

    #define VOLT_TRACE(cat, msg_expr) VOLT_LOG(::volt::LogLevel::TRACE_L, (cat), (msg_expr))
    #define VOLT_DEBUG(cat, msg_expr) VOLT_LOG(::volt::LogLevel::DEBUG_L, (cat), (msg_expr))
    #define VOLT_INFO(cat,  msg_expr) VOLT_LOG(::volt::LogLevel::INFO_L,  (cat), (msg_expr))
    #define VOLT_WARN(cat,  msg_expr) VOLT_LOG(::volt::LogLevel::WARN_L,  (cat), (msg_expr))
    #define VOLT_ERROR(cat, msg_expr) VOLT_LOG(::volt::LogLevel::ERROR_L, (cat), (msg_expr))

    #define VOLT_LOG_INDENT_PUSH() ::volt::logIndentPush()
    #define VOLT_LOG_INDENT_POP()  ::volt::logIndentPop()

#else

    // Fully zero-cost when disabled: macros compile to nothing.

    #define VOLT_LOG(level, category, msg_expr)     do {} while (0)
    #define VOLT_TRACE(cat, msg_expr)              do {} while (0)
    #define VOLT_DEBUG(cat, msg_expr)              do {} while (0)
    #define VOLT_INFO(cat,  msg_expr)              do {} while (0)
    #define VOLT_WARN(cat,  msg_expr)              do {} while (0)
    #define VOLT_ERROR(cat, msg_expr)              do {} while (0)

    #define VOLT_LOG_INDENT_PUSH()                 do {} while (0)
    #define VOLT_LOG_INDENT_POP()                  do {} while (0)

#endif
