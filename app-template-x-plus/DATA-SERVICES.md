# Front and Back data services

Each session has two independent services/stores. Front is authored by the client
and replicated on the server; Back is authored by the server and replicated on
the client. `DataConnection` binds them to binary talkers 2 and 3. Talker 1 remains
the echo example. Additional independent domains can use their own stores/talkers.

## Local handles, wire slots

Each store owns its generations. Local handles contain a uint32 slot and uint32
generation; messages contain only uint32 slots. The author validates a handle
before emitting a change. Replicas install the specified slot and use their own
local generation for views and container membership. Replica allocation does not
follow the author's free list. Occupancy checks reject updates to absent slots
and allocations over occupied slots; type, payload, and parent checks still apply.

The permanent root is container slot zero, created locally without a message.
Use `store.root()` to obtain its current local handle. A newly constructed store
starts at generation one; reset advances occupied slots' generations and retains
the history. Never cache `{0,1}` across a reset. Old views fail after reset even
when a fresh browser author reuses the same slots. Generations never wrap; retired
slots cannot be installed again. Root exhaustion requires replacing the store
and discarding all views, rather than silently reusing its identity.

## Fire-and-forget delivery

`MemoryStore::setOnChange` emits a change immediately after a local mutation.
`DataConnection` serializes/copies it synchronously onto the socket. Replicas apply
changes without emitting them back. There are no snapshots, data versions,
acknowledgments, batching timers, retry logs, or application transactions in this
connection protocol. TCP/WebSocket ordering applies within one active connection.

`MemoryWire.hpp` uses little-endian fields: one byte operation, one byte type,
uint32 parent slot (omitted for SetValue), uint32 target slot, and the scalar
payload when applicable. Bool is one byte, integers/floats use 4 or 8 bytes, and
strings are a uint32 byte length followed by bytes (embedded zeroes are preserved).
Malformed/truncated/trailing data is rejected. Strings are limited to 1 MiB;
replica pools accept slots below 1,048,576. These are initial protocol bounds,
not a complete per-session memory quota. Root creation/removal is not transmitted.

## Connection lifetime and reset

Both DI containers register Front, Back, and their `DataConnection`. Transport
connection callbacks reset/activate the stores before application messages arrive.
Talkers are installed before connecting; the server's Ready control precedes data.
On disconnect or takeover, stores are deactivated and reset. A replacement connection
starts empty, including on the retained server Session. Stale socket callbacks
cannot apply to the replacement. No previous edits are recovered or retransmitted.

Decode/apply errors reject the message and close the transport. Encoding/send
errors fail the transport too. Closing resets both directions; a new connection
is required. Direct offline edits are not queued and the next connection discards
them. Application code should check `ready()` before authoring live data.

Readers use `store.setOnUpdated(std::function<void()>)`. It runs after committed
mutations, replica replay, and resets; unchanged author assignments do not notify.
It is independent of the outgoing change callback. Both client stores use it to
invalidate Volt. Callbacks run on the owning event loop, may read the same store,
and must not mutate/reset that store or replace its callback during notification.
They may update a different store. Rebind views after connection changes.

The template's Live shared data example writes an int32 at the Front root. The
server observes it and publishes twice its value as an int64 at the Back root.
The client renders both values; disconnect/refresh/takeover resets the example.
The server hook lives in `Session.cpp`, while the UI lives in `App.x.hpp`.

Actions retain an in-memory helper API, but action wire encoding is not part of
this slice. Future action messages must use the same ordered socket as changes.
Legacy in-memory `MemoryStore::snapshot/restore` helpers are not used by this
protocol; restoring historical snapshots does not preserve reset invalidation.

## Five-second ping, two-second deadline

`shared/ConnectionHeartbeat.hpp` is a clock-driven watchdog, one per socket.
Poll it on the owning event loop using `steady_clock`. It emits the first ping
five seconds after connection and subsequent pings five seconds after the prior
ping was issued. Echo its token in the corresponding pong; this token correlates
heartbeats only and is not a data version. Two seconds is the round-trip limit.
At the deadline, with no matching pong, `failed()` becomes true. A pong arriving
at or beyond that deadline fails even if the timer callback was delayed.
Unexpected pong tokens also fail. Send/socket errors call `fail()` immediately.

