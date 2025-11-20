#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <memory>

namespace volt {

class VNode;

class IdManager {
public:

    class StringStack {
    public:
        StringStack(): m_nSize(0) {}

        void push(std::string a_sItem) {
            if (m_nSize < m_stack.size()) {
                m_stack[m_nSize] = a_sItem;
            } else {
                m_stack.push_back(a_sItem);
            }
            m_nSize++;
        }

        // ASSUMPTION!
        void unsafe_pop() {
            m_nSize--; // ASSUMPTION! We are assuming there is a matching token to pop
        }

        // ASSUMPTION!
        std::string unsafe_top() {
            return m_stack[m_nSize - 1]; // ASSUMPTION! We are assuming there is at least one item
        }

        void clear() {
            m_nSize = 0;
        }
    private:
        int m_nSize;
        std::vector<std::string> m_stack;
    };

    class StableKeyBuilder {
    public:
        StableKeyBuilder() {
            m_stack.push(""); // Dummy entry to avoid empty stack checks
        }

        void pushString(const std::string& a_s) {
            m_stack.push(m_stack.unsafe_top() + a_s);
        }

        void pushVNodeToken(VNode* a_pNode);

        // ASSUMPTION! At least one token to pop
        void popToken() {
            m_stack.unsafe_pop(); // ASSUMPTION! At least one token to pop
        }

        std::string build() {
            return m_stack.unsafe_top();
        }
    private:
        StringStack m_stack;
    };

public:
    IdManager() {}
    ~IdManager() = default;

    // Starts a new generation
    // Returns a free list of VNodes
    void startGeneration(VNode** a_ppOutFreeListHead);

    static std::string concatIds(std::string a_sLeftId, std::string a_sRightId) {
        if (a_sRightId.empty()) { // Nothing to add
            return a_sLeftId;
        } else if (a_sRightId[0] == 'D') { // ID token always takes precedence
            return a_sRightId;
        }
        return a_sLeftId + a_sRightId;
    }

    std::string getBuilderId(VNode* a_pNode) {
        pushVNodeToken(a_pNode);
        std::string sId = build();
        popToken();
        return sId;
    }

    VNode* findVNode(std::string a_sId) {
        auto it = m_oldStore.find(a_sId);
        return it != m_oldStore.end() ? it->second : nullptr;
    }

    void addVNode(std::string a_sId, VNode* a_pNode) {
        auto it = m_newStore.find(a_sId);
        if (it != m_newStore.end()) {
            m_sDuplicateKeyDescription += a_sId + ", "; // Duplicate key detected
        }
        else {
            m_newStore[a_sId] = a_pNode;
        }
    }


    // StableKeyBuilder - For building stable keys during VNode construction
    void pushIntToken(int a_nToken) { m_keyBuilder.pushString("I" + std::to_string(a_nToken) + "_"); }
    void pushStringToken(const std::string& a_sToken) { m_keyBuilder.pushString("S" + a_sToken + "_"); }
    void pushVNodeToken(VNode* a_pNode) { m_keyBuilder.pushVNodeToken(a_pNode); }
    void popToken() { m_keyBuilder.popToken(); }
    std::string build() { return m_keyBuilder.build(); }

    std::string getDuplicateKeyDescription() const { return m_sDuplicateKeyDescription; }

private:
    // MEMBERS
    std::unordered_map<std::string, VNode*> 
                        m_oldStore;
    std::unordered_map<std::string, VNode*> 
                        m_newStore;
    StableKeyBuilder    m_keyBuilder;
    std::string         m_sDuplicateKeyDescription = "";
};

} // namespace volt