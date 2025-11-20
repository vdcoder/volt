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
        return tag::div({attr::style("font-family: sans-serif; padding: 20px;")},
            tag::h1(message).track(__COUNTER__),
            tag::p("Counter: " + std::to_string(counter)).track(__COUNTER__),

            // Using reusable Button component
            // Note: No need to call invalidate() - auto-invalidate in event handlers
            Button(getRuntime()).render("Increment", [this](emscripten::val e) {
                log("Increment button clicked");
                counter++;
            }, "primary").track(__COUNTER__),

            Button(getRuntime()).render("Reset", [this](emscripten::val e) {
                log("Reset button clicked");
                counter = 0;
            }, "danger").track(__COUNTER__),

            map(std::vector<std::string>{"Apple", "Banana", "Cherry"}, [this](const std::string& fruit) {
                return tag::_fragment({ attr::key(fruit) },
                    tag::div({attr::style("border: 1px solid #ccc; padding: 10px; margin: 10px 0;")},
                        tag::h1(fruit).track(__COUNTER__),
                        tag::br().track(__COUNTER__),
                        tag::a({attr::href("https://example.com/" + fruit)}, fruit).track(__COUNTER__),
                        map(std::vector<int>{0}, [fruit](int num) {
                            std::vector<VNodeHandle> lines;
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(tag::span({attr::id("fruit-" + fruit + "-line-span-" + std::to_string(i))}, " - " + fruit + " #" + std::to_string(i) + " ").track(__COUNTER__));
                                lines.push_back(tag::br({attr::id("fruit-" + fruit + "-line-br-" + std::to_string(i))}).track(__COUNTER__));
                            }
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(
                                    tag::_fragment({attr::key("fruit-" + fruit + "-fragment-" + std::to_string(i))},
                                        tag::span("in fragment - " + fruit + " #" + std::to_string(i) + " ").track(__COUNTER__),
                                        tag::br().track(__COUNTER__)
                                    ).track(__COUNTER__)
                                );
                            }
                            return tag::_fragment(lines).track(__COUNTER__);
                        }).track(__COUNTER__)
                    ).track(__COUNTER__)
                ).track(__COUNTER__);
            }).track(__COUNTER__)
        ).track(__COUNTER__);
    }
};
