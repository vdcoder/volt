# ============================================================================
# test/appcore – AppCore-based Ultralight Smoke Test
#
# Links against the pre-built SDK (ultralight-sdk). Demonstrates full
# AppCore platform integration: DirectWrite fonts, native file system,
# window creation with D3D11 GPU acceleration.
#
# Usage:
#   cmake --preset native
#   cmake --build build/native --target volt-test-appcore --config Debug
#   cd build/native/test/appcore/Debug & volt-test-appcore
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(volt-test-appcore LANGUAGES CXX)

# Force C++17 (parent project is C++17, but subdirectory project() needs explicit)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Sources ----
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# ---- Executable ----
add_executable(volt-test-appcore ${TEST_SOURCES})

# Inherit SDK includes and link libs from parent targets
ul_target_link_sdk(volt-test-appcore)

# Additional system libs needed by AppCore's D3D11 path
target_link_libraries(volt-test-appcore PRIVATE d3d11 dxgi dwrite)

# ---- Copy DLLs + resources to output directory ----
ul_copy_sdk_dlls(volt-test-appcore)
