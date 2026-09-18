# X+ validation

From the Volt repository:

```powershell
python -m unittest discover -s tests -p test_xplus_generator.py
.\framework\user-scripts\create-volt-app.ps1 xplus-check -Template 'x+' -OutputDir '..\xplus-check' -NoGit
```

Build the generated solution's Debug and Release configurations with Visual
Studio 2026 or MSBuild, then run (Node 22+; port 8000 must be available):

```powershell
node tests\smoke-xplus.cjs ..\xplus-check Debug
node tests\smoke-xplus.cjs ..\xplus-check Release
.\tests\check-xplus-build.ps1 -AppRoot ..\xplus-check
.\tests\check-xplus-locked-log.ps1 -AppRoot ..\xplus-check
```

Use a disposable generated app for `check-xplus-build.ps1`: it cleans Debug
output and temporarily prepends a compiler error to main.x.cpp, restoring the
source in a finally block. Its final rebuild restores the complete web output.
The locked-log check holds a file exclusively open in the client's IntDir while
Build, Rebuild, Clean, and a subsequent Build run. Generated files must remain
inside the dedicated `client/generated` subtree, leaving IDE logs untouched.

## Verified September 16, 2026

- Visual Studio 2026 18.10.1, MSVC v145, Emscripten 6.0.9, Node 24.19.0.
- PowerShell and Git Bash creation entry points both generate X+ successfully.
- Generator tests check standalone dependency copies, unique/matching project
  GUIDs, names, output paths with spaces, and refusal to overwrite an existing app.
- Debug and Release build both projects successfully with MEMORY64 (`-m64`).
- HTTP bytes match all browser artifacts, including the 3.6 MB Debug WASM;
  correct WASM MIME type, 404 responses, text/binary WebSocket echoes and clean close.
- Debug Clean leaves Release intact. Rebuild restores HTML, CSS, bootstrap JS,
  compiled JS and WASM. An edited source causes a rebuild; injected diagnostics
  identify the original client/src/main.x.cpp at line 1 via #line.
- Browser: Debug counter, panel toggle and keyed list removal work; Release
  loads and the counter works. These exercise the MEMORY64 event pointer bridge.
- A wasm32 syntax-only build of the generated Volt X client also passes using
  the updated framework headers.
- Existing PowerShell `x` and `raw` scaffolding still succeeds. An attempted
  compile of the legacy raw example found API mismatches (AppBase, ComponentBase,
  VoltRuntime and event naming); repairing that example is outside this change.

Build/run configuration was checked through MSBuild and direct server launches.
The Visual Studio F5 keyboard action itself was not automated. No browser-side
WASM debugger integration or Volt X IntelliSense is included.

## Dependency injection checks

From the repository in a VS Developer PowerShell with Emscripten activated:

```powershell
New-Item -ItemType Directory -Force output/di-tests | Out-Null
cl /nologo /std:c++17 /EHsc /W4 /Iapp-template-x-plus/shared /Iframework/include tests/test_dependency_injection.cpp /Fe:output/di-tests/native.exe /Fo:output/di-tests/native.obj
./output/di-tests/native.exe
em++ tests/test_dependency_injection.cpp -Iapp-template-x-plus/shared -Iframework/include -std=c++17 -m64 -fexceptions -o output/di-tests/unit.js
node output/di-tests/unit.js
em++ tests/di-isolation.cpp -Iapp-template-x-plus/shared -Iframework/include -std=c++20 -m64 -fexceptions -lembind -sMODULARIZE=1 -sEXPORT_NAME=VoltApp -o output/di-tests/isolation.js
node tests/check-di-isolation.cjs output/di-tests/isolation.js
```

These check null/duplicate/missing-service errors, stable references after a
rejected replacement, reverse teardown with provider access from a consumer
destructor, reentrant base release, repeated cleanup, reuse, derived cleanup
through a base pointer, and reference-based runtime access. The isolation test
uses the template's actual `services()` and `invalidate()` helpers with test
runtimes in two MEMORY64 instances and interleaved timers/Promise callbacks.
Both generated solution configurations and the HTTP/WebSocket smoke checks also
pass with the DI integration.

## Handle-based memory tests

Run the handle-based memory tests from a Developer PowerShell with Emscripten:

```powershell
New-Item -ItemType Directory -Force output/memory-tests | Out-Null
cl /nologo /std:c++17 /EHsc /W4 /Iapp-template-x-plus/shared tests/test_memory_store.cpp /Fe:output/memory-tests/native.exe /Fo:output/memory-tests/native.obj
./output/memory-tests/native.exe
em++ tests/test_memory_store.cpp -Iapp-template-x-plus/shared -std=c++17 -m64 -fexceptions -o output/memory-tests/unit.js
node output/memory-tests/unit.js
em++ tests/memory-isolation.cpp -Iapp-template-x-plus/shared -std=c++17 -m64 -fexceptions -lembind -sMODULARIZE=1 -o output/memory-tests/isolation.js
node tests/check-memory-isolation.cjs output/memory-tests/isolation.js
```

