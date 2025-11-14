#include "ComponentBase.hpp"
#include "IVoltRuntime.hpp"

// Base component interface
void volt::ComponentBase::scheduleRender() {
    m_pRuntime->scheduleRender();
}