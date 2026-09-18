#pragma once
#include "VoltRuntimeService.hpp"
#include <emscripten/val.h>
#include <Talkers.hpp>
#include <stdexcept>
#include <string>

namespace voltxp {
// DI owns this service; JS owns socket/timers. No C++ pointer crosses into JS.
class ClientWebsocketService {
    VoltRuntimeService& m_runtime;
    emscripten::val m_client = emscripten::val::undefined();
    TalkerRegistry m_talkers;
    std::function<void(bool)> m_onConnection;
    std::string m_state = "stopped";
public:
    explicit ClientWebsocketService(VoltRuntimeService& runtime) : m_runtime(runtime) {}
    ClientWebsocketService(const ClientWebsocketService&) = delete;
    ClientWebsocketService& operator=(const ClientWebsocketService&) = delete;
    ~ClientWebsocketService() {
        if (!m_client.isUndefined()) m_client.call<void>("dispose");
    }
    const std::string& state() const noexcept { return m_state; }
    bool connected() const noexcept { return m_state == "connected"; }
    void start() {
        if (m_client.isUndefined()) {
            auto factory = emscripten::val::global("VoltSession");
            if (factory.isUndefined()) throw std::runtime_error("Load session.js before starting ClientWebsocketService");
            m_client = factory.call<emscripten::val>("create",
                emscripten::val::module_property("onClientWebsocketStateChanged"),
                emscripten::val::module_property("onClientWebsocketMessage"));
        }
        m_client.call<void>("start");
    }
    void setOnConnection(std::function<void(bool)> callback) { m_onConnection = std::move(callback); }
    void fail() { if (!m_client.isUndefined()) m_client.call<void>("reconnect"); }
    TalkerRegistry& talkers() noexcept { return m_talkers; }
    void send(TalkerId id, MessageView payload) {
        if (!id || !connected()) throw std::logic_error("Cannot send application message");
        auto bytes = emscripten::val(emscripten::typed_memory_view(payload.size, payload.data));
        if (!m_client.call<bool>("send", id, bytes)) throw std::runtime_error("WebSocket send failed");
    }
    bool onMessage(TalkerId id, emscripten::val payload) {
        try {
            auto bytes = emscripten::convertJSArrayToNumberVector<std::uint8_t>(payload);
            m_talkers.dispatch(id, view(bytes));
            return true;
        } catch (...) { return false; } // JS closes/reconnects on protocol or handler failure
    }
    void stop() {
        if (!m_client.isUndefined()) m_client.call<void>("stop");
    }
    // Called by this module's Embind export; the JS instance drops it on dispose.
    void onStateChanged(std::string state) {
        if (m_state == state) return;
        const bool wasConnected = connected();
        m_state = std::move(state);
        if (m_onConnection && (wasConnected || connected())) m_onConnection(connected());
        m_runtime.getRuntime().invalidate();
    }
};
} // namespace voltxp
