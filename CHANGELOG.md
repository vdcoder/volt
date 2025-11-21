# Changelog

All notable changes to the Volt framework are documented in this file.

Format follows **Keep a Changelog**  
Project follows **Semantic Versioning**  
https://keepachangelog.com/en/1.0.0/  
https://semver.org/spec/v2.0.0.html

---

## [Unreleased]

---

## [0.2.0] – 2025-11-20  
### The *Structural Reuse* + *X-DSL* Release  
(Branch: `VDIAZ-REPURPOSING-FIX`)

This release represents a **major architectural breakthrough** for Volt:  
stable identity, surgical DOM reuse, preprocessable JSX-like syntax, and a modern JS bootstrap API.

---

### ✨ New Features

#### **X-DSL (Volt Extended Syntax)**
A new JSX-inspired declarative syntax:

```cpp
<div({ style:=("padding:10px") },
    <h1("Hello")/>,
    <map(items, [](auto& x){ return <li(x)/>; })/>
)/>
```

Includes:

- `<tag(...) />` template expansion  
- `<map(...)/>` with automatic tracking  
- `<loop(...)/>` repeating renderer  
- `<code(fn)/>` inline logic  
- `<props(fn)/>` dynamic attributes  
- `<( ... )/>` fragments  
- `<render(...) />` component calls  
- `<:= (...) />` expression inserts  

All transformed into raw C++ via the new Python preprocessor.

---

#### **Python Preprocessor**
A complete preprocessing tool:

- Converts X-DSL into valid C++17  
- Injects `.track(__COUNTER__)` automatically  
- Maintains line-to-line stability for debugging  
- Handles nested tags, props, map/loop, expressions, and fragments  
- Designed to be deterministic, minimal, and predictable

---

#### **Stable Identity Model (Track System)**
New multi-axis identity system:

- `.track(__COUNTER__)` inserted automatically  
- Identity derived from lexical/component location  
- Supports key overrides with `key:=("id")`  
- Deeply stable across renders

This allows **perfect physical node reuse**, even across massive structural changes.

---

#### **Structural Reuse Algorithm (Surgical DOM Diff-Patch)**
Volt now performs **true DOM-preserving updates**:

- Reuse DOM nodes by identity  
- Move instead of recreate  
- DOM removal only when strictly necessary  
- Lifecycle hooks around add → move → remove  
- Preserves scroll, selection, canvas contexts, WebGL state, and JS-attached data

This is the core of Volt’s performance model.

---

#### **Element Lifecycle Hooks**
Four new DOM-level lifecycle callbacks:

- `onAddElement`
- `onRemoveElement`
- `onBeforeMoveElement`
- `onMoveElement`

Perfect for:

- canvas/WebGL pairing  
- animations  
- scroll preservation  
- debugging  
- third-party widget interop  

---

#### **VoltBootstrap.js**
A clean JS loader to replace inline code:

- Automatic event bubbling  
- Error overlay support  
- Debug logging  
- Modular loader independent of HTML  
- Fully compatible with Emscripten’s modularized output  
- Supports future `createRuntime(rootId)` enhancements

---

#### **Dual App Templates**
Two official templates:

- **raw/** — classical C++ API
- **x/** — preprocessor-powered X-DSL

`create-volt-app.sh` now supports:

```
./create-volt-app.sh myapp --template x
./create-volt-app.sh myapp --template raw
```

(X-template is the *default*.)

---

### 🧹 Improvements

- `.track()` usage simplified (macro-ready)
- Clean separation of X template and raw template
- Vastly improved readability and developer ergonomics
- Structural reuse fixes for fragments and reordered siblings
- 100% stable DOM node reuse across tree reshuffling

---

### 🐛 Bug Fixes

- Fixed issues where DOM elements lost scroll position after movement  
- Fixed stale DOM pointer problems after deep structural updates  
- Fixed repurposing bug where incorrectly matched nodes caused resets  
- Fixed rare cases of event handlers binding twice or not detaching on removal  
- Preprocessor now correctly skips:
  - strings  
  - nested parentheses  
  - block / line comments  
  - escaped characters  

---

### 📚 Documentation

Major doc overhaul:

- New **QUICKSTART**, **COMPONENTS**, **ADVANCED**, **CONTRIBUTING**  
- Documentation now X-DSL-first  
- New explanations of identity, surgery, lifecycle hooks  
- Template readmes updated  
- Added VoltBootstrap integration section

---

### 🔧 Build System

- X-template build now:
  - preprocesses all `*.x.cpp` and `*.x.hpp`
  - copies final results to `_generated/`  
  - compiles using Emscripten from the generated folder

- Raw template unchanged, compiles from `src/`

---

### 🚨 Breaking Changes

- Raw tag API still supported but no longer recommended  
- Apps using pre-0.2.0 identity assumptions must adopt `.track()` or `<key:=...>`  
- Inline JS loader removed in favor of `volt-app.js`

---

## [0.1.0] – 2025-11-03  
### Initial Release

(Your original changelog content remains intact here.)

---

## [Unreleased]

- Additional X-DSL features  
- Better C++ error messaging for preprocessor output  
- Component-level partial updating  
- DevTools inspector for identity/movement visualization  
- Scoped CSS / shadow-root mounting  
- Hydration from static markup  

---

## Version History Links

[0.2.0]: https://github.com/vdcoder/volt/compare/v0.1.0...v0.2.0  
[0.1.0]: https://github.com/vdcoder/volt/releases/tag/v0.1.0  
[Unreleased]: https://github.com/vdcoder/volt/compare/v0.2.0...HEAD
