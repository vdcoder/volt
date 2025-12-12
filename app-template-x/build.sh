#!/bin/bash

# ⚡ Volt App (X template) - Build Script 

# Define some color/style variables for clarity
BOLD=$(tput bold)
GREEN=$(tput setaf 2)
RED=$(tput setaf 1)
NORMAL=$(tput sgr0) # Reset to normal mode

echo -e "\033[2K\r" 
echo "${BOLD}${GREEN}========================================================================"
echo "========================================================================"
echo ">>>>>>>                STARTING NEW BUILD STEP                 <<<<<<<"
echo "========================================================================"
echo "========================================================================"
echo "${NORMAL}"

set -e

echo "⚡ Building Volt App (X template)..."

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten not found!"
    echo "   Please source the Emscripten environment:"
    echo "   source ~/emsdk/emsdk_env.sh"
    exit 1
fi

# Check if preprocessor exists
if [ ! -f preprocesor.py ]; then
    echo "❌ Error: preprocesor.py not found in project root!"
    echo "   Make sure the Volt preprocessor is available."
    exit 1
fi

# Create output directory
mkdir -p output

# Set GUID (can be overridden with: VOLT_GUID='custom' ./build.sh)
GUID="${VOLT_GUID:-demo}"

# Sanitize GUID for display (replace hyphens with underscores for JS namespace)
GUID_DISPLAY=$(echo "$GUID" | sed 's/[^a-zA-Z0-9_]/_/g')

echo "📦 Preparing generated sources..."
GENERATED_DIR="_generated"

# Clean and recreate generated directory
rm -rf "$GENERATED_DIR"
mkdir -p "$GENERATED_DIR/src"

# Copy and preprocess all files from src/ → _generated/src/
# - If filename contains ".x." → run preprocesor.py
# - Otherwise copy as-is
echo "   - Scanning src/ for source files..."
find src -type f -print0 | while IFS= read -r -d '' SRC_FILE; do
    # Compute relative path and destination path
    ABS_SRC_FILE=$(realpath "$SRC_FILE")
    REL_PATH="${SRC_FILE#src/}"
    DEST_PATH="$GENERATED_DIR/src/$REL_PATH"

    mkdir -p "$(dirname "$DEST_PATH")"

    if [[ "$SRC_FILE" == *".x."* ]]; then
        echo "   • Preprocessing: $SRC_FILE -> $DEST_PATH"
        python3 preprocesor.py "$ABS_SRC_FILE" > "$DEST_PATH"
    else
        # Normal copy
        cp "$SRC_FILE" "$DEST_PATH"
    fi
done

# Make sure our main file exists in generated tree
MAIN_SRC="$GENERATED_DIR/src/main.x.cpp"
if [ ! -f "$MAIN_SRC" ]; then
    echo "❌ Error: $MAIN_SRC not found!"
    echo "   Make sure your X template has src/main.x.cpp"
    exit 1
fi

echo "📦 Compiling to WebAssembly..."
echo "   GUID: $GUID"
echo "   Namespace: volt_$GUID_DISPLAY"
echo "   Using sources from: $GENERATED_DIR/src"

emcc "$MAIN_SRC" \
    -DVOLT_GUID=\"$GUID\" \
    -DDEBUG \
    -DVOLT_ENABLE_LOG \
    -I./dependencies/volt/include \
    -o output/app.js \
    -lembind \
    -std=c++20 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="VoltApp" \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MEMORY64=1 \
    -s WASM_BIGINT=1 \
    --bind \
    -O0 \
    -g
# -O0 is for debugging
# -g is for debugging
# > -O3 is for best optimization

# Copy HTML
cp index.html output/
cp global.css output/

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
