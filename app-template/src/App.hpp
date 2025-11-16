#pragma once
#include <Volt.hpp>
#include "components/Button.hpp"

using namespace volt;

// VOLT_APP_NAME - Main Application
class VOLT_APP_NAME_CAMEL : public AppBase {
private:
    int counter = 0;
    std::string message = "Hello from VOLT_APP_NAME!";
    
public:
    VOLT_APP_NAME_CAMEL(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNodeHandle render() override {
        return tag::div(x::key_i(0), {attr::style("font-family: sans-serif; padding: 20px;")},
            tag::h1(x::key_i(1), message),
            tag::p(x::key_i(2), "Counter: " + std::to_string(counter)),

            // Using reusable Button component
            // Note: No need to call invalidate() - auto-invalidate in event handlers
            Button(getRuntime()).render(3, "Increment", [this](emscripten::val e) {
                log("Increment button clicked");
                counter++;
            }, "primary"),

            Button(getRuntime()).render(4, "Reset", [this](emscripten::val e) {
                log("Reset button clicked");
                counter = 0;
            }, "danger"),

            map(std::vector<std::string>{"Apple", "Banana", "Cherry"}, [this](const std::string& fruit, auto key) {
                key(fruit); // Stable key based on fruit name
                return fragment(
                    tag::div(x::key_i(5), 
                        {attr::style("border: 1px solid #ccc; padding: 10px; margin: 10px 0;")},
                        tag::h1(x::key_i(6), fruit),
                        tag::br(x::key_i(7)),
                        tag::a(x::key_i(8), {attr::href("https://example.com/" + fruit)}, fruit),
                        map(std::vector<int>{0}, [fruit](int num, auto key) {
                            std::vector<VNodeHandle> lines;
                            for (int i = 0; i < 200; i++) {
                                lines.push_back(tag::span(x::key_i(9), {attr::id("fruit-" + fruit + "-line-span-" + std::to_string(i))}, " - " + fruit + " #" + std::to_string(i) + " "));
                                lines.push_back(tag::br(x::key_i(10), {attr::id("fruit-" + fruit + "-line-br-" + std::to_string(i))}));
                            }
                            return fragment(lines);
                        }
                    ))
                );
            })
        );
    }
};
