#include <seasocks/PrintfLogger.h>
#include <seasocks/Server.h>
#include <seasocks/WebSocket.h>
#include "ApplicationServices.hpp"
#include "network/SessionHandler.hpp"

#include <atomic>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
namespace fs = std::filesystem;

struct Options {
    fs::path staticRoot;
    int port = 8000;
};

fs::path defaultStaticRoot()
{
    wchar_t modulePath[32768];
    const DWORD length = GetModuleFileNameW(nullptr, modulePath, 32768);
    if (length == 0 || length == 32768)
        throw std::runtime_error("Cannot locate the executable");

    const auto directory = fs::path(modulePath).parent_path();
    // output/Debug/server/Server.exe serves output/Debug/web (likewise Release).
    return directory.parent_path() / "web";
}

int parsePort(std::string_view value)
{
    int port = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), port);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size()
        || port < 1 || port > 65535)
        throw std::runtime_error("Port must be an integer from 1 to 65535");
    return port;
}

Options parseOptions(int argc, char* argv[])
{
    if (argc > 3)
        throw std::runtime_error("Usage: Server.exe [static-folder] [port]");

    Options options;
    options.staticRoot = fs::absolute(argc > 1 ? fs::path(argv[1]) : defaultStaticRoot());
    if (argc > 2)
        options.port = parsePort(argv[2]);
    if (!fs::is_regular_file(options.staticRoot / "index.html"))
        throw std::runtime_error("Static folder has no index.html. Build Client first: "
            + options.staticRoot.string());
    return options;
}

std::atomic<bool> stopping{false};

BOOL WINAPI consoleControl(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT)
        return FALSE;
    stopping.store(true);
    return TRUE;
}

class EchoHandler final : public seasocks::WebSocket::Handler {
public:
    void onConnect(seasocks::WebSocket*) override
    {
        std::cout << "WebSocket connected\n";
    }

    void onData(seasocks::WebSocket* socket, const char* message) override
    {
        socket->send(message);
    }

    void onData(seasocks::WebSocket* socket, const uint8_t* data, size_t size) override
    {
        socket->send(data, size);
    }

    void onDisconnect(seasocks::WebSocket*) override
    {
        std::cout << "WebSocket disconnected\n";
    }
};
}

int main(int argc, char* argv[])
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    try {
        const auto options = parseOptions(argc, argv);
        services().registerDependencies();
        auto logger = std::make_shared<seasocks::PrintfLogger>(seasocks::Logger::Level::Info);
        seasocks::Server server(logger);
        auto& http = services().resolveDependency<voltxp::HttpServerService>();
        server.addPageHandler(http.handler());
        server.addWebSocketHandler("/ws", std::make_shared<EchoHandler>());
        auto sessions = std::make_shared<SessionHandler>(services().resolveDependency<SessionRegistry<Session>>());
        server.addWebSocketHandler("/session", sessions);
        server.setStaticPath(options.staticRoot.string().c_str());
        if (!server.startListening(INADDR_LOOPBACK, options.port)) {
            std::cerr << "Cannot listen on port " << options.port << ". Is another server running?\n";
            return 1;
        }
        SetConsoleCtrlHandler(consoleControl, TRUE);
        std::cout << "Serving " << options.staticRoot << "\nHTTP: http://127.0.0.1:" << options.port
                  << "/\nWebSocket echo: ws://127.0.0.1:" << options.port << "/ws\nCtrl+C to stop.\n";
        while (!stopping.load()) {
            sessions->poll();
            const auto result = server.poll(100);
            if (result == seasocks::Server::PollResult::Error)
                return 1;
            if (result == seasocks::Server::PollResult::Terminated)
                break;
        }
        SetConsoleCtrlHandler(consoleControl, FALSE);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
