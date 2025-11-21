#include "VNode.hpp"
#include "Tags.hpp"

namespace volt {

// ============================================================================
// VNode Implementation
// ============================================================================

VNode::VNode(tag::ETag a_nTag) : m_nTag(a_nTag) {}

void VNode::reuse(tag::ETag a_nTag) {
    m_nTag = a_nTag;
    m_props.clear();
    m_sIdProp.clear();
    m_sKeyProp.clear();
    m_bubbleEvents.clear();
    m_onAddElementEvent = [](emscripten::val){};
    m_onBeforeMoveElementEvent = [](emscripten::val){};
    m_onMoveElementEvent = [](emscripten::val){};
    m_onRemoveElementEvent = [](emscripten::val){};
    m_nonBubbleEvents.clear();
    m_children.clear();
    m_sStableKeyPrefix.clear();
    m_nStableKeyPosition = -1;
    m_matchingElement = emscripten::val::undefined();
    m_pParent = nullptr;
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

    // Sort non-bubble events by attribute ID for efficient diffing
    std::sort(m_nonBubbleEvents.begin(), m_nonBubbleEvents.end(), 
        [](const auto& a_a, const auto& a_b) { return a_a.first < a_b.first; });
}

void VNode::setChildren(std::vector<VNode*> a_children) {
    m_children = std::move(a_children);
}

void VNode::setAsText(std::string a_sTextContent) {
    reuse(tag::ETag::_TEXT);
    m_props.push_back({attr::ATTR_NODEVALUE, std::move(a_sTextContent)});
}

// Get text content (only valid for TEXT nodes)
std::string VNode::getText() const {
    if (isText() && !m_props.empty() && m_props[0].first == attr::ATTR_NODEVALUE) {
        return m_props[0].second;
    }
    return "";
}

// ============================================================================
// Helpers
// ============================================================================

template<typename Renderer>
inline VNodeHandle code(Renderer a_fnRenderer) {
    return a_fnRenderer();
}

template<typename PropsGenerator>
inline std::vector<std::pair<short, PropValueType>> props(PropsGenerator a_fnPropsGenerator) {
    return a_fnPropsGenerator();
}

template<typename Renderer>
inline VNodeHandle loop(int a_nRepeat, Renderer a_fnRenderer) {
    std::vector<VNodeHandle> children;
    children.reserve(a_nRepeat);
    while (children.size() < a_nRepeat) {
        children.push_back(a_fnRenderer(children.size()).track(children.size()));
    }
    return tag::_fragment(children);
}

template<typename Container, typename Renderer>
inline VNodeHandle map(const Container& a_container, Renderer a_fnRenderer) {
    std::vector<VNodeHandle> children;
    children.reserve(a_container.size());
    for (const auto& item : a_container) {
        children.push_back(a_fnRenderer(item).track(children.size()));
    }
    return tag::_fragment(children);
}

} // namespace volt