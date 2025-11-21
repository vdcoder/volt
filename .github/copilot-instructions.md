# Volt Framework AI Coding Instructions

## Architecture Overview

Volt is a C++ WebAssembly UI framework with Virtual DOM, focusing on performance and isolation. Each app compiles to WASM with GUID-based namespacing for multiple instances on the same page.

**Core Architecture Pattern:**
- Apps inherit `VoltRuntime::AppBase`, implement `render()` → `VNode` tree
- **Auto-invalidate**: Event callbacks automatically trigger re-render (no manual `invalidate()` needed)
- Virtual DOM diffing/patching happens automatically via `VoltRuntime`
- Event callbacks registered during render, cleared each frame
- **Lifecycle**: Optional `start()` method called once after mount, before first render

## Essential File Patterns

### App Structure (from `app-template/`)
```cpp
class MyApp : public VoltRuntime::AppBase {
private:
    int state = 0;  // App state
    
public:
    MyApp(VoltRuntime* runtime) : AppBase(runtime) {}
    
    // Optional: Called once after mount, before first render
    void start() override {
        // Initialize state, fetch data, etc.
    }
    
    VNode render() override {
        return div({style("...")},
            h1("Title"),
            button({onClick([this]() { 
                state++;  // No invalidate() needed - auto-invalidate!
            })}, "Click")
        );
    }
};
```

### Component Pattern
- **IRuntime interface**: Components receive `IRuntime*` (not full VoltRuntime access)
- **Stateless**: `Button(getRuntime()).render("Label", callback)` - created inline, no state
- **Stateful**: `Counter counter(getRuntime(), 0);` as member - maintains state across renders
- **Auto-invalidate**: Components don't call `invalidate()` - handled automatically
- **Runtime safety**: `IRuntime*` pointer safe for async callbacks (lives until unmount)
- See `COMPONENTS.md` for comprehensive patterns

### Virtual DOM (VNode.hpp)
- **Tag enum** for performance: `Tag::DIV`, `Tag::BUTTON`, etc.
- **Fragment support**: `Tag::FRAGMENT` for grouping without extra DOM elements
- **Props as `vector<pair<short, string>>`** sorted by attr ID for efficient diffing
- **4 overload patterns** for every element: `div()`, `div("text")`, `div(child1, child2, ...)`, `div({props}, children...)`
- **Text nodes**: Strings automatically convert to text nodes via implicit constructors, or use `text()` helper for clarity
- **Fragments & map()**: `map(container, mapper)` returns fragment, `fragment({nodes...})` groups without wrapper

### Build System
- **GUID-based isolation**: Each app gets unique namespace `volt_<sanitized_guid>`
- **Single TU compilation**: Include `VoltRuntime.cpp` in `main.cpp`
- **Include paths**: Use `-I./dependencies/volt/include` flag, then `#include <Volt.hpp>`
- **Emscripten binding pattern**: Export runtime functions for JS callbacks
- **Command**: `emcc src/main.cpp -DVOLT_GUID="..." -I./dependencies/volt/include -lembind -std=c++17 -O3`

## Critical Patterns

### Event Handling
```cpp
// Event helpers register callbacks with currentRuntime during render
// Auto-invalidate: scheduleRender() called automatically after callback
onClick([this]() { /* handler - no invalidate needed */ })
onInput([this](const std::string& val) { /* handler with value */ })
```
- Callbacks stored in `VoltRuntime` vectors, cleared each frame
- Event IDs passed to JS, invoke via `invokeEventCallback(id)`
- **Auto-invalidate wrapping**: User callbacks automatically trigger re-render

### State Management
- **Local state** in app class members
- **No manual invalidate**: Auto-invalidate in event handlers
- **Computed properties** as member functions called in `render()`
- **No built-in state management** - use standard C++ patterns
- **Manual invalidate available**: Call `invalidate()` for non-event updates (timers, async)

### Multiple Instances (`ADVANCED.md`)
- Each app needs unique GUID: `VOLT_GUID='app1' ./build.sh`
- Mount to different DOM elements: `VoltRuntime("root1")` vs `VoltRuntime("root2")`
- Communication via custom events or `window.sharedVoltState`

## Development Workflows

### Create New App
```bash
./framework/user-scripts/create-volt-app.sh my-app --guid "my_app"
cd my-app && ./build.sh
cd output && python3 -m http.server 8001
```

### Framework Development
- Headers in `framework/include/` - single include via `Volt.hpp`
- Implementation in `framework/src/VoltRuntime.cpp` (single TU pattern)
- Update apps: `./framework/user-scripts/update-framework.sh`

### Build Patterns
- **Debug**: Change `-O3` to `-O0` in `build.sh`
- **Custom GUID**: `VOLT_GUID='custom-name' ./build.sh`
- **GUID sanitization**: Special chars become underscores for JS namespace

## Memory/Performance Considerations

### Efficient VNode Construction
- Use **Tag enum** not strings for element types
- **Attribute IDs** (shorts) instead of string keys for props
- **Move semantics** for VNode vectors: `std::move(children)`
- **Minimal copying** in Virtual DOM diffing

### Event System
- **Frame-scoped callbacks** cleared after each render
- **Callback registration** only during render phase
- **Direct JS invocation** via callback IDs, no string lookups
- **Auto-invalidate wrapping**: User callbacks wrapped to automatically trigger re-render

### WASM Optimization
- **Single translation unit** compilation for better optimization
- **Emscripten-specific** patterns for DOM manipulation
- **Memory growth** enabled: `-s ALLOW_MEMORY_GROWTH=1`

## Common Anti-patterns

❌ **Wrong**: Manually calling `invalidate()` in event handlers (auto-invalidate does this)
❌ **Wrong**: Storing callbacks outside render phase  
❌ **Wrong**: Manual DOM manipulation (bypasses Virtual DOM)
❌ **Wrong**: Using same GUID for multiple instances
❌ **Wrong**: Including implementation files instead of single TU pattern
❌ **Wrong**: Passing `VoltRuntime*` to components (use `IRuntime*` from `getRuntime()`)

✅ **Right**: Let auto-invalidate handle re-renders after event callbacks
✅ **Right**: Use `invalidate()` only for non-event updates (timers, async fetch)
✅ **Right**: Let Virtual DOM handle all DOM operations
✅ **Right**: Use 4-overload pattern for clean element construction
✅ **Right**: Include `VoltRuntime.cpp` in main.cpp for optimal builds
✅ **Right**: Pass `getRuntime()` to child components for proper encapsulation

## Key Dependencies & Integration

- **Emscripten only** - no other runtime dependencies
- **C++17 standard** - use modern features like `std::optional`, lambdas
- **Browser integration** via Emscripten bindings and EM_JS macros
- **Module pattern**: Apps export `VoltApp()` function returning Promise\<Module>

---

Focus on the Virtual DOM abstraction - apps describe UI declaratively via VNode trees, runtime handles all imperative DOM operations efficiently.