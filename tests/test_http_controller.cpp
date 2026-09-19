#include "../app-template-x-plus/server/services/HttpServerService.hpp"
#include <DependencyInjection.hpp>
#include <cassert>
#include <iostream>
#include <optional>

struct Request : seasocks::Request {
    std::string uri;
    Verb method = Verb::Get;
    explicit Request(std::string path) : uri(std::move(path)) {}
    seasocks::Server& server() const override { throw std::logic_error("unused"); }
    Verb verb() const override { return method; }
    std::shared_ptr<seasocks::Credentials> credentials() const override { return {}; }
    const sockaddr_in& getRemoteAddress() const override { static sockaddr_in address{}; return address; }
    const std::string& getRequestUri() const override { return uri; }
    size_t contentLength() const override { return 0; }
    const uint8_t* content() const override { return nullptr; }
    bool hasHeader(const std::string&) const override { return false; }
    std::string getHeader(const std::string&) const override { return {}; }
};
struct Writer : seasocks::ResponseWriter {
    seasocks::ResponseCode status{};
    std::string body;
    void begin(seasocks::ResponseCode code, TransferEncoding) override { status = code; }
    void header(const std::string&, const std::string&) override {}
    void payload(const void* data, size_t size, bool) override { body.append(static_cast<const char*>(data), size); }
    void finish(bool) override {}
    void error(seasocks::ResponseCode, const std::string&) override { assert(false); }
    bool isActive() const override { return true; }
};
using Http = voltxp::HttpServerService;
auto ok(const seasocks::Request&) { return Http::reply(seasocks::ResponseCode::Ok, "ok"); }
template<class F> void rejects(F fn) {
    bool caught = false; try { fn(); } catch (const std::exception&) { caught = true; } assert(caught);
}
struct FailingController {
    voltxp::HttpController routes;
    explicit FailingController(Http& http) : routes(http.controller("/api/items")) {
        routes.get("/temporary", ok);
        routes.get("/", ok); // collides, member destruction must remove only temporary
    }
};
struct OwnedController {
    int calls = 0;
    voltxp::HttpController routes;
    explicit OwnedController(Http& http) : routes(http.controller("/owned")) {
        routes.get("/", [this](const seasocks::Request& request) { ++calls; return ok(request); });
    }
};
int main() {
    Http http;
    auto handler = http.handler();
    Request root("/api/items"), child("/api/items/child?x=1"), temporary("/api/items/temporary");
    {
        auto routes = http.controller("/api/items/");
        routes.get("/", ok); routes.get("child", ok);
        assert(handler->handle(root) && handler->handle(child));
        rejects([&] { FailingController failed(http); });
        assert(handler->handle(root)); assert(!handler->handle(temporary));
        auto moved = std::move(routes);
        rejects([&] { routes.get("/lost", ok); });
        assert(handler->handle(root));
        moved.get("/throws", [](const seasocks::Request&) -> std::shared_ptr<seasocks::Response> { throw std::runtime_error("private"); });
        auto writer = std::make_shared<Writer>();
        handler->handle(Request("/api/items/throws"))->handle(writer);
        assert(writer->status == seasocks::ResponseCode::InternalServerError && writer->body == "Request failed");
    }
    assert(!handler->handle(root) && !handler->handle(child));
    {
        auto routes = http.controller("/"); routes.get("/", ok);
        assert(handler->handle(Request("/")));
    }
    assert(!handler->handle(Request("/")));
    std::optional<voltxp::HttpController> survivor;
    {
        Http temporaryHttp;
        survivor.emplace(temporaryHttp.controller("/temporary"));
        survivor->get("/", ok);
    }
    survivor.reset(); // router lifetime is retained safely
    std::shared_ptr<seasocks::PageHandler> adapter;
    {
        voltxp::DependencyInjection di;
        di.registerDependency<Http>(std::make_unique<Http>());
        auto& service = di.resolveDependency<Http>();
        di.registerDependency<OwnedController>(std::make_unique<OwnedController>(service));
        adapter = service.handler();
        assert(adapter->handle(Request("/owned")));
        assert(di.resolveDependency<OwnedController>().calls == 1);
        di.releaseDependencies();
    }
    assert(!adapter->handle(Request("/owned")));
    std::cout << "PASS: controller prefixes, root routes, duplicate/constructor rollback, moves, error responses and DI teardown\n";
}
