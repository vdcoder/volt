#include "VNode.hpp"

namespace volt {

// ============================================================================
// VNode Implementation
// ============================================================================

VNode::VNode(tag::ETag a_nTag) : m_nTag(a_nTag) {}

void VNode::reuse(tag::ETag a_nTag) {
    m_nTag = a_nTag;
    m_props.clear();
    m_children.clear();
    m_stableKey.clear();
    m_pMatchingOldNode = nullptr;
    m_matchingElement = emscripten::val::undefined();
    m_pNext = nullptr;
    m_pParent = nullptr;
}

void VNode::setStableKey(StableKeyManager::StableKey a_stableKey) {
    m_stableKey = std::move(a_stableKey);
}

void VNode::setProps(std::vector<std::pair<short, std::string>> a_props) {
    m_props = std::move(a_props);
    
    // Sort props by attribute ID for efficient diffing
    std::sort(m_props.begin(), m_props.end(), 
        [](const auto& a_a, const auto& a_b) { return a_a.first < a_b.first; });
}

void VNode::setBubbleEvents(std::unordered_map<std::string, std::function<void(emscripten::val)>> a_events) {
    m_bubbleEvents = std::move(a_events);
}

void VNode::setNonBubbleEvents(std::vector<std::pair<short, std::function<void(emscripten::val)>>> a_events) {
    m_nonBubbleEvents = std::move(a_events);
}

void VNode::setChildren(std::vector<VNode*> a_children) {
    m_children = std::move(a_children);
}

void VNode::setAsText(std::string a_sTextContent) {
    reuse(tag::ETag::_TEXT);
    m_props.push_back({attr::ATTR_TEXTCONTENT, std::move(a_sTextContent)});
}

// Get text content (only valid for TEXT nodes)
std::string VNode::getText() const {
    if (isText() && !m_props.empty() && m_props[0].first == attr::ATTR_TEXTCONTENT) {
        return m_props[0].second;
    }
    return "";
}

// ============================================================================
// Text Helper - Transparent container for textual content
// ============================================================================
inline VNodeHandle text(std::string a_sTextContent) {
    return VNodeHandle(a_sTextContent);
}

// ============================================================================
// Fragment Helper - Transparent container for grouping nodes
// ============================================================================

inline VNodeHandle fragment(std::vector<VNodeHandle> a_children) {
    return VNodeHandle(tag::ETag::_FRAGMENT, StableKeyManager::StableKey(), {}, a_children);
}


template<typename... Children>
inline VNodeHandle fragment(Children&&... children) {
    return VNodeHandle(tag::ETag::_FRAGMENT, StableKeyManager::StableKey(), {}, {std::forward<Children>(children)...});
}

// ============================================================================
// Map Helper - Creates a fragment with mapped children
// ============================================================================
// Usage:
//   map(users, [](const User& u, auto key) {
//       key(std::to_string(u.id));  // Optional: set stable key
//       return <div>(<h1>(u.name), <p>(u.email));
//   })
//
// If you don't call key(), it uses positional index automatically.
// Calling key() provides stable identity for proper list reconciliation.
// ============================================================================

template<typename Container, typename Renderer>
inline VNodeHandle map(const Container& container, Renderer renderer) {
    std::vector<VNodeHandle> children;
    children.reserve(container.size());
    int mapIndex = 0;
    
    for (const auto& item : container) {
        // Push map index initially
        g_pRenderingRuntime->getKeyManager().pushIntToken(mapIndex);
        
        // Callback for user to optionally set custom key
        auto fnKey = [](const std::string& a_sKey) {
            // Replace the index with a custom key
            g_pRenderingRuntime->getKeyManager().popToken();  // Remove the index we just pushed
            g_pRenderingRuntime->getKeyManager().pushStringToken(a_sKey);
        };
        
        // Call renderer - it may call key() to set custom key
        VNodeHandle child = renderer(item, fnKey);

        children.push_back(child);

        // Pop back to restore builder state
        g_pRenderingRuntime->getKeyManager().popToken();
        
        mapIndex++;
    }
    
    return fragment(children);
}

} // namespace volt