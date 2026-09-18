#pragma once
#include <seasocks/WebSocket.h>
#include "../sessions/ISessionRegistry.hpp"
#include "../sessions/services/SessionWebsocketService.hpp"
#include <charconv>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// Thin Seasocks adapter: cookie/origin validation, socket routing and takeover.
class SessionHandler final : public seasocks::WebSocket::Handler {
    using Clock = voltxp::ConnectionHeartbeat::Clock;
    ISessionRegistry& registry;
    std::uint64_t nextConnection = 0;
    struct Peer {
        SessionBase* session;
        std::uint64_t connection;
        bool active = true;
        Clock::time_point retiredAt{};
    };
    std::map<seasocks::WebSocket*, Peer> peers;
    std::map<std::string, seasocks::WebSocket*> sessions;
    static std::string cookie(const std::string& header) {
        std::string_view rest(header);
        while (!rest.empty()) {
            auto end = rest.find(';');
            auto part = rest.substr(0, end);
            while (!part.empty() && part.front() == ' ') part.remove_prefix(1);
            constexpr std::string_view name = "volt_session=";
            if (part.substr(0, name.size()) == name) {
                part.remove_prefix(name.size());
                if (part.size() != 32 || part.find_first_not_of("0123456789abcdef") != part.npos) return {};
                return std::string(part);
            }
            if (end == rest.npos) break;
            rest.remove_prefix(end + 1);
        }
        return {};
    }
    void forget(seasocks::WebSocket* socket) {
        auto found = peers.find(socket);
        if (found == peers.end()) return;
        auto current = sessions.find(found->second.session->id());
        if (current != sessions.end() && current->second == socket) sessions.erase(current);
        found->second.session->services().resolveDependency<voltxp::SessionWebsocketService>().detach(found->second.connection);
        peers.erase(found);
    }
    void drop(seasocks::WebSocket* socket) {
        forget(socket); // no pointer use after close, including delayed callbacks
        socket->close();
    }
public:
    explicit SessionHandler(ISessionRegistry& sessions) : registry(sessions) {}
    ~SessionHandler() override {
        // Registry outlives adapter. Drop all callbacks capturing server-owned sockets.
        for (auto& [socket, peer] : peers) peer.session->services().resolveDependency<voltxp::SessionWebsocketService>().detach(peer.connection);
    }
    void onConnect(seasocks::WebSocket* socket) override {
        auto id = cookie(socket->getHeader("Cookie"));
        // Browser requests must originate from this host. Cookie is identity, not authentication.
        auto origin = socket->getHeader("Origin");
        auto host = socket->getHeader("Host");
        if (id.empty() || (origin != "http://" + host && origin != "https://" + host)) {
            std::cerr << "Session rejected: " << (id.empty() ? "missing/invalid session cookie" : "origin does not match host")
                      << "; origin=" << origin << "; host=" << host << std::endl;
            socket->close(); return;
        }
        auto previous = sessions.find(id);
        if (previous != sessions.end()) {
            auto* old = previous->second;
            auto& peer = peers.at(old);
            peer.active = false;
            peer.retiredAt = Clock::now();
            const auto replaced = voltxp::controlMessage(voltxp::Control::Replaced);
            old->send(replaced.data(), replaced.size()); // give the browser time to stop reconnecting before closing
        }
        if (nextConnection == (std::numeric_limits<std::uint64_t>::max)()) { socket->close(); return; }
        auto& session = registry.getOrCreate(id);
        const auto connection = ++nextConnection;
        peers.emplace(socket, Peer{&session, connection});
        session.services().resolveDependency<voltxp::SessionWebsocketService>().attach(connection, [socket](const voltxp::MessageBytes& message) { socket->send(message.data(), message.size()); });
        sessions[id] = socket;
        const auto ready = voltxp::controlMessage(voltxp::Control::Ready);
        socket->send(ready.data(), ready.size());
    }
    void onDisconnect(seasocks::WebSocket* socket) override { forget(socket); }
    void onData(seasocks::WebSocket* socket, const uint8_t* data, size_t size) override {
        auto found = peers.find(socket);
        if (found == peers.end() || !found->second.active) return;
        auto& peer = found->second;
        if (!peer.session->services().resolveDependency<voltxp::SessionWebsocketService>().receive(peer.connection, {data, size})) drop(socket);
    }
    void onData(seasocks::WebSocket* socket, const char*) override {
        if (peers.count(socket)) { std::cerr << "Session rejected text frame (binary required)" << std::endl; drop(socket); }
    }
    void poll() {
        std::vector<seasocks::WebSocket*> expired;
        const auto now = Clock::now();
        for (auto& [socket, peer] : peers) {
            if (!peer.active) {
                if (now - peer.retiredAt >= std::chrono::seconds(2)) expired.push_back(socket);
                continue;
            }
            if (!peer.session->services().resolveDependency<voltxp::SessionWebsocketService>().poll(peer.connection, now)) expired.push_back(socket);
        }
        for (auto* socket : expired) drop(socket);
    }
};
