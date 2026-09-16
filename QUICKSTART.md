# Volt Quickstart

Choose `x+` for a Visual Studio client/server solution, or `x` for the existing
script-based browser app. `x` remains the default when no template is specified.

## Volt X+ — Windows and Visual Studio 2026

You need Visual Studio 2026 with the **Desktop development with C++** workload
(MSVC v145 and a Windows SDK), Git, Python 3, and an activated Emscripten SDK.
The template has been tested with Emscripten 6.0.9 and uses MEMORY64.

From your projects folder:

```powershell
git clone https://github.com/vdcoder/volt.git
.\volt\framework\user-scripts\create-volt-app.ps1 my-app -Template 'x+' -OutputDir .\my-app
```

PowerShell uses `-Template` with one dash. The Git Bash equivalent is:

```bash
./volt/framework/user-scripts/create-volt-app.sh my-app --template 'x+' --output ./my-app
```

1. Open `my-app/my-app.sln` in Visual Studio with the SDK environment available.
2. Select **Debug | x64** or **Release | x64**.
3. Press **Ctrl+Shift+B** to build the client and server.
4. Set **Server** as the startup project, then press **F5** or **Ctrl+F5**.
5. Open [http://127.0.0.1:8000/](http://127.0.0.1:8000/).

For exact SDK installation and launch commands, follow the
[Volt X+ guide](app-template-x-plus/README.md). Starting VS from an activated
Developer PowerShell avoids the common missing-SDK environment issue.

The app ships with a Volt X counter/panel/list demo, native Seasocks static
server, `/ws` echo endpoint, build tools, and local dependency sources. SDKs
are installed separately. F5 debugs the native server; open the browser manually.

| Edit or inspect | Location in the generated app |
|---|---|
| UI | `client/src/App.x.hpp` |
| Reusable UI components | `client/src/components/` |
| HTML, CSS, static assets | `client/public/` |
| Server and WebSocket handler | `server/main.cpp` |
| Client/server service registration and teardown | `client/src/AppDI.hpp`, `server/AppDI.hpp` |
| App service access and client runtime ownership | `client/src/ApplicationServices.hpp`, `server/ApplicationServices.hpp` |
| Shared standard C++ DI container | `shared/DependencyInjection.hpp` |
| App GUID | `app.json` |
| Debug website | `output/Debug/web/` |
| Release website | `output/Release/web/` |
| Native executable | `output/<configuration>/server/Server.exe` |

Rebuild after edits, then refresh the browser. Stop the server before rebuilding
its executable. Generated C++ lives under `intermediate/`; edit the original
sources instead. Compiler errors use the existing `#line` source mapping.

Include your project's `ApplicationServices.hpp` to use `services()`, which
returns its `AppDI&`. Register services in `AppDI::registerDependencies()`, with
providers before their consumers. Client code can call global `invalidate()`
after updating state in an async callback; the runtime service is registered
before the app mounts. The server has its own container and no Volt runtime.
See [application services](app-template-x-plus/README.md#application-services)
for examples and lifetime rules.

## Volt X — script-based browser app

With Emscripten activated and Python 3 available, from your projects folder:

```bash
git clone https://github.com/vdcoder/volt.git
./volt/framework/user-scripts/create-volt-app.sh my-app --template x
cd my-app
./build.sh
cd output
python3 -m http.server 8001
```

Open [http://localhost:8001](http://localhost:8001). For the PowerShell equivalent,
see [Volt on Windows](WINDOWS.md). The `x` layout keeps UI sources in `src/`,
assets at the app root, and browser artifacts in `output/`.

## Next steps

- [Volt X+ guide](app-template-x-plus/README.md): creation options, SDK setup,
  configurations, source mapping, WebSocket example, and troubleshooting.
- [README](README.md): framework overview and template comparison.
- [Windows guide](WINDOWS.md): PowerShell setup and the existing script workflow.
- [Changelog](CHANGELOG.md): current unreleased changes.

Fresh-system validation of X+ is deferred; the documented development-machine
checks are recorded in [the test notes](tests/README.md).
