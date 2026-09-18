#pragma once
#include <DependencyInjection.hpp>
#include "SessionCloseReason.hpp"
#include <memory>
#include <string>
#include <stdexcept>

template<class T> class SessionRegistry;

class SessionBase {
    template<class T> friend class SessionRegistry;
    std::string m_id;
    std::unique_ptr<voltxp::DependencyInjection> m_services;
    bool m_closed = false;
    void start() {
        try { onStarted(); }
        catch (...) { close(SessionCloseReason::StartupFailed); throw; }
    }
protected:
    SessionBase(std::string id, std::unique_ptr<voltxp::DependencyInjection> services)
        : m_id(std::move(id)), m_services(std::move(services)) {
        if (!m_services) throw std::invalid_argument("Session requires services");
    }
    virtual void onStarted() {}
    virtual void onClosed(SessionCloseReason) noexcept {}
public:
    virtual ~SessionBase() = default;
    SessionBase(const SessionBase&) = delete;
    SessionBase& operator=(const SessionBase&) = delete;
    const std::string& id() const noexcept { return m_id; }
    voltxp::DependencyInjection& services() noexcept { return *m_services; }
    // Stop networking before closing. The registry closes before derived destruction.
    void close(SessionCloseReason reason) noexcept {
        if (m_closed) return;
        m_closed = true;
        onClosed(reason);
        m_services->releaseDependencies();
    }
};
