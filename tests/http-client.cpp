#include "../app-template-x-plus/client/src/services/HttpClientService.hpp"
#include <emscripten/bind.h>
#include <memory>
std::unique_ptr<voltxp::HttpClientService> http;
voltxp::HttpResponse last;
int calls = 0;
EMSCRIPTEN_BINDINGS(http_client_test) {
    emscripten::function("initialize", +[] { http = std::make_unique<voltxp::HttpClientService>(); });
    emscripten::function("release", +[] { http.reset(); });
    emscripten::function("request", +[](std::string url, std::string method, std::string body, int timeout) {
        return http->request({url, method, {{"X-Test", "yes"}}, body, timeout}, [](voltxp::HttpResponse response) {
            last = std::move(response); ++calls;
        });
    });
    emscripten::function("cancel", +[](std::uint32_t id) { http->cancel(id); });
    emscripten::function("onHttpResponse", +[](std::uint32_t id, int status, emscripten::val headers, std::string body, std::string error) {
        http->onResponse(id, status, headers, std::move(body), std::move(error));
    });
    emscripten::function("calls", +[] { return calls; });
    emscripten::function("status", +[] { return last.status; });
    emscripten::function("body", +[] { return last.body; });
    emscripten::function("error", +[] { return last.error; });
    emscripten::function("header", +[] { return last.headers.at("x-reply"); });
}
