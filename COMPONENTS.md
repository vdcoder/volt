# 🧩 Volt Components Guide

Volt components allow you to build reusable, composable UI pieces using a clean HTML-inspired DSL. This guide focuses primarily on the **Volt X DSL**, and provides raw C++ equivalents where appropriate.

---

# ✨ Overview

Volt components are lightweight objects that:

- Can be **stateless** or **stateful**
- Integrate seamlessly with Volt’s Virtual DOM
- Use **stable identity** with `.TRACK`
- Re-render automatically after event handlers
- Provide local state similar to React components, but with **full C++ power**
- Can accept props, return nodes, compose other components, and manage their own lifecycle

---

# 🧱 Component Types

Volt offers **two** component patterns:

1. **Stateless Components** — simple, functional, no internal state  
2. **Stateful Components** — maintain private internal state across renders

Both follow the same usage model in X DSL:

```cpp
<MyComponent(props...)/>
```

---

# 1️⃣ Stateless Components

Stateless components are ideal for simple UI pieces where everything is driven by props.

### ✔ Example: Button (X DSL)

```cpp
// src/components/Button.x.hpp
class Button : public volt::ComponentBase {
public:
    using ComponentBase::ComponentBase;

    VNodeHandle render(
        std::string label,
        std::function<void()> onClickFn
    ) {
        return <button({
            onClick:=([onClickFn](emscripten::val e){
                onClickFn();
            })
        }, label)/>.TRACK;
    }
};
```

### ✔ Usage in App.x.hpp

```cpp
<Button("Click Me", [this](){ counter++; })/>
```

### When to Use
- UI elements with no memory  
- Display-only widgets  
- Pure transformations of props  

---

# 2️⃣ Stateful Components

Stateful components preserve values across renders—perfect for counters, input fields, toggles, panels, etc.

### ✔ Example: Counter (X DSL)

```cpp
// src/components/Counter.x.hpp
class Counter : public volt::ComponentBase {
private:
    int value = 0;

public:
    using ComponentBase::ComponentBase;

    VNodeHandle render(std::string label) {
        return <div(
            <h3(label + ": " + std::to_string(value))/>,
            <button({
                onClick:=([this](emscripten::val e){
                    value++;
                })
            }, "+")/>,
            <button({
                onClick:=([this](emscripten::val e){
                    value--;
                })
            }, "-")/>
        )/>.TRACK;
    }
};
```

### ✔ Usage in App.x.hpp

```cpp
Counter c1(this, 0);
Counter c2(this, 10);

...

<c1.render("Primary Counter")/>
<c2.render("Secondary Counter")/>
```

### When to Use
- Persistent local state  
- Sealed logic: inputs → output  
- Reusable smart UI blocks  

---

# 🎛 Props

Props are just arguments to the component’s `render()` function.

```cpp
<MyComponent(name, age, theme)/>
```

Inside the component:

```cpp
VNodeHandle render(std::string name, int age, Theme theme)
```

---

# 🔒 Local State

State is stored as private variables:

```cpp
private:
    bool open = false;

onClick:=([this](auto e){ open = !open; })
```

Volt automatically re-renders after the event.

---

# 🌀 Component Composition

Components can include other components just like HTML tags.

```cpp
<Layout(
    <Header("Dashboard")/>,
    <Sidebar items/>,
    <Content user/>
)/>
```

---

# 🗂 Example: Input Field Component (Stateful)

```cpp
class TextField : public volt::ComponentBase {
private:
    std::string value;

public:
    using ComponentBase::ComponentBase;

    VNodeHandle render(std::string placeholder) {
        return <input({
            value:=(value),
            onInput:=([this](std::string v){ value = v; })
        })/>.TRACK;
    }

    std::string get() const { return value; }
};
```

Usage:

```cpp
TextField name(this);
...
<name.render("Your name")/>
<p("Hello, " + name.get())/>
```

---

# 🧩 Fragments & Lists

Fragments allow grouping without a DOM wrapper:

```cpp
<(
    <h1("Title")/>,
    <h2("Subtitle")/>,
    <hr/>
)/>
```

## Lists using map

```cpp
<map(items, [this](auto item){
    return <li(item)/>;
})/>
```

Fragments + map naturally preserve stable identity when `key:=` is provided:

```cpp
<li({ key:=("id-" + item.id) }, item.name)/>
```

---

# 🔁 Component Lifecycle

Volt components have a natural lifecycle:

### ✔ Constructor
Initialize local state, timers, resources.

### ✔ render()
Called whenever Volt detects a needed update.

### ✔ Destructor
Triggered automatically when the parent removes this component from the VDOM.

Example:

```cpp
class Loader : public ComponentBase {
public:
    Loader(auto* p) : ComponentBase(p) {
        startAsyncLoad();
    }

    ~Loader() {
        cancelAsyncTasks();
    }
};
```

No special hooks required; clean and predictable.

---

# ⚙️ Best Practices

### ✔ Choose Stateless Components when possible  
Less overhead, simpler, faster.

### ✔ Use Stateful Components for independent logic  
Avoid pushing all state into App.

### ✔ Never store VNodeHandle  
Components return them; the runtime manages identity.

### ✔ Always `.TRACK` all returned nodes  
Included automatically in X DSL—safe to rely on it.

### ✔ Keep render() pure  
No mutations inside root render except in handlers.

---

# 🧨 Anti-Patterns

### ❌ Mutating state outside event handlers without requestRender()

```cpp
value++;   // Won't re-render!
```

### ❌ Returning different root node types without `.TRACK`

```cpp
// Bad: unstable identity
if (cond) return <h1("A")/>;
else return <p("B")/>;

// Good:
return cond ? <h1("A")/> : <p("B")/>; // both TRACKed
```

### ❌ Creating long-lived JS refs without cleanup  
Use destructors to release resources.

---

# 📦 Raw C++ Equivalent (for curiosity)

Every X DSL component:

```cpp
<Counter("A")/>
```

Expands to:

```cpp
Counter(this).render("A").track(__COUNTER__)
```

Every attribute:

```cpp
onClick:=(...)
```

Expands to:

```cpp
volt::attr::onClick(...)
```

---

# 🔗 Next Steps

- **QUICKSTART.md** for the basics  
- **ADVANCED.md** for lifecycles, refs, and VDOM surgery  
- **README.md** for the full overview  
- **CONTRIBUTING.md** for development guidelines  

---

Made with ⚡ Volt X — simple, powerful, modern C++ UI.
