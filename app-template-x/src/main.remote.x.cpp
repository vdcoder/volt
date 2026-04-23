#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <filesystem>
#include <string>

#include <VoltRemServer.hpp>

namespace fs = std::filesystem;

#ifndef VOLT_SRC_DIR
#define VOLT_SRC_DIR "."
#endif

static fs::path EnvStaticRoot() {
    if (const char* v = std::getenv("VOLT_STATIC_ROOT")) {
        return fs::path(v);
    }
    return {};
}

static fs::path SourceStaticRoot() {
    return fs::path(VOLT_SRC_DIR) / "static";
}

static fs::path GetExeDir(char* argv0) {
    // argv0 can be relative; canonicalize what we can.
    fs::path p = argv0 ? fs::path(argv0) : fs::path();
    std::error_code ec;

    // If argv0 is relative, make it absolute using current_path().
    if (!p.empty() && !p.is_absolute()) {
        p = fs::absolute(p, ec);
    }

    // weakly_canonical handles missing segments better than canonical.
    p = fs::weakly_canonical(p, ec);

    // If it still isn't usable, fall back to current_path().
    if (p.empty() || ec) {
        return fs::current_path();
    }

    return p.parent_path();
}

static fs::path PickStaticRoot(char* argv0) {
    fs::path exeDir = GetExeDir(argv0);

    // Try a few reasonable layouts. Adjust if your exe lands elsewhere.
    fs::path candidates[] = {
        EnvStaticRoot(),
        exeDir / "static",
        exeDir / ".." / "static",
        exeDir / ".." / ".." / "static",
        SourceStaticRoot(),
        fs::current_path() / "static",
    };

    for (auto& c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && fs::is_directory(c, ec)) {
            return fs::weakly_canonical(c, ec);
        }
    }

    // Last resort: return a default (will fail later with a clear message)
    return fs::current_path() / "static";
}

static std::string MakeSid() {
    // good enough for dev; you can swap in a stronger generator later
    return "demo123";
}

int main(int argc, char** argv) {
    std::cout << "Volt remote starting...\n";

    //std::string staticRoot = "./static";
    //if (argc >= 2) {
    //    staticRoot = argv[1]; // e.g. pass absolute path from VS run config
    //}

    std::string sid = MakeSid();
    boost::asio::io_context ioc{ 1 };

    auto staticRoot = PickStaticRoot(argv[0]).string();
    StartVoltRemoteServer(ioc, 8787, staticRoot, sid);

    std::thread netThread([&] { ioc.run(); });

    std::cout << "Press Enter to quit.\n";
    std::cin.get();

    ioc.stop();
    netThread.join();
    return 0;
}
