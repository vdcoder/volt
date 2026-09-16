#pragma once
#include <IRuntime.hpp>

namespace voltxp {

class VoltRuntimeService {
public:
    explicit VoltRuntimeService(volt::IRuntime& runtime) noexcept : m_runtime(runtime) {}
    volt::IRuntime& getRuntime() const noexcept { return m_runtime; }

private:
    volt::IRuntime& m_runtime;
};

} // namespace voltxp
