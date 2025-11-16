#include "StableKeyManager.hpp"
#include "VNode.hpp"

namespace volt {

#define HASH_SIZE 4096
#define HASH_SIZE_MINUS_1 4095

// ============================================================================
// StableKeyManager Implementation

StableKeyManager::StableKeyManager() : m_hashChains(HASH_SIZE) {
}

void StableKeyManager::startNewGeneration(VNode** a_ppOutFreeListHead) {
    m_sDuplicateKeyDescription = "";

    VNode* pFreeSubListHead = nullptr;
    VNode* pFreeSubListTail = nullptr;
    for (auto& entry : m_hashChains) {
        entry.startNewGeneration(&pFreeSubListHead, &pFreeSubListTail);
        if (pFreeSubListHead != nullptr) {
            // Link to free list
            if (*a_ppOutFreeListHead == nullptr) {
                *a_ppOutFreeListHead = pFreeSubListHead;
            } else {
                pFreeSubListTail->setNext(*a_ppOutFreeListHead);
                *a_ppOutFreeListHead = pFreeSubListHead;
            }
        }
    }
}

void xlog(volt::StableKeyManager::StableKey const& key, const std::string& message) {
    if (key.toString() == "StableKey_K-Apple_I1-5_") {
        log(message);
    }
}

void StableKeyManager::matchVNode(VNode* a_pNode) {
    xlog(a_pNode->getStableKey(), "StableKeyManager::matchVNode: in " + a_pNode->getStableKey().toString());
    unsigned int index = a_pNode->getStableKey().hash() & HASH_SIZE_MINUS_1;
    xlog(a_pNode->getStableKey(), "StableKeyManager::matchVNode: index " + std::to_string(index) + " for key " + a_pNode->getStableKey().toString());
    auto& entry = m_hashChains[index];
    if (entry.matchVNode(a_pNode)) { // Duplicate key detected
        m_sDuplicateKeyDescription += a_pNode->getStableKey().toString();
    }
}

// ============================================================================
// HashCollisionChain Implementation

StableKeyManager::HashCollisionChain::HashCollisionChain() : m_pHead(nullptr), m_pTail(nullptr), m_pNextGenHead(nullptr), m_pNextGenTail(nullptr) {
}

StableKeyManager::HashCollisionChain::~HashCollisionChain() {
}

void StableKeyManager::HashCollisionChain::startNewGeneration(VNode** a_ppOutFreeListHead, VNode** a_ppOutFreeListTail) {
    *a_ppOutFreeListHead = m_pHead;
    *a_ppOutFreeListTail = m_pTail;
    m_pHead = m_pNextGenHead;
    m_pTail = m_pNextGenTail;
    m_pNextGenHead = nullptr;
    m_pNextGenTail = nullptr;
}

bool StableKeyManager::HashCollisionChain::matchVNode(VNode* a_pNode) {
    xlog(a_pNode->getStableKey(), "HashCollisionChain::matchVNode: in " + a_pNode->getStableKey().toString());
    //log("HashCollisionChain::matchVNode: in " + a_pNode->getStableKey().toString());

    VNode* pPrevNode = nullptr;
    if (findInOld(&a_pNode->getStableKey(), &pPrevNode)) { // Found in old map

        xlog(a_pNode->getStableKey(), "HashCollisionChain::matchVNode: matched old node for key " + a_pNode->getStableKey().toString());

        // Link with matching old node's data
        a_pNode->setMatchingOldNode(pPrevNode != nullptr ? pPrevNode->getNext() : m_pHead);

        // Remove from old chain
        if (pPrevNode != nullptr) {
            pPrevNode->setNext(a_pNode->getMatchingOldNode()->getNext());
        } else {
            m_pHead = a_pNode->getMatchingOldNode()->getNext();
        }
    }

    xlog(a_pNode->getStableKey(), "HashCollisionChain::matchVNode: validating if in new " + a_pNode->getStableKey().toString());

    // Validate key uniqueness state, for error reporting
    bool bAlreadyInNew = findInNew(&a_pNode->getStableKey());

    // Add to new chain
    if (m_pNextGenHead == nullptr) {
        m_pNextGenHead = a_pNode;
    } else {
        m_pNextGenTail->setNext(a_pNode);
    }
    m_pNextGenTail = a_pNode;

    xlog(a_pNode->getStableKey(), "HashCollisionChain::matchVNode: out " + a_pNode->getStableKey().toString());

    return !bAlreadyInNew;
}

bool StableKeyManager::HashCollisionChain::findInOld(StableKey* a_pKey, VNode** a_ppOutPrevNode) {
    xlog(*a_pKey, "HashCollisionChain::findInOld: in");
    VNode * pPrevNode = nullptr;
    VNode * node = m_pHead;
    while (node != nullptr) {
        xlog(*a_pKey, "HashCollisionChain::findInOld: checking node " + node->getStableKey().toString() + " against key " + a_pKey->toString());
        if (node->getStableKey().equals(a_pKey)) {
            *a_ppOutPrevNode = pPrevNode;
            xlog(*a_pKey, "HashCollisionChain::findInOld: found matching old node for key " + a_pKey->toString());
            return true;
        }
        pPrevNode = node;
        node = node->getNext();
    }
    xlog(*a_pKey, "HashCollisionChain::findInOld: no matching old node found for key " + a_pKey->toString());
    return false;
}

bool StableKeyManager::HashCollisionChain::findInNew(StableKey* a_pKey) {
    VNode * pNode = m_pHead;
    while (pNode != nullptr) {
        if (pNode->getStableKey().equals(a_pKey)) {
            return true;
        }
        pNode = pNode->getNext();
    }
    return false;
}

// ============================================================================
// StableKeyBuilder Implementation

void StableKeyManager::StableKeyBuilder::pushIntToken(int a_nToken) {
    //log("StableKeyBuilder::pushIntToken: " + std::to_string(a_nToken) + " " + std::to_string(m_stringTokens.size()));
    m_sIntTokens.append(reinterpret_cast<const char*>(&a_nToken), sizeof(int)); // ASSUMPTION! We are assuming that the int token is never -1, which is reserved for string tokens
    //log("StableKeyBuilder::pushIntToken out " + std::to_string(m_sIntTokens.size()) + " " + std::to_string(m_stringTokens.size()));
    //log("StableKeyBuilder::pushIntToken string tokens size: " + std::to_string(m_stringTokens.size()));
}

void StableKeyManager::StableKeyBuilder::pushStringToken(const std::string& a_sToken) {
    //log("StableKeyBuilder::pushStringToken: " + a_sToken);
    int nMarker = -1;
    m_sIntTokens.append(reinterpret_cast<const char*>(&nMarker), sizeof(int)); // Marker for string token
    m_stringTokens.push_back(std::make_shared<StringToken>(a_sToken));
    //log("StableKeyBuilder::pushStringToken string tokens size: " + std::to_string(m_stringTokens.size()));
}

void StableKeyManager::StableKeyBuilder::popToken() {
    //log("StableKeyBuilder::popToken in " + std::to_string(m_sIntTokens.size()));
    if (m_sIntTokens.size() >= sizeof(int)) {
        const int* pLastvalue = reinterpret_cast<const int*>(m_sIntTokens.data() + m_sIntTokens.size() - sizeof(int));
        //log("StableKeyBuilder::popToken last value: " + std::to_string(*pLastvalue));
        if (*pLastvalue == -1) { // String token
            m_stringTokens.pop_back(); // ASSUMPTION! We are assuming there is a matching string token for this -1
        }
        m_sIntTokens.resize(m_sIntTokens.size() - sizeof(int));
    }
    //log("StableKeyBuilder::popToken out " + std::to_string(m_sIntTokens.size()));
    //log("StableKeyBuilder::popToken string tokens size: " + std::to_string(m_stringTokens.size()));
}

StableKeyManager::StableKey StableKeyManager::StableKeyBuilder::build() {
    // Compute hash and combine
    //log("StableKeyBuilder::build in");
    uint64_t nHash = std::hash<std::string>{}(m_sIntTokens);
    //log("StableKeyBuilder::build here 1, hash after int tokens: " + std::to_string(nHash) + ", m_stringTokens tokens size: " + std::to_string(m_stringTokens.size()));
    for (const auto& strToken : m_stringTokens) {
        nHash ^= strToken->_hash + 0x9e3779b9 + (nHash << 6) + (nHash >> 2);
    }
    //log("StableKeyBuilder::build here 2");
    const auto& key = StableKeyManager::StableKey(nHash, m_sIntTokens, m_stringTokens);
    log("StableKeyBuilder::build: " + key.toString() + " built with hash: " +  std::to_string(nHash));
    return std::move(key);
}

} // namespace volt