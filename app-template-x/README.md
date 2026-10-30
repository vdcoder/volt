# ⚡ Volt App (X Template)

A web application built with the Volt Framework — using the **Volt X DSL**, an HTML-inspired syntax compiled to pure Volt C++ through a Python preprocessor.

This template is ideal for fast UI development with clean, declarative syntax.

---

## 🚀 Quick Start

### Build

```bash
./build.sh
```

### Run

```bash
cd output
python3 -m http.server 8001
# Open http://localhost:8001
```

### Emscripten SDK

`build.sh` uses the first SDK it finds: the folder named in `emsdk-path.txt`
(written when the app was created with `--emsdk-path`), then `EMSDK`, then `emcc`
on PATH, then `.tools/emsdk` in this app or its parent, or an `emsdk` folder beside
the app. To pin an SDK for this app:

```bash
echo ~/emsdk > emsdk-path.txt
```

The file is machine-specific and listed in `.gitignore`.

`build.sh` compiles with `em++` (which links the C++ standard library) and `-m64`
(64-bit memory), so it needs Emscripten 6.0.0 or newer.

---

## 📁 Project Structure

```
.
├── src/
│   ├── components/
│   │   └── Button.x.hpp      # Example X-DSL component
│   ├── App.x.hpp             # Your main application (start here!)
│   └── main.x.cpp            # Boilerplate entry point
├── dependencies/
│   └── volt/                 # Volt Framework headers
├── index.html                # Main HTML container
├── build.sh                  # Preprocessing + WASM build
└── output/                   # Build output
```

### DSL Source File Rule

Any file whose name contains:

```
.x.
```

(e.g. `App.x.hpp`, `main.x.cpp`, `Button.x.hpp`)

will be **preprocessed** and written into:

```
_generated/src/
```

The build script compiles the generated files, not the originals.

---

## 🎯 Development

Your primary app code lives in:

```
src/App.x.hpp
```

The `App` class inherits from `VoltRuntime::AppBase` and implements:

```cpp
VNodeHandle render() override;
```

The Volt X DSL lets you write clean UI declarations similar to HTML.

---

### 🧱 Example (X DSL)

```cpp
VNodeHandle render() override {
    return <div({ style:=("padding: 20px;") },
        <h1("Hello World")/>,

        <button({
            onClick:=([this](emscripten::val e) {
                // Handle click
            })
        }, "Click Me")/>

    )/>;
}
```

### 📝 Notes

- The **Volt preprocessor** converts all `<tag(...) />` into standard Volt C++.
- Props use the syntax:

  ```cpp
  prop:=("value")
  ```

  which becomes:

  ```cpp
  volt::attr::prop("value")
  ```

- Event handlers automatically request a re-render.
- Manually call `requestRender()` only for:
  - timers  
  - async operations  
  - external data changes  

---

## 🔧 Build Options

### Custom GUID (run multiple Volt apps on one page)

```bash
VOLT_GUID='myapp_v1' ./build.sh
```

The GUID becomes part of the JavaScript module name.

---

### Optimization Levels

Inside `build.sh`, adjust:

| Level | Meaning |
|-------|---------|
| `-O0` | Fastest builds, easiest debugging |
| `-O2` | Recommended dev mode |
| `-O3` | Maximum optimization for production |

---

## 📚 Learn More

- Volt framework details live under:  
  **`dependencies/volt/`**
- More examples are available in the Volt repository.

---

## 🚀 Deployment

You can deploy the `output/` folder to **any** static host:

- Railway  
- Vercel  
- Netlify  
- GitHub Pages  
- Nginx / Apache / static servers

Volt runs entirely in the browser via WebAssembly — no backend required.

---

Enjoy building with Volt X! ⚡