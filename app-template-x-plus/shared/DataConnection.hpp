#pragma once
#include "DataServices.hpp"
#include "MemoryWire.hpp"

namespace voltxp {
// Own after the stores and transport in DI, so handlers detach before providers.
class DataConnection {
    FrontDataService& m_front;
    BackDataService& m_back;
    TalkerRegistry& m_talkers;
public:
    static constexpr TalkerId Front = 2, Back = 3;
    using Send = std::function<void(TalkerId, MessageView)>;
    DataConnection(FrontDataService& front, BackDataService& back, TalkerRegistry& talkers, Send send, std::function<void()> fail)
        : m_front(front), m_back(back), m_talkers(talkers) {
        auto bind = [&](ReplicatedDataService& service, TalkerId id) {
            if (service.role() == DataRole::Author)
                service.setChangeSender([send, fail, id](MemoryChange change) {
                    try { auto bytes = encodeChange(change); send(id, view(bytes)); }
                    catch (...) { fail(); throw; }
                });
            else m_talkers.registerTalker(id, [&service](MessageView bytes) { service.receive(decodeChange(bytes)); });
        };
        bind(m_front, Front); bind(m_back, Back);
    }
    ~DataConnection() {
        if (m_front.role() == DataRole::Replica) m_talkers.unregisterTalker(Front);
        else m_front.setChangeSender({});
        if (m_back.role() == DataRole::Replica) m_talkers.unregisterTalker(Back);
        else m_back.setChangeSender({});
    }
    void connected() { m_front.beginConnection(); m_back.beginConnection(); }
    void disconnected() { m_front.disconnect(); m_back.disconnect(); }
};
} // namespace voltxp
