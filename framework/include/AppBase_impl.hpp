#include "AppBase.hpp"
#include "VoltRuntime.hpp"

// Base app interface
void volt::AppBase::scheduleRender() {
    m_pRuntime->scheduleRender();
}