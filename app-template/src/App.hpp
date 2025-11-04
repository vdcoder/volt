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
            Button(this).render("Increment", [this]() {
                counter++;
                invalidate();
            }, "primary"),
            
            Button(this).render("Reset", [this]() {
                counter = 0;
                invalidate();
            }, "danger")
        );
    }
};
