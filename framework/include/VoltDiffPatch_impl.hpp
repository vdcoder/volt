#include "VoltDiffPatch.hpp"
#include "VNode.hpp"
#include "DOM.hpp"
#include "VoltLog.hpp"

namespace volt {

void VoltDiffPatch::rebuild(IdManager& a_idManager, FocusManager& a_focusManager, VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    VOLT_INFO("Volt>DiffPatch", "rebuild() called: clearing container and rebuilding full tree");
    VOLT_LOG_INDENT_PUSH();

    // Clear existing content
    a_hRootContainer.set("innerHTML", emscripten::val(""));

    // Add all new nodes
    auto& children = a_pNewVTree->getChildren();
    VOLT_DEBUG("Volt>DiffPatch", "rebuild(): adding " + std::to_string(children.size()) + " root children");
    for (VNode* pNewNode : children) {
        VOLT_TRACE("Volt>DiffPatch", "rebuild(): addNode for root child");
        addNode(a_idManager, pNewNode, a_hRootContainer, emscripten::val::undefined());
    }

    VOLT_LOG_INDENT_POP();
}

void VoltDiffPatch::diffPatch(IdManager& a_idManager, FocusManager& a_focusManager, VNode* a_pPrevVTree, VNode* a_pNewVTree, emscripten::val a_hRootContainer) {
    VOLT_INFO("Volt>DiffPatch", "diffPatch() called: performing structural reuse between previous and new tree");
    VOLT_LOG_INDENT_PUSH();

    auto& prevChildren = a_pPrevVTree->getChildren();
    auto& newChildren  = a_pNewVTree->getChildren();
    VOLT_DEBUG(
        "Volt>DiffPatch",
        "diffPatch(): prevChildren=" + std::to_string(prevChildren.size()) +
        " newChildren=" + std::to_string(newChildren.size())
    );

    std::unordered_set<VNode*> unclaimedOldNodes;

    walk(
        a_idManager,
        a_focusManager,
        unclaimedOldNodes,
        prevChildren,
        newChildren,
        a_hRootContainer
    );

    // Remove any remaining unlinked nodes, this unlinks the VNode and removes the DOM child
    for (VNode* pUnclaimedNode : unclaimedOldNodes) {
        VOLT_DEBUG(
            "Volt>DiffPatch",
            "diffPatch(): removing unclaimed node in pending list with tag=" + pUnclaimedNode->getTagName()
        );

        pUnclaimedNode->onRemoveElement(pUnclaimedNode->getMatchingElement());
        dom::removeChild(pUnclaimedNode->getParent()->getMatchingElement(), pUnclaimedNode->getMatchingElement());
        pUnclaimedNode->unlink();
    }

    VOLT_LOG_INDENT_POP();
}

// ASSUMPTION!
void VoltDiffPatch::walk(
    IdManager& a_idManager, 
    FocusManager& a_focusManager,
    std::unordered_set<VNode*>& a_unclaimedOldNodes,
    std::vector<VNode*>& a_prevNodes, 
    std::vector<VNode*>& a_newNodes, 
    emscripten::val a_hContainer) {

    VOLT_DEBUG(
        "Volt>DiffPatch",
        "walk(): prevCount=" + std::to_string(a_prevNodes.size()) +
        " newCount=" + std::to_string(a_newNodes.size())
    );
    VOLT_LOG_INDENT_PUSH();
    
    // Walk both lists in parallel
    size_t newIdx = 0;
    size_t prevIdx = 0;
    while (newIdx < a_newNodes.size() && prevIdx < a_prevNodes.size()) {
        VNode* pNewNode  = a_newNodes[newIdx];
        VNode* pPrevNode = a_prevNodes[prevIdx];

        VOLT_TRACE(
            "Volt>DiffPatch",
            "walk(): newIdx=" + std::to_string(newIdx) +
            " prevIdx=" + std::to_string(prevIdx) +
            " newTag=" + pNewNode->getTagName() +
            " prevTag=" + pPrevNode->getTagName() +
            " newIsText=" + std::to_string(pNewNode->isText()) +
            " prevIsText=" + std::to_string(pPrevNode->isText())
        );

        if (pNewNode->isText() && pPrevNode->isText()) {
            // Both are text nodes, reuse regardless of stable identity
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "walk(): both text nodes → syncTextNodes and reuse DOM"
            );
            syncTextNodes(a_idManager, pPrevNode, pNewNode);
            ++newIdx;
            ++prevIdx;
        } else if (pNewNode->isText()) {
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "walk(): new is text, prev is not → addNode before prev DOM element"
            );
            addNode(a_idManager, pNewNode, a_hContainer, pPrevNode->getMatchingElement());
            ++newIdx;
        } else {
            a_idManager.pushVNodeToken(pNewNode);
            std::string sId = a_idManager.build();

            VOLT_TRACE(
                "Volt>DiffPatch",
                "walk(): non-text, built stable id: " + sId
            );

            VNode* pOldNode = a_idManager.findVNode(sId);

            if (pOldNode == pPrevNode) { // Matches at the same position, ready to diff
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "walk(): identity match at same index → syncNodes (reuse in place)"
                );
                syncNodes(a_idManager, a_focusManager, a_unclaimedOldNodes, pNewNode, pOldNode);
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
                ++prevIdx;
            } else if (
                a_focusManager.isFocused(pPrevNode->getMatchingElement()) && 
                pOldNode != nullptr) { 
                    // Matches node somewhere else, but prev-node has focus, bring matching element in, it will cause a move
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "walk(): identity match, prev-node has focus → bringAndSyncNodes (DOM move)"
                );
                bringAndSyncNodes(
                    a_idManager,
                    a_focusManager,
                    a_unclaimedOldNodes,
                    pNewNode,
                    pOldNode,
                    a_hContainer,
                    pPrevNode->getMatchingElement()
                );
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
            } else if (
                pOldNode != nullptr && 
                pOldNode->getParent() != nullptr && 
                pOldNode->getParent()->getMatchingElement() == a_hContainer && 
                a_unclaimedOldNodes.count(pOldNode) == 0) { 
                    // Matches an existing node, they are sibilings, and the old-node is linked & below us, remove/unlink all prev-nodes until old-node is at front, this avoids moves on the new nodes
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "walk(): identity match with sibling linked later on, unlink prev-nodes → syncNodes"
                );
                // Remove intervening prev nodes
                do {
                    a_unclaimedOldNodes.insert(a_prevNodes[prevIdx]);
                    ++prevIdx;
                } while (a_prevNodes[prevIdx] != pOldNode); // ASSUMPTION! There is a matching node later on
                // Now prev-node == pOldNode <matching> pNewNode
                syncNodes(a_idManager, a_focusManager, a_unclaimedOldNodes, pNewNode, pOldNode);
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
                ++prevIdx;
            } else if (pOldNode != nullptr) { // Matches node somewhere else, bring it in, it will cause a move
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "walk(): identity match at different index → bringAndSyncNodes (DOM move)"
                );
                // Remove from unclaimed set if present as it is being reused now
                if (a_unclaimedOldNodes.count(pOldNode) > 0) { 
                    a_unclaimedOldNodes.erase(pOldNode);
                }
                bringAndSyncNodes(
                    a_idManager,
                    a_focusManager,
                    a_unclaimedOldNodes,
                    pNewNode,
                    pOldNode,
                    a_hContainer,
                    pPrevNode->getMatchingElement()
                );
                a_idManager.addVNode(sId, pNewNode);
                ++newIdx;
            } else { // New node, no match, add it
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "walk(): no identity match → addNode as new DOM element"
                );
                // TEMPORARY: Balance stack before
                a_idManager.popToken();
                addNode(a_idManager, pNewNode, a_hContainer, pPrevNode->getMatchingElement());
                // TEMPORARY: Dummy token to balance stack
                a_idManager.pushIntToken(0);
                ++newIdx;
            }

            a_idManager.popToken();
        }
    }

    // Remove any remaining prev nodes, this unlinks the VNode and removes the DOM child
    // but the element's val and VNode can still be brought-in by a later match
    while (prevIdx < a_prevNodes.size()) {
        VNode* pPrevNode = a_prevNodes[prevIdx];
        VOLT_DEBUG(
            "Volt>DiffPatch",
            "walk(): removing remaining prev node at index=" + std::to_string(prevIdx) +
            " tag=" + pPrevNode->getTagName()
        );

        pPrevNode->onRemoveElement(pPrevNode->getMatchingElement());
        dom::removeChild(a_hContainer, pPrevNode->getMatchingElement());
        pPrevNode->unlink();
        // size() is already decremented by unlink()
    }

    // Add any remaining new nodes, this sets the new element handle
    while (newIdx < a_newNodes.size()) {
        VNode* pNewNode = a_newNodes[newIdx];
        VOLT_DEBUG(
            "Volt>DiffPatch",
            "walk(): addNode for remaining new node at index=" + std::to_string(newIdx) +
            " tag=" + pNewNode->getTagName()
        );
        addNode(a_idManager, pNewNode, a_hContainer, emscripten::val::undefined());
        ++newIdx;
    }

    VOLT_TRACE("Volt>DiffPatch", "walk(): completed");
    VOLT_LOG_INDENT_POP();
}

