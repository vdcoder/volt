#!/bin/bash
set -e

# Ensure emcc is on PATH
if ! command -v emcc &> /dev/null; then
  echo "❌ Emscripten not found. Run: source ~/emsdk/emsdk_env.sh"
  exit 1
fi

PROFILE="${1:-debug}"

case "${PROFILE,,}" in
  debug)
    BUILD_TYPE="Debug"
    BUILD_DIR="build_wasm_debug"
    ;;
  release)
    BUILD_TYPE="Release"
    BUILD_DIR="build_wasm_release"
    ;;
  relwithdebinfo|reldeb|staging)
    BUILD_TYPE="RelWithDebInfo"
    BUILD_DIR="build_wasm_relwithdebinfo"
    ;;
  *)
    echo "Usage: ./build.sh [debug|release|relwithdebinfo]"
    exit 1
    ;;
esac

echo "⚡ Building Volt WASM (${BUILD_TYPE}) -> ${BUILD_DIR}"

emcmake cmake -S . -B "${BUILD_DIR}" \
  -DVOLT_BUILD_WASM=ON \
  -DVOLT_BUILD_NATIVE_REMOTE=OFF \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"

echo ""
echo "✅ Build complete!"
echo "Output in: ${BUILD_DIR}/output"
echo "Run:"
echo "  cd ${BUILD_DIR}/output && python3 -m http.server 8001"
