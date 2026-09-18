# HTTP services

X+ includes a small Seasocks routing service and a browser-only HTTP client.
Neither requires a session cookie or WebSocket connection. JSON is optional;
handlers and callers choose the body format. Native outbound HTTP is deferred.

## Server routes

Register application endpoints in `server/HttpRoutes.hpp`. `main.cpp` calls this
once after DI registration and attaches the service's handler to Seasocks.

```cpp
http.route(voltxp::HttpServerService::Method::Get, "/api/hello",
    [](const seasocks::Request& request) {
        return seasocks::Response::jsonResponse(R"({"message":"hello"})");
    });
```

Paths match exactly, excluding the query string. Read the original URI/query with
`request.getRequestUri()`, headers with `getHeader`, and raw body bytes with
`content()` plus `contentLength()`. No automatic URL decoding or JSON parsing is
performed. Use `ResponseBuilder` to choose a status, content type, headers, and
body for successful responses. For API errors use `HttpServerService::reply(status,
body, contentType, headers)`, which preserves the body and headers on 4xx/5xx too
(Seasocks ResponseBuilder otherwise substitutes an HTML error page).
Request references/content are borrowed only for the synchronous call.

Handlers run on the single Seasocks event loop: keep work short. Duplicate routes
are rejected; handler exceptions become a generic 500 response. A known path with
an unregistered method returns 405 and Allow. Unmatched requests continue to static
files/other Seasocks handlers, eventually returning 404. WebSockets bypass this
router. GET does not implicitly register HEAD. Supported server methods are the
bundled Seasocks GET, POST, PUT, DELETE, HEAD, and OPTIONS; PATCH is not included.

The examples are `GET /api/hello` (JSON) and `POST /api/echo` (raw body echo).
Any HTTP client can call them without a Volt session. The development server still
binds to loopback; this feature does not publish it on the Internet. Cross-origin
browser callers inherit bundled Seasocks' `Access-Control-Allow-Origin: *` default;
requests needing preflight still need an application OPTIONS route/allowed headers.
This default does not allow credentialed cross-origin browser calls. A configurable
server CORS policy is outside this first slice.

## Browser requests

`HttpClientService` is registered in client DI. `http.js` wraps browser fetch;
Embind delivers responses to the owning WASM module.

```cpp
auto& http = services().resolveDependency<voltxp::HttpClientService>();
auto id = http.request({"/api/hello"}, [](voltxp::HttpResponse response) {
    if (!response.error.empty()) { /* network error, timeout, or cancellation */ }
    else if (!response.ok()) { /* HTTP error; status/body are still available */ }
    else { /* consume response.body and response.headers */ }
});
// http.cancel(id); // completes once with error == "cancelled"
```

Requests accept relative or absolute URLs, method, string headers, UTF-8 text body,
and timeoutMs (default 15 seconds). Responses contain status, exposed headers,
UTF-8 text body, and error. This first client does not support streaming, binary
bodies, automatic JSON conversion, or native outbound HTTP. Browser fetch rules
apply: CORS, mixed-content checks, forbidden headers, and default same-origin
credentials. Do not put private API keys in browser code.

HTTP 4xx/5xx completes normally with an empty error and `ok() == false`. Network
failures use status zero; cancellation and timeout use errors `cancelled` and
`timeout`. Timeout covers reading the response body too. Each request completes
at most once. Disposing the service aborts pending work and suppresses callbacks.
Callbacks execute on the browser thread; they own their response value. Call
`invalidate()` when changing UI state. Keep captured objects alive or use a weak
lifetime guard, as the template example does. Callback exceptions are logged and
are not redelivered as network errors.

Try the **Call hello API** button in the generated app. It displays the response
status and body without relying on the session WebSocket.
