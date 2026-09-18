#pragma once
#include "services/HttpServerService.hpp"

// App-owned HTTP endpoints. No session identity is required.
inline void registerHttpRoutes(voltxp::HttpServerService& http) {
    using Method = voltxp::HttpServerService::Method;
    http.route(Method::Get, "/api/hello", [](const seasocks::Request&) {
        return voltxp::HttpServerService::reply(seasocks::ResponseCode::Ok,
            R"({"message":"Hello from Volt X+!"})", "application/json");
    });
    http.route(Method::Post, "/api/echo", [](const seasocks::Request& request) {
        seasocks::ResponseBuilder response;
        response.withContentType("application/octet-stream");
        if (request.contentLength())
            response << std::string(reinterpret_cast<const char*>(request.content()), request.contentLength());
        return response.build();
    });
}
