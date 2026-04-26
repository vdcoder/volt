# ============================================================================
# framework – Volt Core Framework
# ============================================================================

add_library(volt_framework STATIC)

# Collect all framework source files
file(GLOB_RECURSE FRAMEWORK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Collect all framework include directories
file(GLOB_RECURSE FRAMEWORK_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

target_sources(volt_framework PRIVATE ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})

target_include_directories(volt_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(volt_framework PUBLIC
    VOLT_TARGET_${VOLT_TARGET}
)

target_link_libraries(volt_framework PUBLIC
    volt_dom
)

# ---- Emscripten-specific flags ----
if(VOLT_TARGET STREQUAL "EMSCRIPTEN")
    target_link_options(volt_framework PUBLIC
        "SHELL:--bind"
        "SHELL:-lembind"
    )
endif()

# ---- Ultralight-specific flags ----
if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    target_link_libraries(volt_framework PUBLIC
        Ultralight::AppCore
    )
endif()
