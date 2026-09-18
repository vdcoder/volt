#pragma once
#include <ConnectionHeartbeat.hpp>
#include <Talkers.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace voltxp {
// Session-owned, event-loop-only service. No Seasocks types escape the adapter.
class SessionWebsocketService {
public:
    using ConnectionId = std::uint64_t;
    using Clock = ConnectionHeartbeat::Clock;
    using Send = std::function<void(const MessageBytes&)>;
    void attach(ConnectionId connection, Send send) {
        if (m_connection) detach(m_connection);
        m_connection = connection;
        m_send = std::move(send);
        m_heartbeat = ConnectionHeartbeat(Clock::now());
        if (m_onConnection) m_onConnection(true);
    }
    void detach(ConnectionId connection) noexcept {
        if (m_connection != connection) return; // old socket cannot detach its replacement
        m_connection = 0;
        m_send = {};
        if (m_onConnection) m_onConnection(false);
    }
    void setOnConnection(std::function<void(bool)> callback) { m_onConnection = std::move(callback); }
    void fail() { detach(m_connection); }
    bool connected() const noexcept { return m_connection != 0; }
    bool isCurrent(ConnectionId connection) const noexcept { return connection && connection == m_connection; }
    // False tells the network adapter to close this connection.
    bool poll(ConnectionId connection, Clock::time_point now = Clock::now()) {
        if (!isCurrent(connection)) return false;
        try {
            if (auto token = m_heartbeat.poll(now)) m_send(controlMessage(Control::Ping, *token));
            if (!m_heartbeat.failed()) return true;
            std::cerr << "Session heartbeat timed out" << std::endl;
        } catch (const std::exception& error) { std::cerr << "Session send failed: " << error.what() << std::endl; }
        catch (...) { std::cerr << "Session send failed" << std::endl; }
        detach(connection);
        return false;
    }
    TalkerRegistry& talkers() noexcept { return m_talkers; }
    void send(TalkerId id, MessageView payload) {
        if (!id || !connected()) throw std::logic_error("Cannot send application message");
        try { m_send(envelope(id, payload)); }
        catch (...) { detach(m_connection); throw; }
    }
    bool receive(ConnectionId connection, MessageView bytes) {
        auto now = Clock::now();
        if (!poll(connection, now)) return false;
        try {
            auto message = decodeEnvelope(bytes);
            if (message.id) {
                m_talkers.dispatch(message.id, message.payload);
                return isCurrent(connection);
            }
            auto control = decodeControl(message.payload);
            if (control.op == Control::Ping) {
                m_send(controlMessage(Control::Pong, control.token)); return true;
            }
            if (control.op == Control::Pong && m_heartbeat.pong(control.token, now)) return true;
            std::cerr << "Session rejected unexpected control/pong" << std::endl;
        } catch (const std::exception& error) { std::cerr << "Session message rejected: " << error.what() << std::endl; }
        catch (...) { std::cerr << "Session message rejected" << std::endl; }
        detach(connection);
        return false;
    }
private:
    TalkerRegistry m_talkers;
    ConnectionId m_connection = 0;
    Send m_send;
    std::function<void(bool)> m_onConnection;
    ConnectionHeartbeat m_heartbeat{Clock::now()};
};
} // namespace voltxp
