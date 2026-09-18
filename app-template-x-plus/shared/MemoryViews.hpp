#pragma once
#include "MemoryStore.hpp"

namespace voltxp {
// Runtime store identity keeps fields and object views compatible across stores.
// Stores must outlive every view; reads copy values rather than expose pool memory.
template<class T> class Field {
    MemoryStore* m_store;
    Handle m_handle;
public:
    Field(MemoryStore& store, Handle handle) : m_store(&store), m_handle(handle) { store.validate<T>(handle); }
    Field(const Field&) = default;
    T get() const { return m_store->get<T>(m_handle); }
    operator T() const { return get(); }
    Handle handle() const noexcept { return m_handle; }
    bool set(T value) { return m_store->set<T>(m_handle, std::move(value)); }
    Field& operator=(T value) { set(std::move(value)); return *this; }
    // Assignment copies values, even across stores. Construction binds a view.
    Field& operator=(const Field& other) { return *this = other.get(); }
    Field& operator+=(const T& value) {
        const auto before = get();
        if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            if ((value > 0 && before > (std::numeric_limits<T>::max)() - value) ||
                (value < 0 && before < (std::numeric_limits<T>::min)() - value))
                throw std::overflow_error("Field addition overflow");
        }
        return *this = before + value;
    }
    template<class U = T, std::enable_if_t<std::is_arithmetic_v<U> && !std::is_same_v<U, bool>, int> = 0>
    Field& operator++() { return *this += T(1); }
    template<class U = T, std::enable_if_t<std::is_arithmetic_v<U> && !std::is_same_v<U, bool>, int> = 0>
    T operator++(int) { auto before = get(); ++*this; return before; }
};

class ContainerView {
    MemoryStore* m_store;
    Handle m_handle;
public:
    ContainerView(MemoryStore& store, Handle handle) : m_store(&store), m_handle(handle) { store.validateContainer(handle); }
    ContainerView(const ContainerView&) = default;
    ContainerView& operator=(const ContainerView&) = delete;
    Handle handle() const noexcept { return m_handle; }
    MemoryStore& store() const noexcept { return *m_store; }
    template<class T> Field<T> field(std::size_t index) const {
        return Field<T>(*m_store, m_store->field<T>(m_handle, index));
    }
    std::size_t size() const { return m_store->memberCount(m_handle, MemoryType::Container); }
    Handle atIndex(std::size_t index) const { return m_store->memberAt(m_handle, MemoryType::Container, index); }
    Handle add() { return m_store->createContainer(m_handle); }
    void remove(Handle item) { m_store->removeItem(m_handle, item); }

};

// Optional typed facade over a container view, not a separate storage type.
// Object/list meaning belongs to the application, not the storage layer.
template<class View> class List {
    ContainerView m_container;
public:
    List(MemoryStore& store, Handle handle) : m_container(store, handle) {}
    Handle handle() const noexcept { return m_container.handle(); }
    std::size_t size() const { return m_container.size(); }
    View at(Handle item) const {
        if (!m_container.store().containsContainer(handle(), item)) throw std::invalid_argument("Item belongs to another container");
        return View(m_container.store(), item);
    }
    View atIndex(std::size_t index) const { return at(m_container.atIndex(index)); }
    template<class... Args> View add(Args&&... args) {
        auto& store = m_container.store();
        auto item = m_container.add();
        try {
            View::initialize(store, item, std::forward<Args>(args)...);
            return View(store, item);
        } catch (...) {
            store.removeItem(handle(), item);
            throw;
        }
    }
    void remove(Handle item) { m_container.remove(item); }
};
static_assert(sizeof(Field<std::int32_t>) == sizeof(void*) + sizeof(Handle), "Field contains a store pointer and handle");
static_assert(sizeof(ContainerView) == sizeof(void*) + sizeof(Handle), "Container view contains a store pointer and handle");
} // namespace voltxp
