#include <emscripten/bind.h>
#include <Volt.hpp>
#include "App.hpp"

using namespace volt;
using namespace emscripten;

std::unique_ptr<VoltRuntime> g_runtime;

EMSCRIPTEN_BINDINGS(VOLT_APP_NAME_UNDERSCORE_module) {
    function("getVoltNamespace", &volt::config::getVoltNamespace);
    
    function("createRuntime", +[](std::string rootId) {
        g_runtime = std::make_unique<VoltRuntime>(rootId);
        g_runtime->mountApp<VOLT_APP_NAME_CAMEL>();
    });
    
    function("invokeBubbleEvent", +[](intptr_t __cpp_ptr_as_int, emscripten::val event) {
        VNode* pVNode = reinterpret_cast<volt::VNode*>(__cpp_ptr_as_int);
        (void)pVNode->bubbleCallback(event["type"].as<std::string>(), event);
    }, emscripten::allow_raw_pointers());
}
