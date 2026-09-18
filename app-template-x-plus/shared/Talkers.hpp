#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

namespace voltxp {
using TalkerId = std::uint16_t;
using MessageBytes = std::vector<std::uint8_t>;
// Borrowed bytes: valid only for the synchronous handler call. Copy to retain.
struct MessageView { const std::uint8_t* data; std::size_t size; };
inline MessageView view(const MessageBytes& bytes) { return {bytes.data(), bytes.size()}; }
inline MessageBytes envelope(TalkerId id, MessageView payload) {
    MessageBytes result{static_cast<std::uint8_t>(id), static_cast<std::uint8_t>(id >> 8)};
    if (payload.size) result.insert(result.end(), payload.data, payload.data + payload.size);
    return result;
}
struct TalkerMessage { TalkerId id; MessageView payload; };
inline TalkerMessage decodeEnvelope(MessageView bytes) {
    if (bytes.size < 2) throw std::invalid_argument("Truncated talker envelope");
    return {static_cast<TalkerId>(bytes.data[0] | (std::uint16_t(bytes.data[1]) << 8)),
            {bytes.data + 2, bytes.size - 2}};
}
enum class Control : std::uint8_t { Ready = 1, Ping = 2, Pong = 3, Replaced = 4 };
inline MessageBytes controlMessage(Control op, std::uint64_t token = 0) {
    MessageBytes result{0, 0, static_cast<std::uint8_t>(op)};
    if (op == Control::Ping || op == Control::Pong) {
        if (!token) throw std::invalid_argument("Zero heartbeat token");
        for (int i = 0; i < 8; ++i) result.push_back(static_cast<std::uint8_t>(token >> (8 * i)));
    }
    return result;
}
struct ControlMessage { Control op; std::uint64_t token; };
inline ControlMessage decodeControl(MessageView payload) {
    if (!payload.size) throw std::invalid_argument("Missing control opcode");
    auto op = static_cast<Control>(payload.data[0]);
    if (op == Control::Ready || op == Control::Replaced) {
        if (payload.size != 1) throw std::invalid_argument("Invalid control length");
        return {op, 0};
    }
    if ((op != Control::Ping && op != Control::Pong) || payload.size != 9)
        throw std::invalid_argument("Invalid heartbeat message");
    std::uint64_t token = 0;
    for (int i = 0; i < 8; ++i) token |= std::uint64_t(payload.data[i + 1]) << (8 * i);
    if (!token) throw std::invalid_argument("Zero heartbeat token");
    return {op, token};
}
class TalkerRegistry {
    std::map<TalkerId, std::function<void(MessageView)>> m_handlers;
public:
    void registerTalker(TalkerId id, std::function<void(MessageView)> handler) {
        if (!id || !handler) throw std::invalid_argument("Talker zero is reserved; handler is required");
        if (!m_handlers.emplace(id, std::move(handler)).second) throw std::logic_error("Talker already registered");
    }
    void unregisterTalker(TalkerId id) { m_handlers.erase(id); }
    void dispatch(TalkerId id, MessageView payload) const {
        auto found = m_handlers.find(id);
        if (found == m_handlers.end()) throw std::invalid_argument("Unknown talker");
        auto handler = found->second; // permits unregistering from inside the callback
        handler(payload);
    }
};
} // namespace voltxp
