#!/bin/bash

# ⚡ Volt App - Build Script

set -e

echo "⚡ Building Volt App..."

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten not found!"
    echo "   Please source the Emscripten environment:"
    echo "   source ~/emsdk/emsdk_env.sh"
    exit 1
fi

# Create output directory
mkdir -p output

# Set GUID (can be overridden with: VOLT_GUID='custom' ./build.sh)
GUID="${VOLT_GUID:-demo}"

# Sanitize GUID for display (replace hyphens with underscores for JS namespace)
GUID_DISPLAY=$(echo "$GUID" | sed 's/[^a-zA-Z0-9_]/_/g')

echo "📦 Compiling to WebAssembly..."
echo "   GUID: $GUID"
echo "   Namespace: volt_$GUID_DISPLAY"

emcc src/main.cpp \
    -DVOLT_GUID=\"$GUID\" \
    -I./dependencies/volt/include \
    -o output/app.js \
    -lembind \
    -std=c++17 \
    -O3 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="VoltApp" \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    --bind

# Copy HTML
cp index.html output/

echo ""
echo "✅ Build complete!"
echo ""
echo "📁 Output files:"
echo "   - output/app.js"
echo "   - output/app.wasm"
echo "   - output/index.html"
echo ""
echo "🚀 To run:"
echo "   cd output && python3 -m http.server 8001"
echo "   # Then open http://localhost:8001"
echo ""
