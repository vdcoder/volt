#pragma once

#include "IRuntime.hpp"
#include "VNodeHandle.hpp"

namespace volt {

// Base app interface
class App {
public:
    App(IRuntime& a_runtime) : m_runtime(a_runtime) {}
    virtual ~App() = default;
    
    void invalidate() { m_runtime.invalidate(); }
    
    // Get runtime pointer to pass child components
    IRuntime& getRuntime() { return m_runtime; }
    
    // Lifecycle: Called once after mount, before first render
    virtual void start() {}
    
    // Render method to be implemented by user's app
    virtual VNodeHandle render() = 0;

private:
    IRuntime& m_runtime;
};

} // namespace volt