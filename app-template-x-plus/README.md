# Volt X+ — Visual Studio 2026

Volt X in the browser, with a native Windows Seasocks server in the same solution.
This template uses normal `.sln` and `.vcxproj` files; no CMake or editor extension.

## What ships with X+

- A Visual Studio solution with three C++ projects: **Client** builds the Volt X
  browser app with Emscripten; **Server** builds a native Windows executable with MSVC;
  **[Desktop](DESKTOP.md)** hosts the client in an Edge WebView2 window with its own hidden server.
- The Volt X starter UI: counter, conditional panel, and a keyed fruit list.
- Debug and Release configurations, separate web/server/desktop outputs, and Build,
  Rebuild, Clean, and native run/debug settings.
- The Python DSL preprocessor and existing `#line` source mapping for compiler errors.
- Local copies of Volt headers, `volt.js`, and pinned Seasocks sources/resources
  with license notices. Generated apps build independently of the Volt checkout.
- A loopback HTTP server serving the client output and one text/binary WebSocket
  echo endpoint at `/ws`.
- An automatic cookie-based session connection at `/session`, with five-second
  pings, two-second pong deadlines, reconnects and single-tab takeover. The DI-owned
  `ClientWebsocketService` wraps JavaScript through Emscripten and drives the
  connection status shown in the starter UI.
- Binary session messages with a uint16 talker ID, symmetric C++ handler registries,
  reserved connection-control talker zero and a server echo example on talker one.
- A standard C++ dependency container shared by Client and Server, app-owned `AppDI`
  classes, and client `services()` / `invalidate()` helpers for async code.
- A first-pass [typed memory store](MEMORY-STORE.md) with reusable generational
  handles, store-aware field/container views, and a compositional Student example.

The Emscripten SDK and Visual Studio are prerequisites, not bundled dependencies. There is
no live reload, DSL editor extension, or browser WASM
debugger integration. The browser automatically connects to `/session`;
`/ws` remains an echo test endpoint.

## Get the template and create an app

The template lives in the Volt repository as `app-template-x-plus/`. Use the app
creator rather than copying that folder by hand: creation also supplies the
framework and preprocessor, replaces name tokens, and assigns project GUIDs.

From a **Developer PowerShell for Visual Studio 2026** terminal in your projects
folder, clone Volt (or use your existing checkout containing the X+ template):

```powershell
git clone https://github.com/vdcoder/volt.git
```

Install and activate an SDK if needed. This version was tested with 6.0.9:

```powershell
git clone https://github.com/emscripten-core/emsdk.git
.\emsdk\emsdk.bat install 6.0.9
.\emsdk\emsdk.bat activate 6.0.9
. .\emsdk\emsdk_env.ps1
```

Create the app and launch Visual Studio from that same terminal so it inherits
the SDK environment:

```powershell
.\volt\framework\user-scripts\create-volt-app.ps1 my-app -Template 'x+' -OutputDir .\my-app
devenv .\my-app\my-app.sln
```

PowerShell uses **one dash**: `-Template 'x+'`. In Git Bash the equivalent is:

```bash
./volt/framework/user-scripts/create-volt-app.sh my-app --template 'x+' --output ./my-app
```

The Bash entry point can generate X+; building and running its native server still
targets Windows/Visual Studio. Python 3 must be available to either creator.

| Option | PowerShell | Bash | Default |
|---|---|---|---|
| Template | `-Template 'x+'` | `--template 'x+'` | `x`; explicitly select `x+` |
| Destination | `-OutputDir .\my-app` | `--output ./my-app` | Sibling of the Volt repository |
| Volt instance ID | `-Guid my_app_v1` | `--guid my_app_v1` | App name |
| Skip Git initialization | `-NoGit` | `--no-git` | Initialize and commit the generated app |

