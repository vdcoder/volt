#pragma once
#include <DependencyInjection.hpp>
#include <DataConnection.hpp>
#include "services/VoltRuntimeService.hpp"
#ifdef __EMSCRIPTEN__
#include "services/ClientWebsocketService.hpp"
#include "services/HttpClientService.hpp"
#endif

class AppDI : public voltxp::DependencyInjection {
public:
    ~AppDI() override { releaseDependencies(); }

    void registerDependencies(volt::IRuntime& runtime) {
        registerDependency<voltxp::VoltRuntimeService>(
            std::make_unique<voltxp::VoltRuntimeService>(runtime));
#ifdef __EMSCRIPTEN__
        registerDependency<voltxp::HttpClientService>(std::make_unique<voltxp::HttpClientService>());
        registerDependency<voltxp::ClientWebsocketService>(
            std::make_unique<voltxp::ClientWebsocketService>(
                resolveDependency<voltxp::VoltRuntimeService>()));
        registerDependency<voltxp::FrontDataService>(std::make_unique<voltxp::FrontDataService>(voltxp::DataSide::Client));
        registerDependency<voltxp::BackDataService>(std::make_unique<voltxp::BackDataService>(voltxp::DataSide::Client));
        auto& socket = resolveDependency<voltxp::ClientWebsocketService>();
        auto& front = resolveDependency<voltxp::FrontDataService>();
        auto& back = resolveDependency<voltxp::BackDataService>();
        registerDependency<voltxp::DataConnection>(std::make_unique<voltxp::DataConnection>(
            front, back, socket.talkers(),
            [&socket](voltxp::TalkerId id, voltxp::MessageView bytes) { socket.send(id, bytes); },
            [&socket] { socket.fail(); }));
        auto& data = resolveDependency<voltxp::DataConnection>();
        socket.setOnConnection([&data](bool connected) {
            if (connected) data.connected(); else data.disconnected();
        });
        front.store().setOnUpdated([&runtime] { runtime.invalidate(); });
        back.store().setOnUpdated([&runtime] { runtime.invalidate(); });
#endif
    }

    void releaseDependencies() noexcept override {
#ifdef __EMSCRIPTEN__
        if (auto* socket = tryResolveDependency<voltxp::ClientWebsocketService>()) socket->setOnConnection({});
#endif
        // Cancel app-owned callbacks/subscriptions here before releasing services.
        // Keep cleanup safe to repeat and non-throwing, including partial startup.
        DependencyInjection::releaseDependencies();
    }
};
