#pragma once
#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>

namespace voltxp {
// One instance per socket. Drive with steady_clock on the owning event loop.
// The owner closes the socket/disconnects both services when failed() becomes true.
class ConnectionHeartbeat {
public:
    using Clock = std::chrono::steady_clock;
    using Time = Clock::time_point;
    static constexpr auto interval = std::chrono::seconds(5);
    static constexpr auto timeout = std::chrono::seconds(2);
    explicit ConnectionHeartbeat(Time connectedAt) : m_next(connectedAt + interval) {}
    bool failed() const noexcept { return m_failed; }
    void fail() noexcept { m_failed = true; }
    // Returns a ping token to send immediately. Token correlates pongs only;
    // it is not a data version. A send error must call fail().
    std::optional<std::uint64_t> poll(Time now) {
        if (m_failed) return std::nullopt;
        if (m_waiting && now >= m_deadline) { fail(); return std::nullopt; }
        if (m_waiting || now < m_next) return std::nullopt;
        if (m_token == (std::numeric_limits<std::uint64_t>::max)()) {
            fail(); return std::nullopt;
        }
        ++m_token;
        m_waiting = true;
        m_deadline = now + timeout;
        m_next = now + interval;
        return m_token;
    }
    // Check the clock here as well: a delayed timer must not admit a late pong.
    bool pong(std::uint64_t token, Time now) noexcept {
        if (m_failed) return false;
        if (!m_waiting || token != m_token || now >= m_deadline) {
            fail(); return false;
        }
        m_waiting = false;
        return true;
    }
private:
    Time m_next;
    Time m_deadline{};
    std::uint64_t m_token = 0;
    bool m_waiting = false;
    bool m_failed = false;
};
} // namespace voltxp
