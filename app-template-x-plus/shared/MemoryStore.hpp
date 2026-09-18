#pragma once
#include "MemoryChanges.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace voltxp {

namespace memory_detail {

template<class T> constexpr MemoryType type() {
    if constexpr (std::is_same_v<T, bool>) return MemoryType::Bool;
    else if constexpr (std::is_same_v<T, std::int32_t>) return MemoryType::I32;
    else if constexpr (std::is_same_v<T, std::int64_t>) return MemoryType::I64;
    else if constexpr (std::is_same_v<T, float>) return MemoryType::F32;
    else if constexpr (std::is_same_v<T, double>) return MemoryType::F64;
    else { static_assert(std::is_same_v<T, std::string>, "Unsupported memory scalar"); return MemoryType::String; }
}

template<class T> bool equal(const T& a, const T& b) {
    if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(a) && std::isnan(b)) return true;
        if (a == 0 && b == 0) return std::signbit(a) == std::signbit(b);
    }
    return a == b;
}

// Free links live in vacant slots, so recursive release does not allocate.
template<class T, std::uint32_t MaxGeneration = (std::numeric_limits<std::uint32_t>::max)()> class Pool {
    static_assert(MaxGeneration > 0, "Generation zero is reserved");
    static constexpr auto none = (std::numeric_limits<std::uint32_t>::max)();
    struct Slot {
        T value{};
        std::uint32_t generation = 1;
        std::uint32_t next = none;
        bool occupied = false; // Wire slots have no generation to establish liveness.
    };
    std::vector<Slot> m_slots;
    std::uint32_t m_free = none;
public:
    Handle nextHandle() const {
        if (m_free != none) return {m_free, m_slots[m_free].generation};
        if (m_slots.size() >= none) throw std::length_error("Memory pool exhausted");
        return {static_cast<std::uint32_t>(m_slots.size()), 1};
    }
    Handle allocate(T value) {
        if (m_free != none) {
            auto index = m_free;
            auto& slot = m_slots[index];
            slot.value = std::move(value);
            m_free = slot.next;
            slot.occupied = true;
            return {index, slot.generation};
        }
        if (m_slots.size() >= none) throw std::length_error("Memory pool exhausted");
        auto index = static_cast<std::uint32_t>(m_slots.size());
        m_slots.push_back(Slot{std::move(value), 1, none, true});
        return {index, 1};
    }
    T& get(Handle handle) {
        return const_cast<T&>(std::as_const(*this).get(handle));
    }
    const T& get(Handle handle) const {
        if (handle.slot >= m_slots.size()) throw std::out_of_range("Invalid memory handle");
        const auto& slot = m_slots[handle.slot];
        if (!slot.occupied || handle.generation == 0 || slot.generation != handle.generation)
            throw std::out_of_range("Stale memory handle");
        return slot.value;
    }
    void release(Handle handle) {
        get(handle);
        auto& slot = m_slots[handle.slot];
        // Swapping with a fresh default releases string/container allocations,
        // rather than merely clearing their contents while retaining capacity.
        {
            T empty{};
            using std::swap;
            swap(slot.value, empty);
        }
        slot.occupied = false;
        if (slot.generation != MaxGeneration) {
            ++slot.generation;
            slot.next = m_free;
            m_free = handle.slot;
        } else {
            slot.generation = 0; // Retired: invalidate the final issued handle.
        }
    }
    Handle localHandle(std::uint32_t index) const {
        if (index >= m_slots.size() || !m_slots[index].occupied)
            throw std::out_of_range("Slot is not occupied");
        return {index, m_slots[index].generation};
    }
    // Replica installation uses local generations, never the author's free list.
    Handle install(std::uint32_t index, T value) {
        constexpr std::uint32_t slotLimit = 1u << 20;
        if (index >= slotLimit) throw std::length_error("Replica slot limit exceeded");
        if (index >= m_slots.size()) m_slots.resize(static_cast<std::size_t>(index) + 1);
        auto& slot = m_slots[index];
        if (slot.occupied || !slot.generation) throw std::logic_error("Slot occupied or retired");
        slot.value = std::move(value);
        slot.occupied = true;
        return {index, slot.generation};
    }
    void reset() {
        for (std::size_t i = m_slots.size(); i-- > 0;)
            if (m_slots[i].occupied) release({static_cast<std::uint32_t>(i), m_slots[i].generation});
        // Rebuild author allocation links; replica installation ignores them.
        m_free = none;
        for (std::size_t i = m_slots.size(); i-- > 0;) {
            auto& slot = m_slots[i];
            if (slot.generation) { slot.next = m_free; m_free = static_cast<std::uint32_t>(i); }
        }
    }
    std::size_t capacity() const noexcept { return m_slots.size(); }
};

