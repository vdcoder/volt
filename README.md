# ⚡ Volt

**A blazingly fast, instanceable C++ WebAssembly UI framework**

Volt is a modern, lightweight framework for building interactive web applications using C++ compiled to WebAssembly. It features a Virtual DOM for efficient updates, complete instance isolation, and an elegant component-based architecture.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Built with: C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform: WebAssembly](https://img.shields.io/badge/Platform-WebAssembly-purple.svg)](https://webassembly.org/)

## ✨ Features

- **⚡ Virtual DOM**: Efficient diffing and patching algorithm for minimal DOM updates
- **🔒 Instance Isolation**: Run multiple Volt apps on the same page with GUID-based namespacing
- **🎯 Type-Safe**: Strongly-typed C++ with compile-time safety
- **🪶 Lightweight**: Zero dependencies except Emscripten
- **🎨 Component-Based**: Build reusable, composable UI components
- **🚀 Fast**: Compiled to WebAssembly for near-native performance
- **🛠️ Developer Experience**: One-command app creation with `create-volt-app`

## 🚀 Quick Start

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (emsdk)
- C++17 compatible compiler
- Python 3 (for development server)

### Create Your First App

```bash
# Clone Volt
git clone https://github.com/vdcoder/volt.git

# Create a new app
./volt/framework/user-scripts/create-volt-app.sh my-app

# Build and run
cd my-app
./build.sh
cd output
python3 -m http.server 8001

# Open http://localhost:8001 in your browser
```

That's it! You now have a running Volt application.

## 📖 Example

Here's a complete counter app showing the full setup:

**src/App.hpp** - Your C++ Component:
```cpp
#include "../dependencies/volt/include/Volt.hpp"
using namespace volt;

class CounterApp : public VoltRuntime::AppBase {
private:
    int count = 0;
    
public:
    CounterApp(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNode render() override {
        return div({style("font-family: sans-serif; padding: 20px;")}, {
            h1({}, "Counter: " + std::to_string(count)),
            button({onClick([this]() { 
                count++; 
                invalidate();  // Triggers re-render
            })}, "Increment"),
            button({onClick([this]() { 
                count--; 
                invalidate();
            })}, "Decrement")
        });
    }
};
```

**src/main.cpp** - Wiring & Emscripten Bindings:
```cpp
#include <emscripten/bind.h>
#include "../dependencies/volt/include/Volt.hpp"
#include "../dependencies/volt/src/VoltRuntime.cpp"
#include "App.hpp"

std::unique_ptr<VoltRuntime> g_runtime;

EMSCRIPTEN_BINDINGS(counter_module) {
    function("createRuntime", +[]() {
        g_runtime = std::make_unique<VoltRuntime>("root");
        g_runtime->mount<CounterApp>();
    });
    // ... event callbacks ...
}
```

**index.html** - Loading the WebAssembly:
```html
<!DOCTYPE html>
<html>
<body>
    <div id="root"></div>
    <script src="app.js"></script>
    <script>
        VoltApp().then(module => {
            module.createRuntime();  // Mount app to #root
        });
    </script>
</body>
</html>
```

**Build & Run**:
```bash
./build.sh                    # Compiles to WebAssembly
cd output && python3 -m http.server 8001
# Open http://localhost:8001 - Click buttons to see live updates!
```

## 🏗️ How It Works

1. **Component Definition**: Create classes extending `VoltApp` with a `render()` method
2. **Virtual DOM**: `render()` returns a `VNode` tree describing your UI
3. **Efficient Updates**: When state changes, call `invalidate()` to trigger a diff
4. **Smart Patching**: Only the changed parts of the DOM are updated

## 📦 Project Structure

```
volt/
├── LICENSE                        # MIT License
├── README.md                      # This file
├── CONTRIBUTING.md                # Contribution guidelines
├── CHANGELOG.md                   # Version history
├── framework/                     # Core framework
│   ├── include/                   # Public headers
│   │   ├── Volt.hpp              # Single include file
│   │   ├── VoltRuntime.hpp       # Runtime engine
│   │   ├── VoltConfig.hpp        # Configuration
│   │   ├── VNode.hpp             # Virtual DOM nodes
│   │   ├── Attrs.hpp             # HTML attributes
│   │   ├── VoltEvents.hpp        # Event handlers
│   │   ├── Diff.hpp              # Virtual DOM diffing
│   │   ├── Patch.hpp             # DOM patching
│   │   └── String.hpp            # String utilities
│   ├── src/
│   │   └── VoltRuntime.cpp       # Runtime implementation
│   └── user-scripts/
│       ├── create-volt-app.sh    # App creation script
│       └── update-framework.sh   # Framework updater
└── app-template/                  # Template for new apps
    ├── src/
    │   ├── App.hpp               # Your app code
    │   └── main.cpp              # Boilerplate
    ├── build.sh                  # Build script
    ├── index.html                # HTML entry point
    └── README.md                 # App documentation
```

## 🎯 API Overview

### Core Components

- **`VoltApp`**: Base class for your applications
  - `render()`: Returns `VNode` tree representing UI
  - `invalidate()`: Triggers re-render on next frame

- **`VoltRuntime`**: Manages app lifecycle and rendering
  - `mount<TApp>()`: Mounts app to DOM element
  - Handles diffing, patching, and event callbacks

### Building UI

- **HTML Elements**: `div()`, `span()`, `button()`, `input()`, `h1()`-`h6()`, etc.
- **Attributes**: `id()`, `className()`, `style()`, `placeholder()`, `value()`
- **Events**: `onClick()`, `onInput()`, `onChange()`, `onSubmit()`, `onKeyDown()`, etc.

### Macro System

Volt provides overloaded helpers for concise UI code:

```cpp
// Four overload patterns for each tag
div()                          // Empty element
div({}, "Text")               // Text content
div({}, child1, child2, ...)  // Children
div(attrs, children...)        // Attributes + children
```

## 🔧 Configuration

Set your app's GUID in `build.sh`:

```bash
GUID="${VOLT_GUID:-my-app-name}"
```

Or override at build time:

```bash
VOLT_GUID='custom-id' ./build.sh
```

**Note**: GUIDs with special characters (hyphens, spaces, etc.) are automatically sanitized for JavaScript compatibility: `my-app` → `volt_my_app`.

## 🌐 Multiple Instances (Advanced)

Want to run multiple Volt apps on the same page? Each app can have its own isolated namespace using GUIDs. See [ADVANCED.md](ADVANCED.md) for details on running parallel instances.

## 🛠️ Development

### Building Your App

```bash
./build.sh
```

### Running Your App

```bash
cd output
python3 -m http.server 8001
# Open http://localhost:8001
```

## 📚 Documentation

- [Quick Reference](QUICKSTART.md) - Common patterns and examples
- [Contributing Guidelines](CONTRIBUTING.md) - How to contribute
- [Changelog](CHANGELOG.md) - Version history

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🎉 Acknowledgments

- Built with [Emscripten](https://emscripten.org/)
- Inspired by modern reactive frameworks
- Special thanks to the WebAssembly community

## 🔗 Links

- **GitHub**: [github.com/vdcoder/volt](https://github.com/vdcoder/volt)
- **Issues**: [github.com/vdcoder/volt/issues](https://github.com/vdcoder/volt/issues)
- **Discussions**: [github.com/vdcoder/volt/discussions](https://github.com/vdcoder/volt/discussions)

---

**Made with ⚡ by [@vdcoder](https://github.com/vdcoder)**
