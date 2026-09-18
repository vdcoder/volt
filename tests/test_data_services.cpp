#include <DataConnection.hpp>
#include <ConnectionHeartbeat.hpp>
#include <MemoryViews.hpp>
#include <cassert>
#include <iostream>
using namespace voltxp;
template<class F> void rejects(F fn) {
    bool rejected = false;
    try { fn(); } catch (const std::exception&) { rejected = true; }
    assert(rejected);
}
int main() {
    FrontDataService front(DataSide::Client), frontCopy(DataSide::Server);
    BackDataService back(DataSide::Server), backCopy(DataSide::Client);
    TalkerRegistry client, server;
    bool failed = false;
    auto fail = [&] { failed = true; front.disconnect(); frontCopy.disconnect(); back.disconnect(); backCopy.disconnect(); };
    DataConnection clientData(front, backCopy, client,
        [&](TalkerId id, MessageView bytes) { server.dispatch(id, bytes); }, fail);
    DataConnection serverData(frontCopy, back, server,
        [&](TalkerId id, MessageView bytes) { client.dispatch(id, bytes); }, fail);
    serverData.connected(); clientData.connected();
    int notifications = 0;
    frontCopy.store().setOnUpdated([&] { ++notifications; });
    auto child = front.store().createContainer(front.store().root());
    auto age = front.store().allocate<std::int32_t>(child, 20);
    auto remoteAge = frontCopy.store().localHandle(MemoryType::I32, age.slot);
    front.store().set<std::int32_t>(age, 21);
    front.store().set<std::int32_t>(age, 21);
    assert(notifications == 3 && frontCopy.store().get<std::int32_t>(remoteAge) == 21);
    auto bytes = encodeChange({MemoryOperation::SetValue, MemoryType::I32, {}, age, std::int32_t(42)});
    assert(bytes.size() == 10); // op + type + uint32 slot + int32 value
    auto original = bytes;
    bytes.push_back(0); rejects([&] { decodeChange(view(bytes)); });
    original.pop_back(); rejects([&] { decodeChange(view(original)); });
    auto value = back.store().allocate<std::int64_t>(back.store().root(), 9007199254740993LL);
    assert(backCopy.store().get<std::int64_t>(backCopy.store().localHandle(MemoryType::I64, value.slot)) == 9007199254740993LL);
    auto text = back.store().allocate<std::string>(back.store().root(), std::string("a\0b", 3));
    assert(backCopy.store().get<std::string>(backCopy.store().localHandle(MemoryType::String, text.slot)) == std::string("a\0b", 3));
    front.store().removeItem(front.store().root(), child);
    rejects([&] { frontCopy.store().get<std::int32_t>(remoteAge); });
    auto again = front.store().createContainer(front.store().root());
    age = front.store().allocate<std::int32_t>(again, 22);
    remoteAge = frontCopy.store().localHandle(MemoryType::I32, age.slot);
    auto oldRoot = frontCopy.store().root();
    serverData.disconnected(); clientData.disconnected();
    rejects([&] { frontCopy.store().validateContainer(oldRoot); });
    rejects([&] { front.store().get<std::int32_t>(age); });
    rejects([&] { front.store().set<std::int32_t>(age, 123); }); // rejected locally before any send
    // A brand-new author reuses slots at generation 1; the surviving replica does not.
    FrontDataService fresh(DataSide::Client);
    fresh.setChangeSender([&](MemoryChange change) {
        auto wire = encodeChange(change); frontCopy.receive(decodeChange(view(wire)));
    });
    frontCopy.beginConnection(); fresh.beginConnection();
    auto freshChild = fresh.store().createContainer(fresh.store().root());
    auto freshAge = fresh.store().allocate<std::int32_t>(freshChild, 99);
    auto localAge = frontCopy.store().localHandle(MemoryType::I32, freshAge.slot);
    assert(localAge.generation != freshAge.generation);
    assert(frontCopy.store().get<std::int32_t>(localAge) == 99);
    rejects([&] { frontCopy.store().get<std::int32_t>(remoteAge); });
    // Duplicate allocation is rejected, with a fresh empty store required afterward.
    rejects([&] { frontCopy.receive({MemoryOperation::AllocateValue, MemoryType::I32,
        freshChild, freshAge, std::int32_t(1)}); });
    assert(!frontCopy.ready());
    frontCopy.beginConnection();
    fresh.disconnect(); fresh.beginConnection();
    fresh.setChangeSender([](MemoryChange) { throw std::runtime_error("send failed"); });
    rejects([&] { fresh.store().allocate<bool>(fresh.store().root(), true); });
    assert(!fresh.ready() && fresh.store().memberCount(fresh.store().root(), MemoryType::Bool) == 0);
    assert(!failed);
    using namespace std::chrono_literals;
    const ConnectionHeartbeat::Time zero{};
    ConnectionHeartbeat heartbeat(zero);
    assert(!heartbeat.poll(zero + 4999ms));
    auto ping = heartbeat.poll(zero + 5s);
    assert(ping && !heartbeat.poll(zero + 6s));
    assert(heartbeat.pong(*ping, zero + 6999ms));
    assert(!heartbeat.poll(zero + 9999ms));
    auto next = heartbeat.poll(zero + 10s);
    assert(next && *next != *ping);
    assert(!heartbeat.poll(zero + 12s) && heartbeat.failed()); // no pong needed to detect failure
    assert(!heartbeat.pong(*next, zero + 12s));
    ConnectionHeartbeat late(zero);
    auto latePing = late.poll(zero + 5s);
    assert(!late.pong(*latePing, zero + 8s) && late.failed()); // timer callback delayed
    ConnectionHeartbeat mismatch(zero);
    auto expected = mismatch.poll(zero + 5s);
    assert(!mismatch.pong(*expected + 1, zero + 6s) && mismatch.failed());
    ConnectionHeartbeat socketError(zero);
    socketError.fail();
    assert(!socketError.poll(zero + 5s) && socketError.failed());
    ConnectionHeartbeat boundary(zero);
    auto boundaryPing = boundary.poll(zero + 5s);
    assert(!boundary.pong(*boundaryPing, zero + 7s));
    std::cout << "PASS: binary changes, local generations, reset, fresh author, notifications, slot reuse and 5s/2s heartbeat\n";
}
