#pragma once
#include "../dependencies/volt/include/Volt.hpp"

using namespace volt;

// Volt App - Main Application
class App : public VoltRuntime::AppBase {
private:
    int counter = 0;
    String message = "Hello from Volt!";
    
public:
    App(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNode render() override {
        return div({style("font-family: sans-serif; padding: 20px;")}, {
            h1({text(message.std_str())}),
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
