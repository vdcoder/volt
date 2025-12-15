#!/bin/bash
set -e

# Colors omitted for brevity; you can keep your banners.

# 1. Ensure emcc is on PATH
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten not found. Run: source ~/emsdk/emsdk_env.sh"
    exit 1
fi

# 2. Configure + build using emcmake
BUILD_DIR="build_wasm"
mkdir -p "$BUILD_DIR"

emcmake cmake -S . -B "$BUILD_DIR" -DVOLT_BUILD_WASM=ON -DVOLT_BUILD_NATIVE_REMOTE=OFF
cmake --build "$BUILD_DIR" --config Debug

echo ""
echo "✅ Build complete!"
echo "Output in: $BUILD_DIR/output"
echo " - app.js"
echo " - app.wasm"
echo " - index.html"
echo ""
echo "To run:"
echo "  cd $BUILD_DIR/output && python3 -m http.server 8001"
