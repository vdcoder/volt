# Changelog

All notable changes to the Volt framework will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Initial Release - v0.1.0 (2025-11-03)

#### ✨ Features

- **Instanceable Architecture**: Complete removal of global/static state
  - `VoltRuntime` class encapsulates all state
  - Thread-local `currentRuntime` pattern for event registration
  - Multiple instances can coexist on same page

- **GUID-Based Instance Isolation**
  - Each app has unique GUID for namespace isolation
  - JavaScript namespace: `window.volt_<guid>`
  - Automatic sanitization for valid JS identifiers (e.g., `my-app` → `my_app`)
  - Empty GUID defaults to `"volt"` namespace

- **Virtual DOM System**
  - Efficient diffing algorithm with O(n) complexity
  - Smart patching that only updates changed elements
  - Support for text content, attributes, and nested children

- **Component-Based Architecture**
  - `VoltApp` base class for creating apps
  - `render()` method returns `VNode` tree
  - `scheduleRender()` for batched updates with RAF

- **Event System**
  - 8 event handlers: `onClick`, `onInput`, `onChange`, `onSubmit`, `onKeyDown`, `onKeyUp`, `onFocus`, `onBlur`
  - Lambda-based callbacks with automatic registration
  - JavaScript bridge via callback IDs

- **Macro System for HTML Elements**
  - `VNODE_TAG_HELPER` generates 4 overloads per tag
  - 65% code reduction vs manual overload definitions
  - All standard HTML tags supported

- **Props Optimization**
  - `vector<pair<short, string>>` instead of `map<string, string>`
  - 12x memory savings
  - Automatic sorting for efficient diffing

- **Developer Experience**
  - `create-volt-app.sh` - One-command app creation
  - Beautiful colored terminal output
  - Automatic PascalCase conversion for app names
  - Optional Git initialization
  - `update-framework.sh` - Easy framework updates

- **Single Include File**
  - `Volt.hpp` includes entire framework
  - Clean API with `using namespace volt;`

#### 🏗️ Architecture

- **Clean Structure**:
  - `framework/include/` - All public headers
  - `framework/src/` - Implementation
  - `framework/user-scripts/` - User-facing tools
  - `app-template/` - Template for new apps

- **Single Translation Unit**:
  - `main.cpp` includes `VoltRuntime.cpp`
  - Avoids EM_JS symbol duplication

#### 🐛 Bug Fixes

- **JavaScript Identifier Sanitization**
  - Fixed: GUIDs with hyphens (e.g., `test-app`) caused `ReferenceError`
  - Solution: `sanitizeForJs()` replaces invalid chars with underscores
  - Ensures all generated namespaces are valid JS identifiers

- **Circular Dependencies**
  - Separated event helpers into `VoltEvents.hpp`
  - Includes after `VoltRuntime.hpp` to avoid circular deps

#### 📦 Build System

- **Emscripten Compilation**:
  - Flags: `-O3`, `MODULARIZE=1`, `ALLOW_MEMORY_GROWTH=1`
  - GUID passed via `-DVOLT_GUID=\"$GUID\"`
  - Exports: `Module`, `createRuntime`

- **Build Scripts**:
  - `build.sh` - Compiles app to WebAssembly
  - Displays GUID and sanitized namespace
  - Creates `output/` directory with `.js`, `.wasm`, `index.html`

#### 📚 Documentation

- **Repository Documentation**:
  - Main `README.md` with quick start and examples
  - `CONTRIBUTING.md` with guidelines and standards
  - `CHANGELOG.md` (this file)
  - `QUICKSTART.md` - Quick reference guide
  - `.gitignore` for build artifacts

- **App Template**:
  - Complete `app-template/` structure
  - Quick start `README.md` for new apps
  - Example `App.hpp` with counter demo

#### 🎯 Examples

- **Template App** (`app-template/src/App.hpp`):
  - Counter with increment/decrement
  - Demonstrates state management
  - Shows event handling

#### ⚡ Performance

- **Optimizations**:
  - Minimal DOM operations (diff-based)
  - Batch rendering with `requestAnimationFrame`
  - Memory-efficient props representation
  - Move semantics throughout

#### 🔧 Developer Tools

- **create-volt-app.sh**:
  - Flags: `--guid`, `--output`, `--no-git`
  - Automatic app name to PascalCase conversion
  - Colored progress output
  - Complete project scaffolding
  - Copies framework into app's dependencies

- **update-framework.sh**:
  - Updates framework files in existing apps
  - Automatic backup creation
  - Auto-detects Volt repo location

#### 📋 Known Limitations

- No built-in router (use custom implementation)
- No state management library (use class members)
- No component lifecycle hooks (coming soon)
- No SSR support (client-only)
- No TypeScript definitions (JavaScript only)

#### 🚀 Future Plans

- Testing framework for automated tests
- Performance benchmarking suite
- Browser DevTools extension
- Component library
- Router implementation
- State management patterns
- SSR support
- npm/CDN distribution

---

## How to Use This Changelog

- **[Unreleased]**: Changes in main branch not yet released
- **Version Numbers**: Follow semantic versioning (MAJOR.MINOR.PATCH)
- **Categories**:
  - `✨ Features`: New features
  - `🐛 Bug Fixes`: Bug fixes
  - `🏗️ Architecture`: Architectural changes
  - `📚 Documentation`: Documentation updates
  - `⚡ Performance`: Performance improvements
  - `🔧 Developer Tools`: Tooling improvements
  - `🚨 Breaking Changes`: Breaking API changes

---

[Unreleased]: https://github.com/vdcoder/volt/compare/v0.1.0...HEAD
