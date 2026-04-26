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

# ---- AppCore (Ultralight native target only) ----
# AppCore provides window creation, GPU context, input handling, and
# auto-downloads the prebuilt Ultralight SDK (WebCore, UltralightCore,
# Ultralight engine) from S3 via ExternalProject_Add.
if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    set(PORT "UltralightWin" CACHE STRING "Ultralight platform port")
    set(UL_D3D_DRIVER "d3d11" CACHE STRING "Direct3D driver for Ultralight")
    set(ALLINONE_BUILD OFF CACHE BOOL "Not an all-in-one build")
    add_subdirectory(third_party/AppCore)
endif()

# ---- Subdirectories ----
add_subdirectory(dom)
add_subdirectory(framework)

# ---- Test subdirectory (Ultralight only, links SDK directly) ----
if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    # Requires: SDK downloaded via AppCore ExternalProject_Add first
    add_subdirectory(test/ultralight)
endif()
