#include "VoltEngine.hpp"

namespace volt {

//using namespace emscripten;
// struct DiffNode;
// inline void patch(emscripten::EM_VAL, const DiffNode&);
// inline DiffNode diffNodes(const VNode&, const VNode&);

// ============================================================================
// VoltEngine Implementation
// ============================================================================

VoltEngine::VoltEngine(std::string a_sElementId) {
    // Get the DOM element to mount to
    val document = val::global("document");
    val element = document.call<val>("getElementById", a_sElementId);

    if (element.isNull() || element.isUndefined()) {
        // Throw or handle error
        emscripten_log(EM_LOG_ERROR, "Volt: Element with id '%s' not found!", a_sElementId.c_str());
        return;
    }

    m_hHostElement = element;
}

VoltEngine::~VoltEngine() {
    // TODO: Clear and free other things here
}

void VoltEngine::invalidate() {
    if (m_bHasInvalidated) {
        return; // Already requested
    }

    m_bHasInvalidated = true;

    // Request animation frame with this runtime as user data
    emscripten_request_animation_frame(onAnimationFrame, this);
}

template<typename TApp> 
void VoltEngine::mountApp() {
    static_assert(std::is_base_of<App, TApp>::value, "App must inherit from VoltEngine::AppBase");
    m_pApp = std::make_unique<TApp>(*this);
    m_pApp->start();  // Call lifecycle method
    invalidate();
}

void VoltEngine::clearFocussedElements() {
    m_focusManager.clear();
}
    
void VoltEngine::addFocussedElement(emscripten::val a_hElement) {
    m_focusManager.add(a_hElement);
}

VNode* VoltEngine::recycleVNode() {
    if (m_pVNodeFreeListHead == nullptr) {
        // Allocate a new VNode
        m_poolVNode.push_back(std::make_unique<VNode>(tag::ETag::div));
        return m_poolVNode.back().get();
    } else {
        // Reuse from free list
        VNode* pFreeNode = m_pVNodeFreeListHead;
        m_pVNodeFreeListHead = m_pVNodeFreeListHead->getParent();
        return pFreeNode;
    }
}

EM_BOOL VoltEngine::onAnimationFrame(double a_nTimestamp, void* a_pThisAsVoidStar) {
    auto* pRuntime = static_cast<VoltEngine*>(a_pThisAsVoidStar);

    if (pRuntime->m_bHasInvalidated) {
        pRuntime->doRender();
        pRuntime->m_bHasInvalidated = false;
    }
    
    return EM_FALSE; // Don't repeat automatically
}

void VoltEngine::doRender() {
    //log("VoltEngine::doRender called");

    if (!m_pApp) {
        return;
    }

    //log("VoltEngine::doRender here 1");

    // Prepare key manager for new render
    m_idManager.startGeneration(&m_pVNodeFreeListHead);

    //log("VoltEngine::doRender here 2");

    //log("VoltEngine::doRender here 3");

    // Set rendering runtime, this simplifies user's final API
    g_pRenderingEngine = this;
    
    // Render the new VTree, put inside a fragment to always work with a list of children
    VNode* pNewVTree = tag::_fragment(m_pApp->render()).getNodePtr();

    // Clear rendering runtime
    g_pRenderingEngine = nullptr;

    //log("VoltEngine::doRender here 4");

    if (m_pCurrentVTree == nullptr) {
        // Initial render: create DOM from scratch
        VoltDiffPatch::rebuild(m_idManager, m_focusManager, pNewVTree, m_hHostElement);
    } else {
        // Reconcile the prev and new trees, then patch the DOM
        VoltDiffPatch::diffPatch(m_idManager, m_focusManager, m_pCurrentVTree, pNewVTree, m_hHostElement);
    }

    std::string duplicateKeyDescription = m_idManager.getDuplicateKeyDescription();
    if (!duplicateKeyDescription.empty()) {
        emscripten_log(EM_LOG_WARN, "Volt: Duplicate keys detected:\n%s", duplicateKeyDescription.c_str());
    }

    //log("VoltEngine::doRender here 5 out");

    m_pCurrentVTree = pNewVTree;
    //m_idManager.toString();
}

} // namespace volt
