# ⚡ Contributing to Volt

Welcome! Volt is a modern C++/WebAssembly UI engine with a powerful X-DSL, a structural-reuse surgical algorithm, and a tiny runtime.  
This document serves two contributor audiences:

1. **Volt Internal Contributors** – People improving the framework itself  
2. **Volt App Developers** – People building apps using the X-DSL

Both roles are important. This guide helps you succeed in either track.

---

# 📘 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Contributor Roles](#contributor-roles)
- [Development Setup](#development-setup)
- [Contributing to the Framework (Volt Internals)](#contributing-to-the-framework-volt-internals)
  - [Framework Coding Standards](#framework-coding-standards)
  - [Project Structure](#project-structure)
  - [Testing the Framework](#testing-the-framework)
  - [Pull Request Process](#pull-request-process)
- [Contributing as a Volt App Developer](#contributing-as-a-volt-app-developer)
  - [X-DSL Best Practices](#x-dsl-best-practices)
  - [Structural Reuse Identity Model](#structural-reuse-identity-model)
  - [Component Authoring](#component-authoring)
  - [Lifecycle Hooks](#lifecycle-hooks)
  - [Performance Guidelines](#performance-guidelines)
- [Areas to Contribute](#areas-to-contribute)
- [Getting Help](#getting-help)

---

# 🧭 Code of Conduct

Volt aims to be:

- respectful  
- helpful  
- inclusive  
- collaborative  

We welcome all contributors. Treat each other with kindness and curiosity.

---

# 👥 Contributor Roles

Volt has **two contributor types**, each equally valued.

## **1. Framework Contributors**  
People modifying:
- VoltRuntime internals  
- Structural Reuse Surgical diff algorithm  
- DOM patching logic  
- Binding layer (JavaScript glue)  
- Compiler/preprocessor for X-DSL  
- Build scripts and templates  

These changes affect *everyone*, so higher rigor applies.

## **2. Volt App Developers**  
People building applications *using* Volt.  
Your contributions often include:
- demos  
- templates  
- reusable components  
- docs  
- bug reports  
- DSL usage improvements  

This category is more flexible and friendly to newcomers.

---

# 🛠️ Development Setup

Required:

- Emscripten (emsdk)  
- C++17 toolchain  
- Python 3  
- Git  
- A browser (Chrome, Firefox, Safari)  

## Linux / macOS

Install Emscripten:

```bash
git clone https://github.com/emscripten-core/emsdk
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Test:

```bash
./volt/framework/user-scripts/create-volt-app.sh demo
cd demo
./build.sh
cd output
python3 -m http.server 8001
```

## 🪟 Windows (PowerShell)

Install Emscripten:

```powershell
git clone https://github.com/emscripten-core/emsdk
cd emsdk
.\emsdk install latest
.\emsdk activate latest
. .\emsdk_env.ps1
```

Test:

```powershell
.\volt\framework\user-scripts\create-volt-app.ps1 demo
cd demo
. <path-to-emsdk>\emsdk_env.ps1
.\build.ps1
cd output
python -m http.server 8001
```

> See **[WINDOWS.md](WINDOWS.md)** for the full Windows guide, known limitations, and troubleshooting.

---

# 🔧 Contributing to the Framework (Volt Internals)

The Volt internals are compact but powerful. Changes here influence rendering, performance, identity, X-DSL rules, and diff behavior.

---

## Framework Coding Standards

### **Language**
- C++17  
- Minimize macros  
- Prefer templates and inline functions  

### **Naming**
- **Classes**: `PascalCase`  
- **Functions**: `camelCase`  
- **Members**: `camelCase`  
- **Constants/Macros**: `UPPER_SNAKE_CASE`  
- **DSL tags/attrs** follow `volt::tag::div`, `volt::attr::onClick`

### **Style**
- 4-space indentation  
- No tabs  
- 100-char line limit  
- RAII for allocations  
- No raw `new`/`delete`  
- Use `std::vector`, `std::string`, and move semantics aggressively  

### **Comments**
Explain *why* something is done, not *what*.

Good:

```cpp
// Structural-reuse heuristic: move node instead of delete+create
```

Bad:

```cpp
// Loop through children
```

---

## Project Structure

```
volt/
├── framework/
│   ├── include/
│   │   ├── Volt.hpp
│   │   ├── VoltRuntime.hpp
│   │   ├── VNode.hpp
│   │   ├── Diff.hpp
│   │   ├── Patch.hpp
│   │   ├── Attrs.hpp
│   │   ├── VoltEvents.hpp
│   │   ├── VoltConfig.hpp
│   │   └── String.hpp
│   ├── src/
│   │   └── VoltRuntime.cpp
│   └── user-scripts/
│       └── create-volt-app.sh
├── app-template/
├── app-template-x/
└── docs/
```

Framework contributors should familiarize themselves especially with:

- `VNode` – identity, children, props  
- `Diff.hpp` – structural reuse surgical algorithm  
- `Patch.hpp` – applies DOM operations  
- `VoltRuntime` – render pipeline, `.requestRender()`  
- Preprocessor (in Python) for X-DSL  

---

## Testing the Framework

Before submitting a PR, ensure:

- A new app created with both templates builds successfully  
- The X-DSL correctly expands via the Python preprocessor  
- Identity model works (`.track(__COUNTER__)`)  
- map / loop / code / props behave correctly  
- No regressions in event bubbling  
- DOM patching still works in real browsers  
- No JS console errors  
- No memory leaks in val<->C++ interactions  

---

## Pull Request Process

1. Keep PRs focused on a single responsibility  
2. Update docs if behavior changes  
3. Update CHANGELOG.md  
4. Provide benchmarks if performance-sensitive  
5. Avoid introducing unnecessary abstractions  
6. Squash commits if requested  

Commit message style:

- `feat:` new feature  
- `fix:` bug  
- `perf:` performance improvement  
- `refactor:` implementation change  
- `docs:` documentation update  
- `chore:` housekeeping  

---

# 🎨 Contributing as a Volt App Developer

Most contributors belong here. These guidelines help you build clean, stable, fast Volt apps using the X-DSL.

---

## X-DSL Best Practices

Volt X reduces boilerplate and improves readability:

```cpp
return <div({style:=("padding:20px;")}, 
    <h1("Hello")/>,
    <button({onClick:=([this]{ clicked++; })}, "Click")/>
)/>;
```

### Tips

✔ indentation matters—keep tags aligned  
✔ group props using `{ ... }`  
✔ use meaningful variable names inside `<code(...)>`  
✔ prefer `<map(...)>` for lists  
✔ prefer `<loop(n)>` instead of manual for-loops  
✔ avoid large C++ blocks inside DSL nodes—factor them out  

---

## Structural Reuse Identity Model

Volt uses a surgical diff algorithm. Stability comes from:

- **Structural identity** (position within siblings)  
- **Explicit identity** via `.track(...)` prefix  
- **Stable keys** supplied by the DSL (array index in `map`, loop counter)  

Preprocessor automatically injects:

```
.track(__COUNTER__)
```

You do not add this manually.

### Rules for developers

✔ keep list order stable  
✔ prefer `<map>` over manual loops  
✔ use stable data keys for dynamic lists  
✔ fragments `<()>` are cheap and encouraged  

---

## Component Authoring

Component patterns:

- **Stateless (inline)**  
- **Stateful (class member)**  

Both can use X-DSL inside `.render(...)`.

Example:

```cpp
VNodeHandle render(const std::string& label) {
    return <button({onClick:=([this]{ count++; })}, label)/>;
}
```

---

## Lifecycle Hooks

Volt X supports attribute-style lifecycle callbacks:

- `onAddElement(elem, parent)`  
- `onRemoveElement(elem)`  
- `onBeforeMoveElement(elem, newParent)`  
- `onMoveElement(elem, newParent)`  

These are powerful for animations, Portal patterns, custom transitions, etc.

---

## Performance Guidelines

✔ prefer fragments (`<()>`) to avoid wrapper elements  
✔ prefer `<map>` (C++ loop → fragment)  
✔ prefer component instances only when state must persist  
✔ avoid large objects in VNode props  
✔ limit JS↔WASM event chatter  
✔ don’t allocate strings in tight loops—reuse where possible  

---

# 🧭 Areas to Contribute

### High Priority
- Stable JS bootstrap file  
- Improved DOM lifecycle integration  
- More DSL primitives  
- Performance metrics  
- Animation support  

### Medium
- Component library  
- DevTools panel  
- Advanced examples  

### Low
- SSR experiments  
- CDN packaging  
- Vite/Webpack templates  

---

# 📞 Getting Help

- GitHub Issues  
- GitHub Discussions  
- Future Discord community  

Volt welcomes you — whether you're hacking the runtime or building your first X-DSL app.  
**Thank you for helping shape the future of Volt! ⚡**
