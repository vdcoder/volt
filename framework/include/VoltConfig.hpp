#pragma once

#include <string>

// ============================================================================
// Volt Framework Configuration
// ============================================================================
// This file defines the instance identifier for the framework.
// Set VOLT_GUID at compile time to create isolated instances.
// GUID can include version prefix if desired (e.g., "v1", "myapp_v2", etc.)
// ============================================================================

// Instance GUID (set at compile time)
// Example: -DVOLT_GUID="myapp" or -DVOLT_GUID="myapp_v1"
#ifndef VOLT_GUID
#define VOLT_GUID ""
#endif

// Helper macros
#define VOLT_STRINGIFY(x) #x
#define VOLT_TOSTRING(x) VOLT_STRINGIFY(x)

namespace volt {

namespace config {

// Sanitize GUID for use as JavaScript identifier (replace invalid chars with _)
inline std::string sanitizeForJs(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '_')) {
            c = '_';
        }
    }
    return result;
}

// Get the unique namespace identifier for this framework instance
inline std::string getVoltNamespace() {
    std::string guid = VOLT_GUID;
    
    if (guid.empty()) {
        return "volt"; // Default namespace
    }
    
    return "volt_" + sanitizeForJs(guid);
}

// Get the JavaScript global object name for event callbacks
inline std::string getVoltJsGlobal() {
    return getVoltNamespace();
}

} // namespace config

} // namespace volt
