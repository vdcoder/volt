#pragma once
#include <vector>
#include <unordered_set>
#include <emscripten/val.h>
#include "IdManager.hpp"
#include "FocusManager.hpp"

namespace volt {

class VNode;

class VoltDiffPatch {
public:
    static void rebuild(IdManager& a_idManager, FocusManager& a_focusManager, VNode* a_pNewVTree, emscripten::val a_hRootContainer);
    static void diffPatch(IdManager& a_idManager, FocusManager& a_focusManager, VNode* a_pPrevVTree, VNode* a_pNewVTree, emscripten::val a_hRootContainer);
private:
    static void walk(    
        IdManager& a_idManager, 
        FocusManager& a_focusManager,
        std::unordered_set<VNode*>& a_unclaimedOldNodes,
        std::vector<VNode*>& a_prevNodes,
        std::vector<VNode*>& a_newNodes,
        emscripten::val a_hContainer);
    static void syncTextNodes(
        IdManager& a_idManager, 
        VNode* a_pPrevNode, 
        VNode* a_pNewNode);
    static void syncNodes(
        IdManager& a_idManager, 
        FocusManager& a_focusManager,
        std::unordered_set<VNode*>& a_unclaimedOldNodes,
        VNode* a_pNewNode,
        VNode* a_pOldNode);
    static void bringAndSyncNodes(
        IdManager& a_idManager, 
        FocusManager& a_focusManager,
        std::unordered_set<VNode*>& a_unclaimedOldNodes,
        VNode* a_pNewNode, 
        VNode* a_pOldNode,
        emscripten::val a_hContainer,
        emscripten::val a_hReferenceNode);
    static void addNode(
        IdManager& a_idManager, 
        VNode* a_pNewNode, 
        emscripten::val a_hContainer,
        emscripten::val a_hReferenceNode);

    static void transferNode(
        VNode* a_pNewNode, emscripten::val a_hElement);
};

}