Choose a new destination: by default, creation refuses to overwrite an existing directory.
App names start with a letter and use letters, digits, `_`, or `-`; the GUID uses
the same characters without the initial-letter restriction. The GUID identifies
the Volt instance and is separate from the generated Visual Studio project GUIDs.
Git initialization requires a configured Git author; use `-NoGit` to skip it.

## Build and run

Install Visual Studio 2026's **Desktop development with C++** workload, including
MSVC v145 and a Windows SDK. Install and activate Emscripten and have Python 3
available when creating the app (the SDK includes Python).

Open the generated solution, select **Debug | x64**, and press **Ctrl+Shift+B**.
Set **Server** as the startup project if Visual Studio has selected another one.
Press **F5** to debug the native server, or **Ctrl+F5** to run it without debugging.
Open http://127.0.0.1:8000/. Ctrl+C stops the console server; stop it before rebuilding.
F5 debugs the Windows server, not the browser's WebAssembly. The browser is opened
manually; the same URL works for both configurations.

For a Windows 11 desktop window, set **Desktop** as startup project and press F5.
The solution build includes Desktop and restores its WebView2 SDK from NuGet
on first build; see [Desktop setup and distribution](DESKTOP.md).

Tested with Emscripten 6.0.9; the build uses `-m64` for MEMORY64 and its default
BigInt integration. Use a browser with WebAssembly memory64 support.

Emscripten discovery checks `EMSDK`, then `em++` on PATH, then `.tools/emsdk` in this app
or its parent directory. The SDK must already be installed and activated. No
Emscripten SDK is downloaded by Build; Desktop restores its separate WebView2 SDK.
Restart Visual Studio after changing its environment.

## Files

```text
<app-name>.sln
app.json                         App name and VOLT_GUID
client/
  Client.vcxproj                 Emscripten Makefile project
  src/                          Edit these Volt X sources
    AppDI.hpp                   Client service registration and teardown
    ApplicationServices.hpp     App ownership, services(), invalidate()
    services/VoltRuntimeService.hpp  Non-owning runtime reference
  public/                       index.html, global.css, other static assets
server/
  Server.vcxproj                 Native MSVC project; depends on Client
  main.cpp                      Static serving and /ws echo handler
  seasocks_impl.cpp              Seasocks C++ unity translation unit
  AppDI.hpp                     Server service registration and teardown
  ApplicationServices.hpp       Server services() accessor
desktop/
  Desktop.vcxproj                Native WebView2 host; depends on Server
  main.cpp                      Window and WebView2 setup
  DesktopServer.hpp             Hidden server process ownership
shared/
  DependencyInjection.hpp        Standard C++17 container in namespace voltxp
dependencies/
  volt/include/                 Header-only framework
  volt/src/volt.js               Source copy of the browser bootstrap
  seasocks/                     Pinned sources, generated resources, licenses
tools/
  build-client.cmd              SDK discovery and PowerShell entry point
  build-client.ps1              Preprocess, compile, copy browser assets
  preprocesor.py                Copied from the Volt X preprocessor
  restore-webview2.ps1           Restore the pinned WebView2 SDK on first build
output/<Debug|Release>/
  web/                          Complete static website
  server/Server.exe              Native server (outside the served folder)
  desktop/Desktop.exe            Desktop entry point (outside the served folder)
intermediate/<Debug|Release>/    Generated C++ and compiler intermediates
```

The x64 label selects the native Server and Desktop architecture; the browser uses MEMORY64.
Client Debug builds use `-O0 -g`, assertions, and Volt logging. Client Release
uses `-O2 -DNDEBUG`.
Both use C++20, Embind, and the `VoltApp` module expected by VoltBootstrap.
Client C++ exception handling is enabled for service registration/resolution errors.

Edit UI code in `client/src/App.x.hpp` and components under `client/src/components/`.
Place HTML, CSS, and other static assets in `client/public/`. Edit server wiring in `server/main.cpp`, HTTP routes in `server/controllers/`,
and per-session behavior in `server/sessions/Session.cpp`. `app.json` stores the app name and
the `guid` used on subsequent client builds. Never edit generated output as source.

