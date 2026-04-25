# ============================================================================
# dom – JS Value Abstraction Library
# ============================================================================

add_library(volt_dom STATIC
    val_emscripten.cpp
    val_ultralight.cpp
)

target_include_directories(volt_dom PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(volt_dom PUBLIC
    VOLT_TARGET_${VOLT_TARGET}
)

if(VOLT_TARGET STREQUAL "ULTRALIGHT")
    # Ultralight SDK includes
    target_include_directories(volt_dom PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/ultralight/include
    )
    target_link_directories(volt_dom PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/ultralight/lib/Win64
    )
    target_link_libraries(volt_dom PUBLIC
        Ultralight
        UltralightCore
        WebCore
        AppCore
    )
endif()
