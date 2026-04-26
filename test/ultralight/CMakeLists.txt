# ============================================================================
# test/ultralight – Ultralight SDK Headless Smoke Test
#
# Builds directly against the SDK binaries (no AppCore source compilation).
# Requires: ExternalProject_Add has already downloaded and extracted the SDK
# to third_party/AppCore/deps/{UltralightCore,WebCore,Ultralight}/.
#
# Usage:
#   cd build/native
#   cmake --build . --target volt-test-ultralight
#   cd test/ultralight/Debug & volt-test-ultralight
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(volt-test-ultralight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDK paths (resolved relative to source tree) ----
get_filename_component(SDK_ROOT "../../third_party/AppCore/deps" REALPATH
  BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(ULTRALIGHTCORE_DIR "${SDK_ROOT}/UltralightCore")
set(WEBCORE_DIR "${SDK_ROOT}/WebCore")
set(ULTRALIGHT_DIR "${SDK_ROOT}/Ultralight")

message(STATUS "Ultralight SDK deps: ${ULTRALIGHTCORE_DIR}")

# ---- Collect sources ----
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# ---- Executable ----
add_executable(volt-test-ultralight ${TEST_SOURCES})

target_include_directories(volt-test-ultralight PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ULTRALIGHTCORE_DIR}/include
  ${WEBCORE_DIR}/include
  ${WEBCORE_DIR}/DerivedSources/ForwardingHeaders
  ${ULTRALIGHT_DIR}/include
)

target_link_directories(volt-test-ultralight PRIVATE
  ${ULTRALIGHTCORE_DIR}/lib
  ${WEBCORE_DIR}/lib
  ${ULTRALIGHT_DIR}/lib
)

target_link_libraries(volt-test-ultralight PRIVATE
  UltralightCore
  WebCore
  Ultralight
  dwrite
)

# ---- Copy DLLs to output directory ----
add_custom_command(TARGET volt-test-ultralight POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHTCORE_DIR}/bin/UltralightCore.dll"
    "$<TARGET_FILE_DIR:volt-test-ultralight>/UltralightCore.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WEBCORE_DIR}/bin/WebCore.dll"
    "$<TARGET_FILE_DIR:volt-test-ultralight>/WebCore.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_DIR}/bin/Ultralight.dll"
    "$<TARGET_FILE_DIR:volt-test-ultralight>/Ultralight.dll"
  COMMENT "Copying SDK DLLs"
)

# Copy resources (icudt, cert) alongside the DLLs
add_custom_command(TARGET volt-test-ultralight POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${WEBCORE_DIR}/bin/resources"
    "$<TARGET_FILE_DIR:volt-test-ultralight>/resources"
  COMMENT "Copying WebCore resources"
)