using Membership = std::array<std::vector<Handle>, 7>;
struct Container {
    Membership members;
};

} // namespace memory_detail

class MemoryStore {
    using Container = memory_detail::Container;
    template<class T> using Pool = memory_detail::Pool<T>;
    using ValuePools = std::tuple<Pool<bool>, Pool<std::int32_t>, Pool<std::int64_t>, Pool<float>, Pool<double>, Pool<std::string>>;
    ValuePools m_values;
    Pool<Container> m_containers;
    std::function<void(MemoryChange)> m_onChange;
    std::function<void()> m_onUpdated;

    template<class T> auto& value_pool() { return std::get<Pool<T>>(m_values); }
    template<class T> const auto& value_pool() const { return std::get<Pool<T>>(m_values); }

    static auto& members(Container& node, MemoryType type) { return node.members.at(static_cast<std::size_t>(type)); }
    static const auto& members(const Container& node, MemoryType type) { return node.members.at(static_cast<std::size_t>(type)); }

    bool m_replica = false;
    void record(MemoryChange change) {
        if (m_replica) throw std::logic_error("Cannot modify replica");
        if (m_onChange) m_onChange(std::move(change));
        if (m_onUpdated) m_onUpdated();
    }
    void requireWritable() const {
        if (m_replica) throw std::logic_error("Memory store is read-only");
    }

    // Silent storage operations shared by public author writes and replica apply.
    Handle createContainerImpl(Handle parent) {
        if (parent) validateContainer(parent);
        auto handle = m_containers.allocate(Container{});
        bool linked = false;
        try {
            if (parent) { members(m_containers.get(parent), MemoryType::Container).push_back(handle); linked = true; }
        } catch (...) {
            if (linked) members(m_containers.get(parent), MemoryType::Container).pop_back();
            m_containers.release(handle);
            throw;
        }
        return handle;
    }
    template<class T> Handle allocateImpl(Handle parent, T initial = {}) {
        auto& fields = members(m_containers.get(parent), memory_detail::type<T>());
        auto handle = value_pool<T>().allocate(std::move(initial));
        bool linked = false;
        try {
            fields.push_back(handle); linked = true;
        } catch (...) {
            if (linked) fields.pop_back();
            value_pool<T>().release(handle);
            throw;
        }
        return handle;
    }
    template<class T> bool setImpl(Handle handle, T value) {
        auto& current = value_pool<T>().get(handle);
        if (memory_detail::equal(current, value)) return false;
        current = std::move(value);
        return true;
    }
    void removeItemImpl(Handle list, Handle child) {
        validateContainer(child);
        auto& owner = m_containers.get(list);
        auto found = std::find(members(owner, MemoryType::Container).begin(), members(owner, MemoryType::Container).end(), child);
        if (found == members(owner, MemoryType::Container).end()) throw std::invalid_argument("Item does not belong to this container");
        members(owner, MemoryType::Container).erase(found);
        releaseTree(child);
    }

