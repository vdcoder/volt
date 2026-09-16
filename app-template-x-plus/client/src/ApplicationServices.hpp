#pragma once
#include <Volt.hpp>
#include "AppDI.hpp"

namespace app_detail {
struct Application {
    // Members are destroyed in reverse order: services can still use the engine
    // during teardown. One engine is supported per Emscripten module instance.
    std::unique_ptr<volt::VoltEngine> engine;
    AppDI dependencies;
};

inline Application& application() {
    static Application instance;
    return instance;
}
} // namespace app_detail

inline AppDI& services() { return app_detail::application().dependencies; }

// Safe outside rendering, including callbacks belonging to this module.
inline void invalidate() {
    services().resolveDependency<voltxp::VoltRuntimeService>().getRuntime().invalidate();
}
