#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "VNodeHandle.hpp"
#include "VNode.hpp"
#include "RenderingEngine.hpp"
#include "VoltEngine.hpp"
#include "EventBridge.hpp"

namespace volt {

// ============================================================================
// VNodeHandle Implementation
// ============================================================================

// ASSUMPTION! Constructor with stable key, assumed to be non-default
VNodeHandle::VNodeHandle(tag::ETag a_nTag, std::vector<std::pair<short, PropValueType>> a_props, std::vector<VNodeHandle> a_children) {
    m_pNode = g_pRenderingEngine->recycleVNode();
    m_pNode->reuse(a_nTag);

    // Separate props into attributes and event handlers
    std::vector<std::pair<short, std::string>> attrProps;
    std::unordered_map<std::string, std::function<void(emscripten::val)>> bubbleEventProps;
    std::vector<std::pair<short, std::function<void(emscripten::val)>>> nonBubbleEventProps;
    for (const auto& prop : a_props) {
        if (prop.first == attr::ATTR_undefined) {
            continue; // Skip undefined props
        }

        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                switch (prop.first)
                {
                case attr::ATTR_id:
                    m_pNode->setIdProp(arg);
                    attrProps.push_back({prop.first, std::move(arg)});
                    break;
                case attr::ATTR_key:
                    m_pNode->setKeyProp(arg);
                    break;
                default:
                    attrProps.push_back({prop.first, std::move(arg)});
                    break;
                }
            } else if constexpr (std::is_same_v<T, std::function<void(emscripten::val)>>) {
                switch (prop.first)
                {
                case attr::ATTR_EVT_onaddelement:
                    m_pNode->setOnAddElementEvent(std::move(arg));
                    break;
                case attr::ATTR_EVT_onbeforemoveelement:
                    m_pNode->setOnBeforeMoveElementEvent(std::move(arg));
                    break;
                case attr::ATTR_EVT_onmoveelement:
                    m_pNode->setOnMoveElementEvent(std::move(arg));
                    break;
                case attr::ATTR_EVT_onremoveelement:
                    m_pNode->setOnRemoveElementEvent(std::move(arg));
                    break;
                default:
                    auto wrapper = [arg = std::move(arg), runtimeInstance = g_pRenderingEngine](emscripten::val e) {
                        arg(e); 
                        runtimeInstance->invalidate();
                    };
                    if (prop.first >= attr::ATTR_EVT_NON_BUBBLE_START && prop.first < attr::ATTR_EVT_NON_BUBBLE_END) {
                        // Non-bubble event  
                        nonBubbleEventProps.push_back({prop.first, std::move(wrapper)});
                    } else {
                        // Bubble event
                        bubbleEventProps[attr::attrIdToName(prop.first)] = std::move(wrapper);
                    }
                    break;
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
        VNode* pChildNode = a_children[i].m_pNode;
        if (pChildNode->isFragment()) {
            // Flatten away inner fragments
            for (VNode* pGrandChild : pChildNode->getChildren()) {
                pGrandChild->setStableKeyPrefix(
                    IdManager::concatIds(
                        IdManager::concatIds(
                            pChildNode->getStableKeyPrefix(), 
                            pChildNode->getId()), 
                        pGrandChild->getStableKeyPrefix()));
                pGrandChild->setParent(m_pNode);
                children.push_back(pGrandChild);
            }
        }
        else {
            pChildNode->setParent(m_pNode);
            children.push_back(pChildNode);
        }
    }
    m_pNode->setChildren(std::move(children));
}

VNodeHandle::VNodeHandle(std::string_view a_sTextContent) {
    m_pNode = g_pRenderingEngine->recycleVNode();
    
    m_pNode->setAsText(a_sTextContent);
}

VNodeHandle::VNodeHandle(const char * a_sTextContent) {
    m_pNode = g_pRenderingEngine->recycleVNode();

    m_pNode->setAsText(std::string(a_sTextContent));
}

VNodeHandle::VNodeHandle(std::string a_sTextContent) {
    m_pNode = g_pRenderingEngine->recycleVNode();

    m_pNode->setAsText(std::move(a_sTextContent));
}

VNodeHandle VNodeHandle::track(int a_nStableKeyPosition) const { 
    m_pNode->setStableKeyPosition(a_nStableKeyPosition); 
    return *this;
}

VNodeHandle VNodeHandle::wrap(VNode * a_pNode) {
    return VNodeHandle(a_pNode);
}

VNodeHandle::VNodeHandle(VNode * a_pNode) : m_pNode(a_pNode) {}

} // namespace volt