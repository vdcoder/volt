#pragma once

namespace volt {

// ============================================================================
// IVoltRuntime - Public interface for components to use and share with others
// ============================================================================

class IVoltRuntime {
public:
    virtual void requestRender() = 0;
    virtual ~IVoltRuntime() = default;
};

}