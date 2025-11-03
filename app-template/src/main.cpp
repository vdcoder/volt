#include <emscripten/bind.h>
#include "../dependencies/volt/include/Volt.hpp"
#include "../dependencies/volt/src/VoltRuntime.cpp"  // Single TU
#include "App.hpp"

using namespace volt;
using namespace emscripten;

std::unique_ptr<VoltRuntime> g_runtime;

EMSCRIPTEN_BINDINGS(volt_app_module) {
    function("getVoltNamespace", &volt::getVoltNamespace);
    
    function("createRuntime", +[]() {
        g_runtime = std::make_unique<VoltRuntime>("root");
        g_runtime->mount<App>();
    });
    
    function("invokeEventCallback", +[](int id) {
        if (g_runtime) g_runtime->invokeEventCallback(id);
    });
    
    function("invokeStringEventCallback", +[](int id, const std::string& value) {
        if (g_runtime) g_runtime->invokeStringEventCallback(id, value);
    });
}
