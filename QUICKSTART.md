# ⚡ Volt Quickstart

Welcome to Volt! This guide gives you the fastest possible introduction to building reactive WebAssembly apps using Volt’s Virtual DOM, stable identity tracking, and component model.

---

# 🚀 Installation & First App

```bash
# Clone framework
git clone https://github.com/vdcoder/volt.git

# Create a new Volt app
./volt/framework/user-scripts/create-volt-app.sh my-app

# Build and run
cd my-app
./build.sh
cd output
python3 -m http.server 8001
# Open http://localhost:8001
```

---

# 🏗️ Basic App Structure

Every Volt app defines a class inheriting from `VoltRuntime::AppBase`:

```cpp
#include <Volt.hpp>
using namespace volt;

class MyApp : public VoltRuntime::AppBase {
private:
    int count = 0;

public:
    MyApp(VoltRuntime* r) : AppBase(r) {}

    VNodeHandle render() override {
        return tag::div(
            tag::h1("Counter: " + std::to_string(count)).TRACK,

            tag::button({
                attr::onClick([this](emscripten::val e) {
                    count++;
                })
            }, "Increment").TRACK

        ).TRACK;
    }
};

int main() {
    VoltRuntime runtime("root");
    runtime.mount<MyApp>();
    return 0;
}
```

### Notes
- Event handlers **automatically schedule re-rendering**.  
- `.TRACK` expands to `.track(__COUNTER__)` — ensuring **stable identity** across renders.

---

# 🎨 Elements & Attributes

Volt provides first-class bindings for HTML-like elements:

```cpp
tag::div(...)
tag::span(...)
tag::section(...)
tag::header(...)
tag::footer(...)
tag::nav(...)
tag::input(...)
```

## Common attributes

```cpp
attr::id("header")
attr::className("title")
attr::style("color: red; font-size: 20px;")
attr::href("https://example.com")
attr::src("/img.png")
attr::disabled()
```

---

# 🎯 Events

Volt supports many event types:

```cpp
attr::onClick([this](emscripten::val e) { ... })
attr::onInput([this](std::string value) { ... })
attr::onChange([this](std::string value) { ... })
attr::onSubmit([this](emscripten::val e) { ... })
attr::onKeyDown([this](std::string key) { ... })
attr::onFocus([this](emscripten::val e) { ... })
attr::onBlur([this](emscripten::val e) { ... })
```

All events automatically trigger a rerender unless you opt out.

---

# 🔄 State Management

Volt promotes simple, clear state updates:

```cpp
class MyApp : public VoltRuntime::AppBase {
private:
    std::string text = "";

public:
    VNodeHandle render() override {
        return tag::div(
            tag::input({
                attr::value(text),
                attr::onInput([this](std::string v) { text = v; })
            }).TRACK,

            tag::p("You typed: " + text).TRACK
        ).TRACK;
    }
};
```

No manual invalidation is needed — the framework handles it.

---

# 🧩 Component Patterns

Volt supports two component patterns:

---

## 1. Stateless Components

```cpp
class Button : public VoltRuntime::ComponentBase {
public:
    using ComponentBase::ComponentBase;

    VNodeHandle render(std::string label, std::function<void()> fn) {
        return tag::button({
            attr::onClick([this, fn](emscripten::val e) {
                fn();
            })
        }, label).TRACK;
    }
};
```

Use:

```cpp
Button(this).render("Click", [this] { ... });
```

---

## 2. Stateful Components

```cpp
class Counter : public VoltRuntime::ComponentBase {
private:
    int value = 0;

public:
    using ComponentBase::ComponentBase;

    VNodeHandle render() {
        return tag::div(
            tag::h1(std::to_string(value)).TRACK,
            tag::button({
                attr::onClick([this](emscripten::val e) { value++; })
            }, "+").TRACK
        ).TRACK;
    }
};
```

---

# 🧱 Fragments

Use fragments when you don’t want to create a wrapper `<div>`:

```cpp
tag::_fragment(
    tag::h1("Title").TRACK,
    tag::h2("Subtitle").TRACK
).TRACK;
```

Fragments maintain stable identity just like elements.

---

# 🔁 Lists & Loops

Volt includes a built-in `map()` helper:

```cpp
auto items = std::vector<std::string>{"A", "B", "C"};

return tag::ul(
    volt::map(items, [](const std::string& s) {
        return tag::li(s).TRACK;
    })
).TRACK;
```

If you use `attr::key()`, Volt preserves DOM nodes across reordering.

---

# 🧪 Conditional Rendering

```cpp
return isReady
    ? tag::div("Loaded").TRACK
    : tag::div("Loading...").TRACK;
```

---

# 🔧 Build Options

## Custom GUID

```bash
VOLT_GUID='myapp_v1' ./build.sh
```

## Optimization levels

Inside `build.sh` change `-O0`, `-O2`, or `-O3`.

---

# 🐛 Common Issues

### "volt_X is not defined"
GUID may contain invalid characters.

### Event doesn’t re-render
You may be mutating data without the framework detecting it.  
Wrap such updates inside an event handler or explicit `requestRender()`.

### Build fails
Run:

```bash
source ~/emsdk/emsdk_env.sh
```

---

# 📚 Resources

- GitHub: https://github.com/vdcoder/volt  
- Issues: https://github.com/vdcoder/volt/issues  
- Full documentation: see README.md & ADVANCED.md

---

**Version:** 0.2.0  
**License:** MIT  
Made with ⚡ by @vdcoder
