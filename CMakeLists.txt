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
#   native – MSVC + Ultralight SDK (for VS debugging)
# ============================================================================

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Target selection ----
# VOLT_TARGET must be set by the preset or the user:
#   set(VOLT_TARGET EMSCRIPTEN)
#   set(VOLT_TARGET ULTRALIGHT)
if(NOT VOLT_TARGET)
    message(FATAL_ERROR "VOLT_TARGET must be set. Use: -DVOLT_TARGET=EMSCRIPTEN or -DVOLT_TARGET=ULTRALIGHT")
endif()

if(VOLT_TARGET STREQUAL "EMSCRIPTEN")
    message(STATUS "Volt target: Emscripten / WebAssembly")
elseif(VOLT_TARGET STREQUAL "ULTRALIGHT")
    message(STATUS "Volt target: Ultralight / native desktop")
else()
    message(FATAL_ERROR "Unknown VOLT_TARGET '${VOLT_TARGET}'. Use EMSCRIPTEN or ULTRALIGHT.")
endif()

# ---- Subdirectories ----
add_subdirectory(dom)
add_subdirectory(framework)
