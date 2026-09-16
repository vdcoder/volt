#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace voltxp {

// Register dependencies before their consumers. Registration/release must not
// race with resolution; returned references remain valid until release.
class DependencyInjection {
public:
    DependencyInjection() = default;
    DependencyInjection(const DependencyInjection&) = delete;
    DependencyInjection& operator=(const DependencyInjection&) = delete;
    DependencyInjection(DependencyInjection&&) = delete;
    DependencyInjection& operator=(DependencyInjection&&) = delete;
    virtual ~DependencyInjection() { DependencyInjection::releaseDependencies(); }

    template<class T>
    void registerDependency(std::unique_ptr<T> dependency) {
        if (m_releasing)
            throw std::logic_error("Cannot register dependencies during release");
        if (!dependency)
            throw std::invalid_argument("Cannot register a null dependency");
        const auto key = std::type_index(typeid(T));
        if (m_dependencies.find(key) != m_dependencies.end())
            throw std::logic_error("Dependency already registered");

        // If either allocation fails, ownership and order remain consistent.
        m_registrationOrder.push_back(key);
        try {
            m_dependencies.emplace(key, VoidPtr(dependency.release(),
                [](void* value) { delete static_cast<T*>(value); }));
        } catch (...) {
            m_registrationOrder.pop_back();
            throw;
        }
    }

    template<class T>
    T& resolveDependency() const {
        const auto found = m_dependencies.find(std::type_index(typeid(T)));
        if (found == m_dependencies.end())
            throw std::runtime_error("Dependency not found");
        return *static_cast<T*>(found->second.get());
    }

    // Derived destructors must call their own override while derived state is
    // still alive. The base destructor can only perform this base cleanup.
    virtual void releaseDependencies() noexcept {
        if (m_releasing) return;
        m_releasing = true;
        while (!m_registrationOrder.empty()) {
            const auto key = m_registrationOrder.back();
            m_registrationOrder.pop_back();
            // Detach before destruction: a consumer's destructor may resolve
            // an earlier dependency or reenter base release safely.
            auto entry = m_dependencies.extract(key);
        }
        m_releasing = false;
    }

private:
    using VoidPtr = std::unique_ptr<void, void(*)(void*)>;
    std::unordered_map<std::type_index, VoidPtr> m_dependencies;
    std::vector<std::type_index> m_registrationOrder;
    bool m_releasing = false;
};

} // namespace voltxp
