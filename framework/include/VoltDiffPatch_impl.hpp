#pragma once

#include "VoltDiffPatch.hpp"
#include "VNode.hpp"
#include "DOM.hpp"
#include <list>

namespace volt {

// int level = 0;

// void llog(std::string msg) {
//     std::string indent(level * 2, ' ');
//     emscripten_log(EM_LOG_CONSOLE, "%s%s", indent.c_str(), msg.c_str());
// }

void VoltDiffPatch::rebuild(IdManager& a_idManager, VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    //level = 0;
    //log("VoltDiffPatch::rebuild called");

    // Clear existing content
    a_hRootContainer.set("innerHTML", emscripten::val(""));

    // Add all new nodes
    //level++;
    for (VNode* pNewNode : a_pNewVTree->getChildren()) {
        addNode(a_idManager, pNewNode, a_hRootContainer, emscripten::val::undefined());
    }
}

void VoltDiffPatch::diffPatch(IdManager& a_idManager, VNode* a_pPrevVTree, VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    //level = 0;
    //llog("VoltDiffPatch::diffPatch called");
    walk(
        a_idManager,
        a_pPrevVTree->getChildren(),
        a_pNewVTree->getChildren(),
        a_hRootContainer
    );
}

void VoltDiffPatch::walk(
    IdManager& a_idManager, 
    std::vector<VNode*>& a_prevNodes, 
    std::vector<VNode*>& a_newNodes, 
    emscripten::val a_hContainer) {

    //llog("VoltDiffPatch::walk called (0,0)");
    
    // Walk both lists in parallel
    size_t newIdx = 0;
    size_t prevIdx = 0;
    while (newIdx < a_newNodes.size() && prevIdx < a_prevNodes.size()) {
        VNode* pNewNode = a_newNodes[newIdx];
        VNode* pPrevNode = a_prevNodes[prevIdx];

        //llog("VoltDiffPatch::walk: NEW[" + std::to_string(newIdx) + "/" + std::to_string(a_newNodes.size()) + "] tag: " + pNewNode->getTagName() + " is text: " + std::to_string(pNewNode->isText()) + " and text content: " + pNewNode->getText() + ", PREV[" + std::to_string(prevIdx) + "/" + std::to_string(a_prevNodes.size()) + "] tag: " + pPrevNode->getTagName() + " is text: " + std::to_string(pPrevNode->isText()) + " and text content: " + pPrevNode->getText());

        if (pNewNode->isText() && pPrevNode->isText()) { // Both are text nodes, reuse regardless of stable identity
            //llog("VoltDiffPatch::walk: syncTextNodes called");
            syncTextNodes(a_idManager, pPrevNode, pNewNode);
            ++newIdx;
            ++prevIdx;
        } else if (pNewNode->isText()) {
            //llog("VoltDiffPatch::walk: add text node called");
            addNode(a_idManager, pNewNode, a_hContainer, pPrevNode->getMatchingElement());
            ++newIdx;
        } else {
            a_idManager.pushVNodeToken(pNewNode);
            std::string sId = a_idManager.build();
            //llog("VoltDiffPatch::walk: looking for VNode with id: " + sId);

            // if (sId == "Dfruit-Apple-line-span-9_") {
            //     log("VoltDiffPatch::walk: matched special debug id");
            //     a_idManager.toString()
            // }

            VNode* pOldNode = a_idManager.findVNode(sId);
            if (pOldNode == pPrevNode) { // Matches at the same position, ready to diff
                //llog("VoltDiffPatch::walk: syncNodes called");
                syncNodes(a_idManager, pNewNode, pOldNode);
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
                ++prevIdx;
            }  else if (pOldNode != nullptr) { // Matches node somewhere else, bring it in
                //llog("VoltDiffPatch::walk: bringAndSyncNodes called");
                bringAndSyncNodes(a_idManager, pNewNode, pOldNode, a_hContainer, pPrevNode->getMatchingElement());
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
            } else { // New node, no match, add it
                //llog("VoltDiffPatch::walk: addNode called");
                a_idManager.popToken(); // TEMPORARY: Balance stack before
                addNode(a_idManager, pNewNode, a_hContainer, pPrevNode->getMatchingElement());
                a_idManager.pushIntToken(0); // TEMPORARY: Dummy token to balance stack
                ++newIdx;
            }

            a_idManager.popToken();
        }
    }

    // Remove any remaining prev nodes, this unlinks the VNode and removes the DOM child
    // but the element's val and VNode can still be brought-in by a later match
    while (prevIdx < a_prevNodes.size()) {
        //llog("VoltDiffPatch::walk: removing remaining prev nodes");
        VNode* pPrevNode = a_prevNodes[prevIdx];
        pPrevNode->onRemoveElement(pPrevNode->getMatchingElement());
        dom::removeChild(a_hContainer, pPrevNode->getMatchingElement());
        pPrevNode->unlink();
        ++prevIdx;
    }

    // Add any remaining new nodes, this set the new element handle
    while (newIdx < a_newNodes.size()) {
        //llog("VoltDiffPatch::walk: addNode called for remaining new nodes");
        VNode* pNewNode = a_newNodes[newIdx];
        addNode(a_idManager, pNewNode, a_hContainer, emscripten::val::undefined());
        ++newIdx;
    }
    
    //llog("VoltDiffPatch::walk out");
}

void VoltDiffPatch::syncTextNodes(
    IdManager& a_idManager, 
    VNode* a_pPrevNode, 
    VNode* a_pNewNode) {
    emscripten::val hElement = a_pPrevNode->getMatchingElement();

    if (a_pNewNode->getText() != a_pPrevNode->getText()) {
        //llog("VoltDiffPatch::syncTextNodes: Two Text nodes found, reuse with new content, from '" + a_pPrevNode->getText() + "' to '" + a_pNewNode->getText() + "'");
        hElement.set("nodeValue", emscripten::val(a_pNewNode->getText()));
    }
    else {
        // Both are text nodes with same content, reuse existing element
        //llog("VoltDiffPatch::syncTextNodes: Text content unchanged: '" + a_pNewNode->getText() + "'");
    }

    transferNode(a_pNewNode, hElement);
}

void VoltDiffPatch::syncNodes(
    IdManager& a_idManager, 
    VNode* a_pNewNode,
    VNode* a_pOldNode) {

    emscripten::val hElement = a_pOldNode->getMatchingElement();

    transferNode(a_pNewNode, hElement);

    // Sync non-bubble props
    // ---------------------------
    auto& newEvents = a_pNewNode->getNonBubbleEvents();
    auto& oldEvents = a_pOldNode->getNonBubbleEvents();
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
    auto& oldProps = a_pOldNode->getProps();
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

    //llog("old children: " + std::to_string(a_pOldNode->getChildren().size()));
    //llog("new children: " + std::to_string(a_pNewNode->getChildren().size()));
    if (a_pOldNode->getChildren().size() > 0 || a_pNewNode->getChildren().size() > 0) {
        //level++;
        VoltDiffPatch::walk(
            a_idManager,
            a_pOldNode->getChildren(), // The old node = prev node
            a_pNewNode->getChildren(),
            hElement
        );
        //level--;
    }

    // for testing
    //if (a_pNewNode->isText()) return;
    //dom::setAttribute(hElement, std::string("voltid"), a_pNewNode->getStableKey().toString());
}

void VoltDiffPatch::bringAndSyncNodes(
    IdManager& a_idManager, 
    VNode* a_pNewNode, 
    VNode* a_pOldNode,
    emscripten::val a_hContainer,
    emscripten::val a_hReferenceNode) {

    // Remove from parents' child list
    a_pOldNode->unlink();

    a_pNewNode->onBeforeMoveElement(a_pOldNode->getMatchingElement());

    // Insert into new DOM position
    if (a_hReferenceNode.isUndefined()) {
        dom::appendChild(a_hContainer, a_pOldNode->getMatchingElement());
    } else {
        dom::insertBefore(a_hContainer, a_pOldNode->getMatchingElement(), a_hReferenceNode);
    }

    a_pNewNode->onMoveElement(a_pOldNode->getMatchingElement());

    // Sync props and children
    syncNodes(a_idManager, a_pNewNode, a_pOldNode);
}

void VoltDiffPatch::addNode(
    IdManager& a_idManager, 
    VNode* a_pNewNode, 
    emscripten::val a_hContainer,
    emscripten::val a_hReferenceNode) {

    emscripten::val hNewElement = emscripten::val::undefined();

    if (a_pNewNode->isText()) {
        hNewElement = dom::createTextNode(a_pNewNode->getText());
    }
    else {
        hNewElement = dom::createElement(tag::tagToString(a_pNewNode->getTag()));

        for (auto & [attrId, value] : a_pNewNode->getProps()) {
            dom::setAttribute(hNewElement, attr::attrIdToName(attrId), value);
        }

        // for (auto & [eventName, callback] : a_pNewNode->getNonBubbleEvents()) {
        //     hNewElement.set(eventName, emscripten::val(callback));
        // }

        //llog("VoltDiffPatch::addNode: processing children of new VNode, tag: " + std::string(tag::tagToString(a_pNewNode->getTag())));
        a_idManager.pushVNodeToken(a_pNewNode);
        std::string sId = a_idManager.build();
        //llog("VoltDiffPatch::addNode: adding VNode with id: " + sId);
        
        //level++;
        for (VNode* pChild : a_pNewNode->getChildren()) {
            addNode(a_idManager, pChild, hNewElement, emscripten::val::undefined());
        }
        //level--;
        
        a_idManager.popToken();

        a_idManager.addVNode(sId, a_pNewNode);

        a_pNewNode->onAddElement(hNewElement);

        // for testing
        //dom::setAttribute(hNewElement, std::string("voltid"), a_pNewNode->getStableKey().toString());
    }

    transferNode(a_pNewNode, hNewElement);

    if (a_hReferenceNode.isUndefined()) {
        dom::appendChild(a_hContainer, hNewElement);
    } else {
        dom::insertBefore(a_hContainer, hNewElement, a_hReferenceNode);
    }
}

void VoltDiffPatch::transferNode(
    VNode* a_pNewNode, 
    emscripten::val a_hElement) {

    // Update VNode <-> Element mapping
    a_pNewNode->setMatchingElement(a_hElement);

    // For bubble events. Set back-reference to this VNode in the DOM element
    uintptr_t pointerAddress = reinterpret_cast<uintptr_t>(a_pNewNode);
    a_hElement.set("__cpp_ptr", static_cast<double>(pointerAddress));
}

} // namespace volt