#include "../app-template-x-plus/server/sessions/SessionRegistry.hpp"
#include "../app-template-x-plus/server/sessions/Session.hpp"
#include "../app-template-x-plus/server/sessions/SessionDI.hpp"
#include <cassert>
#include <iostream>
#include <vector>

struct ProbeSession : SessionBase {
    inline static std::vector<int> events;
    explicit ProbeSession(std::string id)
        : SessionBase(std::move(id), std::make_unique<voltxp::DependencyInjection>()) {
        services().registerDependency<int>(std::make_unique<int>(42));
        assert(events.empty()); // no startup during construction
    }
    ~ProbeSession() override { events.push_back(3); }
    void onStarted() override {
        assert(services().resolveDependency<int>() == 42);
        events.push_back(1);
        if (id() == "failed") throw std::runtime_error("startup failure");
    }
    void onClosed(SessionCloseReason reason) noexcept override {
        assert(services().resolveDependency<int>() == 42);
        events.push_back(reason == SessionCloseReason::StartupFailed ? 4 :
                         reason == SessionCloseReason::ServerShutdown ? 5 : 2);
    }
};
struct PlainSession : SessionBase {
    explicit PlainSession(std::string id)
        : SessionBase(std::move(id), std::make_unique<voltxp::DependencyInjection>()) {}
};
int main() {
    auto rejected = [](auto action) {
        bool caught = false;
        try { action(); } catch (const std::exception&) { caught = true; }
        assert(caught);
    };
    voltxp::TalkerRegistry registryChecks;
    rejected([&] { registryChecks.registerTalker(0, [](voltxp::MessageView) {}); });
    registryChecks.registerTalker(42, [](voltxp::MessageView) {});
    rejected([&] { registryChecks.registerTalker(42, [](voltxp::MessageView) {}); });
    rejected([&] { registryChecks.dispatch(43, {nullptr, 0}); });
    registryChecks.unregisterTalker(42);
    rejected([&] { registryChecks.dispatch(42, {nullptr, 0}); });
    {
        SessionDI dependencies;
        dependencies.registerDependencies();
        dependencies.resolveDependency<voltxp::SessionWebsocketService>().talkers()
            .registerTalker(1, [](voltxp::MessageView) {}); // DI does not start activity
    }
    { SessionRegistry<PlainSession> plain; plain.getOrCreate("plain"); }
    for (const auto* id : {"explicit", "shutdown", "failed"}) {
        ProbeSession::events.clear();
        {
            SessionRegistry<ProbeSession> probes;
            ISessionRegistry& networkView = probes;
            if (std::string(id) == "failed") {
                rejected([&] { networkView.getOrCreate(id); });
                assert(probes.size() == 0);
            } else {
                auto& session = probes.getOrCreate(id);
                assert(&networkView.getOrCreate(id) == &session);
                assert((ProbeSession::events == std::vector<int>{1}));
                if (std::string(id) == "explicit") {
                    session.close(SessionCloseReason::ApplicationRequested);
                    session.close(SessionCloseReason::Expired);
                }
            }
        }
        const int closeEvent = std::string(id) == "failed" ? 4 : std::string(id) == "shutdown" ? 5 : 2;
        assert((ProbeSession::events == std::vector<int>{1, closeEvent, 3}));
    }
    SessionRegistry<Session> registry;
    auto& first = registry.getOrCreate("first");
    first.services().registerDependency<int>(std::make_unique<int>(42));
    auto& other = registry.getOrCreate("other");
    assert(&registry.getOrCreate("first") == &first && registry.size() == 2);
    auto& ws = first.services().resolveDependency<voltxp::SessionWebsocketService>();
    std::vector<voltxp::MessageBytes> oldMessages, newMessages;
    ws.attach(1, [&](const voltxp::MessageBytes& s) { oldMessages.push_back(s); });
    assert(ws.connected() && !other.services().resolveDependency<voltxp::SessionWebsocketService>().connected());
    assert(ws.receive(1, voltxp::view(voltxp::controlMessage(voltxp::Control::Ping, 1))) && oldMessages.back() == voltxp::controlMessage(voltxp::Control::Pong, 1));
    ws.attach(2, [&](const voltxp::MessageBytes& s) { newMessages.push_back(s); });
    ws.detach(1); // late disconnect from replaced socket
    assert(ws.connected() && ws.isCurrent(2));
    assert(!ws.receive(1, voltxp::view(voltxp::controlMessage(voltxp::Control::Ping, 2))) && ws.isCurrent(2));
    assert(ws.receive(2, voltxp::view(voltxp::controlMessage(voltxp::Control::Ping, 3))) && newMessages.back() == voltxp::controlMessage(voltxp::Control::Pong, 3));
    voltxp::MessageBytes payload{0, 255, 128, 42};
    assert(ws.receive(2, voltxp::view(voltxp::envelope(1, voltxp::view(payload)))));
    assert(newMessages.back() == voltxp::envelope(1, voltxp::view(payload)));
    int calls = 0;
    ws.talkers().registerTalker(0x1234, [&](voltxp::MessageView bytes) { ++calls; assert(bytes.size == 4); });
    auto frame = voltxp::envelope(0x1234, voltxp::view(payload));
    assert(frame[0] == 0x34 && frame[1] == 0x12);
    assert(ws.receive(2, voltxp::view(frame)) && calls == 1);
    ws.detach(2);
    assert(!ws.connected());
    assert(registry.getOrCreate("first").services().resolveDependency<int>() == 42);
    ws.attach(3, [&](const voltxp::MessageBytes& s) { newMessages.push_back(s); });
    const auto now = voltxp::SessionWebsocketService::Clock::now();
    assert(ws.poll(3, now + std::chrono::seconds(5)));
    assert(newMessages.back() == voltxp::controlMessage(voltxp::Control::Ping, 1));
    assert(!ws.poll(3, now + std::chrono::seconds(7)) && !ws.connected());
    ws.attach(4, [](const voltxp::MessageBytes&) { throw std::runtime_error("send failed"); });
    assert(!ws.receive(4, voltxp::view(voltxp::controlMessage(voltxp::Control::Ping, 1))) && !ws.connected());
    ws.attach(5, [](const voltxp::MessageBytes&) {});
    assert(!ws.receive(5, {nullptr, 0}) && !ws.connected());
    std::cout << "PASS: per-session DI, reconnect persistence, connection replacement, stale callback isolation, heartbeat and send errors\n";
}
