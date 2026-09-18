#include <MemoryViews.hpp>
#include <examples/Student.hpp>
#include <cassert>
#include <iostream>
#include <memory>
using namespace voltxp;
template<class F> void rejects(F action) {
    bool caught = false;
    try { action(); } catch (const std::exception&) { caught = true; }
    assert(caught);
}
struct FailingStudent : examples::Student {
    using examples::Student::Student;
    static void initialize(MemoryStore& store, Handle handle) {
        store.allocate<std::int32_t>(handle, 1);
        throw std::runtime_error("failed initialization");
    }
};
int main() {
    {
        MemoryStore author, replica;
        replica.makeReadOnly();
        replica.setOnChange([](MemoryChange) { assert(false); });
        author.setOnChange([&](MemoryChange change) { replica.apply(change); });
        const auto root = author.createRoot();
        const auto child = author.createContainer(root);
        const auto value = author.allocate<std::int32_t>(child, 1);
        author.set<std::int32_t>(value, 2);
        assert(replica.get<std::int32_t>(value) == 2 && replica.isReadOnly());
        rejects([&] { replica.createRoot(); });
        rejects([&] { replica.createContainer(root); });
        rejects([&] { replica.allocate<bool>(child, true); });
        rejects([&] { replica.set<std::int32_t>(value, 3); });
        rejects([&] { replica.removeItem(root, child); });
        rejects([&] { replica.destroyRoot(root); });
        rejects([&] { replica.apply({MemoryOperation::SetValue, MemoryType::I32, {}, value, std::string("invalid")}); });
        assert(replica.get<std::int32_t>(value) == 2 && replica.isReadOnly());
        author.set<std::int32_t>(value, 4); // failed replay leaves no temporary mode to reset
        assert(replica.get<std::int32_t>(value) == 4);
        author.removeItem(root, child);
        rejects([&] { replica.validateContainer(child); });
        author.destroyRoot(root);
        rejects([&] { replica.validateContainer(root); });
    }
    {
        MemoryStore observed;
        auto root = observed.createRoot(); // no listener is also valid
        int events = 0;
        observed.setOnChange([&](MemoryChange change) {
            ++events;
            if (change.operation == MemoryOperation::AllocateValue || change.operation == MemoryOperation::SetValue)
                assert(observed.get<std::string>(change.handle) == std::get<std::string>(change.value));
            if (change.operation == MemoryOperation::RemoveContainer)
                rejects([&] { observed.validateContainer(change.handle); });
        });
        auto field = observed.allocate<std::string>(root, "before");
        observed.set<std::string>(field, "after");
        observed.set<std::string>(field, "after");
        assert(events == 2);
        MemoryStore replica;
        replica.makeReadOnly();
        replica.setOnChange([](MemoryChange) { assert(false); });
        replica.restore(observed.snapshot());
        replica.apply({MemoryOperation::SetValue, MemoryType::String, {}, field, std::string("remote")});
        assert(replica.get<std::string>(field) == "remote");
        observed.setOnChange([](MemoryChange) { throw std::runtime_error("send failed"); });
        rejects([&] { observed.set<std::string>(field, "committed"); });
        assert(observed.get<std::string>(field) == "committed");
        observed.setOnChange([&](MemoryChange c) {
            assert(c.operation == MemoryOperation::RemoveContainer);
            rejects([&] { observed.validateContainer(c.handle); });
            ++events;
        });
        observed.destroyRoot(root);
        assert(events == 3);
        MemoryStore committed;
        const auto parent = committed.createRoot();
        committed.setOnChange([&](MemoryChange change) {
            committed.validateContainer(change.handle);
            assert(committed.containsContainer(parent, change.handle));
            throw std::runtime_error("send failed after creation");
        });
        rejects([&] { committed.createContainer(parent); });
        assert(committed.memberCount(parent, MemoryType::Container) == 1);
        committed.setOnChange({});
        committed.destroyRoot(parent);
    }
    std::vector<MemoryChange> changes;
    auto& store = authoringMemoryStore();
    store.setOnChange([&](MemoryChange change) { changes.push_back(std::move(change)); });
    auto root = store.createRoot();
    auto listHandle = store.createContainer(root);
    examples::StudentList students(store, listHandle);
    auto made = examples::Student::make(store, listHandle, "Factory", 18, 75.0);
    assert(made.name.get() == "Factory" && made.age.get() == 18 && made.grade.get() == 75.0);
    assert(students.at(made.handle()).age.get() == 18);
    students.remove(made.handle());
    rejects([&] { made.age.get(); });
    auto alice = students.add("Alice", 20, 80.0);
    auto bob = students.add("Bob", 30, 90.0);
    const auto aliceHandle = alice.age.handle();
    auto copied = alice;
    const auto beforeBind = changes.size();
    auto bound = students.at(alice.handle());
    assert(changes.size() == beforeBind);
    assert(int(bound.age) == 20 && copied.name.get() == "Alice");
    changes.clear();
    alice.age = bob.age;
    assert(alice.age.handle() == aliceHandle && alice.age.get() == 30);
    assert(changes.size() == 1);
    const auto change = changes.front();
    assert(change.operation == MemoryOperation::SetValue && change.type == MemoryType::I32);
    assert(change.handle == aliceHandle && !change.parent);
    alice.age = 30; assert(changes.size() == 1);
    ++copied.age; assert(alice.age.get() == 31 && bob.age.get() == 30);
    alice.grade += 2.5; assert(alice.grade.get() == 82.5);
    MemoryStore secondStore;
    auto secondRoot = secondStore.createRoot();
    examples::StudentList secondStudents(secondStore, secondRoot);
    auto other = secondStudents.add("Other", 55);
    alice.age = other.age;
    assert(alice.age.get() == 55 && other.age.get() == 55);
    secondStore.makeReadOnly();
    rejects([&] { other.age = 9; });
    rejects([&] { secondStudents.add("Forbidden", 1); });
    rejects([&] { secondStudents.remove(other.handle()); });
    alice.age = other.age; // reading a read-only source remains valid
    assert(other.name.get() == "Other");
    rejects([&] { students.at(root); });
    const auto beforeWrongParent = changes.size();
    rejects([&] { store.removeItem(root, alice.handle()); });
    assert(changes.size() == beforeWrongParent && alice.age.get() == 55);
    assert(store.containsContainer(listHandle, alice.handle()));
    assert(!store.containsContainer(root, alice.handle()));
    // Binding an object does not close an initialization phase: structure can grow.
    auto extra = store.allocate<std::int32_t>(alice.handle(), 7);
    assert(store.get<std::int32_t>(extra) == 7);
    ContainerView aliceContainer(store, alice.handle());
    auto extraObject = aliceContainer.add();
    auto nestedValue = store.allocate<bool>(extraObject, true);
    assert(store.memberAt(alice.handle(), MemoryType::Container, 0) == extraObject);
    aliceContainer.remove(extraObject);
    rejects([&] { store.get<bool>(nestedValue); });
    assert(alice.age.get() == 55); // existing field bindings still work
    changes.clear();
    students.remove(alice.handle());
    assert(students.size() == 1 && students.atIndex(0).handle() == bob.handle());
    assert(changes.size() == 1 && changes.front().parent == listHandle);
    rejects([&] { copied.age = 10; });
    rejects([&] { copied.name.get(); });
    rejects([&] { store.get<std::int32_t>(extra); });
    auto replacement = students.add("Carol", 40);
    assert(replacement.age.handle().slot == extra.slot);
    assert(replacement.age.handle().generation != extra.generation);
    rejects([&] { copied.age.get(); });
    rejects([&] { students.remove(alice.handle()); });
    rejects([&] { students.at(alice.handle()); });
    assert(store.allocatedSlots<std::int32_t>() == 3); // two students plus the extra field

    // Nested objects/lists are all containers.
    auto nestedOwner = store.createContainer(listHandle);
    auto nestedList = store.createContainer(nestedOwner);
    examples::StudentList nested(store, nestedList);
    auto child = nested.add("Child", 1);
    changes.clear();
    students.remove(nestedOwner);
    assert(changes.size() == 1); // descendant frees are implicit
    rejects([&] { child.age.get(); });
    rejects([&] { nested.size(); });
    List<FailingStudent> failing(store, listHandle);
    const auto countBeforeFailure = students.size();
    rejects([&] { failing.add(); });
    assert(students.size() == countBeforeFailure);

    auto primitives = store.createContainer(root);
    auto flag = store.allocate<bool>(primitives, false);
    auto big = store.allocate<std::int64_t>(primitives, 9007199254740993LL);
    auto real = store.allocate<float>(primitives, 1.5f);
    assert(store.get<std::int64_t>(big) == 9007199254740993LL);
    assert(store.set(flag, true)); assert(!store.set(flag, true));
    assert(store.set(real, 2.5f));
    auto nan = store.allocate<double>(primitives, std::numeric_limits<double>::quiet_NaN());
    assert(!store.set(nan, std::numeric_limits<double>::quiet_NaN()));
    auto zero = store.allocate<double>(primitives, 0.0);
    assert(store.set(zero, -0.0));
    auto maximum = store.allocate<std::int32_t>(primitives, (std::numeric_limits<std::int32_t>::max)());
    Field<std::int32_t> checked(store, maximum);
    const auto beforeOverflow = changes.size();
    rejects([&] { ++checked; });
    assert(changes.size() == beforeOverflow && checked.get() == (std::numeric_limits<std::int32_t>::max)());

    store.destroyRoot(root);
    rejects([&] { replacement.grade.get(); });
    rejects([&] { store.get<bool>(flag); });
    changes.clear();
    rejects([&] { store.destroyRoot(root); });
    assert(changes.size() == 0);

    // Small generation ceiling exercises the same retirement branch as UINT32_MAX.
    memory_detail::Pool<int, 2> pool;
    auto first = pool.allocate(1); pool.release(first);
    rejects([&] { pool.get(first); }); // invalid immediately, before slot reuse
    rejects([&] { pool.release(first); }); // no duplicate entry in the free list
    auto second = pool.allocate(2);
    assert(first.slot == second.slot && second.generation == 2);
    rejects([&] { pool.get(first); });
    pool.release(second);
    rejects([&] { pool.get(second); }); // exhausted slot invalidates its last handle
    rejects([&] { pool.get(Handle{second.slot, 0}); });
    rejects([&] { pool.release(second); });
    auto third = pool.allocate(3);
    assert(third.slot != first.slot);
    rejects([&] { pool.get(second); });
    memory_detail::Pool<std::shared_ptr<int>> resources;
    auto resource = std::make_shared<int>(42);
    std::weak_ptr<int> observer = resource;
    auto owned = resources.allocate(std::move(resource));
    assert(!observer.expired());
    resources.release(owned);
    assert(observer.expired()); // default reset releases ownership immediately
    store.setOnChange({});
    std::cout << "PASS: typed pools, reuse/retirement, cross-store views, value assignment, nested ownership and change detection\n";
}
