#pragma once
#include <DependencyInjection.hpp>

class AppDI : public voltxp::DependencyInjection {
public:
    ~AppDI() override { releaseDependencies(); }

    void registerDependencies() {
        // Register server services here, dependencies before their consumers.
    }

    void releaseDependencies() noexcept override {
        // Stop app-owned background work here before releasing services.
        // Keep cleanup safe to repeat and non-throwing, including partial startup.
        DependencyInjection::releaseDependencies();
    }
};
