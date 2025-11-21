#pragma once
#include <Volt.hpp>

using namespace volt;

// ============================================================================
// Button Component - Stateless, reusable button with customizable appearance
// ============================================================================
class Button : public ComponentBase {
public:
    Button(IVoltRuntime* a_pRuntime) : ComponentBase(a_pRuntime) {}
    
    VNodeHandle render(
        const std::string& label, 
        std::function<void(emscripten::val)> onButtonClick,
        const std::string& variant = "primary"
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
        }, label)/>;
    }
};
