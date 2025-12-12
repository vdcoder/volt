# ⚡ Volt

**A modern C++ WebAssembly UI framework powered by identity-preserving diffing, a declarative DSL, and the Structural Reuse Algorithm.**

Volt brings declarative UI to native C++ — with stable DOM identity, surgical updates, and a JSX-inspired syntax that compiles into pure C++.

If you ever wished the web had a first-class, modern, elegant, fast C++ UI framework…

**Welcome home.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++: 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Bits: MEMORY64](https://img.shields.io/badge/Bits-MEMORY64-purple.svg)](https://webassembly.github.io/spec/)
[![Platform: WebAssembly](https://img.shields.io/badge/WebAssembly-Enabled-purple.svg)](https://webassembly.org/)

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
- `create-volt-app.sh` — instant project scaffolding  
- Choose **Raw C++** or **Volt X** template  
- Simple Python preprocessor
- Clear, debuggable output
- Clean project layout
- Browser bootstrap handled automatically by ```volt.js```

---

# 🚀 Quick Start

## 1. Install Emscripten

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## 2. Create a Volt App

```bash
git clone https://github.com/vdcoder/volt.git
./volt/framework/user-scripts/create-volt-app.sh my-app
```

## 3. Build & Run

```bash
cd my-app
./build.sh
cd output
python3 -m http.server 8001
```

Open → **http://localhost:8001**

Your first C++ web app is live.

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
└── app-template-x/
```

---

# 🔧 Namespacing (GUID)

Volt apps are isolated via GUID:
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

---

# 📄 License

MIT — do what you love.  
Volt is for the people.

---

# ⚡ Volt  
**C++ on the standard web — beautifully.**
