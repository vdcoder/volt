# Desktop host (Windows 11 x64)

**Desktop** runs the existing Client UI inside a native WebView2 window and starts
its own hidden Server process. Your Volt components, HTTP controllers, sessions,
talkers, and data services work through the same loopback HTTP/WebSocket endpoints.

## Build and run

1. Use the same Visual Studio 2026 C++ workload, Emscripten, and Python setup as
   the [normal X+ build](README.md#build-and-run).
2. Build **Debug | x64** or **Release | x64**. Desktop depends on Server, which
   depends on Client. The first Desktop build downloads the pinned Microsoft
   WebView2 SDK from NuGet and verifies its SHA-256 hash; later builds use the
   app-local `.tools/webview2/` cache. Internet access is needed for that first restore.
3. Right-click **Desktop → Set as Startup Project**, then press **F5** or **Ctrl+F5**.
   Alternatively, launch `output/Debug/desktop/Desktop.exe` (or Release).
4. Close the window to stop its server. Stop Desktop before rebuilding its binaries.

The Microsoft Edge WebView2 **Evergreen Runtime** must be installed. Windows 11
normally includes it. If environment creation fails, install or repair the runtime
using [Microsoft's WebView2 distribution guidance](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution).
The SDK is a build dependency; it does not install the browser runtime. The loader
is linked statically, so there is no separate WebView2Loader.dll to copy.

Debug enables browser context menus and developer tools (F12); Release disables
them. F5 debugs Desktop's native host; attach separately to Server for server-side
breakpoints. This does not add Visual Studio debugging of browser WebAssembly.

## Lifetime and files

Each window starts its own loopback server on an OS-assigned port, receives its
readiness notification, then navigates to that port. It can run alongside the normal
Server on port 8000. Each Desktop window has independent in-memory server data.
The ordinary Server startup project and command line still work as before.

An inherited event requests normal server shutdown. After a short grace period,
a Windows job object ensures the child process exits; the job also cleans up if
Desktop is terminated unexpectedly. Startup failure and server exit close the host
with an error instead of connecting to an unrelated process.

WebView2's writable profile and per-launch server logs live under
`%LOCALAPPDATA%/Volt/<app-name>-<desktop-project-guid>/`. Logs are named
`server-<desktop-process-id>.log` and can be deleted when no longer needed.
The profile persists between launches, but the changing localhost port means
origin-scoped storage such as localStorage is not a stable application database.
No native JavaScript bridge is added.

For distribution, keep the configuration's **desktop**, **server**, and **web**
directories together. This is a development host, not an installer or single-file
bundle. Release machines also need the appropriate Microsoft Visual C++ runtime.

The optional `Desktop.exe --smoke-test <report-file>` launch runs hidden, checks
the starter UI's WebSocket data round trip and HTTP hello action, writes server PID,
port, and result to the report, then exits. It is for testing the unmodified starter;
normal launches do not inspect or manipulate the app's UI.
Tests use a separate `test-profile` in the app-data directory and disable browser
background throttling so that a hidden test window can render the client reliably.
