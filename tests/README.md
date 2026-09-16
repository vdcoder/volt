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

## Deferred validation

Fresh-system testing and any setup fixes it reveals are deferred to a later pass.
The current validation used the existing development machine and installed SDK.
