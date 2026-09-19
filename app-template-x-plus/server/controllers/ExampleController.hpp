#pragma once
#include "../services/HttpServerService.hpp"

// Register this service after its providers. Its member owns and removes its routes.
class ExampleController {
    voltxp::HttpController m_routes;
public:
    explicit ExampleController(voltxp::HttpServerService& http)
        : m_routes(http.controller("/api")) {
        m_routes.get("/hello", [](const seasocks::Request&) {
            return voltxp::HttpServerService::reply(seasocks::ResponseCode::Ok,
                R"({"message":"Hello from Volt X+!"})", "application/json");
        });
        m_routes.post("/echo", [](const seasocks::Request& request) {
            std::string body;
            if (request.contentLength())
                body.assign(reinterpret_cast<const char*>(request.content()), request.contentLength());
            return voltxp::HttpServerService::reply(seasocks::ResponseCode::Ok,
                std::move(body), "application/octet-stream");
        });
    }
};
