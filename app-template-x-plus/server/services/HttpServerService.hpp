#pragma once
#include <seasocks/PageHandler.h>
#include <seasocks/Request.h>
#include <seasocks/ResponseBuilder.h>
#include <seasocks/ResponseWriter.h>
#include <functional>
#include <cctype>
#include <map>
#include <stdexcept>
#include <utility>

namespace voltxp {
// Synchronous handlers run on the Seasocks event loop. Request references are
// borrowed for the call; no session cookie or WebSocket connection is required.
class HttpServerService {
public:
    using Method = seasocks::Request::Verb;
    using Handler = std::function<std::shared_ptr<seasocks::Response>(const seasocks::Request&)>;
private:
    // Seasocks ResponseBuilder substitutes an HTML page for non-2xx responses.
    // Write directly to preserve API bodies and headers at every status.
    class Reply final : public seasocks::Response {
        seasocks::ResponseCode m_status;
        std::string m_body, m_type;
        std::map<std::string, std::string> m_headers;
    public:
        Reply(seasocks::ResponseCode status, std::string body, std::string type,
              std::map<std::string, std::string> headers)
            : m_status(status), m_body(std::move(body)), m_type(std::move(type)), m_headers(std::move(headers)) {}
        void handle(std::shared_ptr<seasocks::ResponseWriter> writer) override {
            writer->begin(m_status);
            writer->header("Content-Type", m_type);
            writer->header("Content-Length", std::to_string(m_body.size()));
            writer->header("Cache-Control", "no-store");
            for (const auto& header : m_headers) writer->header(header.first, header.second);
            writer->payload(m_body.data(), m_body.size());
            writer->finish(true);
        }
        void cancel() override {}
    };
    class Router final : public seasocks::PageHandler {
    public:
        std::map<std::pair<Method, std::string>, Handler> routes;
        std::shared_ptr<seasocks::Response> handle(const seasocks::Request& request) override {
            if (request.verb() == Method::WebSocket) return seasocks::Response::unhandled();
            const auto& uri = request.getRequestUri();
            const auto path = uri.substr(0, uri.find('?'));
            auto found = routes.find({request.verb(), path});
            if (found != routes.end()) {
                auto handler = found->second;
                try {
                    auto response = handler(request);
                    return response ? response : reply(seasocks::ResponseCode::InternalServerError, "Handler returned no response");
                } catch (...) {
                    return reply(seasocks::ResponseCode::InternalServerError, "Request failed");
                }
            }
            std::string allow;
            for (const auto& route : routes) if (route.first.second == path) {
                if (!allow.empty()) allow += ", ";
                std::string method = seasocks::Request::name(route.first.first);
                for (auto& c : method) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                allow += method;
            }
            if (!allow.empty()) {
                return reply(seasocks::ResponseCode::MethodNotAllowed, "Method not allowed", "text/plain", {{"Allow", allow}});
            }
            return seasocks::Response::unhandled(); // keep static files and other handlers working
        }
    };
    std::shared_ptr<Router> m_router = std::make_shared<Router>();
public:
    static std::shared_ptr<seasocks::Response> reply(seasocks::ResponseCode status, std::string body,
            std::string contentType = "text/plain", std::map<std::string, std::string> headers = {}) {
        for (const auto& header : headers) {
            auto name = header.first;
            for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (name.empty() || name.find_first_of("\r\n: ") != std::string::npos ||
                header.second.find_first_of("\r\n") != std::string::npos ||
                name == "content-length" || name == "transfer-encoding" || name == "content-type" || name == "connection")
                throw std::invalid_argument("Invalid or reserved response header");
        }
        if (contentType.find_first_of("\r\n") != std::string::npos) throw std::invalid_argument("Invalid content type");
        return std::make_shared<Reply>(status, std::move(body), std::move(contentType), std::move(headers));
    }
    void route(Method method, std::string path, Handler handler) {
        if (method == Method::Invalid || method == Method::WebSocket || path.empty() ||
            path.front() != '/' || path.find_first_of("?#") != std::string::npos || !handler)
            throw std::invalid_argument("Invalid HTTP route");
        if (!m_router->routes.emplace(std::make_pair(method, std::move(path)), std::move(handler)).second)
            throw std::logic_error("HTTP route already registered");
    }
    std::shared_ptr<seasocks::PageHandler> handler() const { return m_router; }
};
} // namespace voltxp
