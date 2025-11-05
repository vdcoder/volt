#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/html5.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "VNode.hpp"

// Forward declarations to avoid including heavy inline function headers
struct DiffNode;
inline void patch(emscripten::EM_VAL, const DiffNode&);
inline DiffNode diffNodes(const VNode&, const VNode&);

namespace volt {

using namespace emscripten;

// Forward declaration
class VoltRuntime;

// ============================================================================
// IRuntime - Public interface for components
// ============================================================================
class IRuntime {
public:
    virtual void scheduleRender() = 0;
    virtual ~IRuntime() = default;
};

// ============================================================================
// VoltRuntime - Isolated runtime for a single app instance
// ============================================================================

class VoltRuntime : public IRuntime {
public:
    // Callback types
    using EventCallback = std::function<void()>;
    using StringEventCallback = std::function<void(const std::string&)>;
    
    // Base app interface
    class AppBase {
    private:
        VoltRuntime* runtime;
        
    public:
        AppBase(VoltRuntime* rt) : runtime(rt) {}
        virtual ~AppBase() = default;
        
        void invalidate() {
            if (runtime) {
                runtime->scheduleRender();
            }
        }
        
        // Get runtime pointer for creating child components
        VoltRuntime* getRuntime() { return runtime; }
        
        // Lifecycle: Called once after mount, before first render
        virtual void start() {}
        
        // Render method to be implemented by user's app
        virtual VNode render() = 0;
    };

    class ComponentBase {
    private:
        IRuntime* runtime;
        
    public:
        ComponentBase(IRuntime* rt) : runtime(rt) {}
        virtual ~ComponentBase() = default;

        void invalidate() {
            if (runtime) {
                runtime->scheduleRender();
            }
        }
        
        // Get runtime interface for creating child components
        IRuntime* getRuntime() { return runtime; }
    };

private:
    // DOM element handle for mounting
    EM_VAL rootElement = 0;
    
    // Current app instance
    std::unique_ptr<AppBase> app;
    
    // Virtual DOM state
    std::optional<VNode> currentVTree;
    EM_VAL currentDomTree = 0;
    
    // Render scheduling
    bool hasInvalidated = false;
    
    // Frame-scoped callback registry
    std::vector<EventCallback> eventCallbacks;
    std::vector<StringEventCallback> stringEventCallbacks;
    
public:
    // Constructor: Create a runtime bound to a DOM element by ID
    explicit VoltRuntime(const std::string& elementId);
    
    // Destructor: Clean up DOM and callbacks
    ~VoltRuntime();
    
    // IRuntime interface implementation
    void scheduleRender() override;
    
    // Mount an app to this runtime
    template<typename TApp>
    void mount() {
        static_assert(std::is_base_of<AppBase, TApp>::value, 
                      "App must inherit from VoltRuntime::AppBase");
        
        app = std::make_unique<TApp>(this);
        app->start();  // Call lifecycle method
        scheduleRender();
    }
    
    // Callback registry (called by Attrs.hpp helper functions)
    int registerEventCallback(EventCallback callback);
    int registerStringEventCallback(StringEventCallback callback);
    
    // Invoke callbacks from JavaScript (via instance pointer)
    void invokeEventCallback(int id);
    void invokeStringEventCallback(int id, const std::string& value);
    
private:
    // Render loop callback
    static EM_BOOL onAnimationFrame(double timestamp, void* userData);
    
    // Perform the actual render
    void doRender();
    
    // Clear frame-scoped callbacks
    void clearFrameCallbacks();
};

} // namespace volt
