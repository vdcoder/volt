#include "Session.hpp"
#include "SessionDI.hpp"

namespace {
std::unique_ptr<SessionDI> createServices() {
    auto services = std::make_unique<SessionDI>();
    services->registerDependencies();
    return services;
}
}

Session::Session(std::string id) : SessionBase(std::move(id), createServices()) {}

void Session::onStarted() {
    auto& socket = services().resolveDependency<voltxp::SessionWebsocketService>();
    socket.talkers().registerTalker(1, [this](voltxp::MessageView payload) {
        services().resolveDependency<voltxp::SessionWebsocketService>().send(1, payload);
    });
    m_echoRegistered = true;
    // Demo: observe the client's counter and publish a server-computed value.
    auto& front = services().resolveDependency<voltxp::FrontDataService>();
    front.store().setOnUpdated([this] {
        auto& front = services().resolveDependency<voltxp::FrontDataService>();
        auto& back = services().resolveDependency<voltxp::BackDataService>();
        if (!front.ready() || !back.ready()) return;
        auto& input = front.store();
        if (!input.memberCount(input.root(), voltxp::MemoryType::I32)) return;
        const auto value = std::int64_t(input.get<std::int32_t>(input.field<std::int32_t>(input.root(), 0))) * 2;
        auto& output = back.store();
        if (!output.memberCount(output.root(), voltxp::MemoryType::I64))
            output.allocate<std::int64_t>(output.root(), value);
        else output.set<std::int64_t>(output.field<std::int64_t>(output.root(), 0), value);
    });
}

void Session::onClosed(SessionCloseReason reason) noexcept {
    // Providers are still alive, including after partial startup.
    // Cancel app work here; cleanup must not throw.
    (void)reason;
    services().resolveDependency<voltxp::FrontDataService>().store().setOnUpdated({});
    if (m_echoRegistered) {
        services().resolveDependency<voltxp::SessionWebsocketService>()
            .talkers().unregisterTalker(1);
        m_echoRegistered = false;
    }
}
