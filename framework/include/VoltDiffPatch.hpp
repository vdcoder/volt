#pragma once
#include <vector>
#include <emscripten/val.h>

namespace volt {

class VNode;

class VoltDiffPatch {
public:
    static void rebuild(VNode* a_pNewVTree, emscripten::val a_hRootContainer);
    static void diffPatch(VNode* a_pPrevVTree, VNode* a_pNewVTree, emscripten::val a_hRootContainer);
private:
    static void walk(    
        std::vector<VNode*>& a_prevNodes,
        std::vector<VNode*>& a_newNodes,
        emscripten::val a_hContainer);
    static void syncNodes(VNode* a_pNewNode);
    static void bringAndSyncNodes(VNode* a_pNewNode, emscripten::val a_hContainer);
    static void addNode(VNode* a_pNewNode, emscripten::val a_hContainer);
};

}