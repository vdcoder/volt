#pragma once
#include <Volt.hpp>

using namespace volt;

// ============================================================================
// Button Component - Stateless, reusable button with customizable appearance
// ============================================================================
class Button : public VoltRuntime::ComponentBase {
public:
    Button(VoltRuntime::IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render(
        const std::string& label, 
        std::function<void()> onButtonClick,
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
        
        return button({
            style(baseStyle + variantStyle),
            onClick([this, onButtonClick]() {
                onButtonClick();
            })
        }, label);
    }
};
