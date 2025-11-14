#pragma once

#include "VoltDiffPatch.hpp"
#include "VNode.hpp"
#include "DOM.hpp"
#include <list>

namespace volt {

void VoltDiffPatch::rebuild(VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    // Clear existing content
    a_hRootContainer.set("innerHTML", emscripten::val(""));

    // Add all new nodes
    if (a_pNewVTree->isFragment()) {
        for (VNode* pNewNode : a_pNewVTree->getChildren()) {
            addNode(pNewNode, a_hRootContainer);
        }
    }
    else {
        addNode(a_pNewVTree, a_hRootContainer);
    }
}

void VoltDiffPatch::diffPatch(VNode* a_pPrevVTree, VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    std::vector<VNode*> prevRootNodes;
    if (a_pPrevVTree->isFragment()) {
        for (VNode* pChild : a_pPrevVTree->getChildren()) {
            prevRootNodes.push_back(pChild);
        }
    } else {
        prevRootNodes.push_back(a_pPrevVTree);
    }

    std::vector<VNode*> newRootAsNodes = std::vector<VNode*>{a_pNewVTree};
    walk(
        prevRootNodes,
        a_pNewVTree->isFragment() ? a_pNewVTree->getChildren() : newRootAsNodes,
        a_hRootContainer
    );
}

void VoltDiffPatch::walk(
    std::vector<VNode*>& a_prevNodes, 
    std::vector<VNode*>& a_newNodes, 
    emscripten::val a_hContainer) {

    // Walk both lists in parallel
    auto newIt = a_newNodes.begin();
    auto prevIt = a_prevNodes.begin();
    while (newIt != a_newNodes.end() && prevIt != a_prevNodes.end()) {
        VNode* pNewNode = *newIt;
        VNode* pPrevNode = *prevIt;
       
        if (pNewNode->getMatchingOldNode() == pPrevNode) { // Matches at the same position, ready to diff
            syncNodes(pNewNode);
            ++newIt;
            ++prevIt;
        } else if (pNewNode->getMatchingOldNode() != nullptr) { // Matches node somewhere else, bring it in
            bringAndSyncNodes(pNewNode, a_hContainer);
            ++newIt;
        } else { // New node, no match, add it
            addNode(pNewNode, a_hContainer);
            ++newIt;
        }
    }

    // Add any remaining new nodes, this set the new element handle
    while (newIt != a_newNodes.end()) {
        VNode* pNewNode = *newIt;
        addNode(pNewNode, a_hContainer);
        ++newIt;
    }

    // Remove any remaining prev nodes, this unlinks the VNode and removes the DOM child
    // but the element's val and VNode can still be brought-in by a later match
    while (prevIt != a_prevNodes.end()) {
        VNode* pPrevNode = *prevIt;
        dom::removeChild(a_hContainer, pPrevNode->getMatchingElement());
        pPrevNode->unlink();
        ++prevIt;
    }
}

void VoltDiffPatch::syncNodes(VNode* a_pNewNode) {

    VNode* pOldNode = a_pNewNode->getMatchingOldNode();
    emscripten::val hElement = pOldNode->getMatchingElement();

    // Sync/swapp internal things ("__cpp_ptr" and non-bubble event handlers)
    // ---------------------------

    // Update VNode <-> Element mapping
    a_pNewNode->setMatchingElement(hElement);

    // For bubble events. Set back-reference to this VNode in the DOM element
    uintptr_t pointerAddress = reinterpret_cast<uintptr_t>(a_pNewNode);
    hElement.set("__cpp_ptr", static_cast<double>(pointerAddress));

    // Sync non-bubble props
    // ---------------------------
    auto& newEvents = a_pNewNode->getNonBubbleEvents();
    auto& oldEvents = pOldNode->getNonBubbleEvents();
    size_t nNewEventIdx = 0;
    size_t nOldEventIdx = 0;

    // Walk both sorted vectors in parallel
    while (nOldEventIdx < oldEvents.size() || nNewEventIdx < newEvents.size()) {
        if (nOldEventIdx == oldEvents.size()) { // Remaining items in new are additions
            while (nNewEventIdx < newEvents.size()) { 
                hElement.set(
                    attr::attrIdToName(newEvents[nNewEventIdx].first), 
                    emscripten::val(newEvents[nNewEventIdx].second));
                ++nNewEventIdx;
            }
        } else if (nNewEventIdx == newEvents.size()) { // Remaining items in old are removals
            while (nOldEventIdx < oldEvents.size()) {
                hElement.delete_(attr::attrIdToName(oldEvents[nOldEventIdx].first));
                ++nOldEventIdx;
            }
        } else if (oldEvents[nOldEventIdx].first < newEvents[nNewEventIdx].first) {
            // Old key not in new = removal
            hElement.delete_(attr::attrIdToName(oldEvents[nOldEventIdx].first));
            ++nOldEventIdx;
        } else if (newEvents[nNewEventIdx].first < oldEvents[nOldEventIdx].first) {
            // New key not in old = addition
            hElement.set(
                attr::attrIdToName(newEvents[nNewEventIdx].first), 
                emscripten::val(newEvents[nNewEventIdx].second));
            ++nNewEventIdx;
        } else {
            // Same key, update to new callback
            hElement.set(
                attr::attrIdToName(newEvents[nNewEventIdx].first), 
                emscripten::val(newEvents[nNewEventIdx].second));
            ++nOldEventIdx;
            ++nNewEventIdx;
        }
    }

    // Sync props
    // ---------------------------
    auto& newProps = a_pNewNode->getProps();
    auto& oldProps = pOldNode->getProps();
    size_t nNewPropIdx = 0;
    size_t nOldPropIdx = 0;

    // Walk both sorted vectors in parallel
    while (nOldPropIdx < oldProps.size() || nNewPropIdx < newProps.size()) {
        if (nOldPropIdx == oldProps.size()) { // Remaining items in new are additions
            while (nNewPropIdx < newProps.size()) { 
                dom::setAttribute(
                    hElement,
                    attr::attrIdToName(newProps[nNewPropIdx].first),
                    newProps[nNewPropIdx].second.c_str()
                );
                ++nNewPropIdx;
            }
        } else if (nNewPropIdx == newProps.size()) { // Remaining items in old are removals
            while (nOldPropIdx < oldProps.size()) {
                dom::removeAttribute(
                    hElement,
                    attr::attrIdToName(oldProps[nOldPropIdx].first)
                );
                ++nOldPropIdx;
            }
        } else if (oldProps[nOldPropIdx].first < newProps[nNewPropIdx].first) {
            // Old key not in new = removal
            dom::removeAttribute(
                hElement,
                attr::attrIdToName(oldProps[nOldPropIdx].first)
            );
            ++nOldPropIdx;
        } else if (newProps[nNewPropIdx].first < oldProps[nOldPropIdx].first) {
            // New key not in old = addition
            dom::setAttribute(
                hElement,
                attr::attrIdToName(newProps[nNewPropIdx].first),
                newProps[nNewPropIdx].second.c_str()
            );
            ++nNewPropIdx;
        } else {
            // Same key, check if value changed
            if (oldProps[nOldPropIdx].second != newProps[nNewPropIdx].second) {
                dom::setAttribute(
                    hElement,
                    attr::attrIdToName(newProps[nNewPropIdx].first),
                    newProps[nNewPropIdx].second.c_str()
                );
            }
            ++nOldPropIdx;
            ++nNewPropIdx;
        }
    }

    // Sync children
    // ---------------------------

    VoltDiffPatch::walk(
        pOldNode->getChildren(), // The old node = prev node
        a_pNewNode->getChildren(),
        hElement
    );
}

void VoltDiffPatch::bringAndSyncNodes(VNode* a_pNewNode, emscripten::val a_hContainer) {
    // Bring existing old node from elsewhere in the old Vtree (and DOM)
    VNode* pMatchingOldNode = a_pNewNode->getMatchingOldNode();
    dom::removeChild(
        pMatchingOldNode->getParent()->getMatchingElement(),
        pMatchingOldNode->getMatchingElement());
    pMatchingOldNode->unlink();
    dom::appendChild(a_hContainer, a_pNewNode->getMatchingElement());

    // Sync props and children
    syncNodes(a_pNewNode);
}

void VoltDiffPatch::addNode(VNode* a_pNewNode, emscripten::val a_hContainer) {
    emscripten::val hNewElement = emscripten::val::undefined();

    if (a_pNewNode->isText()) {
        hNewElement = dom::createTextNode(a_pNewNode->getText());
    }
    else {
        hNewElement = dom::createElement(tag::tagToString(a_pNewNode->getTag()));

        for (auto & [attrId, value] : a_pNewNode->getProps()) {
            dom::setAttribute(hNewElement, attr::attrIdToName(attrId), value);
        }
        for (VNode* pChild : a_pNewNode->getChildren()) {
            addNode(pChild, hNewElement);
        }
    }

    a_pNewNode->setMatchingElement(hNewElement);
    uintptr_t pointerAddress = reinterpret_cast<uintptr_t>(a_pNewNode);
    hNewElement.set("__cpp_ptr", static_cast<double>(pointerAddress));

    dom::appendChild(a_hContainer, hNewElement);
}

} // namespace volt