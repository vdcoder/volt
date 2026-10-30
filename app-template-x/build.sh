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

# Find the Emscripten SDK. First match wins:
#   1. emsdk-path.txt next to this script (create-volt-app --emsdk-path writes it)
#   2. the EMSDK environment variable
#   3. emcc already on PATH
#   4. .tools/emsdk in this app or its parent, or an emsdk folder beside the app
# The chosen SDK is activated for this build unless it is already active.
APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
to_unix_path() {
    # Git Bash: emsdk-path.txt and EMSDK may hold Windows paths.
    if command -v cygpath &> /dev/null; then cygpath -u "$1"; else printf '%s\n' "$1"; fi
}
same_dir() {
    [ -d "$1" ] && [ -d "$2" ] && [ "$(cd "$1" && pwd -P)" = "$(cd "$2" && pwd -P)" ]
}
EMSDK_DIR=""
EMSDK_FROM=""
if [ -f "$APP_DIR/emsdk-path.txt" ]; then
    EMSDK_DIR="$(awk '{ sub(/\r$/, ""); gsub(/^[ \t]+|[ \t]+$/, "") } $0 != "" { print; exit }' "$APP_DIR/emsdk-path.txt")"
    if [ -n "$EMSDK_DIR" ]; then
        EMSDK_DIR="$(to_unix_path "$EMSDK_DIR")"
        case "$EMSDK_DIR" in /*) ;; *) EMSDK_DIR="$APP_DIR/$EMSDK_DIR" ;; esac
    fi
    if [ -z "$EMSDK_DIR" ] || [ ! -f "$EMSDK_DIR/emsdk_env.sh" ]; then
        echo "❌ Error: emsdk-path.txt must name the emsdk folder (the one containing emsdk_env.sh)."
        echo "   It names: ${EMSDK_DIR:-(nothing)}"
        echo "   Fix the path in that file, or delete it to use EMSDK, PATH or a local SDK folder."
        exit 1
    fi
    EMSDK_FROM="emsdk-path.txt"
elif [ -n "${EMSDK:-}" ]; then
    EMSDK_DIR="$(to_unix_path "$EMSDK")"
    EMSDK_FROM="EMSDK"
    if ! command -v emcc &> /dev/null && [ ! -f "$EMSDK_DIR/emsdk_env.sh" ]; then
        echo "❌ Error: EMSDK is $EMSDK, which has no emsdk_env.sh."
        exit 1
    fi
elif ! command -v emcc &> /dev/null; then
    for CANDIDATE in "$APP_DIR/.tools/emsdk" "$APP_DIR/../.tools/emsdk" "$APP_DIR/../emsdk"; do
        if [ -f "$CANDIDATE/emsdk_env.sh" ]; then
            EMSDK_DIR="$(cd "$CANDIDATE" && pwd)"
            EMSDK_FROM="local folder"
            break
        fi
    done
fi
if [ -n "$EMSDK_DIR" ] && command -v emcc &> /dev/null && [ -n "${EMSDK:-}" ] &&
   { [ "$EMSDK_FROM" = "EMSDK" ] || same_dir "$EMSDK_DIR" "$(to_unix_path "$EMSDK")"; }; then
    EMSDK_DIR=""   # this shell already has the chosen SDK active
fi
if [ -n "$EMSDK_DIR" ]; then
    echo "🔧 Activating Emscripten SDK ($EMSDK_FROM): $EMSDK_DIR"
    export EMSDK_QUIET=1
    set +e
    source "$EMSDK_DIR/emsdk_env.sh" > /dev/null
    set -e
    if ! command -v emcc &> /dev/null; then
        echo "❌ Error: the Emscripten SDK in $EMSDK_DIR has no active version."
        echo "   Run once: \"$EMSDK_DIR/emsdk\" activate latest"
        exit 1
    fi
fi

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten not found!"
    echo "   Checked emsdk-path.txt, EMSDK, emcc on PATH, and .tools/emsdk or ../emsdk near this app."
    echo "   Fix one of:"
    echo "   - Save the SDK folder for this app:  echo ~/emsdk > emsdk-path.txt"
    echo "   - Activate it in this shell:         source ~/emsdk/emsdk_env.sh"
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

# em++ links the C++ standard library (std::string etc.) on every Emscripten
# version; plain emcc stopped doing so in 6.0.6. -m64 selects MEMORY64.
em++ "$MAIN_SRC" \
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
    -m64 \
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
