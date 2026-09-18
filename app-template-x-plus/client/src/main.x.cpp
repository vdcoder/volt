#include <emscripten/bind.h>
#include <Volt.hpp>
#include "ApplicationServices.hpp"
#include "App.x.hpp"

using namespace volt;
using namespace emscripten;

EMSCRIPTEN_BINDINGS(VOLT_APP_NAME_UNDERSCORE_module) {
    function("onHttpResponse", +[](std::uint32_t id, int status, val headers, std::string body, std::string error) {
        services().resolveDependency<voltxp::HttpClientService>().onResponse(id, status, headers, std::move(body), std::move(error));
    });
    emscripten::function("onClientWebsocketMessage", +[](std::uint16_t id, emscripten::val payload) {
        return services().resolveDependency<voltxp::ClientWebsocketService>().onMessage(id, payload);
    });
    function("getVoltNamespace", &volt::config::getVoltNamespace);
    function("onClientWebsocketStateChanged", +[](std::string state) {
        services().resolveDependency<voltxp::ClientWebsocketService>().onStateChanged(std::move(state));
    });

    function("createVoltEngine", +[](std::string rootId) {
        auto& application = app_detail::application();
        if (application.engine)
            throw std::logic_error("A Volt engine already exists in this module instance");
        application.engine = std::make_unique<VoltEngine>(rootId, "VOLT_APP_NAME_UNDERSCORE_module");
        services().registerDependencies(*application.engine);
        application.engine->mountApp<VOLT_APP_NAME_CAMEL>();
        services().resolveDependency<voltxp::ClientWebsocketService>().start();
    });

    function("invokeVoltBubbleEvent", +[](emscripten::val event) {
        invokeBubbleEvent(event);
    });

    function("invokeVoltNonBubbleEvent", +[](emscripten::val event) {
        invokeNonBubbleEvent(event);
    });

    function("clearVoltFocussedElements", +[]() {
        app_detail::application().engine->clearFocussedElements();
    });

    function("addVoltFocussedElement", +[](emscripten::val element) {
        app_detail::application().engine->addFocussedElement(element);
    });
}
