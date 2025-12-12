#include "EventBridge.hpp"

namespace volt {

void invokeBubbleEvent(emscripten::val event) {
    if (!event.hasOwnProperty("__volt_cpp_ptr")) {
        EM_ASM({ console.warn("invokeBubbleEvent: missing __volt_cpp_ptr"); });
        return;
    }

    intptr_t cpp_ptr_as_int = event["__volt_cpp_ptr"].as<intptr_t>();
    if (cpp_ptr_as_int == 0) {
        return;
    }

    auto* pVNode = reinterpret_cast<volt::VNode*>(cpp_ptr_as_int);

    std::string sEventType = event["type"].as<std::string>();

    (void)pVNode->bubbleCallback(sEventType, event);
}

void invokeNonBubbleEvent(emscripten::val event) {
    emscripten::val target = event["target"];

    if (target.isUndefined() || target.isNull()) {
        return;
    }

    if (!target.hasOwnProperty("__cpp_ptr")) {
        EM_ASM({ console.warn("Non-bubble event target has no __cpp_ptr"); });
        return;
    }

    intptr_t cpp_ptr_as_int = target["__cpp_ptr"].as<intptr_t>();
    if (cpp_ptr_as_int == 0) {
        return;
    }

    auto* pVNode = reinterpret_cast<volt::VNode*>(cpp_ptr_as_int);

    std::string sEventType = event["type"].as<std::string>();

    (void)pVNode->nonBubbleCallback(sEventType, event);
}

} // namespace volt