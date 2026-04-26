cmake_minimum_required(VERSION 3.20)
project(volt VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# Volt – Dual-Target Build System
#
# Targets:
#   volt_dom         – JS value abstraction library (volt::Val, volt::dom::*)
#   volt_framework   – Core framework (VNode, VoltEngine, diff/patch, etc.)
#
# Build presets (CMakePresets.json):
#   wasm   – Emscripten/WebAssembly target (default)
#   native – MSVC + Ultralight SDK via AppCore (for VS debugging)
# ============================================================================

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Target selection ----
if(NOT VOLT_TARGET)
    message(FATAL_ERROR "VOLT_TARGET must be set. Use: -DVOLT_TARGET=EMSCRIPTEN or -DVOLT_TARGET=ULTRALIGHT")
endif()

if(VOLT_TARGET STREQUAL "EMSCRIPTEN")
    message(STATUS "Volt target: Emscripten / WebAssembly")
elseif(VOLT_TARGET STREQUAL "ULTRALIGHT")
    message(STATUS "Volt target: Ultralight / native desktop (AppCore)")
else()
    message(FATAL_ERROR "Unknown VOLT_TARGET '${VOLT_TARGET}'. Use EMSCRIPTEN or ULTRALIGHT.")
endif()

# ---- Ultralight SDK (native target only) ----
# Uses pre-built SDK at third_party/ultralight-sdk/ (v1.4.0+).
# Creates imported targets: Ultralight::AppCore, ::Ultralight, etc.
if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(UltralightSDK REQUIRED)
endif()

# ---- Subdirectories ----
add_subdirectory(dom)
add_subdirectory(framework)

# ---- Test subdirectories (Ultralight only) ----
if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    add_subdirectory(test/appcore)
endif()