Every requested Client build preprocesses its source tree and copies public assets
and `volt.js` into the selected web output. Generated files retain the existing
`#line` directive pointing to the original source. This maps compiler diagnostics;
it does not provide Volt X IntelliSense. Line accuracy depends on the preprocessor
preserving line counts; generated expressions may have different columns.

Build is intentionally unconditional so changes to DSL files, headers, or tools
are picked up. Clean deletes only the selected client's web output and generated
client tree; native Clean is handled by MSBuild. A clean build restores `volt.js`.

Client-generated sources and response files live under
`intermediate/<configuration>/client/generated/`. The parent client directory
belongs to Visual Studio/MSBuild and can contain an open `Client.log`; the build
script never deletes that parent or its logs.

## Command line

From the app directory in a Visual Studio Developer Command Prompt:

```bat
msbuild <app-name>.sln /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild <app-name>.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
msbuild <app-name>.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
tools\build-client.cmd build Debug
output\Debug\server\Server.exe
```

The server locates the adjacent `../web` folder relative to its executable,
independent of the working directory. Override the folder and port with:

```bat
output\Debug\server\Server.exe "output\Debug\web" 8080
```

The `/session` endpoint connects automatically using a session cookie and runs
ping/pong. A replacement tab sends the old tab into a stopped state through the
server's `replaced` message. See [session behavior](DATA-SERVICES.md#session-connection).

The separate `/ws` endpoint echoes text and binary data. On the served page:

```js
const socket = new WebSocket(`ws://${location.host}/ws`);
socket.onmessage = event => console.log('Echo:', event.data);
socket.onopen = () => socket.send('Hello from Volt X+!');
```

The server binds to loopback only. Seasocks compression is disabled; no zlib is
needed. See `dependencies/seasocks/README.vendor.md` for the pinned revision and
the WASM MIME, close-handshake, and Windows send-buffer fixes. `wepoll.c` is
compiled separately as C; the C++ sources are included through `seasocks_impl.cpp`.

## Application services

`voltxp` names the X+ utilities: `voltxp::DependencyInjection` is standard C++17
in `shared/`; `voltxp::VoltRuntimeService` is client-only. Client and Server each have
their own `AppDI.hpp` and `ApplicationServices.hpp`. Include `ApplicationServices.hpp`
where you need the app's global `services()` accessor (or client `invalidate()`).
`services()` returns **`AppDI&`**, so app-specific methods remain available; it
can also be passed to code accepting `voltxp::DependencyInjection&`.

`AppDI.hpp` describes which services the app owns and their lifetime order.
`ApplicationServices.hpp` supplies the instance and access helpers. These are
app-owned template files; the helpers are not added to the core `volt` namespace
or to the existing `x` and `raw` templates.

Add services to `AppDI::registerDependencies()`, providers before consumers:

```cpp
registerDependency<StoreService>(std::make_unique<StoreService>(
    resolveDependency<voltxp::VoltRuntimeService>()));
