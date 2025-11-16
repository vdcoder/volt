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
    std::vector<VNode*> prevRootNodes = std::vector<VNode*>{a_pPrevVTree};
    std::vector<VNode*> newRootAsNodes = std::vector<VNode*>{a_pNewVTree};
    walk(
        a_pPrevVTree->isFragment() ? a_pPrevVTree->getChildren() : prevRootNodes,
        a_pNewVTree->isFragment() ? a_pNewVTree->getChildren() : newRootAsNodes,
        a_hRootContainer
    );
}

void VoltDiffPatch::walk(
    std::vector<VNode*>& a_prevNodes, 
    std::vector<VNode*>& a_newNodes, 
    emscripten::val a_hContainer) {

    log("VoltDiffPatch::walk called");
    for (auto pPrevNode : a_prevNodes) {
        log("  Prev Node StableKey: " + pPrevNode->getStableKey().toString());
    }
    log("----.");
    for (auto pNewNode : a_newNodes) {
        log("  New Node StableKey: " + pNewNode->getStableKey().toString());
    }
    log("..");

    // Walk both lists in parallel
    auto newIt = a_newNodes.begin();
    auto prevIt = a_prevNodes.begin();
    while (newIt != a_newNodes.end() && prevIt != a_prevNodes.end()) {
        VNode* pNewNode = *newIt;
        VNode* pPrevNode = *prevIt;

        log("VoltDiffPatch::walk: new is text: " + std::to_string(pNewNode->isText()) + " and key: " + pNewNode->getStableKey().toString() + " and text content: " + pNewNode->getText() + ", prev is text: " + std::to_string(pPrevNode->isText()) + " and key: " + pPrevNode->getStableKey().toString() + " and text content: " + pPrevNode->getText());

        if (pNewNode->isText() && pPrevNode->isText()) { // Both are text nodes, reuse regardless of stable identity
            log("VoltDiffPatch::walk: syncTextNodes called");
            syncTextNodes(pNewNode, pPrevNode);
            ++newIt;
            ++prevIt;
        } else if (pNewNode->getMatchingOldNode() == pPrevNode) { // Matches at the same position, ready to diff
            log("VoltDiffPatch::walk: syncNodes called");
            syncNodes(pNewNode);
            ++newIt;
            ++prevIt;
        }  else if (pNewNode->getMatchingOldNode() != nullptr) { // Matches node somewhere else, bring it in
            log("VoltDiffPatch::walk: bringAndSyncNodes called");
            bringAndSyncNodes(pNewNode, a_hContainer);
            ++newIt;
        } else { // New node, no match, add it
            log("VoltDiffPatch::walk: addNode called");
            addNode(pNewNode, a_hContainer);
            ++newIt;
        }
    }

    // Add any remaining new nodes, this set the new element handle
    while (newIt != a_newNodes.end()) {
        log("VoltDiffPatch::walk: addNode called for remaining new nodes");
        VNode* pNewNode = *newIt;
        addNode(pNewNode, a_hContainer);
        ++newIt;
    }

    // Remove any remaining prev nodes, this unlinks the VNode and removes the DOM child
    // but the element's val and VNode can still be brought-in by a later match
    while (prevIt != a_prevNodes.end()) {
        log("VoltDiffPatch::walk: removing remaining prev nodes");
        VNode* pPrevNode = *prevIt;
        dom::removeChild(a_hContainer, pPrevNode->getMatchingElement());
        pPrevNode->unlink();
        ++prevIt;
    }
    log("VoltDiffPatch::walk out");
}

void VoltDiffPatch::syncTextNodes(VNode* a_pNewNode, VNode* a_pPrevNode) {
    emscripten::val hElement = a_pPrevNode->getMatchingElement();

    if (a_pNewNode->getText() != a_pPrevNode->getText()) {
        log("VoltDiffPatch::syncTextNodes: Two Text nodes found, reuse with new content, from '" + a_pPrevNode->getText() + "' to '" + a_pNewNode->getText() + "'");
        hElement.set("nodeValue", emscripten::val(a_pNewNode->getText()));
    }
    else {
        // Both are text nodes with same content, reuse existing element
        log("VoltDiffPatch::syncTextNodes: Text content unchanged: '" + a_pNewNode->getText() + "'");
    }

    transferNode(a_pNewNode, hElement);
}

void VoltDiffPatch::syncNodes(VNode* a_pNewNode) {

    VNode* pOldNode = a_pNewNode->getMatchingOldNode();
    emscripten::val hElement = pOldNode->getMatchingElement();

    transferNode(a_pNewNode, hElement);

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

    log("old children: " + std::to_string(pOldNode->getChildren().size()));
    log("new children: " + std::to_string(a_pNewNode->getChildren().size()));
    VoltDiffPatch::walk(
        pOldNode->getChildren(), // The old node = prev node
        a_pNewNode->getChildren(),
        hElement
    );

    // for testing
    if (a_pNewNode->isText()) return;
    dom::setAttribute(hElement, std::string("voltid"), a_pNewNode->getStableKey().toString());
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

        // for testing
        dom::setAttribute(hNewElement, std::string("voltid"), a_pNewNode->getStableKey().toString());
        //dom::setAttribute(hNewElement, std::string("voltid"), a_pNewNode->getStableKey().toString());
    }

    transferNode(a_pNewNode, hNewElement);
    dom::appendChild(a_hContainer, hNewElement);
}

void VoltDiffPatch::transferNode(VNode* a_pNewNode, emscripten::val a_hElement) {
    // Update VNode <-> Element mapping
    a_pNewNode->setMatchingElement(a_hElement);

    // For bubble events. Set back-reference to this VNode in the DOM element
    uintptr_t pointerAddress = reinterpret_cast<uintptr_t>(a_pNewNode);
    a_hElement.set("__cpp_ptr", static_cast<double>(pointerAddress));
}

} // namespace volt