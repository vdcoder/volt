#pragma once
#include "AppDI.hpp"

// Independent of every browser instance; accessed on the server thread.
inline AppDI& services() {
    static AppDI instance;
    return instance;
}
