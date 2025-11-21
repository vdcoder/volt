# ⚡ Volt App (Raw Template)

A minimal, **pure C++** Volt application template — ideal if you prefer full control without the Volt DSL or preprocessor.

This template uses **direct Virtual DOM construction in C++**, `.TRACK` node tracking, and classic Emscripten build steps.

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

---

## 📁 Project Structure

```
.
├── src/
│   ├── components/
│   │   └── Button.hpp        # Example component
│   ├── App.hpp               # Your main application class (start here!)
│   └── main.cpp              # Boilerplate entry point
├── dependencies/
│   └── volt/                 # Volt Framework headers
├── index.html                # App container / bootstrapping page
├── build.sh                  # Emscripten build script
└── output/                   # Build output (app.js / app.wasm / index.html)
```

---

## 🎯 Development

Your main application class lives in:

```
src/App.hpp
```

It inherits from `VoltRuntime::AppBase` and implements a `render()` method that returns a **VNode tree** (Volt’s Virtual DOM representation).

---

### 🧱 Example Component Structure

```cpp
VNode render() override {
    return tag::div({ attr::style("padding: 20px;") },
        tag::h1("Hello World").TRACK,

        tag::button({ attr::onClick([this](emscripten::val e) {
            // Handle click
            // Automatically triggers a re-render
        })}, "Click Me").TRACK

    ).TRACK;
}
```

### 📝 Notes

- **`.TRACK`** expands to `.track(__COUNTER__)`.  
  It ensures nodes keep stable IDs across renders.  
- **Event handlers automatically trigger re-renders.**  
  No need to call `invalidate()` manually unless:  
  - timers / intervals  
  - async work  
  - external system triggers  
- This template is intentionally **simple and explicit** compared to the "X" DSL template.

---

## 🔧 Build Options

### Custom GUID (for loading multiple Volt apps on one page)

```bash
VOLT_GUID='myapp_v1' ./build.sh
```

This changes the generated WebAssembly module namespace.

---

### Optimization Levels

Inside `build.sh`, adjust:

| Level | Meaning |  
|-------|---------|  
| `-O0` | Fastest compile, easiest debugging |  
| `-O2` | Good optimization |  
| `-O3` | Maximum optimization (default recommended for production) |

---

## 📚 Learn More

- Volt Framework headers are located in:  
  **`dependencies/volt/include/`**  
- Browse additional examples in the main Volt repository.

---

## 🚀 Deployment

Deploy the `output/` folder to any static host:

- **Railway**  
- **Vercel**  
- **Netlify**  
- **GitHub Pages**  
- or any standard web server  

No backend needed. Everything runs in the browser via WebAssembly.

---

Enjoy building with Volt! ⚡  





# ⚡ Volt App

A web application built with the Volt Framework.

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

## 📁 Project Structure

```
.
├── src/
│   ├── components/
│   │   └── Button.hpp   # Sample component
│   ├── App.hpp          # Your app code (start here!)
│   └── main.cpp         # Boilerplate
├── dependencies/
│   └── volt/            # Volt Framework
├── index.html           # Entry point
├── build.sh            # Build script
└── output/             # Build output
```

## 🎯 Development

Your app code lives in `src/App.hpp`. The `App` class inherits from `VoltRuntime::AppBase` and implements a `render()` method that returns a Virtual DOM tree.

### Example:
```cpp
VNode render() override {
    return tag::div({style("padding: 20px;")},
        tag::h1("Hello World").TRACK,
        tag::button({attr::onClick([this](emscripten::val e) {
            // Handle click
        })}, "Click Me").TRACK
    ).TRACK;
}
```

**Note**: Event handlers automatically trigger re-renders. Manual `invalidate()` only needed for non-event updates (timers, async operations).

## 🔧 Build Options

### Custom GUID (for multiple instances)
```bash
VOLT_GUID='myapp_v1' ./build.sh
```

### Optimization Levels
Edit `build.sh` and change `-O3` to:
- `-O0` - No optimization (faster builds, debugging)
- `-O2` - Moderate optimization
- `-O3` - Full optimization (default)

## 📚 Learn More

- **Volt Documentation**: See `/dependencies/volt/` for framework details
- **Examples**: Check the Volt repository for more examples

## 🚀 Deploy

Deploy the `output/` directory to any static host:
- Railway
- Vercel
- Netlify
- GitHub Pages
- Or any web server

Enjoy building with Volt! ⚡
