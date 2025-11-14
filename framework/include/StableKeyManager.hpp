#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <memory>

namespace volt {

class VNode;

class StableKeyManager {
public:

    // KEY TOKENS

    class StringToken {
    public:
        StringToken(const std::string& a_sToken) : _hash(std::hash<std::string>{}(a_sToken)), _value(a_sToken) {}

        uint64_t _hash;
        std::string _value;
    };


    // KEY

    class StableKey {
    public:
        // Constructor for VNodes that don't require stable keys such as _TEXT and _FRAGMENT
        StableKey() : m_nHash(0) {}

        // Full constructor
        StableKey(uint64_t a_nHash, std::string a_sIntTokens, std::vector<std::shared_ptr<StringToken>> a_stringTokens) : m_nHash(a_nHash), m_sIntTokens(a_sIntTokens), m_stringTokens(a_stringTokens) {}

        // Id constructor
        StableKey(const std::string& a_sId) : m_nHash(std::hash<std::string>{}(a_sId)), m_sIntTokens(""), m_stringTokens({std::make_shared<StringToken>(a_sId)}) {}

        bool equals(const StableKey* a_pOther) const {
            return m_nHash == a_pOther->m_nHash && m_sIntTokens == a_pOther->m_sIntTokens && m_stringTokens == a_pOther->m_stringTokens;
        }

        uint64_t hash() const {
            return m_nHash;
        }

        void clear() {
            m_nHash = 0;
            m_sIntTokens.clear();
            m_stringTokens.clear();
        }

        std::string toString() const {
            std::string result;
            size_t intTokenCount = m_sIntTokens.size() / sizeof(int);
            size_t stringTokenIndex = 0;
            for (size_t i = 0; i < intTokenCount; ++i) {
                const int* pValue = reinterpret_cast<const int*>(m_sIntTokens.data() + i * sizeof(int));
                if (*pValue == -1) { // String token
                    if (stringTokenIndex < m_stringTokens.size()) {
                        result += "Id: " + m_stringTokens[stringTokenIndex]->_value + "; ";
                        stringTokenIndex++;
                    }
                } else {
                    result += "Index[" + std::to_string(i) + "]: " + std::to_string(*pValue) + "; ";
                }
            }
            return "KeyInfo( " + result + ")";
        }

    private:
        uint64_t m_nHash;
        std::string m_sIntTokens;
        std::vector<std::shared_ptr<StringToken>> m_stringTokens;
    };

    // KEY BUILDER

    class StableKeyBuilder {
    public:
        StableKeyBuilder() {}

        // ASSUMPTION!
        void pushIntToken(int a_nToken);

        void pushStringToken(const std::string& a_sToken);

        // ASSUMPTION!
        void popToken();

        StableKey build();
    private:
        std::string m_sIntTokens;
        std::vector<std::shared_ptr<StringToken>> m_stringTokens;
    };

    class StableKeyBuilderIntTokenGuard {
    public:
        StableKeyBuilderIntTokenGuard(StableKeyBuilder* a_pBuilder, int a_nToken) : m_pBuilder(a_pBuilder) {
            m_pBuilder->pushIntToken(a_nToken);
        }
        ~StableKeyBuilderIntTokenGuard() {
            m_pBuilder->popToken();
        }
        // Move-only semantics
        StableKeyBuilderIntTokenGuard(StableKeyBuilderIntTokenGuard&& other) : m_pBuilder(other.m_pBuilder) {
            other.m_pBuilder = nullptr;
        }
        StableKeyBuilderIntTokenGuard(const StableKeyBuilderIntTokenGuard&) = delete;
        StableKeyBuilderIntTokenGuard& operator=(const StableKeyBuilderIntTokenGuard&) = delete;
    private:
        StableKeyBuilder * m_pBuilder;
    };

public:
    StableKeyManager();
    ~StableKeyManager() = default;

    // Starts a new generation
    // Returns a free list of VNodes
    void startNewGeneration(VNode** a_ppOutFreeListHead);

    // Match/link/injest a new generation VNode
    void matchVNode(VNode* a_pNode);


    // StableKeyBuilder - For building stable keys during VNode construction
    void pushIntToken(int a_nToken) { m_keyBuilder.pushIntToken(a_nToken); }
    void pushStringToken(const std::string& a_sToken) { m_keyBuilder.pushStringToken(a_sToken); }
    void popToken() { m_keyBuilder.popToken(); }
    StableKey build() { return std::move(m_keyBuilder.build()); }
    StableKeyBuilderIntTokenGuard makeGuard(int a_nToken) { 
        return StableKeyBuilderIntTokenGuard(&m_keyBuilder, a_nToken);
    }

private:

    // 2 GENERATION HASH COLLISION CHAIN

    class HashCollisionChain
    {
    public:
        HashCollisionChain();
        ~HashCollisionChain();

        // Starts a new generation, returns free list of VNodes
        void startNewGeneration(VNode** a_ppOutFreeListHead, VNode** a_ppOutFreeListTail);

        // Match/link/injest a new generation VNode
        bool matchVNode(VNode* a_pNode);

    private:
        // Returns true if found in old map, and sets outPrevNode to the previous node pointer
        bool findInOld(StableKey* a_pKey, VNode** a_ppOutPrevNode);

        // Returns true if found in new map
        bool findInNew(StableKey* a_pKey);

        VNode* m_pHead;
        VNode* m_pTail;
        VNode* m_pNextGenHead;
        VNode* m_pNextGenTail;
    };

    // MEMBERS
    std::vector<HashCollisionChain> 
                        m_hashChains;
    StableKeyBuilder    m_keyBuilder;
    std::string         m_sDuplicateKeyDescription = "";
};

} // namespace volt