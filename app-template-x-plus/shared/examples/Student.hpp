#pragma once
#include <MemoryViews.hpp>

namespace voltxp::examples {
// One Student type for all stores. Copy construction binds another view;
// whole-object assignment is disabled to avoid mixing rebinding and value writes.
class Student {
    ContainerView m_container;

    // Per-type index sequencing (ordered by MemoryType)
    static constexpr size_t Age     = 0; // int32_t
    static constexpr size_t Grade   = 0; // double
    static constexpr size_t Name    = 0; // string
public:
    Field<std::string> name;
    Field<std::int32_t> age;
    Field<double> grade;

    Student(MemoryStore& store, Handle handle)
        : m_container(store, handle),
        name(m_container.field<std::string>(Name)),
        age(m_container.field<std::int32_t>(Age)),
        grade(m_container.field<double>(Grade))
    {}
    Student(const Student&) = default;
    Student& operator=(const Student&) = delete;
    Handle handle() const noexcept { return m_container.handle(); }

    // Make a new Student in the store.
    static Student make(MemoryStore& store, Handle parent_handle, std::string name,
                           std::int32_t age = 0, double grade = 0) {
        const auto handle = store.createContainer(parent_handle);
        try {
            initialize(store, handle, std::move(name), age, grade);
            return Student(store, handle);
        } catch (...) {
            store.removeItem(parent_handle, handle);
            throw;
        }
    }

    // Only creation allocates. Binding an existing Student never allocates.
    static void initialize(MemoryStore& store, Handle handle, std::string name,
                           std::int32_t age = 0, double grade = 0) {
        store.allocate<std::string>(handle, std::move(name));
        store.allocate<std::int32_t>(handle, age);
        store.allocate<double>(handle, grade);
    }
};
using StudentList = List<Student>;
static_assert(sizeof(Student) == 4 * (sizeof(void*) + sizeof(Handle)), "One container view and three fields");
} // namespace voltxp::examples
