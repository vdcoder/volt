#pragma once
#include "MemoryStore.hpp"
#include "ActionMessage.hpp"

namespace voltxp {
enum class DataRole { Author, Replica };
enum class DataSide { Client, Server };

// One store per session/direction. The connection owner supplies ordered delivery
// and rejects callbacks from old sockets. No versions, ACKs, retries or batches.
class ReplicatedDataService {
    DataRole m_role;
    MemoryStore m_working;
    bool m_ready = false;
    bool m_failed = true;
    bool m_sending = false;
    std::function<void(MemoryChange)> m_sendChange;
    void requireRole(DataRole role) const {
        if (m_role != role) throw std::logic_error("Operation is invalid for this data-service role");
    }
    void requireReady() const {
        if (!ready()) throw std::logic_error("Data connection is inactive");
    }
public:
    explicit ReplicatedDataService(DataRole role) : m_role(role) {
        if (role == DataRole::Replica) m_working.makeReadOnly();
        else m_working.setOnChange([this](MemoryChange change) {
            if (!ready()) return; // offline edits live only in the store, never queued
            try {
                if (m_sending) throw std::logic_error("Reentrant data send");
                if (!m_sendChange) throw std::logic_error("No change sender installed");
                m_sending = true;
                m_sendChange(std::move(change));
                m_sending = false;
                requireReady();
            } catch (...) { m_sending = false; disconnect(); throw; }
        });
        m_working.reset();
    }
    ReplicatedDataService(const ReplicatedDataService&) = delete;
    ReplicatedDataService& operator=(const ReplicatedDataService&) = delete;
    MemoryStore& store() noexcept { return m_working; }
    const MemoryStore& store() const noexcept { return m_working; }
    DataRole role() const noexcept { return m_role; }
    bool ready() const noexcept { return m_ready && !m_failed; }
    void disconnect() {
        m_ready = false;
        m_failed = true;
        m_working.reset();
    }
    // Only the connection owner calls this, once a NEW socket replaces the old.
    void beginConnection() {
        m_ready = false;
        m_failed = true;
        m_working.reset();
        m_failed = false;
        m_ready = true;
    }

    void receive(const MemoryChange& change) {
        try {
            requireRole(DataRole::Replica);
            requireReady();
            m_working.apply(change);
        } catch (...) { disconnect(); throw; }
    }
    // Sender must serialize/copy synchronously on the same ordered socket used
    // for actions. Throw on failure. Do not mutate the store/reenter from it.
    void setChangeSender(std::function<void(MemoryChange)> send) {
        requireRole(DataRole::Author);
        if (m_sending) throw std::logic_error("Cannot replace an active sender");
        m_sendChange = std::move(send);
    }
    template<class SendAction>
    void sendAction(ActionMessage action, SendAction&& sendAction) {
        try {
            requireRole(DataRole::Author);
            requireReady();
            if (m_sending) throw std::logic_error("Reentrant action send");
            if (action.name.empty()) throw std::invalid_argument("Action name cannot be empty");
            sendAction(action); // preceding mutations have already emitted their changes
            requireReady();
        } catch (...) { disconnect(); throw; }
    }
    template<class Execute> void receiveAction(const ActionMessage& action, Execute&& execute) {
        try {
            requireRole(DataRole::Replica);
            requireReady();
            if (action.name.empty()) throw std::invalid_argument("Action name cannot be empty");
            execute(action); // dispatch in stream order; async continuation is app-owned
        } catch (...) { disconnect(); throw; }
    }
};

class FrontDataService : public ReplicatedDataService {
public:
    explicit FrontDataService(DataSide side)
        : ReplicatedDataService(side == DataSide::Client ? DataRole::Author : DataRole::Replica) {}
};
class BackDataService : public ReplicatedDataService {
public:
    explicit BackDataService(DataSide side)
        : ReplicatedDataService(side == DataSide::Server ? DataRole::Author : DataRole::Replica) {}
};
} // namespace voltxp
