# ⚡ Volt on Windows

## Visual Studio 2026: Volt X+

For a solution with a browser client and native Seasocks server:

```powershell
.\volt\framework\user-scripts\create-volt-app.ps1 my-app -Template 'x+'
```

Open `my-app.sln`, build Debug or Release with Ctrl+Shift+B, and run the **Server**
project with F5. Browse http://127.0.0.1:8000/. Requires the Desktop development
with C++ workload (MSVC v145), Python 3, and an activated Emscripten SDK.
The `x` and `raw` templates keep their existing workflow.
See [the X+ template guide](app-template-x-plus/README.md) for folder layout,
SDK discovery, command-line builds, source mapping, and the `/ws` endpoint.
X+ also provides client/server dependency injection through app-owned `AppDI`
classes, global `services()` access, and a client `invalidate()` helper for async
updates. See [application services](app-template-x-plus/README.md#application-services)
for registration and teardown rules.

Full Windows support for Volt — create apps, build to WebAssembly, and run in any browser, without leaving PowerShell.

---

## Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| **Emscripten (emsdk)** | latest | Required to compile C++ → WebAssembly |
| **Python 3** | 3.9+ | Ships with emsdk; used by the preprocessor |
| **Git** | any | For cloning and version control |
| **PowerShell** | 5.1+ | Built into Windows 10/11 |
| **Browser** | Chrome / Edge / Firefox | To run the app |

### 1 — Install Emscripten

```powershell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk install latest
.\emsdk activate latest
```

You'll need to **activate** the environment at the start of every build session:

```powershell
. .\emsdk_env.ps1
```

---

## Script-based quick start (`x` / `raw`)

### 2 — Clone Volt

```powershell
git clone https://github.com/vdcoder/volt.git
```

### 3 — Create a new Volt app

```powershell
.\volt\framework\user-scripts\create-volt-app.ps1 my-app
```

This scaffolds a complete app at `..\my-app` relative to the repo root.  
The default template is **Volt X** (DSL with Python preprocessor).

For a raw C++ template (no preprocessor):

```powershell
.\volt\framework\user-scripts\create-volt-app.ps1 my-app -Template raw
```

### 4 — Build

```powershell
cd my-app

# Activate Emscripten (once per session)
. <path-to-emsdk>\emsdk_env.ps1

# Build
.\build.ps1
```

### 5 — Run

```powershell
cd output
python -m http.server 8001
```

Open → **http://localhost:8001**

Your first C++ WebAssembly app is live on Windows. ⚡

---

## Script Reference

### `create-volt-app.ps1`

```
SYNOPSIS
    Create a new Volt app from a template.

USAGE
    .\create-volt-app.ps1 <AppName> [options]

PARAMETERS
    -AppName      Name of the new app (required, positional)
    -Guid         VOLT_GUID override (default: same as AppName)
    -OutputDir    Where to create the app (default: <repo>/../<AppName>)
    -Template     "x" (default), "raw", or "x+"
    -NoGit        Skip git init

EXAMPLES
    .\create-volt-app.ps1 my-app
    .\create-volt-app.ps1 my-app -Template x -Guid my-app-v1
    .\create-volt-app.ps1 my-app -Template 'x+' -NoGit
    .\create-volt-app.ps1 my-app -OutputDir C:\Projects\my-app -NoGit
```

For `x+`, the creator produces a solution, Client/Server projects, isolated Debug
and Release outputs, and local framework/server dependencies. Its detailed
setup and file reference is in the [X+ guide](app-template-x-plus/README.md).

For the existing `x` and `raw` layout, it:

- Copies the chosen template (`app-template-x/` or `app-template/`)
- Substitutes `VOLT_APP_NAME_CAMEL`, `VOLT_APP_NAME_UNDERSCORE`, `VOLT_APP_NAME` tokens
- Copies the framework headers to `dependencies/volt/include/`
- Copies `volt.js` to `output/`
- (X template) Copies the Windows-patched `preprocesor.py`
- Generates a `build.ps1` for the app
- Optionally runs `git init` with an initial commit

### `build.ps1`

Generated for the existing script-based layout by `create-volt-app.ps1`.
X+ instead uses `tools/build-client.cmd` and `tools/build-client.ps1` through VS;
see [its build guide](app-template-x-plus/README.md).

```
SYNOPSIS
    Build this Volt app for WebAssembly.

USAGE
    .\build.ps1 [-Guid <guid>]

PARAMETERS
    -Guid    VOLT_GUID override (default: baked in at app creation time)
```

What it does:

1. Runs the Python preprocessor on all `*.x.*` source files → `_generated/src/`
2. Writes a Clang response file (`_build_flags.rsp`) for GUID quoting
3. Invokes `emcc` with all the right flags
4. Copies `index.html` and `global.css` to `output/`

---

## Windows-Specific Notes

### Activation is per-session

`emsdk_env.ps1` sets environment variables for the current PowerShell session only.  
You must source it again if you open a new terminal:

```powershell
. C:\path\to\emsdk\emsdk_env.ps1
```

### ExecutionPolicy

If PowerShell blocks the scripts, allow them for the current session:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
```

### GUID quoting (how it works under the hood)

Passing `-DVOLT_GUID="my-app"` through PowerShell's argument list strips the inner quotes before they reach the compiler.  
`build.ps1` works around this by writing `-DVOLT_GUID=\"my-app\"` to an ASCII **response file** (`_build_flags.rsp`) and passing `@_build_flags.rsp` to `emcc`. Clang reads response files directly, so the quotes survive intact.

You do not need to do anything special — this is handled automatically.

### Python stdout on Windows (preprocessor)

By default, Windows Python's stdout can use UTF-16LE or collapse newlines when redirected through PowerShell.  
The patched `preprocesor.py` accepts an output **file path** as its second argument and writes UTF-8 directly, bypassing PowerShell I/O entirely:

```python
# Windows fix: write output to file path rather than stdout redirect
if len(sys.argv) >= 3:
    with open(sys.argv[2], 'w', encoding='utf-8') as f:
        f.write(output)
```

Again, handled automatically — `build.ps1` passes the output path to the preprocessor.

---

## Known Limitations

| Limitation | Detail |
|------------|--------|
| **Memory mode differs by template** | X+ uses MEMORY64 (`-m64`) with the Embind pointer compatibility fix. The existing Windows `build.ps1` remains wasm32. |
| **Build entry points** | X+ builds through its Visual Studio solution or `tools/build-client.cmd`. The existing Windows script workflow uses `build.ps1`; Unix `build.sh` requires a suitable shell/toolchain. |
| **ExecutionPolicy** | Some machines require `Set-ExecutionPolicy Bypass -Scope Process` to run `.ps1` scripts. |

---

## Troubleshooting

### `emcc not found`
You need to activate Emscripten first:
```powershell
. C:\path\to\emsdk\emsdk_env.ps1
```

### `Preprocessor failed`
Make sure `$env:EMSDK_PYTHON` is set (it is after activating emsdk). The build script uses the emsdk-bundled Python, which has all required modules.

### `Cannot be loaded because running scripts is disabled`
```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
```

### App loads but shows JS errors
For `x`, serve `output/`. X+ serves `output/Debug/web/` or `output/Release/web/`
through its native server. The folder must contain `volt.js`, `app.js`,
`app.wasm`, and `index.html`. Use HTTP rather than opening HTML with `file://`.

### Build output directory is stale
X+ Clean/Rebuild reconstructs all browser assets from their source copies.
The existing `x` build regenerates `_generated/` and overwrites compiled output,
but its creator initially installs `volt.js` into `output/`: preserve that file
or restore it from `framework/src/volt.js` if manually clearing the old layout.

---

## Project layout (`x` on Windows)

After running `create-volt-app.ps1 my-app`:

```
my-app/
├── src/
│   ├── App.x.hpp          ← Your UI component (Volt X DSL)
│   ├── main.x.cpp         ← Entry point + EMSCRIPTEN_BINDINGS
│   └── components/        ← Additional components
├── dependencies/
│   └── volt/
│       └── include/       ← Framework headers (Volt.hpp, etc.)
├── output/                ← Served to the browser
│   ├── index.html
│   ├── global.css
│   ├── volt.js            ← Volt runtime bootstrap
│   ├── app.js             ← Generated by emcc
│   └── app.wasm           ← Compiled C++ module
├── _generated/            ← Preprocessor output (do not edit)
├── preprocesor.py         ← Windows-patched X preprocessor
├── build.ps1              ← Windows build script  ← use this
├── build.sh               ← Linux/macOS build script
├── index.html
└── global.css
```

---

## Contributing Windows Improvements

If you find issues specific to Windows, please open a GitHub issue and include:

- Windows version (`winver`)
- PowerShell version (`$PSVersionTable.PSVersion`)
- Emscripten version (`emcc --version`)
- The full error output

PRs for Windows fixes are very welcome. See **CONTRIBUTING.md** for guidelines.

---

*Windows support contributed by the community. Tested on Windows 10/11 with PowerShell 5.1 and emsdk latest.*
