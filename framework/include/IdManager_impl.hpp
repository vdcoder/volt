#pragma once

#include "IdManager.hpp"
#include "VNode.hpp"

namespace volt {

void IdManager::startGeneration(VNode** a_ppOutFreeListHead) {
    for (auto& entry : m_oldStore) {
        if (*a_ppOutFreeListHead == nullptr) {
            *a_ppOutFreeListHead = entry.second;
        } else {
            entry.second->setParent(*a_ppOutFreeListHead);
            *a_ppOutFreeListHead = entry.second;
        }
    }
    m_oldStore = std::move(m_newStore);
}

void IdManager::StableKeyBuilder::pushVNodeToken(VNode* a_pNode) {
    std::string str = a_pNode->getIdProp();
    if (!str.empty()) {
        m_stack.push("D" + str + "_");
    }
    else {
        str = a_pNode->getKeyProp();
        if (!str.empty()) {
            str = a_pNode->getStableKeyPrefix() + "S" + str + "_";
        }
        else if (a_pNode->getStableKeyPosition() >= 0) {
            str = a_pNode->getStableKeyPrefix() + "I" + std::to_string(a_pNode->getStableKeyPosition()) + "_";
        }
        else {
            log("ERROR: VNode has no stable identity (no id, key, or stable key position)");
        }
        if (str.empty() || str[0] != 'D') { // Only push if not ID token
            pushString(str);
        } else {
            // ID token takes precedence, push directly
            m_stack.push(str);
        }
    }
}

} // namespace volt

