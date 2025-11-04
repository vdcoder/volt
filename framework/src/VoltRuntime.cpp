#include "../include/VoltRuntime.hpp"
#include "../include/Diff.hpp"
#include "../include/Patch.hpp"

namespace volt {

// Thread-local current runtime for callback registration
thread_local VoltRuntime* currentRuntime = nullptr;

// ============================================================================
// VoltRuntime Implementation
// ============================================================================

VoltRuntime::VoltRuntime(const std::string& elementId) {
    // Get the DOM element to mount to
    val document = val::global("document");
    val element = document.call<val>("getElementById", elementId);
    
    if (element.isNull() || element.isUndefined()) {
        // Throw or handle error
        emscripten_log(EM_LOG_ERROR, "Volt: Element with id '%s' not found!", elementId.c_str());
        return;
    }
    
    rootElement = element.as_handle();
    emscripten::internal::_emval_incref(rootElement);
}

VoltRuntime::~VoltRuntime() {
    // Clean up DOM element reference
    if (rootElement) {
        emscripten::internal::_emval_decref(rootElement);
        rootElement = 0;
    }
    
    // Clean up current DOM tree
    if (currentDomTree) {
        emscripten::internal::_emval_decref(currentDomTree);
        currentDomTree = 0;
    }
    
    // Clear callbacks
    clearFrameCallbacks();
}

void VoltRuntime::scheduleRender() {
    if (hasInvalidated) {
        return; // Already scheduled
    }
    
    hasInvalidated = true;
    
    // Request animation frame with this runtime as user data
    emscripten_request_animation_frame(onAnimationFrame, this);
}

int VoltRuntime::registerEventCallback(EventCallback callback) {
    int id = eventCallbacks.size();
    // Wrap user callback with auto-invalidate
    eventCallbacks.push_back([this, callback = std::move(callback)]() {
        callback();
        this->scheduleRender();
    });
    return id;
}

int VoltRuntime::registerStringEventCallback(StringEventCallback callback) {
    int id = stringEventCallbacks.size();
    // Wrap user callback with auto-invalidate
    stringEventCallbacks.push_back([this, callback = std::move(callback)](const std::string& value) {
        callback(value);
        this->scheduleRender();
    });
    return id;
}

void VoltRuntime::invokeEventCallback(int id) {
    if (id >= 0 && id < static_cast<int>(eventCallbacks.size())) {
        eventCallbacks[id]();
    }
}

void VoltRuntime::invokeStringEventCallback(int id, const std::string& value) {
    if (id >= 0 && id < static_cast<int>(stringEventCallbacks.size())) {
        stringEventCallbacks[id](value);
    }
}

EM_BOOL VoltRuntime::onAnimationFrame(double timestamp, void* userData) {
    auto* runtime = static_cast<VoltRuntime*>(userData);
    
    if (runtime->hasInvalidated) {
        runtime->doRender();
        runtime->hasInvalidated = false;
    }
    
    return EM_FALSE; // Don't repeat automatically
}

void VoltRuntime::doRender() {
    if (!app) {
        return;
    }
    
    // Clear old callbacks before rendering
    clearFrameCallbacks();
    
    // Set current runtime so event helpers can register callbacks
    currentRuntime = this;
    
    // Render the new VTree
    VNode newVTree = app->render();
    
    // Clear current runtime
    currentRuntime = nullptr;
    
    if (!currentVTree.has_value()) {
        // Initial render: create DOM from scratch
        currentDomTree = renderVNode(newVTree);
        
        // Clear the root element and append new content
        val root = val::take_ownership(rootElement);
        emscripten::internal::_emval_incref(rootElement); // Keep alive
        
        // Clear existing content (e.g., loading message)
        root.set("innerHTML", val(""));
        
        val child = val::take_ownership(currentDomTree);
        emscripten::internal::_emval_incref(currentDomTree); // Keep alive
        
        root.call<void>("appendChild", child);
        
        currentVTree = std::move(newVTree);
    } else {
        // Diff the old and new trees
        DiffNode diff = diffNodes(*currentVTree, newVTree);
        patch(currentDomTree, diff);
        currentVTree = std::move(newVTree);
    }
}

void VoltRuntime::clearFrameCallbacks() {
    eventCallbacks.clear();
    stringEventCallbacks.clear();
}

} // namespace volt
