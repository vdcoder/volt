#include <VoltRemServer.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
namespace fs = std::filesystem;

// ------------------------
// Small helpers
// ------------------------
static std::string mimeType(std::string const& path) {
    auto ext = fs::path(path).extension().string();
    for (auto& c : ext) c = (char)std::tolower((unsigned char)c);

    if (ext == ".htm" || ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "text/javascript; charset=utf-8";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".wasm") return "application/wasm";
    return "application/octet-stream";
}

static bool hasDotDot(std::string const& s) {
    return s.find("..") != std::string::npos;
}

// Parse a query param from target like "/ws?sid=abc"
static std::string getQueryParam(std::string_view target, std::string_view key) {
    auto qpos = target.find('?');
    if (qpos == std::string_view::npos) return {};
    auto query = target.substr(qpos + 1);

    while (!query.empty()) {
        auto amp = query.find('&');
        auto pair = query.substr(0, amp);
        auto eq = pair.find('=');

        auto k = (eq == std::string_view::npos) ? pair : pair.substr(0, eq);
        auto v = (eq == std::string_view::npos) ? std::string_view{} : pair.substr(eq + 1);

        if (k == key) return std::string(v);

        if (amp == std::string_view::npos) break;
        query.remove_prefix(amp + 1);
    }
    return {};
}

// ------------------------
// WebSocket session (echo for now)
// ------------------------
class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    explicit WsSession(tcp::socket socket)
        : ws_(std::move(socket)) {
    }

    // `req` is the already-read HTTP upgrade request.
    void run(http::request<http::string_body> req) {
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res) {
                res.set(http::field::server, "VoltRemote");
            }));

        ws_.async_accept(req,
            beast::bind_front_handler(&WsSession::onAccept, shared_from_this()));
    }

private:
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

    void onAccept(beast::error_code ec) {
        if (ec) {
            std::cerr << "[ws] accept error: " << ec.message() << "\n";
            return;
        }
        doRead();
    }

    void doRead() {
        ws_.async_read(buffer_,
            beast::bind_front_handler(&WsSession::onRead, shared_from_this()));
    }

    void onRead(beast::error_code ec, std::size_t /*bytes*/) {
        if (ec == websocket::error::closed) return;
        if (ec) {
            std::cerr << "[ws] read error: " << ec.message() << "\n";
            return;
        }

        // Echo
        ws_.text(ws_.got_text());
        ws_.async_write(buffer_.data(),
            beast::bind_front_handler(&WsSession::onWrite, shared_from_this()));
    }

    void onWrite(beast::error_code ec, std::size_t /*bytes*/) {
        if (ec) {
            std::cerr << "[ws] write error: " << ec.message() << "\n";
            return;
        }
        buffer_.consume(buffer_.size());
        doRead();
    }
};

// ------------------------
// HTTP session (serves files, or upgrades to WS)
// ------------------------
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket socket, std::string staticRoot, std::string sid)
        : stream_(std::move(socket))
        , staticRoot_(std::move(staticRoot))
        , sid_(std::move(sid)) {
    }

    void run() {
        doRead();
    }