    template<class T> void applyScalar(const MemoryChange& change) {
        const auto& value = std::get<T>(change.value);
        if (change.operation == MemoryOperation::AllocateValue) {
            auto& fields = members(m_containers.get(change.parent), memory_detail::type<T>());
            auto handle = value_pool<T>().install(change.handle.slot, value);
            try { fields.push_back(handle); }
            catch (...) { value_pool<T>().release(handle); throw; }
        } else if (change.operation == MemoryOperation::SetValue) {
            if (change.parent) throw std::invalid_argument("SetValue must not contain a parent");
            setImpl<T>(change.handle, value);
        } else throw std::invalid_argument("Invalid scalar operation");
    }

    template<class T> void releaseValues(const Container& node) {
        for (auto handle : members(node, memory_detail::type<T>())) value_pool<T>().release(handle);
    }
    void releaseTree(Handle handle) {
        auto node = std::move(m_containers.get(handle));
        for (auto member : members(node, MemoryType::Container)) releaseTree(member);
        releaseValues<bool>(node); releaseValues<std::int32_t>(node); releaseValues<std::int64_t>(node);
        releaseValues<float>(node); releaseValues<double>(node); releaseValues<std::string>(node);
        m_containers.release(handle);
    }
public:
    // In-memory snapshot, deliberately opaque until a wire encoding is chosen.
    // Copies vacant/retired slot generations and free-list order as well as data.
    class Snapshot {
        friend class MemoryStore;
        ValuePools values;
        Pool<Container> containers;
    public:
        Snapshot() = default;
    };
    MemoryStore() = default;
    // Called synchronously after mutation. Reads are allowed; do not mutate this
    // store or replace the handler from inside it. Exceptions propagate without rollback.
    void setOnChange(std::function<void(MemoryChange)> onChange) {
        m_onChange = std::move(onChange);
    }
    MemoryStore(const MemoryStore&) = delete;
    MemoryStore& operator=(const MemoryStore&) = delete;
    Snapshot snapshot() const {
        Snapshot result;
        result.values = m_values;
        result.containers = m_containers;
        return result;
    }
    // Explicit replication entry points. Normal application writes remain denied.
    // Restoring replaces contents during full synchronization; rebind all views.
    // Authors can also restore the server's accepted state after reconnecting.
    void restore(const Snapshot& snapshot) {
        auto values = snapshot.values;
        auto containers = snapshot.containers;
        using std::swap;
        swap(m_values, values);
        swap(m_containers, containers);
    }
    void apply(const MemoryChange& incoming) {
        auto change = incoming;
        if (change.parent) change.parent = m_containers.localHandle(change.parent.slot);
        if (change.operation == MemoryOperation::SetValue || change.operation == MemoryOperation::RemoveContainer)
            change.handle = localHandle(change.type, change.handle.slot);
        if (!m_replica) throw std::logic_error("Replay requires a replica store");
        if (change.type == MemoryType::Container) {
            if (!std::holds_alternative<std::monostate>(change.value))
                throw std::invalid_argument("Container operation has a scalar payload");
            if (change.operation == MemoryOperation::AllocateContainer) {
                if (change.parent) validateContainer(change.parent);
                auto handle = m_containers.install(change.handle.slot, Container{});
                try {
                    if (change.parent) members(m_containers.get(change.parent), MemoryType::Container).push_back(handle);
                } catch (...) { m_containers.release(handle); throw; }
            } else if (change.operation == MemoryOperation::RemoveContainer) {
                if (change.parent) removeItemImpl(change.parent, change.handle);
                else releaseTree(change.handle);
            } else throw std::invalid_argument("Invalid container operation");
        } else {
            switch (change.type) {
            case MemoryType::Bool: applyScalar<bool>(change); break;
            case MemoryType::I32: applyScalar<std::int32_t>(change); break;
            case MemoryType::I64: applyScalar<std::int64_t>(change); break;
            case MemoryType::F32: applyScalar<float>(change); break;
            case MemoryType::F64: applyScalar<double>(change); break;
            case MemoryType::String: applyScalar<std::string>(change); break;
            default: throw std::invalid_argument("Unknown memory type");
            }
        }
        if (m_onUpdated) m_onUpdated();
    }
    Handle localHandle(MemoryType type, std::uint32_t slot) const {
        switch (type) {
        case MemoryType::Bool: return value_pool<bool>().localHandle(slot);
        case MemoryType::I32: return value_pool<std::int32_t>().localHandle(slot);
        case MemoryType::I64: return value_pool<std::int64_t>().localHandle(slot);
        case MemoryType::F32: return value_pool<float>().localHandle(slot);
        case MemoryType::F64: return value_pool<double>().localHandle(slot);
        case MemoryType::String: return value_pool<std::string>().localHandle(slot);
        case MemoryType::Container: return m_containers.localHandle(slot);
        }
        throw std::invalid_argument("Unknown memory type");
    }
    Handle root() const { return m_containers.localHandle(0); }
    void setOnUpdated(std::function<void()> handler) { m_onUpdated = std::move(handler); }
    // Keep generation history so views from an earlier connection stay stale.
    void reset() {
        std::apply([](auto&... pools) { (pools.reset(), ...); }, m_values);
        m_containers.reset();
        if (m_replica) m_containers.install(0, Container{});
        else if (m_containers.allocate(Container{}).slot != 0)
            throw std::overflow_error("Root generation exhausted; replace the store");
        if (m_onUpdated) m_onUpdated();
    }
    Handle createRoot() {
        return createContainer({});
    }
    Handle createContainer(Handle parent) {
        requireWritable();
        auto handle = createContainerImpl(parent);
        record({MemoryOperation::AllocateContainer, MemoryType::Container, parent, handle});
        return handle;
    }
    template<class T> Handle allocate(Handle parent, T initial = {}) {
        requireWritable();
        // Copy a potentially allocating payload before committing the store change.
        MemoryChange change{MemoryOperation::AllocateValue, memory_detail::type<T>(), parent, {}, initial};
        auto handle = allocateImpl<T>(parent, std::move(initial));
        change.handle = handle;
        record(std::move(change));
        return handle;
    }
    template<class T> void validate(Handle handle) const { value_pool<T>().get(handle); }
    template<class T> T get(Handle handle) const { return value_pool<T>().get(handle); }
    template<class T> bool set(Handle handle, T value) {
        requireWritable();
        MemoryChange change{MemoryOperation::SetValue, memory_detail::type<T>(), {}, handle, value};
        if (!setImpl<T>(handle, std::move(value))) return false;
        record(std::move(change));
        return true;
    }
    template<class T> Handle field(Handle handle, std::size_t index) const {
        return memberAt(handle, memory_detail::type<T>(), index);
    }
    void makeReadOnly() noexcept { m_replica = true; }
    bool isReadOnly() const noexcept { return m_replica; }
    void validateContainer(Handle handle) const { m_containers.get(handle); }
    bool containsContainer(Handle parent, Handle child) const {
        const auto& values = members(m_containers.get(parent), MemoryType::Container);
        return std::find(values.begin(), values.end(), child) != values.end();
    }
    std::size_t memberCount(Handle handle, MemoryType type) const { return members(m_containers.get(handle), type).size(); }
    Handle memberAt(Handle handle, MemoryType type, std::size_t index) const {
        return members(m_containers.get(handle), type).at(index);
    }
    // The list knows the parent. Lookup/ordered erase is O(n), plus subtree release.
    void removeItem(Handle list, Handle child) {
        requireWritable();
        removeItemImpl(list, child);
        record({MemoryOperation::RemoveContainer, MemoryType::Container, list, child});
    }
    // Caller-owned root only: no ancestry metadata is retained to verify this.
    // Nested containers must be removed through their parent with removeItem().
    void destroyRoot(Handle root) {
        requireWritable();
        validateContainer(root);
        releaseTree(root);
        record({MemoryOperation::RemoveContainer, MemoryType::Container, {}, root});
    }
    template<class T> std::size_t allocatedSlots() const noexcept { return value_pool<T>().capacity(); }
};

inline MemoryStore& authoringMemoryStore() { static MemoryStore store; return store; }
} // namespace voltxp
