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
    return div({style("padding: 20px;")}, {
        h1({text("Hello World")}),
        button({onClick([this]() {
            // Handle click
            invalidate(); // Trigger re-render
        })}, {text("Click Me")})
    });
}
```

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
