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

#if defined(DEBUG) || defined(_DEBUG)
    #include "Debug.hpp"
#endif

#include "VoltLog.hpp"
#include "VoltConfig.hpp"
#include "DOM.hpp"
#include "IVoltRuntime.hpp"
#include "ComponentBase.hpp"
#include "AppBase.hpp"
#include "VoltRuntime.hpp"
#include "RenderingRuntime.hpp"
#include "Attrs.hpp"
#include "VNode.hpp"
#include "VoltDiffPatch.hpp"
#include "IdManager.hpp"

// Utilities
#include "Shortcuts.hpp"
#include "String.hpp"

// Implementation files
#include "VoltImpl.hpp"

// For convenience, you can use:
// using namespace volt;
