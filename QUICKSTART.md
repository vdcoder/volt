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
#include <Volt.hpp>
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
        return div(
            h1("Title"),
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
h1("Heading")
p("Paragraph")
a({ href("#") }, "Link")
strong("Bold")
em("Italic")

// Forms
form({}, ...)
input({ placeholder("Text...") })
textarea({}, ...)
button("Click")
select(option("Choice"))

// Lists
ul(
    li("Item 1"),
    li("Item 2")
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
        return div("Loading...");
    }
    return div(content());
}
```

### List Rendering

```cpp
VNode renderList() {
    std::vector<VNode> items;
    for (const auto& item : data) {
        items.push_back(
            li(item.name)
        );
    }
    return ul({}, items);
}

// Using map() helper (cleaner!)
VNode renderList() {
    return ul({},
        map(data, [](const Item& item) {
            return li({}, item.name);
        })
    );
}
```

### Using Fragments

Fragments let you group nodes without adding extra DOM elements:

```cpp
// Return multiple nodes from a component
VNode renderHeader() {
    return fragment(
        h1("Title"),
        h2("Subtitle"),
        hr()
    );
}

// Mix fragments with regular elements
VNode render() override {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    return div(
        h1("My List"),
        map(numbers, [](int n) {
            return li(std::to_string(n));
        }),  // map() returns a fragment
        p("End of list")
    );
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

## 🧩 Component Patterns

Volt supports two component patterns for code reusability:

### Stateless Components (Inline)

Create components without state:

```cpp
class Button : public VoltRuntime::ComponentBase {
public:
    Button(IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render(const std::string& label, std::function<void()> onClick) {
        return button({
            volt::onClick([onClick, this]() {
                onClick();
                invalidate();
            }),
            style("padding: 10px; background: #007bff; color: white;")
        }, label);
    }
};

// Usage in App
VNode render() override {
    return div({},
        Button(this).render("Click Me", [this]() { count++; })
    );
}
```

### Stateful Components (Instance)

Components with persistent state:

```cpp
class Counter : public VoltRuntime::ComponentBase {
private:
    int count = 0;
    
public:
    Counter(IInvalidator* parent, int initial = 0)
        : ComponentBase(parent), count(initial) {}
    
    VNode render(const std::string& label) {
        return div({},
            h3({}, label + ": " + std::to_string(count)),
            button({onClick([this]() { count++; invalidate(); })}, "+")
        );
    }
};

// Usage in App
class MyApp : public VoltRuntime::AppBase {
private:
    Counter counter1;  // State persists across renders
    Counter counter2;
    
public:
    MyApp(VoltRuntime* runtime)
        : AppBase(runtime),
          counter1(this, 0),
          counter2(this, 10) {}
    
    VNode render() override {
        return div({},
            counter1.render("Counter A"),
            counter2.render("Counter B")
        );
    }
};
```

**📚 [Full Components Guide →](COMPONENTS.md)**

## 🎭 4 Overload Patterns

Every HTML element supports 4 patterns:

```cpp
// 1. Empty element
div()

// 2. Text content only
div("Hello")

// 3. Children only
div(child1, child2, child3)

// 4. Attributes + children
div({ id("main"), className("container") }, 
    h1("Title"),
    p("Content")
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
