# ============================================================================
# FindUltralightSDK – Locate the pre-built Ultralight SDK
#
# Creates imported targets:
#   Ultralight::AppCore       – Pre-built AppCore (window, platform helpers)
#   Ultralight::Ultralight    – Ultralight engine DLL
#   Ultralight::UltralightCore– Core engine library
#   Ultralight::WebCore       – WebCore (WebKit engine)
#
# CRITICAL: Only adds SDK_ROOT/include (NOT subdirectories) to the include
# path. Adding include/Ultralight as a search path would cause its
# Ultralight/String.h (capital S) to shadow CRT's <string.h> on
# case-insensitive NTFS, breaking <cstring>'s using ::strcmp; declarations.
# The compiler resolves #include <Ultralight/Ultralight.h> from the root
# include dir without needing the subdirectory as a separate search path.
#
# Default search: ${PROJECT_SOURCE_DIR}/third_party/ultralight-sdk
# Override with:  -DULTRALIGHT_SDK_ROOT=<path>
# ============================================================================

# ---- Locate SDK ----
if(NOT ULTRALIGHT_SDK_ROOT)
  set(ULTRALIGHT_SDK_ROOT "${PROJECT_SOURCE_DIR}/third_party/ultralight-sdk"
      CACHE PATH "Path to Ultralight SDK root")
endif()

if(NOT EXISTS "${ULTRALIGHT_SDK_ROOT}/include/Ultralight/Ultralight.h")
  message(FATAL_ERROR
    "Ultralight SDK not found at ${ULTRALIGHT_SDK_ROOT}.\n"
    "Set ULTRALIGHT_SDK_ROOT or ensure third_party/ultralight-sdk/ exists.\n"
    "Download from: https://ultralig.ht/download")
endif()

message(STATUS "Ultralight SDK: ${ULTRALIGHT_SDK_ROOT}")

# ---- SDK include path (root only) ----
# WARNING: Do NOT add include/Ultralight or other subdirs as separate paths.
# The file Ultralight/String.h exists there and shadows CRT's <string.h> on
# case-insensitive NTFS when placed ahead of UCRT paths in the search order.
set(ULTRALIGHT_INCLUDE_DIRS
  "${ULTRALIGHT_SDK_ROOT}/include"
  CACHE INTERNAL "Ultralight SDK include directories"
)

# ---- Create imported library targets (DLL linkage only) ----
foreach(_pair IN ITEMS
    "AppCore;AppCore"
    "Ultralight;Ultralight"
    "UltralightCore;UltralightCore"
    "WebCore;WebCore")
  list(GET _pair 0 _name)
  list(GET _pair 1 _lib)
  set(_target "Ultralight::${_name}")

  if(NOT TARGET ${_target})
    add_library(${_target} SHARED IMPORTED)
    set_target_properties(${_target} PROPERTIES
      IMPORTED_IMPLIB   "${ULTRALIGHT_SDK_ROOT}/lib/${_lib}.lib"
      IMPORTED_LOCATION "${ULTRALIGHT_SDK_ROOT}/bin/${_lib}.dll"
    )
  endif()
endforeach()

unset(_pair)
unset(_name)
unset(_lib)
unset(_target)

# ---- Copy-DLL helper function for POST_BUILD ----
function(ul_copy_sdk_dlls TARGET)
  foreach(LIB AppCore Ultralight UltralightCore WebCore)
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ULTRALIGHT_SDK_ROOT}/bin/${LIB}.dll"
        "$<TARGET_FILE_DIR:${TARGET}>/${LIB}.dll"
      COMMENT "Copying ${LIB}.dll → $<TARGET_FILE_DIR:${TARGET}>"
    )
  endforeach()

  # Also copy WebCore resources (icudt67l.dat, cacert.pem)
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${ULTRALIGHT_SDK_ROOT}/resources"
      "$<TARGET_FILE_DIR:${TARGET}>/resources"
    COMMENT "Copying SDK resources → $<TARGET_FILE_DIR:${TARGET}>/resources"
  )
endfunction()

# ---- Convenience function to link ALL SDK libs + includes ----
function(ul_target_link_sdk TARGET)
  target_link_libraries(${TARGET} PRIVATE
    Ultralight::AppCore
    Ultralight::Ultralight
    Ultralight::UltralightCore
    Ultralight::WebCore
  )
  # Use SYSTEM INTERFACE to add only the root include dir.
  # This avoids shadowing CRT's <string.h> with Ultralight/String.h.
  target_include_directories(${TARGET} SYSTEM PRIVATE ${ULTRALIGHT_INCLUDE_DIRS})
endfunction()
