#include "ComponentBase.hpp"
#include "IVoltRuntime.hpp"

// Base component interface
void volt::ComponentBase::requestRender() {
    m_pRuntime->requestRender();
}