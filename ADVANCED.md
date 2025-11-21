# ⚡ Volt Advanced Topics

This guide covers the deeper, more powerful features of Volt — including stable node identity, lifecycle hooks, structural reuse, list behavior, advanced rendering helpers, and integration patterns that experienced Volt developers rely on.

Volt X DSL is used throughout. Raw C++ equivalents are shown only when necessary.

---

# 🧠 Structural Reuse Algorithm (Advanced DOM Diffing)

Volt’s Virtual DOM engine uses the **Structural Reuse Algorithm**, a surgical diff-patch system designed to:

- **Preserve existing DOM elements whenever possible**
- **Reuse matching nodes across renders**
- **Move nodes instead of destroying and re-creating them**
- **Maintain internal browser state** (scroll positions, text selection, media buffers, etc.)
- **Minimize DOM churn** for maximum performance

Volt guarantees that:

### ✔ If a VNode has the same *stable identity* as before  
→ The same real DOM element will be reused.

### ✔ If only its position changes  
→ Volt will physically move *the existing element* in the DOM (surgery).

### ✔ If only props/attributes change  
→ Volt updates them in-place.

### ✔ If a node disappears  
→ Volt calls lifecycle hooks and removes it cleanly.

---

# 🧬 Stable Identity

Every node emitted by Volt X automatically includes:

```
.track(__COUNTER__)
```

This gives each VNode a deterministic identity based on:

- its lexical position in the component
- its component instance
- optional `key:=` overrides

### When to use keys

Use `key:=("unique-id")` when rendering lists whose order may change:

```cpp
<map(items, [this](auto& item) {
    return <li({ key:=("item-" + item.id) }, item.name)/>;
})/>
```

With keys, Volt can:

- reuse the same `<li>` element
- move it surgically if the list is reordered
- preserve scroll, focus, and internal state

Without a key, identity is based on iteration order.

---

# 🔁 Fragments, Lists, and Rendering Helpers

Volt provides several rendering helpers using X-DSL syntax.

## 1. `<map(container, renderer)/>`

Equivalent to `volt::map(...)`:

```cpp
<map(items, [this](auto& item){
    return <li("Item: " + item)/>;
})/>
```

- Automatically tracks each child with `.track(index)`
- Returns a fragment
- Keys may override identity

---

## 2. `<loop(n, renderer)/>`

Repeat a render block:

```cpp
<loop(5, [](int i){
    return <span("Step " + std::to_string(i))/>;
})/>
```

---

## 3. `<code(fn)/>`

Inject arbitrary C++ logic directly into JSX-style markup:

```cpp
<code([this](){
    if (count % 2 == 0)
        return <p("Even")/>;
    return <p("Odd")/>;
})/>
```

---

## 4. `<props(fn)/>`

Generate dynamic attribute lists:

```cpp
<div(
    props([&](){
        return std::vector{
            style:=("color:red;"),
            id:=("warning")
        };
    }),
    "ALERT"
)/>
```

---

# 🧩 Element Lifecycle Hooks

Volt exposes granular DOM-level lifecycle callbacks:

### ✔ `onAddElement`
Called when a DOM element is inserted.

```cpp
<div({
    onAddElement:=([this](auto e){
        log("Element added");
    })
})/>
```

### ✔ `onRemoveElement`
Called before removal.

```cpp
<div({
    onRemoveElement:=([this](auto e){
        log("Element removed");
    })
})/>
```

### ✔ `onBeforeMoveElement`
Before a structural move.

```cpp
<div({
    onBeforeMoveElement:=([this](auto e){
        log("Before moving element");
    })
})/>
```

### ✔ `onMoveElement`
After movement.

```cpp
<div({
    onMoveElement:=([this](auto e){
        log("After moving element");
    })
})/>
```

These are ideal for:

- WebGL / Canvas DOM integration  
- Scroll preservation  
- Entry/exit animations  
- Diagnostics  
- Third-party widget interop  

---

# 🧱 Advanced Conditional Rendering

Using `<code/>` with fragments yields extremely clear logic:

```cpp
<div(
    <code([this](){
        return done
            ? <h2("Completed")/>
            : <h2("Working...")/>;
    })/>,
    <p("Status panel below")/>
)/>
```

---

# 🪢 Nested Surgical Reuse

Volt handles arbitrarily deep structural changes:

```cpp
<div(
    <map(rows, [&](auto& row){
        return <(
            <h3(row.title)/>,
            <map(row.items, [&](auto& item){
                return <li(item)/>;
            })/>
        )/>;
    })/>
)/>
```

Every part may appear, disappear, or reorder — Volt keeps identity stable everywhere it logically should.

---

# ⚙️ Component Stability

Stateful components preserve their entire DOM subtree, even as parent components rerender:

```cpp
<Counter("A")/>
<Counter("B")/>
```

Volt guarantees:

- Unique identity for each component instance  
- Internal VNode & DOM stability  
- Predictable rendering lifecycle  

---

# 🧩 How Structural Reuse Works (Conceptual)

1. Volt assigns a **stable identity** to every VNode using:
   - `.track(__COUNTER__)`
   - optional `key:=`
   - component lineage

2. Volt compares new vs old VNode trees.

3. For each node:

   ✔ Same identity → **reuse DOM node**  
   ✔ Same identity, different position → **surgical move**  
   ✔ Same identity, changed props → **patch**  
   ✔ Removed → fire hooks + remove  

4. Browser state stays intact because DOM nodes survive.

5. The algorithm is recursive across arbitrary nesting.

### Compared to React/Vue/Solid:

| Feature | React | Vue | Solid | **Volt** |
|--------|-------|------|--------|-----------|
| Keep DOM node on reorder | Only with keys | With keys | Signals | **Always with identity** |
| Node movement | No (recreate) | No (recreate) | Sometimes | **Full surgical relocation** |
| Identity model | tag+key | tag+key | fine-grained | **hierarchical, multi-axis** |
| Internal DOM preservation | partial | partial | good | **maximum** |

Volt’s reuse model is uniquely stable and deterministic.

---

# ⚡ JavaScript Boot (Current)

```html
<script src="app.js"></script>
<script>
VoltApp().then(Module => {
    Module.createRuntime();  // mounts on #root
});
</script>
```

Or use the optional helper:

```js
VoltBootstrap.start();
```

---

# 📌 DOM Event Bubbling

Volt forwards browser events into C++:

```js
Module.invokeBubbleEvent(domNode.__cpp_ptr, event);
```

Volt then:

- finds the VNode  
- calls the appropriate handler  
- requests a rerender if needed  

---

# 📈 Advanced Performance Tips

### ✔ Prefer keys for reordering lists  
### ✔ Avoid unnecessary nested wrappers  
### ✔ Use fragments freely  
### ✔ Use lifecycle hooks for heavy DOM integrations  
### ✔ Keep render structures stable whenever possible  

---

# 🪄 Future Enhancements (Planned)

- Custom root IDs: `createRuntime("my-root")`
- Dedicated JS API wrapper  
- Component-level scoped updates  
- Devtools for inspecting structural identity  
- Shadow DOM mounting  
- SSR hydration  

---

# 📚 See Also

- **QUICKSTART.md** — get started  
- **COMPONENTS.md** — component patterns  
- **CONTRIBUTING.md** — how to contribute  
- **README.md** — overview  

---

Made with ⚡ Volt — **the UI engine built for surgical precision.**