void VoltDiffPatch::syncTextNodes(
    IdManager& a_idManager, 
    VNode* a_pPrevNode, 
    VNode* a_pNewNode) {

    VOLT_TRACE("Volt>DiffPatch", "syncTextNodes(): entering");
    VOLT_LOG_INDENT_PUSH();

    emscripten::val hElement = a_pPrevNode->getMatchingElement();

    if (a_pNewNode->getText() != a_pPrevNode->getText()) {
        VOLT_DEBUG(
            "Volt>DiffPatch",
            "syncTextNodes(): changing text from '" + a_pPrevNode->getText() +
            "' to '" + a_pNewNode->getText() + "'"
        );
        hElement.set("nodeValue", emscripten::val(a_pNewNode->getText()));
    } else {
        VOLT_TRACE(
            "Volt>DiffPatch",
            "syncTextNodes(): text unchanged '" + a_pNewNode->getText() + "'"
        );
    }

    transferNode(a_pNewNode, hElement);

    VOLT_LOG_INDENT_POP();
    VOLT_TRACE("Volt>DiffPatch", "syncTextNodes(): leaving");
}

void VoltDiffPatch::syncNodes(
    IdManager& a_idManager, 
    FocusManager& a_focusManager,
    std::unordered_set<VNode*>& a_unclaimedOldNodes,
    VNode* a_pNewNode,
    VNode* a_pOldNode) {

    VOLT_TRACE(
        "Volt>DiffPatch",
        "syncNodes(): entering for tag='" + a_pNewNode->getTagName() + "'"
    );
    VOLT_LOG_INDENT_PUSH();

    emscripten::val hElement = a_pOldNode->getMatchingElement();

    transferNode(a_pNewNode, hElement);

    // Sync non-bubble props (event handlers)
    // ---------------------------
    auto& newEvents = a_pNewNode->getNonBubbleEvents();
    auto& oldEvents = a_pOldNode->getNonBubbleEvents();
    size_t nNewEventIdx = 0;
    size_t nOldEventIdx = 0;

    VOLT_TRACE(
        "Volt>DiffPatch",
        "syncNodes(): non-bubble events old=" + std::to_string(oldEvents.size()) +
        " new=" + std::to_string(newEvents.size())
    );

    // Walk both sorted vectors in parallel
    while (nOldEventIdx < oldEvents.size() || nNewEventIdx < newEvents.size()) {
        if (nOldEventIdx == oldEvents.size()) { // Remaining items in new are additions
            while (nNewEventIdx < newEvents.size()) { 
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "syncNodes(): adding non-bubble event attrId=" +
                    std::to_string(newEvents[nNewEventIdx].first)
                );
                hElement.set(
                    attr::attrIdToName(newEvents[nNewEventIdx].first), 
                    emscripten::val(newEvents[nNewEventIdx].second));
                ++nNewEventIdx;
            }
        } else if (nNewEventIdx == newEvents.size()) { // Remaining items in old are removals
            while (nOldEventIdx < oldEvents.size()) {
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "syncNodes(): removing non-bubble event attrId=" +
                    std::to_string(oldEvents[nOldEventIdx].first)
                );
                hElement.delete_(attr::attrIdToName(oldEvents[nOldEventIdx].first));
                ++nOldEventIdx;
            }
        } else if (oldEvents[nOldEventIdx].first < newEvents[nNewEventIdx].first) {
            // Old key not in new = removal
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "syncNodes(): removing non-bubble event attrId=" +
                std::to_string(oldEvents[nOldEventIdx].first)
            );
            hElement.delete_(attr::attrIdToName(oldEvents[nOldEventIdx].first));
            ++nOldEventIdx;
        } else if (newEvents[nNewEventIdx].first < oldEvents[nOldEventIdx].first) {
            // New key not in old = addition
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "syncNodes(): adding non-bubble event attrId=" +
                std::to_string(newEvents[nNewEventIdx].first)
            );
            hElement.set(
                attr::attrIdToName(newEvents[nNewEventIdx].first), 
                emscripten::val(newEvents[nNewEventIdx].second));
            ++nNewEventIdx;
        } else {
            // Same key, update to new callback
            VOLT_TRACE(
                "Volt>DiffPatch",
                "syncNodes(): updating non-bubble event attrId=" +
                std::to_string(newEvents[nNewEventIdx].first)
            );
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

    VOLT_TRACE(
        "Volt>DiffPatch",
        "syncNodes(): props old=" + std::to_string(oldProps.size()) +
        " new=" + std::to_string(newProps.size())
    );

    // Walk both sorted vectors in parallel
    while (nOldPropIdx < oldProps.size() || nNewPropIdx < newProps.size()) {
        if (nOldPropIdx == oldProps.size()) { // Remaining items in new are additions
            while (nNewPropIdx < newProps.size()) { 
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "syncNodes(): adding prop attrId=" +
                    std::to_string(newProps[nNewPropIdx].first)
                );
                dom::setAttribute(
                    hElement,
                    attr::attrIdToName(newProps[nNewPropIdx].first),
                    newProps[nNewPropIdx].second.c_str()
                );
                ++nNewPropIdx;
            }
        } else if (nNewPropIdx == newProps.size()) { // Remaining items in old are removals
            while (nOldPropIdx < oldProps.size()) {
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "syncNodes(): removing prop attrId=" +
                    std::to_string(oldProps[nOldPropIdx].first)
                );
                dom::removeAttribute(
                    hElement,
                    attr::attrIdToName(oldProps[nOldPropIdx].first)
                );
                ++nOldPropIdx;
            }
        } else if (oldProps[nOldPropIdx].first < newProps[nNewPropIdx].first) {
            // Old key not in new = removal
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "syncNodes(): removing prop attrId=" +
                std::to_string(oldProps[nOldPropIdx].first)
            );
            dom::removeAttribute(
                hElement,
                attr::attrIdToName(oldProps[nOldPropIdx].first)
            );
            ++nOldPropIdx;
        } else if (newProps[nNewPropIdx].first < oldProps[nOldPropIdx].first) {
            // New key not in old = addition
            VOLT_DEBUG(
                "Volt>DiffPatch",
                "syncNodes(): adding prop attrId=" +
                std::to_string(newProps[nNewPropIdx].first)
            );
            dom::setAttribute(
                hElement,
                attr::attrIdToName(newProps[nNewPropIdx].first),
                newProps[nNewPropIdx].second.c_str()
            );
            ++nNewPropIdx;
        } else {
            // Same key, check if value changed
            if (oldProps[nOldPropIdx].second != newProps[nNewPropIdx].second) {
                VOLT_DEBUG(
                    "Volt>DiffPatch",
                    "syncNodes(): updating prop attrId=" +
                    std::to_string(newProps[nNewPropIdx].first)
                );
                dom::setAttribute(
                    hElement,
                    attr::attrIdToName(newProps[nNewPropIdx].first),
                    newProps[nNewPropIdx].second.c_str()
                );
            } else {
                VOLT_TRACE(
                    "Volt>DiffPatch",
                    "syncNodes(): prop unchanged attrId=" +
                    std::to_string(newProps[nNewPropIdx].first)
                );
            }
            ++nOldPropIdx;
            ++nNewPropIdx;
        }
    }

    // Sync children
    // ---------------------------
    if (a_pOldNode->getChildren().size() > 0 || a_pNewNode->getChildren().size() > 0) {
        VOLT_TRACE(
            "Volt>DiffPatch",
            "syncNodes(): recursing into children oldCount=" +
            std::to_string(a_pOldNode->getChildren().size()) +
            " newCount=" +
            std::to_string(a_pNewNode->getChildren().size())
        );
        VoltDiffPatch::walk(
            a_idManager,
            a_focusManager, 
            a_unclaimedOldNodes,
            a_pOldNode->getChildren(), // The old node = prev node
            a_pNewNode->getChildren(),
            hElement
        );
    }

    VOLT_LOG_INDENT_POP();
    VOLT_TRACE(
        "Volt>DiffPatch",
        "syncNodes(): finished for tag='" + a_pNewNode->getTagName() + "'"
    );
}

