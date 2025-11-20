#pragma once

namespace volt {

class IVoltRuntime;
class VNode;

// Base component interface
class ComponentBase {
public:
    ComponentBase(IVoltRuntime* a_pRuntime) : m_pRuntime(a_pRuntime) {}
    virtual ~ComponentBase() = default;

    void requestRender();
    
    // Get runtime pointer to pass child components
    IVoltRuntime* getRuntime() { return m_pRuntime; }

private:
    IVoltRuntime* m_pRuntime;
};

} // namespace volt