Checks cover typed pools, all supported scalar types, same-value suppression,
float special cases, stale handles before/after reuse, zero-generation retirement,
double-release rejection and resource cleanup when slots are reset, nested
destruction, caller-parent membership validation, value-only change records,
field value assignment versus view binding,
cross-store views, runtime read-only checks, mixed scalar/container membership,
structural edits after view binding, partial construction cleanup, integer increment overflow,
direct change callbacks and per-module singleton isolation.

## In-memory data-service replication

From a Developer PowerShell with Emscripten activated:

```powershell
New-Item -ItemType Directory -Force output/replication-tests | Out-Null
cl /nologo /std:c++17 /EHsc /W4 /Iapp-template-x-plus/shared tests/test_data_services.cpp /Fe:output/replication-tests/native.exe /Fo:output/replication-tests/native.obj
./output/replication-tests/native.exe
em++ tests/test_data_services.cpp -Iapp-template-x-plus/shared -std=c++17 -m64 -fexceptions -o output/replication-tests/unit.js
node output/replication-tests/unit.js
```

Checks cover binary slot-only changes, independent local generations, fresh-author
slot reuse against a surviving replica, stale views after reset, reader notifications,
64-bit/string payloads, malformed messages, and the five-second/two-second heartbeat.
Live session tests also check Front-to-server-to-Back updates and clean-slate takeover.
The MEMORY64 interop test verifies the emitted bytes, incoming Back data, and UI
invalidation through the actual C++/JavaScript bridge.

## Deferred validation

Fresh-system testing and any setup fixes it reveals are deferred to a later pass.
The current validation used the existing development machine and installed SDK.

## Session WebSocket checks

Build a generated X+ app, then run:

```powershell
python tests/test_sessions.py ../your-app/output/Debug/server/Server.exe
node tests/check-session-client.cjs
```

The standard-library Python test launches its own server on a temporary local
port and checks cookie/origin rejection, independent sessions, takeover notices,
two heartbeat cycles, missing-pong deadlines and malformed-message disconnection.
The JavaScript test runs the actual client script with a simulated browser clock
and WebSocket, checking cookie reuse, ping/pong, reconnects, stale callbacks and
no reconnect after `replaced`. It does not automate a real browser.

## Client WebSocket service interop

With Emscripten activated:

```powershell
New-Item -ItemType Directory -Force output/websocket-tests | Out-Null
em++ tests/websocket-di.cpp -Iapp-template-x-plus/shared -Iframework/include -std=c++17 -m64 -fexceptions -lembind -sMODULARIZE=1 -o output/websocket-tests/interop.js
node tests/check-websocket-di.cjs output/websocket-tests/interop.js
```

This executes the real C++ service and JavaScript factory with simulated browser
sockets. It verifies explicit startup, module-scoped callbacks, runtime invalidation,
state isolation, replacement, and DI teardown cancelling callbacks/listeners.
`check-session-client.cjs` additionally checks start/stop/dispose and virtual-clock
heartbeat/reconnect behavior. The generated Debug solution builds with the status UI.

## Server session services

From a Visual Studio Developer PowerShell:

```powershell
New-Item -ItemType Directory -Force output/session-tests | Out-Null
cl /nologo /std:c++17 /EHsc /W4 /Iapp-template-x-plus/shared tests/test_server_session.cpp app-template-x-plus/server/sessions/Session.cpp /Fe:output/session-tests/native.exe /Fo:output/session-tests/
./output/session-tests/native.exe
```

Checks per-session DI, retained application state across reconnect, independent
sessions, stale connection isolation, heartbeat timeout and send failures without
Seasocks. The live `test_sessions.py` checks also pass against the refactored server.
Lifecycle checks cover custom session types, default hooks, registration without
startup, startup failure, exactly-once close before derived destruction, and
provider availability during close.

Binary talker coverage includes explicit little-endian wire bytes, embedded zero
and high-bit bytes, empty echo payloads, native registry validation, per-module
WASM routing, unknown IDs, text-frame rejection, malformed controls, heartbeat
and replacement over actual WebSockets. Run `test_server_session.cpp`,
`check-session-client.cjs`, `check-websocket-di.cjs` and `test_sessions.py` using
the commands above. Rebuild generated apps before testing the binary protocol.

## HTTP services

Build a generated app and run `python tests/test_http.py ../your-app/output/Debug/server/Server.exe`.
This checks cookie-free routes, query matching, binary/empty request bodies, 405/Allow,
404, and static-file fallback.

The client interop test uses the real MEMORY64 service with a controlled fetch:

```powershell
em++ tests/http-client.cpp -std=c++17 -m64 -fexceptions -lembind -sMODULARIZE=1 -o output/http-client.js
node tests/check-http-client.cjs output/http-client.js
```

It verifies method/header/body forwarding, response headers, module isolation, HTTP
errors, network errors, timeout, cancellation, late completion, and disposal.
