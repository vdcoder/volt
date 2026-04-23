#pragma once
#include <boost/asio/io_context.hpp>
#include <cstdint>
#include <string>

// Starts HTTP + WS server on same port.
// - staticRoot: directory to serve (your repo's ./static)
// - sid: pairing id; WS requires ?sid=<sid> (optional but recommended)
void StartVoltRemoteServer(
    boost::asio::io_context& ioc,
    std::uint16_t port,
    std::string staticRoot,
    std::string sid
);
