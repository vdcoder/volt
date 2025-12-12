#pragma once
#include <emscripten.h>

namespace volt {

void invokeBubbleEvent(emscripten::val event);

void invokeNonBubbleEvent(emscripten::val event);

} // namespace volt