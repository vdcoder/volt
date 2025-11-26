#include <emscripten/bind.h>
#include <Volt.hpp>
#include "App.x.hpp"

using namespace volt;
using namespace emscripten;

std::unique_ptr<VoltEngine> g_voltEngine = nullptr;

EMSCRIPTEN_BINDINGS(VOLT_APP_NAME_UNDERSCORE_module) {
    function("getVoltNamespace", &volt::config::getVoltNamespace);
    
    function("createVoltEngine", +[](std::string rootId) {
        g_voltEngine = std::make_unique<VoltEngine>(rootId);
        g_voltEngine->mountApp<VOLT_APP_NAME_CAMEL>();
    });
    
    function("invokeVoltBubbleEvent", +[](intptr_t __cpp_ptr_as_int, emscripten::val event) {
        VNode* pVNode = reinterpret_cast<volt::VNode*>(__cpp_ptr_as_int);
        (void)pVNode->bubbleCallback(event["type"].as<std::string>(), event);
    }, emscripten::allow_raw_pointers());
}
