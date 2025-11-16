#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "VNodeHandle.hpp"
#include "VNode.hpp"
#include "RenderingRuntime.hpp"
#include "VoltRuntime.hpp"

namespace volt {

// ============================================================================
// VNodeHandle Implementation
// ============================================================================

// ASSUMPTION! Constructor with stable key, assumed to be non-default
VNodeHandle::VNodeHandle(tag::ETag a_nTag, StableKeyManager::StableKey a_stableKey, std::vector<std::pair<short, PropValueType>> a_props, std::vector<VNodeHandle> a_children) 
    : VNodeHandle(a_nTag, std::move(a_props), std::move(a_children)) {
    // Set stable key: take id if available
    std::string sId;
    for (const auto& prop : a_props) {
        if (prop.first == attr::ATTR_ID) {
            sId = std::get<std::string>(prop.second);
            break;
        }
    }
    if (sId.empty()) {
        m_pNode->setStableKey(std::move(a_stableKey));
    } else {
        m_pNode->setStableKey(StableKeyManager::StableKey(sId));
    }

    // Use key to match and link this VNode with old generations' data
    StableKeyManager & keyManager = g_pRenderingRuntime->getKeyManager();
    keyManager.matchVNode(m_pNode); // ASSUMPTION! Assumes to be non-default stable key
}

VNodeHandle::VNodeHandle(tag::ETag a_nTag, std::vector<std::pair<short, PropValueType>> a_props, std::vector<VNodeHandle> a_children) {
    m_pNode = g_pRenderingRuntime->recycleVNode();
    m_pNode->reuse(a_nTag);

    // Separate props into attributes and event handlers
    std::vector<std::pair<short, std::string>> attrProps;
    std::unordered_map<std::string, std::function<void(emscripten::val)>> bubbleEventProps;
    std::vector<std::pair<short, std::function<void(emscripten::val)>>> nonBubbleEventProps;
    for (const auto& prop : a_props) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                attrProps.push_back({prop.first, std::move(arg)});
            } else if constexpr (std::is_same_v<T, std::function<void(emscripten::val)>>) {
                if (prop.first >= attr::ATTR_EVT_NON_BUBBLE_START && prop.first < attr::ATTR_EVT_NON_BUBBLE_END) {
                    // Non-bubble event
                    nonBubbleEventProps.push_back({prop.first, std::move(arg)});
                } else {
                    // Bubble event
                    bubbleEventProps[attr::attrIdToName(prop.first)] = std::move(arg);
                }
            }
        }, prop.second);
    }
    m_pNode->setProps(std::move(attrProps));
    m_pNode->setBubbleEvents(std::move(bubbleEventProps));
    m_pNode->setNonBubbleEvents(std::move(nonBubbleEventProps));

    // Extract child VNode pointers
    std::vector<VNode*> children;
    children.reserve(a_children.size());
    for (size_t i = 0; i < a_children.size(); ++i) {
        if (a_children[i].m_pNode->isFragment()) {
            // Flatten away inner fragments
            for (VNode* pGrandChild : a_children[i].m_pNode->getChildren()) {
                children.push_back(pGrandChild);
            }
        }
        else {
            children.push_back(a_children[i].m_pNode);
        }
    }
    m_pNode->setChildren(std::move(children));
}

VNodeHandle::VNodeHandle(std::string a_sTextContent) {
    m_pNode = g_pRenderingRuntime->recycleVNode();
    
    m_pNode->setAsText(a_sTextContent);
}

VNodeHandle::VNodeHandle(VNode * a_pNode) : m_pNode(a_pNode) {
}

} // namespace volt