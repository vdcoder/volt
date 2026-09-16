# ⚡ Volt

**A modern C++ WebAssembly UI framework powered by identity-preserving diffing, a declarative DSL, and the Structural Reuse Algorithm.**

Volt brings declarative UI to native C++ — with stable DOM identity, surgical updates, and a JSX-inspired syntax that compiles into pure C++.

If you ever wished the web had a first-class, modern, elegant, fast C++ UI framework…

**Welcome home.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++: 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Bits: MEMORY64](https://img.shields.io/badge/Bits-MEMORY64-purple.svg)](https://webassembly.github.io/spec/)
[![Platform: WebAssembly](https://img.shields.io/badge/WebAssembly-Enabled-purple.svg)](https://webassembly.org/)
[![Platform: Windows](https://img.shields.io/badge/Windows-PowerShell-blue.svg)](WINDOWS.md)

---

# 🌟 What Makes Volt Different?
Volt is built on three uncompromising principles:

- Identity correctness first.
- Focus protection first.
- Structural stability first.

These guarantees give Volt a natural “browser-native” feel—scroll positions remain, focus never jumps, text selections stay intact, and widgets behave exactly as users expect.

---

### ⚡ Volt X DSL  
A minimal, expressive, JSX-like syntax compiled directly into C++:

```cpp
<div({ style:=("padding:20px") },
    <h1("Hello World")/>,
    <button({ onClick:=([this](auto){ count++; }) }, "Click")/>
)/>
```
- Zero runtime parsing
- No overhead beyond normal C++ construction
- Clean, predictable preprocessor output
- Header-only (#include <Volt.hpp>)

Volt X gives you declarative UI with the performance profile of handcrafted C++.

---

### 🧠 The Structural Reuse Algorithm (SRA) 
Volt’s diffing engine performs a deterministic paired walk of old and new VTrees, ensuring:
- Stable DOM identity
- Minimal DOM operations
- No unnecessary element moves
- Perfect focus retention
- Predictable, low-cost updates regardless of UI size

### Why Volt’s Algorithm Is Different

Most UI libraries treat reordering as the central case. Volt does not.

In real applications, the dominant pattern is **conditional rendering**:

```cpp
if (showDetails) {
    <Details/>
}
```

When a section appears or disappears, the “correct” identity is almost always still present among the old node’s **sibling list**—just slightly below the current position.

Volt is optimized for this case:

### Dominant Case: Match Found in Siblings Below

Volt:
- Unlinks old nodes that are no longer needed
- Promotes the matching old sibling upward
- Avoids DOM moves entirely
- Preserves identity, scroll position, focus, widgets, and internal state

This behavior makes updates feel native and effortless for the browser.

### Secondary Cases

When the developer intentionally reorders keyed or identified elements:
- Volt pulls elements from unclaimedOldNodes
- Or relocates nodes from elsewhere in the tree
- And applies minimal physical DOM movement

### Identity Rules

Volt supports:
- key — stable identity across reordering
- id — global identity with strict runtime uniqueness
- Both work seamlessly with conditional logic and list operations

Volt guarantees the closest behavior to vanilla HTML/JS, while providing a declarative C++ rendering model.

---

### 🔁 Mutability Is Back!

Volt embraces **mutable state** without dirty-tracking or reactivity systems.

When an event fires:
- Volt re-runs render()
- Rebuilds a lightweight VTree
- And applies the Structural Reuse Algorithm

No expensive effect graphs.
No memoization math.
No stale closures.
No developer micromanagement.

Under the hood:
- VNodes are reused
- Attributes and tags use compact short codes
- UTF-8 strings avoid browser overhead
- Fragment nodes are flattened pre-diff
- Text nodes use specialized fast paths
- Memory allocations are minimized

The result: simple user code with native-level performance.

---

### 🧩 Component Architecture  
- Any function returning a ```VNodeHandle``` is a component
- Stateless or stateful styles supported
- Safe, minimal runtime API (```IRuntime*```)  
- Zero cost when unused
- No hidden magic, everything compiles to clear C++

---

### 🌐 Multiple Instances  
Volt apps are **fully instanceable**.  
Run one, two, or a hundred applications on the same page, each isolated by a GUID.

---

### 🛠️ Developer Experience  
- `create-volt-app.sh` — instant project scaffolding (Linux / macOS)  
- `create-volt-app.ps1` — native Windows PowerShell scaffolding  
- Choose **Raw C++** or **Volt X** template
- Or choose **[Volt X+](app-template-x-plus/README.md)** (`-Template 'x+'`) for a Visual Studio 2026 solution, Debug/Release browser builds, and a native Windows HTTP/WebSocket server.
- Simple Python preprocessor
- Clear, debuggable output
- Clean project layout
- Browser bootstrap handled automatically by ```volt.js```

---

# 🚀 Quick Start

## Choose a template

| Template | What you get | Workflow |
|---|---|---|
| `x` (default) | Volt X DSL client and Python preprocessor | Script build; serve the browser output |
| `raw` | Plain C++ client sources without the DSL preprocessing step | Script-based starter |
| `x+` | Volt X client, native Seasocks HTTP/WebSocket server, and a Visual Studio solution | Windows / Visual Studio 2026; Debug and Release |

### Volt X+ with Visual Studio 2026

With Git, Python 3, an activated Emscripten SDK, and VS 2026's Desktop development
with C++ workload installed, run from your projects folder:

```powershell
git clone https://github.com/vdcoder/volt.git
.\volt\framework\user-scripts\create-volt-app.ps1 my-app -Template 'x+' -OutputDir .\my-app
```

Open `my-app/my-app.sln` in Visual Studio with the SDK environment available.
Select Debug or Release, build with **Ctrl+Shift+B**, and run **Server** with **F5**.
Browse [http://127.0.0.1:8000/](http://127.0.0.1:8000/).

X+ ships the client UI, server, build tools, and local Volt/Seasocks dependencies.
It uses MEMORY64, preserves compiler source locations with `#line`, and includes
a WebSocket echo endpoint at `/ws`.
It also includes a shared standard C++ dependency container, separate client/server
`AppDI` classes, and global `services()` access. On the client, `invalidate()`
requests a render frame from anywhere after startup, including async callbacks.
Each WebAssembly instance owns its own services and runtime.
See the **[Volt X+ guide](app-template-x-plus/README.md)** for SDK setup, creation
options, folder layout, configuration outputs, and troubleshooting. The
[application services guide](app-template-x-plus/README.md#application-services)
covers registration, access, and teardown.

The following script-based quick starts use the default `x` template.

## Linux / macOS

### 1. Install Emscripten

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### 2. Create a Volt App

```bash
git clone https://github.com/vdcoder/volt.git
./volt/framework/user-scripts/create-volt-app.sh my-app
```

### 3. Build & Run

```bash
cd my-app
./build.sh
cd output
python3 -m http.server 8001
```

Open → **http://localhost:8001**

Your first C++ web app is live.

---

## 🪟 Windows

### 1. Install Emscripten

```powershell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk install latest
.\emsdk activate latest
. .\emsdk_env.ps1
```

### 2. Create a Volt App

```powershell
git clone https://github.com/vdcoder/volt.git
.\volt\framework\user-scripts\create-volt-app.ps1 my-app
```

### 3. Build & Run

```powershell
cd my-app
. <path-to-emsdk>\emsdk_env.ps1   # activate Emscripten
.\build.ps1
cd output
python -m http.server 8001
```

Open → **http://localhost:8001**

Your first C++ web app is live on Windows. ⚡

> 📖 Full Windows guide, troubleshooting, and known limitations: **[WINDOWS.md](WINDOWS.md)**

---

# 🖥️ Example (Volt X DSL)

### `src/App.x.hpp`

```cpp
#include <Volt.hpp>
using namespace volt;

class CounterApp : public App {
    int count = 0;

public:
    CounterApp(IRuntime& a_runtime) : App(a_runtime) {}

    VNodeHandle render() override {
        return <div({ style:=("font-family:sans-serif; padding:20px;") },
            <h1("Count: " + std::to_string(count))/>,
            <button({ onClick:=([this]{ count++; }) }, "Increment")/>,
            <button({ onClick:=([this]{ count--; }) }, "Decrement")/>
        )/>;
    }
};
```

### `src/main.x.cpp`

```cpp
#include <Volt.hpp>
#include "App.x.hpp"
#include <emscripten/bind.h>

std::unique_ptr<volt::VoltEngine> g_voltEngine = nullptr;

EMSCRIPTEN_BINDINGS(CounterApp) {
    function("createVoltEngine", +[](std::string rootId) {
        g_voltEngine = std::make_unique<volt::VoltEngine>(rootId, "CounterApp");
        g_voltEngine->mountApp<CounterApp>();
    });
}
```

VoltBootstrap wires up DOM events, lifecycle hooks, and the runtime automatically.

---

# 🧠 How Volt Works (Lifecycle)

Volt provides precise, minimal lifecycle hooks tied to DOM identity:
- onAddElement(el) — element created and attached
- onRemoveElement(el) — element detached
- onBeforeMoveElement(el) — about to move (save browser adjustments)
- onMoveElement(el) — moved (restore browser adjustments)

Hooks are deterministic and map directly to physical DOM operations.

---

# 📂 Project Structure

```
volt/
├── framework/
│   ├── include/
│   │   ├── Volt.hpp
│   │   └── ...
│   ├── src/
│   │   └── volt.js
│   └── user-scripts/
│       ├── create-volt-app.sh
│       └── ...
├── app-template/
├── app-template-x/
└── app-template-x-plus/     # Visual Studio client + native Windows server
```

---

# 🔧 Namespacing (GUID)

Volt apps are isolated via GUID:

For X+, set `-Guid my_app_v1` during creation or edit `guid` in the generated
`app.json`, then rebuild. For the script-based templates:
```bash
VOLT_GUID='my-app' ./build.sh
```

This generates:
```javascript
window.volt_my_app
```

Each runtime remains fully isolated.

---

# 💡 Why C++ on the Web?

- Native-level performance  
- Deterministic memory model  
- No GC pauses  
- Efficient UTF-8 processing
- Perfect for simulation, dashboards, tools, games, or highly interactive UI
- Volt offers a declarative model without sacrificing control

Volt lets C++ developers feel _at home_ on the web.

---

# 🤝 Contributing

PRs and contributions are warmly welcomed.

See **CONTRIBUTING.md** for coding standards, project architecture, and contribution guidelines.

Thanks to OpenAI's Codex for collaborating on the Volt X+ template, Visual Studio integration, Seasocks server, build improvements, and documentation.

---

# 📄 License

MIT — do what you love.  
Volt is for the people.

---

# ⚡ Volt  
**C++ on the standard web — beautifully.**
