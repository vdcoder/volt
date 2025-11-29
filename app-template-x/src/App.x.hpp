#pragma once
#include <Volt.hpp>
#include "components/Button.x.hpp"

using namespace volt;

// VOLT_APP_NAME — Main app
class VOLT_APP_NAME_CAMEL : public App {
private:
    int counter = 0;
    bool showPanel = true;
    std::vector<std::string> fruits = {"Apple", "Banana", "Cherry"};

public:
    VOLT_APP_NAME_CAMEL(IRuntime& a_runtime) : App(a_runtime) {}

    VNodeHandle render() override {
        return <div({ style:=("font-family: sans-serif; padding: 14px; max-width: 600px;") },

            // TITLE
            <h1("Welcome to Volt ⚡")/>,
            <p("A C++ WebAssembly UI framework using the Volt X DSL.")/>,
            <br()/>,

            // COUNTER SECTION
            <h2("Counter")/>,
            <p("Current value: " + std::to_string(counter))/>,

            <div(
                Button(getRuntime()).<render("Increment", [this](auto){
                    counter++;
                }, "primary")/>,

                Button(getRuntime()).<render("Decrement", [this](auto){
                    counter--;
                }, "secondary")/>
            )/>,

            (counter > 10
                ? <p({ style:=("color: #dc3545; font-weight: bold;") },
                    "Careful! That's a high number.")/>
                : <br()/>),
            <br()/>,

            // TOGGLE PANEL
            <h2("Toggle Panel")/>,
            Button(getRuntime()).<render(
                (showPanel ? "Hide Panel" : "Show Panel"),
                [this](auto){ showPanel = !showPanel; },
                "primary"
            )/>,

            <div((!showPanel) 
                ?   <p({style:=("color:#666;")}, "(panel is hidden)")/>
                :   <div({ style:=("padding: 12px; border: 1px solid #ddd; border-radius: 6px;") },
                        <h3("Hello from the Toggle Panel!")/>,
                        <p("This demonstrates conditional rendering and structural reuse.")/>
                    )/>
            )/>,

            <br()/>,

            // FRUIT LIST
            <h2("Fruit List")/>,
            <p("Here Volt demonstrates <map>, keys, and structural reuse:")/>,

            <ul(
                <map(fruits, [this](const std::string& fruit, size_t idx){
                    return <li({
                        attr::key(fruit)
                    },
                        <span(fruit)/>,
                        Button(getRuntime()).<render("Remove", [this, fruit](auto){
                            // Remove fruit
                            fruits.erase(std::remove(fruits.begin(), fruits.end(), fruit),
                                         fruits.end());
                        }, "danger")/>
                    )/>;
                })/>
            )/>,

            Button(getRuntime()).<render("Add Random Fruit", [this](auto){
                static int id = 0;
                fruits.push_back("Fruit" + std::to_string(++id));
            }, "primary")/>,

            <br()/>,
            <br()/>,

            // FOOTER
            <p({style:=("color:#888; font-size: 13px;")},
                "Edit src/App.x.hpp to begin your journey!")/>
        )/>;
    }
};
