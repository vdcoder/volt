# ⚡ Volt

**A modern C++ WebAssembly UI framework powered by the Structural Reuse Algorithm and the Volt X DSL.**

Volt brings **declarative UI**, **surgical DOM diffing**, and **component-based architecture** to C++ — with a JSX-inspired syntax, a clean preprocessor pipeline, and deep identity-preserving updates that make your UI feel instant and rock-solid.

If you ever wished the web had a first-class C++ framework that feels modern, elegant, and *fast*…

**Welcome home.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++: 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Bits: MEMORY64](https://img.shields.io/badge/Bits-MEMORY64-purple.svg)](https://webassembly.github.io/spec/)
[![Platform: WebAssembly](https://img.shields.io/badge/WebAssembly-Enabled-purple.svg)](https://webassembly.org/)

---

# 🌟 What Makes Volt Different?

### ⚡ Volt X DSL  
A minimal, fast, JSX-like syntax for C++:

```cpp
<div({ style:=("padding:20px") },
    <h1("Hello World")/>,
    <button({ onClick:=([this](auto){ count++; }) }, "Click")/>
)/>
```

Zero runtime overhead.  
Coverted into pure C++ via a lightweight preprocessor.
Header only C++ library <volt.hpp>.

---

### 🧠 The Structural Reuse Algorithm  
Volt surgically updates the DOM using **identity-preserving diffing**, ensuring:

- DOM nodes are stable, their identity is fixed, never re-created
- this means element "things" stay intact: scroll position, focus, text selection, etc.
- it's the closest behaviour to vanila HTML/JS, great for third-party JS widgets (or C++)
- similar to React, the "key" prop supports stable identity across re-orderings, via user data
- additionally, the "id" prop automatically gives stable identity, regardless of location, useful when elements move but are the same, for example a canvas or a rich editor
- in short: minimal, predictable and focus-friendly DOM updates

---

### 🧠 Mutability Is Back!

Live your life, modify your memory. Volt does not track changes, instead it efficiently re-renders anytime an event is called, this means:

- no re-rendering nightmares on multiple React effect setups
- end of rendering optimizations at the user code, such as memoing blocks or callbacks
- from external codes, invalitate() can be called to schedule a re-render
- want to write your own change tracker? simply turn off AUTO_INVALIDATE_ON_EVENTS

---

### 🧩 Functional Component Architecture  
- Simple and flexible, anything returning a VNodeHandle is a component
- Support for stateless and stateful components
- Completely safe runtime interface via `IRuntime*`  
- Zero cost when unused

---

### 🌐 Multiple Instances  
Volt apps are **fully instanceable**.  
You can run two, ten, or a hundred Volt runtimes on the same page — each isolated by its GUID.

---

### 🛠️ Developer Experience  
- `create-volt-app.sh` — instant app scaffolding  
- Choose between **raw C++ template** or **X template**  
- Modern JS bootstrap `volt.js`  
- Predictable, transparent, debug-friendly preprocessor output  
- Clean project structure

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

## 2. Create Your App

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

Done. Your first C++ web app is running.

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

VoltBootstrap handles events and setup automatically.

---

# 🧠 How Volt Works

1. You implement a `render()` method that returns a **VNode tree**  
2. Volt compares new vs old using the **Structural Reuse Algorithm**  
3. Matching nodes are sycned  
4. Moved nodes are **physically relocated** in the DOM  
5. Changed props are patched  
6. Removed nodes are gone
7. DOM remains stable and predictable
8. lifecycle hooks are triggered

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

# 📘 Essential Guides

- **QUICKSTART.md** — jump in fast  
- **COMPONENTS.md** — stateless & stateful components  
- **ADVANCED.md** — identity, movement, lifecycle hooks  
- **CONTRIBUTING.md** — developing Volt itself  
- **CHANGELOG.md** — full version history  

---

# 🔧 Configuration (GUID / Namespacing)

Volt apps are isolated by GUID:

```bash
VOLT_GUID='my-app' ./build.sh
```

Automatically becomes:

```
window.volt_my_app
```

---

# 💡 Why C++ on the Web?

- Native-level performance  
- Deterministic memory model  
- No GC pauses  
- Perfect for simulation, real-time UI, dashboards, tools, games  
- And now — with Volt — a *beautiful* declarative UI model

Volt lets C++ developers feel at home on the web without switching paradigms.

---

# 🤝 Contributing

Contributions are welcome!

See **CONTRIBUTING.md** for coding standards, project layout, and PR process.

---

# 📄 License

MIT — do what you love.  
Volt is for the people.

---

# ⚡ Volt  
**C++ on the standard web — beautifully.**
