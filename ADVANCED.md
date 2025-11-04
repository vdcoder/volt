# ⚡ Volt Advanced Topics

## 🌐 Running Multiple Instances on the Same Page

One of Volt's unique features is the ability to run multiple isolated app instances on the same web page. This is achieved through GUID-based namespace isolation.

### Why Multiple Instances?

- **Micro-frontends**: Different teams can develop separate Volt apps that coexist
- **Widget libraries**: Create reusable Volt widgets that can be embedded multiple times
- **A/B testing**: Run different app versions side-by-side
- **Parallel worlds**: Test features in isolation without affecting main app

### How It Works

Each Volt app instance:
1. Has a unique GUID (Global Unique Identifier)
2. Gets its own JavaScript namespace: `window.volt_<sanitized_guid>`
3. Has isolated callbacks, state, and DOM tree
4. Can communicate with other instances via custom events or shared state

### Basic Example

Create two separate apps with different GUIDs:

**app1/src/App.hpp**:
```cpp
#include <Volt.hpp>
using namespace volt;

class App1 : public VoltApp {
    int count = 0;
public:
    VNode render() override {
        return div({ style("padding: 20px; border: 2px solid blue;") },
            h2("App 1 (Blue)"),
            p("Count: " + std::to_string(count)),
            button({ onClick([this] { count++; invalidate(); }) }, "Increment")
        );
    }
};
```

**app2/src/App.hpp**:
```cpp
#include <Volt.hpp>
using namespace volt;

class App2 : public VoltApp {
    int count = 0;
public:
    VNode render() override {
        return div({ style("padding: 20px; border: 2px solid red;") },
            h2("App 2 (Red)"),
            p("Count: " + std::to_string(count)),
            button({ onClick([this] { count++; invalidate(); }) }, "Increment")
        );
    }
};
```

### Building with Different GUIDs

```bash
# Create first app
./volt/framework/user-scripts/create-volt-app.sh app1 --guid "app1"
cd app1
./build.sh
cd ..

# Create second app
./volt/framework/user-scripts/create-volt-app.sh app2 --guid "app2"
cd app2
./build.sh
cd ..
```

### HTML Container Page

Create a single HTML page that loads both apps:

**index.html**:
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Multiple Volt Instances</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 1200px;
            margin: 50px auto;
            padding: 20px;
        }
        .container {
            display: flex;
            gap: 20px;
            margin-top: 20px;
        }
        .app-container {
            flex: 1;
        }
    </style>
</head>
<body>
    <h1>⚡ Multiple Volt Instances Demo</h1>
    <p>Two independent Volt applications running on the same page:</p>
    
    <div class="container">
        <div class="app-container">
            <div id="root1"></div>
        </div>
        <div class="app-container">
            <div id="root2"></div>
        </div>
    </div>

    <!-- Load App 1 -->
    <script src="app1/output/app.js"></script>
    <script>
        VoltApp().then(module => {
            console.log("App1 loaded with namespace:", module.getVoltNamespace());
            module.createRuntime();
        });
    </script>

    <!-- Load App 2 -->
    <script src="app2/output/app.js"></script>
    <script>
        VoltApp().then(module => {
            console.log("App2 loaded with namespace:", module.getVoltNamespace());
            module.createRuntime();
        });
    </script>
</body>
</html>
```

**Note**: You'll need to modify each app's `main.cpp` to mount to different root elements (`root1` and `root2`).

### Communication Between Instances

#### Method 1: Custom Events

**App1** dispatches events:
```cpp
EM_JS(void, dispatchCustomEvent, (const char* name, const char* data), {
    const event = new CustomEvent(UTF8ToString(name), { 
        detail: UTF8ToString(data) 
    });
    window.dispatchEvent(event);
});

// In your app
void sendMessage() {
    dispatchCustomEvent("app1-message", "Hello from App1");
}
```

**App2** listens for events:
```cpp
EM_JS(void, listenForEvents, (), {
    window.addEventListener('app1-message', (e) => {
        console.log('App2 received:', e.detail);
    });
});
```

#### Method 2: Shared Global State

```cpp
EM_JS(void, setSharedState, (const char* key, const char* value), {
    if (!window.sharedVoltState) window.sharedVoltState = {};
    window.sharedVoltState[UTF8ToString(key)] = UTF8ToString(value);
});

EM_JS(const char*, getSharedState, (const char* key), {
    if (!window.sharedVoltState) return "";
    const value = window.sharedVoltState[UTF8ToString(key)] || "";
    return allocateUTF8(value);
});
```

### GUID Sanitization

GUIDs with special characters are automatically sanitized:

- `my-app` → `volt_my_app`
- `app.v2` → `volt_app_v2`
- `app@test` → `volt_app_test`

Only alphanumeric characters and underscores are kept.

### Best Practices

1. **Unique GUIDs**: Always use unique GUIDs for each instance
2. **DOM Elements**: Each instance should mount to a different DOM element
3. **Naming**: Use descriptive GUIDs like `header-widget`, `sidebar-app`, `dashboard-v2`
4. **Communication**: Use custom events or shared state for inter-app communication
5. **Resource Loading**: Load WASM modules asynchronously to avoid blocking

### Use Cases

#### Dashboard with Multiple Widgets

```bash
./volt/framework/user-scripts/create-volt-app.sh header-widget --guid "header"
./volt/framework/user-scripts/create-volt-app.sh stats-widget --guid "stats"
./volt/framework/user-scripts/create-volt-app.sh chart-widget --guid "chart"
./volt/framework/user-scripts/create-volt-app.sh sidebar-widget --guid "sidebar"
```

Each widget can be developed, tested, and deployed independently.

#### A/B Testing

```bash
./volt/framework/user-scripts/create-volt-app.sh feature-a --guid "feature_a"
./volt/framework/user-scripts/create-volt-app.sh feature-b --guid "feature_b"
```

Show different features to different users on the same page for comparison.

#### Progressive Enhancement

Load a lightweight Volt app initially, then load additional apps on demand:

```javascript
// Load initial app immediately
VoltAppCore().then(module => {
    module.createRuntime();
});

// Load additional features later
setTimeout(() => {
    const script = document.createElement('script');
    script.src = 'advanced-feature/output/app.js';
    document.body.appendChild(script);
}, 5000);
```

### Debugging Multiple Instances

Open browser console and check:

```javascript
// List all Volt instances
Object.keys(window).filter(k => k.startsWith('volt_'));

// Inspect specific instance
window.volt_app1  // Should show the module exports
```

Each namespace contains:
- `getVoltNamespace()` - Returns the namespace string
- `createRuntime()` - Creates the app instance
- `invokeEventCallback(id)` - Invokes callbacks
- `invokeStringEventCallback(id, value)` - Invokes callbacks with values

### Limitations

- **Shared Resources**: All instances share the same browser resources (memory, CPU)
- **Name Collisions**: GUIDs must be unique to avoid namespace collisions
- **Load Order**: Script load order can affect initialization timing
- **Module Size**: Each instance loads its own WASM, so total size increases

### Performance Tips

1. **Lazy Loading**: Load instances only when needed
2. **Shared Dependencies**: Consider extracting shared code to reduce duplication
3. **Code Splitting**: Build separate apps for different features
4. **Resource Pooling**: Share expensive resources like WebSocket connections

---

For simpler use cases, start with a single instance. Multiple instances are an advanced feature for specific architectural needs.

**Questions?** Open an issue on [GitHub](https://github.com/vdcoder/volt/issues).
