# Volt X+ — Visual Studio 2026

Volt X in the browser, with a native Windows Seasocks server in the same solution.
This template uses normal `.sln` and `.vcxproj` files; no CMake or editor extension.

## Start

Install Visual Studio 2026's **Desktop development with C++** workload, including
MSVC v145 and a Windows SDK. Install and activate Emscripten and have Python 3
available when creating the app (the SDK includes Python).

Open the generated solution, select **Debug | x64**, and press **Ctrl+Shift+B**.
Set **Server** as the startup project if Visual Studio has selected another one.
Press **F5** to debug the native server, or **Ctrl+F5** to run it without debugging.
Open http://127.0.0.1:8000/. Ctrl+C stops the console server; stop it before rebuilding.
F5 debugs the Windows server, not the browser's WebAssembly. The browser is opened
manually; the same URL works for both configurations.

Tested with Emscripten 6.0.9; the build uses `-m64` for MEMORY64 and its default
BigInt integration. Use a browser with WebAssembly memory64 support.

SDK discovery checks `EMSDK`, then `em++` on PATH, then `.tools/emsdk` in this app
or its parent directory. The SDK must already be installed and activated. No
SDK is downloaded by Build. Restart Visual Studio after changing its environment.

## Files

```text
<app-name>.sln
app.json                         App name and VOLT_GUID
client/
  Client.vcxproj                 Emscripten Makefile project
  src/                          Edit these Volt X sources
  public/                       index.html, global.css, other static assets
server/
  Server.vcxproj                 Native MSVC project; depends on Client
  main.cpp                      Static serving and /ws echo handler
  seasocks_impl.cpp              Seasocks C++ unity translation unit
dependencies/
  volt/include/                 Header-only framework
  volt/src/volt.js               Source copy of the browser bootstrap
  seasocks/                     Pinned sources, generated resources, licenses
tools/
  build-client.cmd              SDK discovery and PowerShell entry point
  build-client.ps1              Preprocess, compile, copy browser assets
  preprocesor.py                Copied from the Volt X preprocessor
output/<Debug|Release>/
  web/                          Complete static website
  server/Server.exe              Native server (outside the served folder)
intermediate/<Debug|Release>/    Generated C++ and compiler intermediates
```

The x64 label selects native server architecture; the browser uses MEMORY64.
Debug builds use `-O0 -g`, assertions, and Volt logging. Release uses `-O2 -DNDEBUG`.
Both use C++20, Embind, and the `VoltApp` module expected by VoltBootstrap.

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

The single WebSocket endpoint echoes text and binary data. On the served page:

```js
const socket = new WebSocket(`ws://${location.host}/ws`);
socket.onmessage = event => console.log('Echo:', event.data);
socket.onopen = () => socket.send('Hello from Volt X+!');
```

The server binds to loopback only. Seasocks compression is disabled; no zlib is
needed. See `dependencies/seasocks/README.vendor.md` for the pinned revision and
the MIME/close-handshake patches. `wepoll.c` is compiled separately as C.