The owner must check the watchdog before processing incoming data, close the
socket on failure, invalidate both services, and discard old callbacks. Use a
fresh watchdog for a new socket. Event-loop stalls can delay actual closing;
on resumption, check deadlines before accepting more data. A strict deadline
also intentionally reconnects slow or suspended clients.

Use application-level ping/pong messages on the same ordered data stream so the
response follows preceding update processing. Browser JavaScript does not expose
WebSocket control-frame ping/pong through its API
([WebSockets Standard](https://websockets.spec.whatwg.org/#ping-and-pong-frames)).
A heartbeat is a liveness/deadline check, not durable-delivery acknowledgment;
a failure immediately after a successful pong may remain undetected until the
next ping's deadline (up to about seven seconds with a responsive event loop).

## Session connection

The browser loads `client/public/session.js`, which exposes the `VoltSession.create`
factory without opening a socket. The DI-owned `ClientWebsocketService` creates
and starts its instance after the Volt app mounts. That instance creates a random
128-bit `volt_session` cookie if absent and connects to `/session`. This is a session
cookie with `Path=/` and `SameSite=Strict` (also `Secure` under HTTPS). It identifies
a session, not an authenticated user. The server validates the cookie format and
same-host Origin. Each cookie can have one active WebSocket. Different cookies
remain independent.

`server/network/SessionHandler.hpp` adapts Seasocks callbacks to the session-owned
`SessionWebsocketService`, which handles protocol messages and heartbeat state.
Both endpoints send binary Ping controls every five seconds and
expect matching Pong controls within two seconds. The browser reconnects one
second after a failure, using its existing cookie and a fresh connection. The
server polls deadlines every 100 ms. Event-loop suspension can delay detection;
late messages are checked against the deadline before processing.

When a second connection uses the same cookie, the server immediately stops
accepting messages from the old connection and sends it `replaced`. That message
means **do not reconnect**: the old tab stops all timers and closes its socket.
The server allows up to two seconds for delivery/closure before forcibly closing
it. Refreshing the displaced tab intentionally starts a new connection and takes
over again. Callbacks from obsolete browser sockets are ignored.

JavaScript reports state changes through the owning WASM module's Embind export
`onClientWebsocketStateChanged`. That resolves `ClientWebsocketService` from DI,
updates its state and calls its `VoltRuntimeService` to invalidate the UI. There
are no raw C++ pointers in this bridge, and each module captures its own callback.
The starter UI displays the resulting server connection state.

```cpp
auto& socket = services().resolveDependency<voltxp::ClientWebsocketService>();
auto state = socket.state(); // stopped, connecting, connected, disconnected, replaced, error
bool online = socket.connected();
socket.stop();  // cancels reconnects and closes this instance
socket.start(); // explicit restart; can intentionally take over a replaced session
```

`AppDI` registers the client-only service after `VoltRuntimeService`; reverse
teardown disposes its JavaScript instance before releasing the runtime provider.
`dispose()` clears timers, detaches socket/page listeners and drops the callback,
so queued events cannot resolve a destroyed service. Loading JS or registering
DI alone does not open a connection. The existing bootstrap destroy function
still does not release the app DI container automatically.

A `ready` message currently means only that the session connection is accepted.

## Current integration boundary

Front/Back changes are connected through binary talkers on real session sockets.
Snapshot recovery, persistence, forwarding between clients, and action wire
encoding remain future work. The initial lifecycle uses fresh empty stores.

## Server session ownership

```text
server/
  AppDI.hpp                         owns SessionRegistry
  network/SessionHandler.hpp        Seasocks/cookie/takeover adapter
  sessions/
    SessionRegistry.hpp             finds or creates sessions by cookie ID
    Session.hpp                     stable identity and per-session DI
    SessionDI.hpp                   register application services here
    Session.cpp                     app-specific startup and close hooks
    services/SessionWebsocketService.hpp
```

Each `Session` owns its `SessionDI`. The websocket service contains no Seasocks
types: the adapter supplies an outgoing binary callback when attaching a connection.
A connection ID prevents late events/disconnects from affecting its replacement.
The adapter closes failed sockets and handles the replacement-message grace period.
Application services can resolve the connection service in their session:

```cpp
auto& socket = session.services().resolveDependency<voltxp::SessionWebsocketService>();
bool online = socket.connected();
```

Add per-session application/data services in `SessionDI::registerDependencies`,
after their providers. The app-wide `SessionRegistry` retains the same session
and DI instances across reconnects. No implicit global current-session accessor
is used; pass the session or required service to application code. All accesses
remain on the Seasocks event-loop thread. The network adapter/server must be
shut down before releasing app DI; the adapter detaches transport callbacks on
teardown, and session DI releases its services in reverse registration order.

## Binary talkers

`/session` now accepts binary WebSocket messages only. Each complete message is:

```text
uint16 little-endian talker ID | payload bytes
```

WebSocket supplies the message boundary, so there is no additional length prefix.
`shared/Talkers.hpp` defines the envelope helpers, `MessageView`, and synchronous
`TalkerRegistry`. Both websocket services expose `talkers()` and `send(id, payload)`.
IDs are agreed protocol constants, not allocated per connection. Registrations
belong to the service instance and survive reconnects. Duplicate registrations,
reserved ID zero and empty handlers are rejected. Unregister a handler before
its captured consumer is destroyed. `MessageView` borrows bytes only for the
callback; copy if asynchronous work needs to retain them.

```cpp
// Client: services().resolveDependency<voltxp::ClientWebsocketService>()
// Server: session.services().resolveDependency<voltxp::SessionWebsocketService>()
auto& socket = /* resolve the appropriate service */;
socket.talkers().registerTalker(7, [](voltxp::MessageView bytes) {
    // Process this talker's payload synchronously.
});
voltxp::MessageBytes payload{0, 255, 128};
socket.send(7, voltxp::view(payload));
```

Talker zero is reserved for connection control. Its payload starts with one byte:

| Opcode | Meaning | Remaining payload |
|---|---|---|
| 1 | Ready (server to client) | None |
| 2 | Ping | Nonzero uint64 token, little-endian |
| 3 | Pong | The echoed uint64 token, little-endian |
| 4 | Replaced: do not reconnect (server to client) | None |

Lengths are exact. Truncated headers, unknown opcodes/talkers, invalid control
lengths, unexpected control direction/state, handler failures and text frames
invalidate the session connection. Messages dispatch in socket order with no
batching, versioning or ACKs. A handler must finish synchronous processing before
returning; launching async work does not preserve its completion order.

`Session::onStarted()` registers talker 1 as a small binary echo example. A client wishing to
use it registers its own handler at ID 1 and sends arbitrary bytes there. Replace
that example registration as the application grows. The independent `/ws` echo
endpoint remains unchanged. Front/Back change serialization is still future work.

In the browser, `binaryType = 'arraybuffer'` avoids asynchronous Blob conversion.
JavaScript handles talker-zero controls; other payloads go through the module's
`onClientWebsocketMessage` export into the C++ registry. Outgoing C++ bytes are
copied into the envelope before sending. Incoming bytes are copied into WASM for
synchronous dispatch; this first pass does not claim zero-copy transport.

## App-specific session lifecycle

Customize `Session.hpp` and `Session.cpp`: the app's `Session` derives from
`SessionBase`, supplies its `SessionDI`, and overrides `onStarted()` and
`onClosed(SessionCloseReason reason) noexcept`. Use `id()` and `services()`
directly; the echo talker captures `this` and resolves its websocket service.
`SessionDI` only registers dependencies. Session behavior is not a DI service.

The app registers `SessionRegistry<Session>`. Networking uses `ISessionRegistry`
and `SessionBase`, without depending on the app's custom type. The registry
constructs the complete derived object before invoking startup and retains it
across socket disconnects, reconnects, and takeover without repeating startup.

Registry ownership closes each session before deleting it, while the derived
object and all its services are still alive. Close runs once, followed by reverse
DI teardown. Startup failure calls close with `StartupFailed` before destruction,
and the failed session is not retained. Cleanup must tolerate partial startup
and must not throw. Base constructors/destructors never invoke application hooks.
Create sessions through the registry to get these lifecycle guarantees.

Reasons are `ApplicationRequested`, `Expired`, `ServerShutdown`, and
`StartupFailed`. Explicit `close(reason)` requires networking to stop using the
session first; it is not a live eviction API. Expiration/eviction remains future
work. Socket timeout, disconnect, and replacement do not close retained sessions.

Visual Studio keeps app-editable `Session.hpp`, `Session.cpp`, and `SessionDI.hpp`
visible alongside application code. The **Session infrastructure** filter groups
`SessionBase`, registry/interface, close reasons, `SessionWebsocketService`, and
the Seasocks `SessionHandler` adapter.