void VoltDiffPatch::bringAndSyncNodes(
    IdManager& a_idManager, 
    FocusManager& a_focusManager,
    std::unordered_set<VNode*>& a_unclaimedOldNodes,
    VNode* a_pNewNode, 
    VNode* a_pOldNode,
    emscripten::val a_hContainer,
    emscripten::val a_hReferenceNode) {

    VOLT_DEBUG(
        "Volt>DiffPatch",
        "bringAndSyncNodes(): moving existing DOM node for tag='" +
        a_pNewNode->getTagName() + "'"
    );
    VOLT_LOG_INDENT_PUSH();

    // Remove from parents' child list
    a_pOldNode->unlink();

    a_pNewNode->onBeforeMoveElement(a_pOldNode->getMatchingElement());

    // Insert into new DOM position
    if (a_hReferenceNode.isUndefined()) {
        VOLT_TRACE(
            "Volt>DiffPatch",
            "bringAndSyncNodes(): appending at end of container"
        );
        dom::appendChild(a_hContainer, a_pOldNode->getMatchingElement());
    } else {
        VOLT_TRACE(
            "Volt>DiffPatch",
            "bringAndSyncNodes(): inserting before reference node"
        );
        dom::insertBefore(a_hContainer, a_pOldNode->getMatchingElement(), a_hReferenceNode);
    }

    a_pNewNode->onMoveElement(a_pOldNode->getMatchingElement());

    // Sync props and children
    syncNodes(a_idManager, a_focusManager, a_unclaimedOldNodes, a_pNewNode, a_pOldNode);

    VOLT_LOG_INDENT_POP();
}

