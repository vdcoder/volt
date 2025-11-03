#pragma once
#include "../dependencies/volt/include/Volt.hpp"

using namespace volt;

// VOLT_APP_NAME - Main Application
class VOLT_APP_NAME_CAMEL : public VoltRuntime::AppBase {
private:
    int counter = 0;
    std::string message = "Hello from VOLT_APP_NAME!";
    
public:
    VOLT_APP_NAME_CAMEL(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNode render() override {
        return div({style("font-family: sans-serif; padding: 20px;")}, {
            h1({text(message)}),
            p({text("Counter: " + std::to_string(counter))}),
            button({onClick([this]() {
                counter++;
                invalidate();
            })}, {text("Increment")}),
            button({onClick([this]() {
                counter = 0;
                invalidate();
            })}, {text("Reset")})
        });
    }
};
