#pragma once
#include <DependencyInjection.hpp>
#include <DataConnection.hpp>
#include "services/SessionWebsocketService.hpp"

class SessionDI : public voltxp::DependencyInjection {
public:
    ~SessionDI() override { releaseDependencies(); }
    void releaseDependencies() noexcept override {
        if (auto* socket = tryResolveDependency<voltxp::SessionWebsocketService>()) socket->setOnConnection({});
        DependencyInjection::releaseDependencies();
    }
    void registerDependencies() {
        registerDependency<voltxp::SessionWebsocketService>(
            std::make_unique<voltxp::SessionWebsocketService>());
        registerDependency<voltxp::FrontDataService>(std::make_unique<voltxp::FrontDataService>(voltxp::DataSide::Server));
        registerDependency<voltxp::BackDataService>(std::make_unique<voltxp::BackDataService>(voltxp::DataSide::Server));
        auto& socket = resolveDependency<voltxp::SessionWebsocketService>();
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
        // Register per-session data/application services here, providers first.
    }
};
