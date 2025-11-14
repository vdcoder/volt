#pragma once

// ============================================================================
// Volt Framework - Single Include Header
// ============================================================================
// Include this file in your app to get access to the entire Volt framework.
//
// Usage:
//   #include "dependencies/volt/include/Volt.hpp"
//   using namespace volt;
// ============================================================================

#include <string>
void log(const std::string& a_sMessage);

#include "VoltConfig.hpp"
#include "DOM.hpp"
#include "StableKeyManager.hpp"
#include "IVoltRuntime.hpp"
#include "ComponentBase.hpp"
#include "AppBase.hpp"
#include "VoltRuntime.hpp"
#include "RenderingRuntime.hpp"
#include "Attrs.hpp"
#include "VNode.hpp"
#include "VoltDiffPatch.hpp"

// Utilities
#include "Shortcuts.hpp"
#include "String.hpp"

// Implementation files
#include "VoltImpl.hpp"

void log(const std::string& a_sMessage) {
    emscripten::val console = emscripten::val::global("console");
    console.call<void>("log", a_sMessage);
}

// For convenience, you can use:
// using namespace volt;
