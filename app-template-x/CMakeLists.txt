cmake_minimum_required(VERSION 3.20)
project(VoltApp LANGUAGES CXX)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# ==========================================
# Options
# ==========================================
option(VOLT_BUILD_WASM "Build WebAssembly (Emscripten) target" ON)
option(VOLT_BUILD_NATIVE_REMOTE "Build native Remote-UI target" OFF)

set(VOLT_GUID "demo" CACHE STRING "Volt GUID identifier")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VOLT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/volt/include")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated")

# ==========================================
# Preprocess .x sources into _generated/
# ==========================================
file(GLOB_RECURSE APP_ALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)

set(GENERATED_ALL_FILES "")
set(GENERATED_COMPILE_SOURCES "")

foreach(SRC ${APP_ALL_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SRC}")
    set(OUT_PATH "${GENERATED_DIR}/src/${REL_PATH}")
    get_filename_component(OUT_DIR "${OUT_PATH}" DIRECTORY)

    if("${SRC}" MATCHES "\\.x\\.")
        add_custom_command(
            OUTPUT "${OUT_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "Preprocessing ${REL_PATH}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/preprocesor.py" "${SRC}" "${OUT_PATH}"
            DEPENDS "${SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/preprocesor.py"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${OUT_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "Copying ${REL_PATH}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC}" "${OUT_PATH}"
            DEPENDS "${SRC}"
            VERBATIM
        )
    endif()

    list(APPEND GENERATED_ALL_FILES "${OUT_PATH}")

    # Compile only real compilation units, but EXCLUDE entrypoints.
    if("${OUT_PATH}" MATCHES "\\.(c|cc|cpp|cxx|C|CC|CPP|CXX)$")
        get_filename_component(OUT_NAME "${OUT_PATH}" NAME)

        if(OUT_NAME STREQUAL "main.x.cpp" OR OUT_NAME STREQUAL "main.remote.x.cpp")
            # skip entrypoints (added per-target below)
        else()
            list(APPEND GENERATED_COMPILE_SOURCES "${OUT_PATH}")
        endif()
    endif()
endforeach()

add_custom_target(volt_generated ALL
    DEPENDS ${GENERATED_ALL_FILES}
)

if(GENERATED_COMPILE_SOURCES)
    add_library(volt_app_obj OBJECT ${GENERATED_COMPILE_SOURCES})
    add_dependencies(volt_app_obj volt_generated)

    target_include_directories(volt_app_obj PRIVATE
        "${VOLT_INCLUDE_DIR}"
        "${GENERATED_DIR}/src"
    )

    target_compile_definitions(volt_app_obj PRIVATE VOLT_GUID="${VOLT_GUID}")
    target_compile_features(volt_app_obj PRIVATE cxx_std_20)
endif()

# Convenient variables for entrypoints (generated)
set(VOLT_WASM_ENTRY   "${GENERATED_DIR}/src/main.x.cpp")
set(VOLT_REMOTE_ENTRY "${GENERATED_DIR}/src/main.remote.x.cpp")

# ==========================================
# WASM (Emscripten) target
# ==========================================
if(VOLT_BUILD_WASM)
    add_executable(volt_app_wasm "${VOLT_WASM_ENTRY}")
    add_dependencies(volt_app_wasm volt_generated)

    # If the object lib exists, add its objects to this executable
    if(TARGET volt_app_obj)
        target_sources(volt_app_wasm PRIVATE $<TARGET_OBJECTS:volt_app_obj>)
    endif()

    target_include_directories(volt_app_wasm PRIVATE
        "${VOLT_INCLUDE_DIR}"
        "${GENERATED_DIR}/src"
    )
    target_compile_definitions(volt_app_wasm PRIVATE
        VOLT_GUID="${VOLT_GUID}"
        $<$<CONFIG:Debug>:DEBUG;VOLT_ENABLE_LOG>
        $<$<CONFIG:RelWithDebInfo>:VOLT_ENABLE_LOG>
    )
    target_compile_features(volt_app_wasm PRIVATE cxx_std_20)

    target_compile_options(volt_app_wasm PRIVATE
        -sMEMORY64=1
        #-sWASM_BIGINT=1
        $<$<CONFIG:Debug>:-O0;-g3>
        $<$<CONFIG:RelWithDebInfo>:-O2;-g2>
        $<$<CONFIG:Release>:-O3>
    )

    if(TARGET volt_app_obj)
        target_compile_options(volt_app_obj PRIVATE
            -sMEMORY64=1
            #-sWASM_BIGINT=1
        )
    endif()

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

        # Debug-only runtime checking
        $<$<CONFIG:Debug>:-sASSERTIONS=2>

        # Release-only tightening (safe defaults)
        $<$<CONFIG:Release>:-sASSERTIONS=0>
    )

    set_target_properties(volt_app_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin"
        OUTPUT_NAME "app"
    )

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
    add_executable(volt_app_remote "${VOLT_REMOTE_ENTRY}")
    add_dependencies(volt_app_remote volt_generated)

    if(TARGET volt_app_obj)
        target_sources(volt_app_remote PRIVATE $<TARGET_OBJECTS:volt_app_obj>)
    endif()

    target_compile_definitions(volt_app_remote PRIVATE
        EM_REMOTE
        DEBUG
        VOLT_ENABLE_LOG
    )

    target_include_directories(volt_app_remote PRIVATE
        "${VOLT_INCLUDE_DIR}"
        "${GENERATED_DIR}/src"
    )

    # TODO later: add Boost.Beast sources or link libs here via vcpkg
    # target_sources(volt_app_remote PRIVATE ...)

endif()
