#pragma once
#include <DependencyInjection.hpp>
#include "services/VoltRuntimeService.hpp"

class AppDI : public voltxp::DependencyInjection {
public:
    ~AppDI() override { releaseDependencies(); }

    void registerDependencies(volt::IRuntime& runtime) {
        registerDependency<voltxp::VoltRuntimeService>(
            std::make_unique<voltxp::VoltRuntimeService>(runtime));
        // Register services consuming VoltRuntimeService after it.
    }

    void releaseDependencies() noexcept override {
        // Cancel app-owned callbacks/subscriptions here before releasing services.
        // Keep cleanup safe to repeat and non-throwing, including partial startup.
        DependencyInjection::releaseDependencies();
    }
};
