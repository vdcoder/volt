#pragma once

#include "IRuntime.hpp"

namespace volt {

class VNode;

// Base component interface
class Component {
public:
    Component(IRuntime& a_runtime) : m_runtime(a_runtime) {}
    virtual ~Component() = default;

    void invalidate() { m_runtime.invalidate(); }
    
    // Get runtime pointer to pass child components
    IRuntime& getRuntime() { return m_runtime; }

private:
    IRuntime& m_runtime;
};

} // namespace volt