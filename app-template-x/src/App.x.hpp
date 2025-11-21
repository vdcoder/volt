#pragma once
#include <Volt.hpp>
#include "components/Button.x.hpp"

using namespace volt;

// VOLT_APP_NAME - Main Application
class VOLT_APP_NAME_CAMEL : public AppBase {
private:
    int counter = 0;
    std::string message = "Hello from VOLT_APP_NAME!";
    
public:
    VOLT_APP_NAME_CAMEL(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNodeHandle render() override {
        return <div({style:=("font-family: sans-serif; padding: 20px;")},
            <h1(message)/>,
            <p("Counter: " + std::to_string(counter))/>, 

            // Using reusable Button component
            // Note: No need to call invalidate() - auto-invalidate in event handlers
            Button(getRuntime()).<render("Increment", [this](emscripten::val e) {
                log("Increment button clicked");
                counter++;
            }, "primary")/>,

            <div(
                x::iff(counter % 2 == 0, [this](){
                    return std::vector<VNodeHandle>{
                        // Button(getRuntime()).render("Reset", [this](emscripten::val e) {
                        //     log("Reset button clicked");
                        //     counter = 0;
                        // }, "danger").track(__COUNTER__) 
                        <button({
                            onAddElement:=([this](emscripten::val e) {
                                log("onAddElement button clicked");
                            }),
                            onBeforeMoveElement:=([this](emscripten::val e) {
                                log("onBeforeMoveElement button clicked");
                            }),
                            onMoveElement:=([this](emscripten::val e) {
                                log("onMoveElement button clicked");
                            }),
                            onRemoveElement:=([this](emscripten::val e) {
                                log("onRemoveElement button clicked");
                            }),
                            onClick:=([this](emscripten::val e) {
                                log("Reset button clicked");
                                counter = 0;
                            })
                        }, "Reset")/>
                    };
                }, []() {
                    return std::vector<VNodeHandle>{"No button for you!"};
                })
            )/>,

            (counter > 5 ? <p({style:=("color: red;")}, "Counter exceeded 5!")/> : <br()/>),

            <:=(counter > 5 ? <p({style:=("color: red;")}, "Counter exceeded 5!")/> : <br()/>)/>,

            <map(std::vector<std::string>{"Apple"}, [this](const std::string& fruit) {
                return <({ attr::key(fruit) },
                    <article({style:=("border: 1px solid #ccc; padding: 10px; margin: 10px 0; height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;"),
                            onAddElement:=([this](emscripten::val e) {
                                log("onAddElement article");
                            }),
                            onBeforeMoveElement:=([this](emscripten::val e) {
                                log("onBeforeMoveElement article");
                            }),
                            onMoveElement:=([this](emscripten::val e) {
                                log("onMoveElement article");
                            }),
                            onRemoveElement:=([this](emscripten::val e) {
                                log("onRemoveElement article");
                            })},
                        <h1(fruit)/>,
                        <br()/>,
                        <a({href:=("https://example.com/" + fruit)}, fruit)/>,
                        <map(std::vector<int>{0}, [fruit](int num) {
                            std::vector<VNodeHandle> lines;
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(<span({attr::id("fruit-" + fruit + "-line-span-" + std::to_string(i))}, " - " + fruit + " #" + std::to_string(i) + " ")/>);
                                lines.push_back(<br({attr::id("fruit-" + fruit + "-line-br-" + std::to_string(i))})/>);
                            }
                            for (int i = 0; i < 20; i++) {
                                lines.push_back(
                                    <({attr::key("fruit-" + fruit + "-fragment-" + std::to_string(i))},
                                        <span("in fragment - " + fruit + " #" + std::to_string(i) + " ")/>,
                                        <br()/>
                                    )/>
                                );
                            }
                            return <(lines)/>;
                        })/>
                    )/>
                )/>;
            })/>
        )/>;
    }
};
