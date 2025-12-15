#pragma once
#include <Volt.hpp>

using namespace volt;

// ============================================================================
// Button Component - Stateless, reusable button with customizable appearance
// ============================================================================
VNodeHandle Button(
    IRuntime& a_runtime,
    std::string_view label, 
    std::function<void(emscripten::val)> onButtonClick,
    std::string_view variant = "primary"
) {
    // Define button styles based on variant
    std::string baseStyle = "padding: 10px 20px; margin: 5px; border: none; "
                            "border-radius: 4px; cursor: pointer; font-size: 14px;";
    
    std::string variantStyle;
    if (variant == "primary") {
        variantStyle = "background: #007bff; color: white;";
    } else if (variant == "danger") {
        variantStyle = "background: #dc3545; color: white;";
    } else {
        variantStyle = "background: #6c757d; color: white;";
    }
    
    // No need to capture 'this' - auto-invalidate handles it
    return <button({
        style:=(baseStyle + variantStyle),
        onClick:=(onButtonClick)
    }, 
        label
    )/>;
}

