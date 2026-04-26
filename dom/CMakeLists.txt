# ============================================================================
# dom – JS Value Abstraction Library (volt::Val, volt::dom::*)
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
    # Pre-built SDK targets (created by FindUltralightSDK.cmake)
    # These already provide include dirs for Ultralight, AppCore, JavaScriptCore.
    target_link_libraries(volt_dom PUBLIC
        Ultralight::AppCore
        Ultralight::UltralightCore
        Ultralight::WebCore
        Ultralight::Ultralight
    )
endif()
