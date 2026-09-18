#include "../app-template-x-plus/client/src/AppDI.hpp"
#include <emscripten/bind.h>
struct Runtime : volt::IRuntime {
    int invalidations = 0;
    void invalidate() override { ++invalidations; }
};
Runtime runtime;
AppDI dependencies;
int received = 0;
auto& websocket() { return dependencies.resolveDependency<voltxp::ClientWebsocketService>(); }
EMSCRIPTEN_BINDINGS(websocket_di) {
    emscripten::function("onClientWebsocketMessage", +[](std::uint16_t id, emscripten::val payload) {
        return dependencies.resolveDependency<voltxp::ClientWebsocketService>().onMessage(id, payload);
    });
    emscripten::function("initialize", +[] {
        dependencies.registerDependencies(runtime);
        websocket().talkers().registerTalker(0x1234, [](voltxp::MessageView bytes) {
            if (bytes.size != 3 || bytes.data[0] != 0 || bytes.data[1] != 255 || bytes.data[2] != 128)
                throw std::runtime_error("Payload mismatch");
            ++received;
        });
    });
    emscripten::function("writeFront", +[] {
        auto& store = dependencies.resolveDependency<voltxp::FrontDataService>().store();
        store.allocate<std::int32_t>(store.root(), 7);
    });
    emscripten::function("readBack", +[] {
        auto& store = dependencies.resolveDependency<voltxp::BackDataService>().store();
        return double(store.get<std::int64_t>(store.field<std::int64_t>(store.root(), 0)));
    });
    emscripten::function("received", +[] { return received; });
    emscripten::function("sendTest", +[] {
        const voltxp::MessageBytes bytes{0, 255, 128};
        websocket().send(0x1234, voltxp::view(bytes));
    });
    emscripten::function("onClientWebsocketStateChanged", +[](std::string state) { websocket().onStateChanged(std::move(state)); });
    emscripten::function("start", +[] { websocket().start(); });
    emscripten::function("stop", +[] { websocket().stop(); });
    emscripten::function("state", +[] { return websocket().state(); });
    emscripten::function("invalidations", +[] { return runtime.invalidations; });
    emscripten::function("release", +[] { dependencies.releaseDependencies(); });
}
