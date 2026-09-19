#pragma once
#include <DependencyInjection.hpp>
#include "services/HttpServerService.hpp"
#include "controllers/ExampleController.hpp"
#include "sessions/SessionRegistry.hpp"
#include "sessions/Session.hpp"

class AppDI : public voltxp::DependencyInjection {
public:
    ~AppDI() override { releaseDependencies(); }

    void registerDependencies() {
        registerDependency<voltxp::HttpServerService>(std::make_unique<voltxp::HttpServerService>());
        registerDependency<SessionRegistry<Session>>(std::make_unique<SessionRegistry<Session>>());
        registerDependency<ExampleController>(std::make_unique<ExampleController>(
            resolveDependency<voltxp::HttpServerService>()));
    }

    void releaseDependencies() noexcept override {
        // Stop app-owned background work here before releasing services.
        // Keep cleanup safe to repeat and non-throwing, including partial startup.
        DependencyInjection::releaseDependencies();
    }
};
