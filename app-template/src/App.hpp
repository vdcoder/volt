#pragma once
#include <Volt.hpp>
#include "components/Button.hpp"

using namespace volt;

// VOLT_APP_NAME - Main Application
class VOLT_APP_NAME_CAMEL : public VoltRuntime::AppBase {
private:
    int counter = 0;
    std::string message = "Hello from VOLT_APP_NAME!";
    
public:
    VOLT_APP_NAME_CAMEL(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNode render() override {
        return div({style("font-family: sans-serif; padding: 20px;")},
            h1(message),
            p("Counter: " + std::to_string(counter)),

            // Using reusable Button component
            // Note: No need to call invalidate() - auto-invalidate in event handlers
            Button(getRuntime()).render("Increment", [this]() {
                counter++;
            }, "primary"),
            
            Button(getRuntime()).render("Reset", [this]() {
                counter = 0;
            }, "danger")
        );
    }
};
