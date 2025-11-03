#pragma once

#include "Attrs.hpp"
#include "VoltRuntime.hpp"
#include "VoltConfig.hpp"

// ============================================================================
// Event Helper Functions - Implementations that need full VoltRuntime definition
// ============================================================================

inline std::pair<short, std::string> onClick(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONCLICK, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + ")"};
}

inline std::pair<short, std::string> onInput(std::function<void(const std::string&)> callback) {
    int callbackId = volt::currentRuntime->registerStringEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONINPUT, jsGlobal + ".invokeStringEventCallback(" + std::to_string(callbackId) + ", this.value)"};
}

inline std::pair<short, std::string> onChange(std::function<void(const std::string&)> callback) {
    int callbackId = volt::currentRuntime->registerStringEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONCHANGE, jsGlobal + ".invokeStringEventCallback(" + std::to_string(callbackId) + ", this.value)"};
}

inline std::pair<short, std::string> onSubmit(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONSUBMIT, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + "); return false;"};
}

inline std::pair<short, std::string> onKeyDown(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONKEYDOWN, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + ")"};
}

inline std::pair<short, std::string> onKeyUp(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONKEYUP, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + ")"};
}

inline std::pair<short, std::string> onFocus(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONFOCUS, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + ")"};
}

inline std::pair<short, std::string> onBlur(std::function<void()> callback) {
    int callbackId = volt::currentRuntime->registerEventCallback(callback);
    std::string jsGlobal = volt::getVoltJsGlobal();
    return {ATTR_ONBLUR, jsGlobal + ".invokeEventCallback(" + std::to_string(callbackId) + ")"};
}
