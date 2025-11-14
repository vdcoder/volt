#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "Attrs.hpp"
#include "Tags.hpp"
#include "StableKeyManager.hpp"
#include "VNodeHandle.hpp"

namespace volt {

// ============================================================================
// VNode - Virtual DOM Node
// ============================================================================

class VNode {
public:
    // Constructor
    VNode(tag::ETag a_nTag);

    // Runtime functions
    // -----

    bool bubbleCallback(std::string a_eventName, emscripten::val a_event) {
        auto it = m_bubbleEvents.find(a_eventName);
        if (it != m_bubbleEvents.end()) {
            it->second(a_event);
            return true;
        }
        return false;
    }

    // Render functions
    // -----
    tag::ETag getTag() { return m_nTag; }
    std::vector<std::pair<short, std::string>>& getProps() { return m_props; }
    std::unordered_map<std::string, std::function<void(emscripten::val)>>& getBubbleEvents() { return m_bubbleEvents; }
    std::vector<std::pair<short, std::function<void(emscripten::val)>>>& getNonBubbleEvents() { return m_nonBubbleEvents; }
    std::vector<VNode*>& getChildren() { return m_children; }

    // Setup VNode data
    void reuse(tag::ETag a_nTag);
    void setStableKey(StableKeyManager::StableKey a_stableKey);
    void setProps(std::vector<std::pair<short, std::string>> a_props);
    void setBubbleEvents(std::unordered_map<std::string, std::function<void(emscripten::val)>> a_events);
    void setNonBubbleEvents(std::vector<std::pair<short, std::function<void(emscripten::val)>>> a_events);
    void setChildren(std::vector<VNode*> a_children);

    // Set as text node
    void setAsText(std::string a_sTextContent);

    // Check if this is a text node
    bool isText() const { return m_nTag == tag::ETag::_TEXT; }
    
    // Check if this is a fragment node
    bool isFragment() const { return m_nTag == tag::ETag::_FRAGMENT; }

    // Get text content (only valid for TEXT nodes)
    std::string getText() const;

    // Intrusive
    StableKeyManager::StableKey& getStableKey() { return m_stableKey; }
    VNode* getMatchingOldNode() const { return m_pMatchingOldNode; }
    void setMatchingOldNode(VNode* a_pNode) { m_pMatchingOldNode = a_pNode; }
    emscripten::val getMatchingElement() const { return m_matchingElement; }
    void setMatchingElement(emscripten::val a_element) { m_matchingElement = a_element; }
    VNode* getNext() const { return m_pNext; }
    void setNext(VNode* a_pNode) { m_pNext = a_pNode; }
    void setParent(VNode* a_pParent) { m_pParent = a_pParent; }
    VNode* getParent() const { return m_pParent; }
    void unlink() {
        if (m_pParent) {
            bool found = m_pParent->m_children[0] == this;
            for (size_t i = 1; i < m_pParent->m_children.size(); ++i) {
                if (found)
                    m_pParent->m_children[i - 1] = m_pParent->m_children[i];
                found = found || (m_pParent->m_children[i] == this);
            }
            if (found)
                m_pParent->m_children.pop_back();
            m_pParent = nullptr;
        }
    }

private:
    std::unordered_map<std::string, std::function<void(emscripten::val)>> m_bubbleEvents;

    tag::ETag m_nTag;
    std::vector<std::pair<short, std::string>> m_props; // Kept sorted for efficient diffing
    std::vector<std::pair<short, std::function<void(emscripten::val)>>> m_nonBubbleEvents; // Kept sorted for efficient diffing
    std::vector<VNode*> m_children;

    // Intrusive storage for efficient reconciliation
    StableKeyManager::StableKey m_stableKey;  // Stable identifier for generational matching
    VNode* m_pMatchingOldNode = nullptr; // Pointer to the matching old node if available for diff/patch
    emscripten::val m_matchingElement = emscripten::val::undefined();  // Associated DOM element handle when available
    VNode* m_pNext = nullptr; // For hash bucket chaining and free list
    VNode* m_pParent = nullptr; // Needed to remove from parent during diff/patch
};

// ============================================================================
// Text Helper - Transparent container for textual content
// ============================================================================
inline VNodeHandle text(std::string a_sTextContent);

// ============================================================================
// Fragment Helper - Transparent container for grouping nodes
// ============================================================================
inline VNodeHandle fragment(std::vector<VNodeHandle> children);

template<typename... Children>
inline VNodeHandle fragment(Children&&... children);

// ============================================================================
// Map Helper - Creates a fragment with mapped children
// ============================================================================
// Usage:
//   map(users, [](const User& u, auto key) {
//       key(std::to_string(u.id));  // Optional: set stable key
//       return <div>(<h1>(<text>(u.name)), <p>(u.email));
//   })
//
// If you don't call key(), it uses positional index automatically.
// Calling key() provides stable identity for efficient list reconciliation.
// ============================================================================

template<typename Container, typename Renderer>
inline VNodeHandle map(const Container& container, Renderer renderer);

} // namespace volt