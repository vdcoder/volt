#pragma once
#include <Volt.hpp>
#define TRACK track(__COUNTER__)

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
            tag::h1(message).TRACK,
            tag::p("Counter: " + std::to_string(counter)).TRACK,

            // Using reusable Button component
            // Note: No need to call invalidate() - auto-invalidate in event handlers
            Button(getRuntime()).render("Increment", [this](emscripten::val e) {
                log("Increment button clicked");
                counter++;
            }, "primary").TRACK,

            tag::div(
                x::iff(counter % 2 == 0, [this](){
                    return std::vector<VNodeHandle>{
                        // Button(getRuntime()).render("Reset", [this](emscripten::val e) {
                        //     log("Reset button clicked");
                        //     counter = 0;
                        // }, "danger").TRACK 
                        tag::button({
                            attr::onAddElement([this](emscripten::val e) {
                                log("onAddElement button clicked");
                            }),
                            attr::onBeforeMoveElement([this](emscripten::val e) {
                                log("onBeforeMoveElement button clicked");
                            }),
                            attr::onMoveElement([this](emscripten::val e) {
                                log("onMoveElement button clicked");
                            }),
                            attr::onRemoveElement([this](emscripten::val e) {
                                log("onRemoveElement button clicked");
                            }),
                            attr::onClick([this](emscripten::val e) {
                                log("Reset button clicked");
                                counter = 0;
                            })
                        }, "Reset").TRACK
                    };
                }, []() {
                    return std::vector<VNodeHandle>{"No button for you!"};
                })
            ).TRACK,

            (counter > 5 ? tag::p({attr::style("color: red;")}, "Counter exceeded 5!").TRACK : tag::br().TRACK),

            map(std::vector<std::string>{"Apple"}, [this](const std::string& fruit) {
                return tag::_fragment({ attr::key(fruit) },
                    tag::article({attr::style("border: 1px solid #ccc; padding: 10px; margin: 10px 0; height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;"),
                    attr::onAddElement([this](emscripten::val e) {
                                log("onAddElement article");
                            }),
                            attr::onBeforeMoveElement([this](emscripten::val e) {
                                log("onBeforeMoveElement article");
                            }),
                            attr::onMoveElement([this](emscripten::val e) {
                                log("onMoveElement article");
                            }),
                            attr::onRemoveElement([this](emscripten::val e) {
                                log("onRemoveElement article");
                            })},
                        tag::h1(fruit).TRACK,
                        tag::br().TRACK,
                        tag::a({attr::href("https://example.com/" + fruit)}, fruit).TRACK,
                        map(std::vector<int>{0}, [fruit](int num) {
                            std::vector<VNodeHandle> lines;
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(tag::span({attr::id("fruit-" + fruit + "-line-span-" + std::to_string(i))}, " - " + fruit + " #" + std::to_string(i) + " ").TRACK);
                                lines.push_back(tag::br({attr::id("fruit-" + fruit + "-line-br-" + std::to_string(i))}).TRACK);
                            }
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(
                                    tag::_fragment({attr::key("fruit-" + fruit + "-fragment-" + std::to_string(i))},
                                        tag::span("in fragment - " + fruit + " #" + std::to_string(i) + " ").TRACK,
                                        tag::br().TRACK
                                    ).TRACK
                                );
                            }
                            return tag::_fragment(lines).TRACK;
                        }).TRACK
                    ).TRACK
                ).TRACK;
            }).TRACK
        ).TRACK;
    }
};
