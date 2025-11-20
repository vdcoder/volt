#pragma once

#include "VNodeHandle.hpp"

namespace volt {

class VoltRuntime;
class VNode;

// Base app interface
class AppBase {
public:
    AppBase(VoltRuntime* a_pRuntime) : m_pRuntime(a_pRuntime) {}
    virtual ~AppBase() = default;
    
    void requestRender();
    
    // Get runtime pointer to pass child components
    VoltRuntime* getRuntime() { return m_pRuntime; }
    
    // Lifecycle: Called once after mount, before first render
    virtual void start() {}
    
    // Render method to be implemented by user's app
    virtual VNodeHandle render() = 0;

private:
    VoltRuntime* m_pRuntime;
};

} // namespace volt