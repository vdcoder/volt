# ⚡ Volt Quick Reference

## 📦 Installation

```bash
# Clone repository
git clone https://github.com/vdcoder/volt.git

# Create new app
./volt/framework/user-scripts/create-volt-app.sh my-app

# Build and run
cd my-app
./build.sh
cd output
python3 -m http.server 8001
```

## 🏗️ App Structure

```cpp
#include "dependencies/volt/include/Volt.hpp"
using namespace volt;

class MyApp : public VoltApp {
private:
    int state = 0;
    
    void updateState() {
        state++;
        invalidate();  // Trigger re-render
    }

public:
    VNode render() override {
        return div({},
            h1({}, "Title"),
            button({ onClick([this] { updateState(); }) }, "Click")
        );
    }
};

int main() {
    VoltRuntime runtime("root");
    runtime.mount<MyApp>();
    return 0;
}
```

## 🎨 HTML Elements

```cpp
// Structure
div({}, ...)
span({}, ...)
section({}, ...)
header({}, ...)
footer({}, ...)
nav({}, ...)

// Text
h1({}, "Heading")
p({}, "Paragraph")
a({ href("#") }, "Link")
strong({}, "Bold")
em({}, "Italic")

// Forms
form({}, ...)
input({ placeholder("Text...") })
textarea({}, ...)
button({}, "Click")
select({}, option({}, "Choice"))

// Lists
ul({},
    li({}, "Item 1"),
    li({}, "Item 2")
)
```

## 📝 Attributes

```cpp
// Common
id("my-id")
className("btn btn-primary")
style("color: red; font-size: 16px;")

// Forms
placeholder("Enter text...")
value("default")
disabled()
checked()
readonly()

// Links/Media
href("https://example.com")
src("/image.png")
alt("Description")
```

## 🎯 Event Handlers

```cpp
// Click
onClick([this] { /* handler */ })

// Input/Change
onInput([this](const std::string& val) { 
    // val contains input value
})
onChange([this](const std::string& val) { 
    // val contains select/input value
})

// Form
onSubmit([this] { 
    // Handle form submission
})

// Keyboard
onKeyDown([this](const std::string& key) { 
    // key is the key pressed
})
onKeyUp([this](const std::string& key) { 
    // key is the key released
})

// Focus
onFocus([this] { /* gained focus */ })
onBlur([this] { /* lost focus */ })
```

## 🔄 State Management

```cpp
class StatefulApp : public VoltApp {
private:
    // State variables
    std::string text = "";
    bool isActive = false;
    std::vector<std::string> items;
    
    // State updaters
    void setText(const std::string& newText) {
        text = newText;
        invalidate();  // Always call after state change
    }
    
    void toggleActive() {
        isActive = !isActive;
        invalidate();
    }
    
public:
    VNode render() override {
        return div({},
            input({ 
                value(text),
                onInput([this](const std::string& val) { 
                    setText(val); 
                })
            }),
            p({}, "You typed: " + text)
        );
    }
};
```

## 🧩 Component Patterns

### Conditional Rendering

```cpp
VNode render() override {
    if (isLoading) {
        return div({}, "Loading...");
    }
    return div({}, content());
}
```

### List Rendering

```cpp
VNode renderList() {
    std::vector<VNode> items;
    for (const auto& item : data) {
        items.push_back(
            li({}, item.name)
        );
    }
    return ul({}, items);
}
```

### Computed Values

```cpp
std::string getStatusClass() {
    return isActive ? "active" : "inactive";
}

VNode render() override {
    return div({ className(getStatusClass()) }, "Status");
}
```

## 🎭 4 Overload Patterns

Every HTML element supports 4 patterns:

```cpp
// 1. Empty element
div()

// 2. Text content only
div({}, "Hello")

// 3. Children only
div({}, child1, child2, child3)

// 4. Attributes + children
div({ id("main"), className("container") }, 
    h1({}, "Title"),
    p({}, "Content")
)
```

## 🔧 Build Configuration

### Custom GUID (for multiple instances)
```bash
VOLT_GUID='myapp_v1' ./build.sh
```

### Optimization Levels
Edit `build.sh` and change `-O3` to:
- `-O0` - No optimization (faster builds, debugging)
- `-O2` - Moderate optimization
- `-O3` - Full optimization (default)

## 🐛 Common Issues

### Error: "volt_X is not defined"

**Cause**: GUID contains special characters  
**Solution**: Framework auto-sanitizes, but ensure GUID doesn't start with number

### Multiple instances not isolated

**Cause**: Missing or duplicate GUID  
**Solution**: Set unique GUID for each app instance

### Event handler not firing

**Cause**: Forgot to call `invalidate()`  
**Solution**: Always call after state changes

### Build fails

**Cause**: Emscripten not activated  
**Solution**: Run `source ~/emsdk/emsdk_env.sh`

## 📚 Example Apps

### Counter

```cpp
class Counter : public VoltApp {
    int count = 0;
public:
    VNode render() override {
        return div({},
            h1({}, std::to_string(count)),
            button({ onClick([this] { count++; invalidate(); }) }, "+"),
            button({ onClick([this] { count--; invalidate(); }) }, "-")
        );
    }
};
```

### Todo List

```cpp
class TodoApp : public VoltApp {
    std::vector<std::string> todos;
    std::string input = "";
    
    void addTodo() {
        if (!input.empty()) {
            todos.push_back(input);
            input = "";
            invalidate();
        }
    }
    
public:
    VNode render() override {
        std::vector<VNode> items;
        for (const auto& todo : todos) {
            items.push_back(li({}, todo));
        }
        
        return div({},
            input({ 
                value(input),
                onInput([this](const std::string& v) { input = v; invalidate(); })
            }),
            button({ onClick([this] { addTodo(); }) }, "Add"),
            ul({}, items)
        );
    }
};
```

## 🔗 Resources

- **GitHub**: https://github.com/vdcoder/volt
- **Issues**: https://github.com/vdcoder/volt/issues
- **Full Docs**: See README.md and CONTRIBUTING.md

---

**Version**: 0.1.0  
**License**: MIT  
**Made with ⚡ by @vdcoder**
