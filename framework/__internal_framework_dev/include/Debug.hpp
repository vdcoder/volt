#pragma once

#include <string>
#include <emscripten/val.h>

namespace volt {

void log(const std::string& a_sMessage) {
    emscripten::val console = emscripten::val::global("console");
    console.call<void>("log", a_sMessage);
}

} // namespace volt