void VoltDiffPatch::addNode(
    IdManager& a_idManager, 
    VNode* a_pNewNode, 
    emscripten::val a_hContainer,
    emscripten::val a_hReferenceNode) {

    VOLT_DEBUG(
        "Volt>DiffPatch",
        std::string("addNode(): creating new ") +
        (a_pNewNode->isText() ? "text node with >> " + a_pNewNode->getText() : "element node tag='" + a_pNewNode->getTagName() + "'")
    );
    VOLT_LOG_INDENT_PUSH();

    emscripten::val hNewElement = emscripten::val::undefined();

    if (a_pNewNode->isText()) {
        hNewElement = dom::createTextNode(a_pNewNode->getText());
    }
    else {
        hNewElement = dom::createElement(tag::tagToString(a_pNewNode->getTag()));

        for (auto & [attrId, value] : a_pNewNode->getProps()) {
            VOLT_TRACE(
                "Volt>DiffPatch",
                "addNode(): setting initial prop attrId=" + std::to_string(attrId)
            );
            dom::setAttribute(hNewElement, attr::attrIdToName(attrId), value);
        }

        a_idManager.pushVNodeToken(a_pNewNode);
        std::string sId = a_idManager.build();
        VOLT_TRACE("Volt>DiffPatch", "addNode(): registering stable id=" + sId);

        for (VNode* pChild : a_pNewNode->getChildren()) {
            addNode(a_idManager, pChild, hNewElement, emscripten::val::undefined());
        }
        
        a_idManager.popToken();

        a_idManager.addVNode(sId, a_pNewNode);

        a_pNewNode->onAddElement(hNewElement);
    }

    transferNode(a_pNewNode, hNewElement);

    if (a_hReferenceNode.isUndefined()) {
        VOLT_TRACE("Volt>DiffPatch", "addNode(): appending to container");
        dom::appendChild(a_hContainer, hNewElement);
    } else {
        VOLT_TRACE("Volt>DiffPatch", "addNode(): inserting before reference node");
        dom::insertBefore(a_hContainer, hNewElement, a_hReferenceNode);
    }

    VOLT_LOG_INDENT_POP();
}

void VoltDiffPatch::transferNode(
    VNode* a_pNewNode, 
    emscripten::val a_hElement) {

    VOLT_TRACE(
        "Volt>DiffPatch",
        "transferNode(): binding VNode to DOM element pointer"
    );
    VOLT_LOG_INDENT_PUSH();

    // Update VNode <-> Element mapping
    a_pNewNode->setMatchingElement(a_hElement);

    // For bubble events. Set back-reference to this VNode in the DOM element
    uintptr_t pointerAddress = reinterpret_cast<uintptr_t>(a_pNewNode);
    a_hElement.set("__cpp_ptr", static_cast<double>(pointerAddress));

    VOLT_LOG_INDENT_POP();
}

} // namespace volt
