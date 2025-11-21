#pragma once

namespace volt {

namespace x {

// If
template<typename Generator1, typename Generator2>
inline std::vector<VNodeHandle> iff(bool a_bCondition, Generator1 a_fnTrueGenerator, Generator2 a_fnFalseGenerator) {
    if (a_bCondition) {
        return (std::vector<VNodeHandle>)a_fnTrueGenerator();
    } else {
        return (std::vector<VNodeHandle>)a_fnFalseGenerator();
    }
}

} // namespace x
 
} // namespace volt