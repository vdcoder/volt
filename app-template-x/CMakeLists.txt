cmake_minimum_required(VERSION 3.20)

project(VoltApp LANGUAGES CXX)

# ==========================================
# Options
# ==========================================
option(VOLT_BUILD_WASM "Build WebAssembly (Emscripten) target" ON)
option(VOLT_BUILD_NATIVE_REMOTE "Build native Remote-UI target" OFF)

# Customizable GUID (like in build.sh)
set(VOLT_GUID "demo" CACHE STRING "Volt GUID identifier")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Volt include path (from create-volt-app layout)
set(VOLT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/volt/include")

# Where generated sources will live (in the build tree)
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated")

# ==========================================
# Preprocess .x sources into _generated/
# ==========================================
# Grab every file under src/ (cpp + headers + anything else)
file(GLOB_RECURSE APP_ALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)

set(GENERATED_ALL_FILES "")
set(GENERATED_COMPILE_SOURCES "")

foreach(SRC ${APP_ALL_FILES})
    # Relative path under src/
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SRC}")
    set(OUT_PATH "${GENERATED_DIR}/src/${REL_PATH}")
    get_filename_component(OUT_DIR "${OUT_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUT_DIR}")

    # Decide if this file should be preprocessed or just copied
    if("${SRC}" MATCHES "\\.x\\.")
        # X-files go through your Python preprocessor
        add_custom_command(
            OUTPUT "${OUT_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "Preprocessing ${REL_PATH}"
            COMMAND bash -c "python3 \"${CMAKE_CURRENT_SOURCE_DIR}/preprocesor.py\" \"${SRC}\" > \"${OUT_PATH}\""
            DEPENDS "${SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/preprocesor.py"
            VERBATIM
        )
    else()
        # Non-X files get copied as-is
        add_custom_command(
            OUTPUT "${OUT_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "Copying ${REL_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC}" "${OUT_PATH}"
            DEPENDS "${SRC}"
            VERBATIM
        )
    endif()

    # Track all generated files (headers + sources) so we can depend on them
    list(APPEND GENERATED_ALL_FILES "${OUT_PATH}")

    # Determine the original extension (to detect headers)
    get_filename_component(OUT_EXT "${OUT_PATH}" EXT)    # e.g. ".hpp", ".cpp", ".x.cpp"

    # Use MATCHES instead of the long OR list.
    # This regex matches any standard C-family source extension, case-insensitively.
    if("${OUT_PATH}" MATCHES "\\.(c|cc|cpp|cxx|C|CC|CPP|CXX)$")
        list(APPEND GENERATED_COMPILE_SOURCES "${OUT_PATH}")
    endif()
endforeach()

# A custom target just to ensure generation happens before build
add_custom_target(volt_generated ALL
    DEPENDS ${GENERATED_ALL_FILES}
)

# Our object library uses ONLY compiled sources (never headers)
add_library(volt_app_obj OBJECT ${GENERATED_COMPILE_SOURCES})
add_dependencies(volt_app_obj volt_generated)

target_include_directories(volt_app_obj PRIVATE
    "${VOLT_INCLUDE_DIR}"
    "${GENERATED_DIR}/src"   # so #include "App.x.hpp" etc. see the generated copies
)
target_compile_definitions(volt_app_obj PRIVATE VOLT_GUID="${VOLT_GUID}")
target_compile_features(volt_app_obj PRIVATE cxx_std_20)

# ==========================================
# WASM (Emscripten) target
# ==========================================
if(VOLT_BUILD_WASM)
    add_executable(volt_app_wasm $<TARGET_OBJECTS:volt_app_obj>)

    target_compile_definitions(volt_app_wasm PRIVATE
        DEBUG
        VOLT_ENABLE_LOG
        # EM_REMOTE NOT defined here → EmremVal.hpp aliases emscripten::val
    )

    # Compile flags must be applied to the object library (where .o files are built)
    target_compile_options(volt_app_obj PRIVATE
        -O0
        -g
        -sMEMORY64=1
        -sWASM_BIGINT=1
        # -mwasm64  # only add if you still see a mismatch
    )

    # Optional safety: keep the executable target consistent too
    target_compile_options(volt_app_wasm PRIVATE
        -sMEMORY64=1
        -sWASM_BIGINT=1
    )

    target_link_options(volt_app_wasm PRIVATE
        -lembind
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sMODULARIZE=1
        -sEXPORT_NAME=VoltApp
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
        -sMEMORY64=1
        -sWASM_BIGINT=1
        --bind
    )

    # Put js/wasm into "output" directory like your old build.sh did
    set_target_properties(volt_app_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin"
        OUTPUT_NAME "app"
    )

    # Copy static assets after link
    add_custom_command(TARGET volt_app_wasm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/output"  
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/output/bin" 
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/output/js"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/output/css"
        COMMAND ${CMAKE_COMMAND} -E rm -f  "${CMAKE_CURRENT_BINARY_DIR}/output/index.html"  
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/static"
                "${CMAKE_CURRENT_BINARY_DIR}/output"
        COMMENT "Copying static assets (static/) into build output"
    )
endif()

# ==========================================
# Native Remote-UI target (Visual Studio / Linux)
# ==========================================
if(VOLT_BUILD_NATIVE_REMOTE)
    add_executable(volt_app_remote $<TARGET_OBJECTS:volt_app_obj>)

    target_compile_definitions(volt_app_remote PRIVATE
        EM_REMOTE          # Activates your remote val implementation
        DEBUG
        VOLT_ENABLE_LOG
    )

    target_include_directories(volt_app_remote PRIVATE "${VOLT_INCLUDE_DIR}")

    # Extra source files for remote backend (WebSocket, etc.)
    # You can adjust these paths once you create them.
    target_sources(volt_app_remote PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteBackend.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteTransport.cpp"
    )
endif()
