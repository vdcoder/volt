#pragma once
#include <emscripten/val.h>
#include <cstdint>
#include <cctype>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

namespace voltxp {
struct HttpRequest {
    std::string url;
    std::string method = "GET";
    std::map<std::string, std::string> headers;
    std::string body;
    int timeoutMs = 15000;
};
struct HttpResponse {
    int status = 0;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string error; // empty for HTTP responses, including 4xx/5xx
    bool ok() const noexcept { return error.empty() && status >= 200 && status < 300; }
};
class HttpClientService {
    emscripten::val m_client = emscripten::val::undefined();
    std::map<std::uint32_t, std::function<void(HttpResponse)>> m_pending;
    std::uint32_t m_next = 0;
public:
    HttpClientService() = default;
    HttpClientService(const HttpClientService&) = delete;
    HttpClientService& operator=(const HttpClientService&) = delete;
    ~HttpClientService() { if (!m_client.isUndefined()) m_client.call<void>("dispose"); }
    std::uint32_t request(HttpRequest request, std::function<void(HttpResponse)> callback) {
        for (auto& c : request.method) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (!callback || request.url.empty() || request.method.empty() || request.timeoutMs <= 0 ||
            ((request.method == "GET" || request.method == "HEAD") && !request.body.empty()))
            throw std::invalid_argument("Invalid HTTP request");
        if (m_next == UINT32_MAX) throw std::overflow_error("HTTP request IDs exhausted");
        if (m_client.isUndefined()) {
            auto factory = emscripten::val::global("VoltHttp");
            if (factory.isUndefined()) throw std::runtime_error("Load http.js before HTTP requests");
            m_client = factory.call<emscripten::val>("create", emscripten::val::module_property("onHttpResponse"));
        }
        auto headers = emscripten::val::object();
        for (const auto& header : request.headers) headers.set(header.first, header.second);
        const auto id = ++m_next;
        m_pending.emplace(id, std::move(callback));
        try { m_client.call<void>("request", id, request.url, request.method, headers, request.body, request.timeoutMs); }
        catch (...) { m_pending.erase(id); throw; }
        return id;
    }
    void cancel(std::uint32_t id) {
        if (m_pending.count(id)) m_client.call<void>("cancel", id);
    }
    void onResponse(std::uint32_t id, int status, emscripten::val headers, std::string body, std::string error) {
        auto entry = m_pending.extract(id);
        if (entry.empty()) return;
        HttpResponse response{status, {}, std::move(body), std::move(error)};
        auto keys = emscripten::val::global("Object").call<emscripten::val>("keys", headers);
        const auto size = keys["length"].as<unsigned>();
        for (unsigned i = 0; i < size; ++i) {
            auto key = keys[i].as<std::string>();
            response.headers.emplace(key, headers[key].as<std::string>());
        }
        entry.mapped()(std::move(response));
    }
};
} // namespace voltxp