```

Here `StoreService` is your own service, with a constructor taking
`voltxp::VoltRuntimeService&`; it is not supplied by the template. The runtime
service is registered first using a non-owning reference to the app's engine.
Server registers `HttpServerService`, `SessionRegistry<Session>`, and
`ExampleController`; it has no Volt runtime. Desktop hosts the processes and
browser window and does not have a separate application DI container.

From client code, including an async callback belonging to the module:

```cpp
auto& store = services().resolveDependency<StoreService>();
// Update state through your store's API, then request a frame:
invalidate();
```

`invalidate()` requests a frame; it does not render synchronously. Multiple
requests before the next frame are coalesced by Volt. Call it after changing
state in timers, network responses, or other callbacks outside Volt's event
handling. Existing Volt DOM event callbacks already invalidate automatically.
Client services become available during `createVoltEngine()`, before the app's
constructor and `start()` run. Do not resolve them from static initializers.
Use the UI thread; this helper does not marshal work from background threads.

One engine/container lives in each Emscripten instance. Separate `VoltApp()`
factory calls have separate C++ service state. JavaScript callbacks must retain
their originating Module; browser globals and DOM objects are still shared.
The template rejects creating a second engine in an existing module instance.
The render-only `g_pRenderingEngine` is unchanged and is not used by these helpers.

The container owns services, rejects null and duplicate registrations, and throws
on missing resolution. References remain valid until release. Teardown destroys
services in reverse registration order; registration during release is rejected.
It is not a concurrent container: register/release on the owning thread, and
coordinate any future server workers before teardown.

Each `AppDI` destructor explicitly calls its `releaseDependencies()` override:
C++ base destructors cannot dispatch to derived cleanup. Put cancellation of
app-owned timers, subscriptions, and background work in that override, before
the base call. Cleanup must be non-throwing, safe after partial startup, and safe
to repeat. Further subclasses must likewise invoke their own cleanup before
their members are destroyed. Runtime ownership ensures services are destroyed
before the engine. App destructors must not resolve services after that cleanup.

The browser bootstrap's existing `destroy()` only removes DOM event listeners;
it does not yet destroy the engine or services. This addition does not introduce
hot replacement or an unmount API. Do not manually release a running app's
container while callbacks or UI code still use it.

### Getting these additions in an existing app

New `x+` apps receive these files automatically. Updating the Volt checkout does
not update generated apps. For an existing app, generate a fresh X+ app beside
it and compare the solution, all three project files, `client/`, `server/`,
`desktop/`, `shared/`, `tools/`, and dependency changes. Include the Seasocks
`listeningPort()` addition when adopting Desktop. Preserve your app-specific registrations,
sources, project GUIDs, and settings. The client build needs the shared include
path and `-fexceptions`; copying the headers alone is insufficient.

## Experimental Front/Back data services

See [Front and Back data services](DATA-SERVICES.md) for ordered, fire-and-forget
replication over the [handle-based memory store](MEMORY-STORE.md), with no data
versions, ACKs or network batching. Binary talkers carry slot-only references;
each store tracks its own local generations. Disconnects reset both stores, and
new connections start empty. A watchdog uses five-second pings and a two-second
pong deadline. Try the Live shared data counter: the server publishes twice the
client value through Back data. Snapshot recovery remains future work.

## HTTP services

The template includes cookie-independent server routes and a browser fetch service.
Try **Call hello API** or `POST /api/echo`. See [HTTP services](HTTP.md) for route
registration, request callbacks, timeouts, cancellation, and browser CORS boundaries.

## Common setup issues

| Symptom | What to check |
|---|---|
| `em++ not found` | Launch VS from the activated SDK terminal, set `EMSDK` before starting VS, or use the documented local `.tools/emsdk` layout. An already-running VS does not inherit later terminal changes. |
| Missing v145 toolset or Windows SDK | Install the Desktop development with C++ workload in VS 2026. |
| Python unavailable during creation | Activate the SDK first so `EMSDK_PYTHON` is set, or install Python 3 on PATH. |
| F5 tries to launch HTML | Set **Server** for a console server or **Desktop** for the embedded browser window as the startup project. |
| Server cannot listen | Stop the other process using port 8000, or supply another port. |
| Server executable cannot be overwritten | Stop its previous run, including any Desktop instance that owns it, before building. |
| WebView2 SDK restore fails | The first Desktop build needs access to NuGet. Check connectivity; the SDK cache is `.tools/webview2/`. |
| Desktop cannot create WebView2 | Check the Evergreen Runtime and the error/log information described in [Desktop setup](DESKTOP.md). |
| An older generated app tries to delete `Client.log` | Update its `tools/build-client.ps1` from this template; generated files must be under `client/generated`, below the MSBuild log directory. |

This workflow has been tested on the development machine, including browser
interactions and both configurations. Fresh-system testing and any resulting
setup fixes are deferred. This guide describes the current template and will
be updated as its features evolve.
