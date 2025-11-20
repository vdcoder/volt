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

} // namespace x
 
} // namespace volt