#pragma once

namespace volt {

// ============================================================================
// IRuntime - Public interface for components to use and share with others
// ============================================================================

class IRuntime {
public:
    virtual ~IRuntime() = default;

    virtual void invalidate() = 0;
};

}