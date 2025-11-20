#include "AppBase.hpp"
#include "VoltRuntime.hpp"

// Base app interface
void volt::AppBase::requestRender() {
    m_pRuntime->requestRender();
}