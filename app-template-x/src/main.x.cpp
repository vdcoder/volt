#include <emscripten/bind.h>
#include <Volt.hpp>
#include "App.x.hpp"

using namespace volt;
using namespace emscripten;

std::unique_ptr<VoltEngine> g_voltEngine = nullptr;

EMSCRIPTEN_BINDINGS(VOLT_APP_NAME_UNDERSCORE_module) {
    function("getVoltNamespace", &volt::config::getVoltNamespace);
    
    function("createVoltEngine", +[](std::string rootId) {
        g_voltEngine = std::make_unique<VoltEngine>(rootId, "VOLT_APP_NAME_UNDERSCORE_module");
        g_voltEngine->mountApp<VOLT_APP_NAME_CAMEL>();
    });
    
    function("invokeVoltBubbleEvent", +[](emscripten::val event) {
        invokeBubbleEvent(event);
    });

    function("invokeVoltNonBubbleEvent", +[](emscripten::val event) {
        invokeNonBubbleEvent(event);
    });

    function("clearVoltFocussedElements", +[]() {
        g_voltEngine->clearFocussedElements();
    });

    function("addVoltFocussedElement", +[](emscripten::val element) {
        g_voltEngine->addFocussedElement(element);
    });
}
