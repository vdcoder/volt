#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/html5.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include "IRuntime.hpp"
#include "App.hpp"
#include "IdManager.hpp"

namespace volt {

class VNode;

// ============================================================================
// VoltEngine - Isolated runtime for a single app instance
// ============================================================================

class VoltEngine : public IRuntime {
public:
    // Constructor: Create a runtime bound to a DOM element by ID
    explicit    VoltEngine                 (std::string a_sElementId);
    
    // Destructor: Clean up DOM and callbacks
                ~VoltEngine                ();
    
    // IRuntime interface implementation
    void        invalidate                  () override;
    
    // Mount app
    template<typename TApp> void 
                mountApp                    ();

    // Id manager for stable element mapping
    IdManager& 
                getIdManager                () { return m_idManager; }

    // VNode free list for recycling
    VNode*      recycleVNode                ();

private:
    // Render loop callback
    static EM_BOOL 
                onAnimationFrame            (double a_nTimestamp, void* a_pThisAsVoidStar);

    // Perform the actual render
    void        doRender                    ();

    // MEMBERS 

    // DOM element handle for mounting
    emscripten::val      
                m_hHostElement = emscripten::val::undefined();
    
    // Current app instance
    std::unique_ptr<App> 
                m_pApp;
    
    // Virtual DOM state
    VNode*      m_pCurrentVTree = nullptr;

    // Render scheduling
    bool        m_bHasInvalidated = false;

    // Id manager for stable element mapping
    IdManager   m_idManager;

    // VNode memory manager
    std::vector<std::unique_ptr<VNode>> 
                m_poolVNode;
    VNode*      m_pVNodeFreeListHead = nullptr; //free list for recycling
};

} // namespace volt
