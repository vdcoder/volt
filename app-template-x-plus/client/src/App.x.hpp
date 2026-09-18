#pragma once
#include <Volt.hpp>
#include "components/Button.x.hpp"
#include "ApplicationServices.hpp"

using namespace volt;

// VOLT_APP_NAME — Main app
class VOLT_APP_NAME_CAMEL : public App {
private:
    int counter = 0;
    std::string httpMessage = "Ready to call /api/hello";
    std::uint32_t httpRequest = 0;
    std::shared_ptr<int> httpLifetime = std::make_shared<int>(0);
    bool showPanel = true;
    std::vector<std::string> fruits = {"Apple", "Banana", "Cherry"};

    void callHello() {
        auto& http = services().resolveDependency<voltxp::HttpClientService>();
        if (httpRequest) http.cancel(httpRequest);
        httpMessage = "Loading...";
        httpRequest = http.request({"/api/hello"}, [this, lifetime = std::weak_ptr<int>(httpLifetime)](voltxp::HttpResponse response) {
            if (lifetime.expired()) return;
            httpRequest = 0;
            httpMessage = response.error.empty() ? std::to_string(response.status) + ": " + response.body : response.error;
            invalidate();
        });
    }
    void incrementShared() {
        auto& front = services().resolveDependency<voltxp::FrontDataService>();
        if (!front.ready()) return;
        auto& store = front.store();
        if (!store.memberCount(store.root(), voltxp::MemoryType::I32))
            store.allocate<std::int32_t>(store.root(), 1);
        else {
            auto handle = store.field<std::int32_t>(store.root(), 0);
            const auto value = store.get<std::int32_t>(handle);
            if (value < INT32_MAX) store.set<std::int32_t>(handle, value + 1);
        }
    }
    std::string sharedValues() {
        auto& front = services().resolveDependency<voltxp::FrontDataService>().store();
        auto& back = services().resolveDependency<voltxp::BackDataService>().store();
        auto sent = front.memberCount(front.root(), voltxp::MemoryType::I32)
            ? front.get<std::int32_t>(front.field<std::int32_t>(front.root(), 0)) : 0;
        auto received = back.memberCount(back.root(), voltxp::MemoryType::I64)
            ? back.get<std::int64_t>(back.field<std::int64_t>(back.root(), 0)) : 0;
        return "Client: " + std::to_string(sent) + " / Server doubled: " + std::to_string(received);
    }
public:
    VOLT_APP_NAME_CAMEL(IRuntime& a_runtime) : App(a_runtime) {}

    VNodeHandle render() override {
        return <div({ style:=("font-family: sans-serif; padding: 14px; max-width: 600px;") },

            // TITLE
            <h1("Welcome to Volt ⚡")/>,
            <p("A C++ WebAssembly UI framework using the Volt X DSL.")/>,
            <p("Server connection: " + services().resolveDependency<voltxp::ClientWebsocketService>().state())/>,
            <br()/>,

            <h2("Live shared data")/>,
            <p(sharedValues())/>,
            <(Button("Send increment", [this](auto){ incrementShared(); }, "primary"))/>,
            <p("Connection changes reset both values.")/>,

            <h2("HTTP API")/>,
            <p(httpMessage)/>,
            <(Button("Call hello API", [this](auto){ callHello(); }, "primary"))/>,

            // COUNTER SECTION
            <h2("Counter")/>,
            <p("Current value: " + std::to_string(counter))/>,

            <div(
                <(Button(
                    "Increment",
                    [this](auto){ counter++; },
                    "primary"
                ))/>,

                <(Button(
                    "Decrement",
                    [this](auto){ counter--; },
                    "secondary"
                ))/>
            )/>,

            (counter > 10
                ? <p({ style:=("color: #dc3545; font-weight: bold;") },
                    "Careful! That's a high number.")/>
                : <br()/>),
            <br()/>,

            // TOGGLE PANEL
            <h2("Toggle Panel")/>,
            <(Button(
                (showPanel ? "Hide Panel" : "Show Panel"),
                [this](auto){ showPanel = !showPanel; },
                "primary"
            ))/>,

            <div((!showPanel)
                ?   <p({ style:=("color:#666;") }, "(panel is hidden)")/>
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
                    return <li({ key:=(fruit) },
                        <span(fruit)/>,
                        <(Button(
                            "Remove",
                            [this, fruit](auto){
                                // Remove fruit
                                fruits.erase(std::remove(fruits.begin(), fruits.end(), fruit),
                                            fruits.end());
                            },
                            "danger"
                        ))/>
                    )/>;
                })/>
            )/>,

            <(Button(
                "Add Random Fruit",
                [this](auto){
                    static int id = 0;
                    fruits.push_back("Fruit" + std::to_string(++id));
                },
                "primary"
            ))/>,

            <br()/>,
            <br()/>,

            // FOOTER
            <p({style:=("color:#888; font-size: 13px;")},
                "Edit client/src/App.x.hpp to begin your journey!")/>
        )/>;
    }
};
