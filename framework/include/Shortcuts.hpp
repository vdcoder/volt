#pragma once

#include "StableKeyManager.hpp"
#include "RenderingRuntime.hpp"
#include "VoltRuntime.hpp"

namespace volt {

namespace x {

// typedef StableKeyManager::StableKey Key;
// typedef StableKeyManager::StableKeyBuilderIntTokenGuard KeyGuard;

// inline Key key_i(int a_nKeyIndex) {
//     g_pRenderingRuntime->getKeyManager().pushIntToken(a_nKeyIndex);

//     Key key = g_pRenderingRuntime->getKeyManager().build();

//     g_pRenderingRuntime->getKeyManager().popToken();
    
//     return key;
// }

// inline KeyGuard key_guard_i(int a_nKeyIndex) {
//     return g_pRenderingRuntime->getKeyManager().makeGuard(a_nKeyIndex);
// }

// If
template<typename Generator1, typename Generator2>
inline std::vector<VNodeHandle> iff(bool a_bCondition, Generator1 a_fnTrueGenerator, Generator2 a_fnFalseGenerator) {
    if (a_bCondition) {
        return (std::vector<VNodeHandle>)a_fnTrueGenerator();
    } else {
        return (std::vector<VNodeHandle>)a_fnFalseGenerator();
    }
}

// inline VNodeHandle iff(bool a_bCondition, std::function<VNodeHandle()> a_fnTrueGenerator, std::function<VNodeHandle()> a_fnFalseGenerator) {
//     return a_bCondition ? a_fnTrueGenerator() : a_fnFalseGenerator();
// }

} // namespace x
 
} // namespace volt