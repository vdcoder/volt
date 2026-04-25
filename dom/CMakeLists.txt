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
    # Through AppCore's deps, the SDK headers are at the AppCore deps path.
    # AppCore already adds these include dirs for its own targets; we add them
    # explicitly here for volt_dom so it can use JavaScriptCore API directly.
    target_include_directories(volt_dom PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/UltralightCore/include
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/WebCore/include
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/Ultralight/include
    )
    target_link_directories(volt_dom PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/UltralightCore/lib
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/WebCore/lib
        ${CMAKE_SOURCE_DIR}/third_party/AppCore/deps/Ultralight/lib
    )
    target_link_libraries(volt_dom PUBLIC
        AppCore
        UltralightCore
        WebCore
        Ultralight
    )
endif()
