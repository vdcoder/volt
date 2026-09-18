#pragma once
#include "ISessionRegistry.hpp"
#include <map>
#include <type_traits>

// Retain session state across reconnects; expiration/persistence come later.
template<class T>
class SessionRegistry : public ISessionRegistry {
    static_assert(std::is_base_of_v<SessionBase, T>, "Session must derive from SessionBase");
    struct CloseAndDelete {
        void operator()(T* session) const noexcept {
            session->close(SessionCloseReason::ServerShutdown);
            delete session;
        }
    };
    using OwnedSession = std::unique_ptr<T, CloseAndDelete>;
    std::map<std::string, OwnedSession> m_sessions;
public:
    T& getOrCreate(const std::string& id) override {
        auto found = m_sessions.find(id);
        if (found != m_sessions.end()) return *found->second;
        OwnedSession session(new T(id));
        static_cast<SessionBase&>(*session).start();
        auto& result = *session;
        m_sessions.emplace(id, std::move(session));
        return result;
    }
    std::size_t size() const noexcept { return m_sessions.size(); }
};