private:
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

    std::string staticRoot_;
    std::string sid_;

    void doRead() {
        req_ = {};
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
    }

    void onRead(beast::error_code ec, std::size_t /*bytes*/) {
        if (ec == http::error::end_of_stream) {
            return doClose();
        }
        if (ec) {
            std::cerr << "[http] read error: " << ec.message() << "\n";
            return;
        }

        // WebSocket upgrade?
        if (websocket::is_upgrade(req_)) {
            return handleWebSocketUpgrade();
        }

        handleHttpRequest();
    }

    void handleWebSocketUpgrade() {
        // We only allow WS on /ws
        std::string target = std::string(req_.target());
        auto pathOnly = target.substr(0, target.find('?'));

        if (pathOnly != "/ws") {
            return sendSimple(http::status::not_found, "WebSocket endpoint is /ws");
        }

        // Pairing: require matching sid if configured
        if (!sid_.empty()) {
            auto qsid = getQueryParam(req_.target(), "sid");
            if (qsid != sid_) {
                return sendSimple(http::status::unauthorized, "Invalid sid");
            }
        }

        // Hand off to WS session; it will own the socket.
        std::make_shared<WsSession>(stream_.release_socket())->run(std::move(req_));
    }

    void handleHttpRequest() {
        if (req_.method() != http::verb::get && req_.method() != http::verb::head) {
            return sendSimple(http::status::method_not_allowed, "Only GET/HEAD supported");
        }

        std::string target = std::string(req_.target());
        auto qpos = target.find('?');
        if (qpos != std::string::npos) target.resize(qpos);

        if (target.empty() || target[0] != '/') {
            return sendSimple(http::status::bad_request, "Invalid target");
        }

        // Route mapping
        // "/" -> "/index.html"
        if (target == "/") target = "/index.html";

        // If you want to allow both "/static/..." and direct "/...":
        // - We'll serve "/static/foo" as "<staticRoot>/foo"
        // - We'll also serve "/index.html" as "<staticRoot>/index.html"
        std::string rel = target;

        if (rel.rfind("/static/", 0) == 0) {
            rel.erase(0, std::string("/static").size()); // keep leading '/'
            // "/static/echo-test.html" -> "/echo-test.html"
        }

        if (hasDotDot(rel)) {
            return sendSimple(http::status::forbidden, "Path traversal blocked");
        }

        fs::path full = fs::path(staticRoot_) / fs::path(rel).relative_path();
        beast::error_code fsec;

        // If directory requested, try index.html inside it
        if (fs::is_directory(full, fsec)) {
            full /= "index.html";
        }

        // Not found
        if (!fs::exists(full, fsec) || !fs::is_regular_file(full, fsec)) {
            return sendSimple(http::status::not_found, "Not found");
        }

        // Read file into memory (fine for dev; later you can stream)
        std::ifstream ifs(full, std::ios::binary);
        if (!ifs) {
            return sendSimple(http::status::internal_server_error, "Failed to open file");
        }
        std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        http::response<http::string_body> res{
            http::status::ok,
            req_.version()
        };
        res.set(http::field::server, "VoltRemote");
        res.set(http::field::content_type, mimeType(full.string()));
        res.keep_alive(req_.keep_alive());

        if (req_.method() == http::verb::head) {
            res.body() = "";
        }
        else {
            res.body() = std::move(body);
        }
        res.prepare_payload();

        auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
        http::async_write(stream_, *sp,
            [self = shared_from_this(), sp](beast::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "[http] write error: " << ec.message() << "\n";
                    return;
                }
                if (!self->req_.keep_alive()) {
                    self->doClose();
                    return;
                }
                self->doRead();
            });
    }

    void sendSimple(http::status st, std::string msg) {
        http::response<http::string_body> res{ st, req_.version() };
        res.set(http::field::server, "VoltRemote");
        res.set(http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(req_.keep_alive());
        res.body() = std::move(msg);
        res.prepare_payload();

        auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
        http::async_write(stream_, *sp,
            [self = shared_from_this(), sp](beast::error_code, std::size_t) {
                self->doClose();
            });
    }

    void doClose() {
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    }
};

// ------------------------
// Listener (accepts TCP, creates HttpSession for each socket)
// ------------------------
class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(asio::io_context& ioc, tcp::endpoint endpoint, std::string staticRoot, std::string sid)
        : ioc_(ioc)
        , acceptor_(asio::make_strand(ioc))
        , staticRoot_(std::move(staticRoot))
        , sid_(std::move(sid)) {

        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.bind(endpoint, ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.listen(asio::socket_base::max_listen_connections, ec);
        if (ec) throw beast::system_error(ec);
    }

    void run() { doAccept(); }

private:
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::string staticRoot_;
    std::string sid_;

    void doAccept() {
        acceptor_.async_accept(
            asio::make_strand(ioc_),
            beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
    }

    void onAccept(beast::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<HttpSession>(std::move(socket), staticRoot_, sid_)->run();
        }
        else {
            std::cerr << "[srv] accept error: " << ec.message() << "\n";
        }
        doAccept();
    }
};

// ------------------------
// Public API
// ------------------------
void StartVoltRemoteServer(asio::io_context& ioc, std::uint16_t port, std::string staticRoot, std::string sid) {
    // normalize static root (nice for debugging)
    try {
        staticRoot = fs::weakly_canonical(fs::path(staticRoot)).string();
    }
    catch (...) {
        // keep as-is if canonicalization fails
    }

    auto endpoint = tcp::endpoint(asio::ip::make_address("127.0.0.1"), port);
    std::make_shared<Listener>(ioc, endpoint, staticRoot, sid)->run();

    std::cout << "[srv] staticRoot: " << staticRoot << "\n";
    std::cout << "[srv] http:  http://127.0.0.1:" << port << "/\n";
    std::cout << "[srv] ws:    ws://127.0.0.1:" << port << "/ws"
        << (sid.empty() ? "" : ("?sid=" + sid)) << "\n";
}
