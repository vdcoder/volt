# 🧩 Volt Components Guide

## Overview

Volt supports flexible component composition using `VoltRuntime::ComponentBase`. Unlike apps (which have a fixed `render()` signature), components can define custom render signatures, making them highly reusable and composable.

**Key Concepts:**
- Components receive `IRuntime*` interface (not full VoltRuntime access)
- **Auto-invalidate**: Event handlers automatically trigger re-renders - no manual `invalidate()` needed
- `IRuntime*` pointer is safe to capture in async callbacks (lives until unmount)
- Use `getRuntime()` to pass runtime interface to child components

## Two Component Patterns

### Pattern 1: Stateless Components (Inline)

Stateless components are created inline and used immediately:

```cpp
class Button : public VoltRuntime::ComponentBase {
public:
    Button(IRuntime* runtime) : ComponentBase(runtime) {}
    
    VNode render(const std::string& label, std::function<void()> onClick) {
        // Auto-invalidate: no need to call invalidate() in event handlers
        return button({onClick(onClick)}, label);
    }
};

// Usage in App::render()
VNode render() override {
    return div({},
        Button(getRuntime()).render("Click Me", [this]() { 
            count++;  // Auto-invalidate handles re-render
        })
    );
}
```

**When to use:**
- Simple, presentation-focused components
- No internal state needed
- Props fully determine output

### Pattern 2: Stateful Components (Instance)

Stateful components maintain state across renders:

```cpp
class Counter : public VoltRuntime::ComponentBase {
private:
    int count = 0;
    
public:
    Counter(IRuntime* runtime, int initial = 0) 
        : ComponentBase(runtime), count(initial) {}
    
    VNode render(const std::string& label) {
        return div({},
            h3(label + ": " + std::to_string(count)),
            button({onClick([this]() { 
                count++;  // Auto-invalidate
            })}, "+"),
            button({onClick([this]() { 
                count--;  // Auto-invalidate
            })}, "-")
        );
    }
    
    int getCount() const { return count; }
};

// Usage in App
class MyApp : public VoltRuntime::AppBase {
private:
    Counter counter1;
    Counter counter2;
    
public:
    MyApp(VoltRuntime* runtime) 
        : AppBase(runtime), 
          counter1(getRuntime(), 0), 
          counter2(getRuntime(), 10) {}
    
    VNode render() override {
        return div({},
            counter1.render("Counter A"),
            counter2.render("Counter B"),
            p({}, "Total: " + std::to_string(
                counter1.getCount() + counter2.getCount()
            ))
        );
    }
};
```

**When to use:**
- Component needs internal state
- State persists across parent re-renders
- Need to query component state from parent

## Complete Examples

### Example 1: Reusable Input Field

```cpp
class TextField : public VoltRuntime::ComponentBase {
private:
    std::string value;
    
public:
    TextField(VoltRuntime::IInvalidator* parent, const std::string& initial = "")
        : ComponentBase(parent), value(initial) {}
    
    VNode render(const std::string& label, const std::string& placeholder) {
        return div({style("margin: 10px 0;")}, {
            label({}, label),
            input({
                value(this->value),
                placeholder(placeholder),
                onInput([this](const std::string& val) {
                    this->value = val;
                    invalidate();
                })
            })
        });
    }
    
    std::string getValue() const { return value; }
    void setValue(const std::string& val) { value = val; invalidate(); }
};

// Usage
class FormApp : public VoltRuntime::AppBase {
private:
    TextField nameField;
    TextField emailField;
    
public:
    FormApp(VoltRuntime* runtime)
        : AppBase(runtime),
          nameField(this),
          emailField(this) {}
    
    VNode render() override {
        return div({style("padding: 20px;")},
            h1("User Form"),
            nameField.render("Name:", "Enter your name"),
            emailField.render("Email:", "Enter your email"),
            button({onClick([this]() {
                // Access field values
                std::string name = nameField.getValue();
                std::string email = emailField.getValue();
                // Process form...
                invalidate();
            })}, "Submit")
        );
    }
};
```

### Example 2: List Component with Items

```cpp
class ListItem : public VoltRuntime::ComponentBase {
public:
    ListItem(VoltRuntime::IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render(const std::string& text, std::function<void()> onDelete) {
        return li({style("padding: 5px; margin: 5px 0; border: 1px solid #ccc;")}, {
            span({}, text),
            button({
                onClick([onDelete, this]() { 
                    onDelete(); 
                    invalidate(); 
                }),
                style("margin-left: 10px;")
            }, "Delete")
        });
    }
};

class TodoList : public VoltRuntime::ComponentBase {
private:
    std::vector<std::string> items;
    
public:
    TodoList(VoltRuntime::IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render() {
        std::vector<VNode> itemNodes;
        for (size_t i = 0; i < items.size(); i++) {
            itemNodes.push_back(
                ListItem(this).render(items[i], [this, i]() {
                    items.erase(items.begin() + i);
                    invalidate();
                })
            );
        }
        
        return ul({}, itemNodes);
    }
    
    void addItem(const std::string& item) {
        items.push_back(item);
        invalidate();
    }
};

// Usage
class TodoApp : public VoltRuntime::AppBase {
private:
    TodoList list;
    TextField inputField;
    
public:
    TodoApp(VoltRuntime* runtime)
        : AppBase(runtime), list(this), inputField(this) {}
    
    VNode render() override {
        return div({style("padding: 20px;")},
            h1("Todo List"),
            inputField.render("New Item:", "Enter todo..."),
            button({onClick([this]() {
                std::string value = inputField.getValue();
                if (!value.empty()) {
                    list.addItem(value);
                    inputField.setValue("");
                    invalidate();
                }
            })}, "Add"),
            list.render()
        );
    }
};
```

### Example 3: Card Component (Stateless)

```cpp
class Card : public VoltRuntime::ComponentBase {
public:
    Card(VoltRuntime::IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render(
        const std::string& title,
        const std::string& content,
        const std::vector<VNode>& actions = {}
    ) {
        return div({style(
            "border: 1px solid #ddd; "
            "border-radius: 8px; "
            "padding: 16px; "
            "margin: 10px; "
            "box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
        )}, {
            h2({style("margin-top: 0;")}, title),
            p({}, content),
            div({style("margin-top: 16px;")}, actions)
        });
    }
};

// Usage
class DashboardApp : public VoltRuntime::AppBase {
private:
    int notifications = 5;
    
public:
    DashboardApp(VoltRuntime* runtime) : AppBase(runtime) {}
    
    VNode render() override {
        return div({}, {
            Card(this).render(
                "Notifications",
                "You have " + std::to_string(notifications) + " new messages",
                {
                    button({onClick([this]() { 
                        notifications = 0; 
                        invalidate(); 
                    })}, "Clear All")
                }
            ),
            Card(this).render(
                "Statistics",
                "App is running smoothly",
                {}
            )
        });
    }
};
```

## Best Practices

### Component Organization

```
src/
  App.hpp              # Main app
  main.cpp             # Entry point
  components/
    Button.hpp         # Reusable button component
    TextField.hpp      # Input field component
    Card.hpp           # Card layout component
    List.hpp           # List container
```

### Naming Conventions

- **Components**: PascalCase (e.g., `TextField`, `TodoList`)
- **Render methods**: Always `render()` but with custom parameters
- **Props**: Pass as function parameters
- **State**: Private members

### Invalidation Pattern

```cpp
// ✅ Good: Component invalidates parent
button({onClick([this]() {
    this->state++;
    invalidate();  // Triggers app re-render
})}, "Click")

// ✅ Also good: Component calls parent callback
button({onClick([callback, this]() {
    callback();    // Parent handles state change
    invalidate();  // Parent triggers re-render
})}, "Click")
```

### Props vs State

```cpp
// Props: Passed to render()
VNode render(const std::string& label, int value) {
    return div({}, label + ": " + std::to_string(value));
}

// State: Private members
class Counter : public VoltRuntime::ComponentBase {
private:
    int count = 0;  // Internal state
    
public:
    VNode render(const std::string& label) {  // label is a prop
        return div({}, label + ": " + std::to_string(count));
    }
};
```

## Component Lifecycle

Unlike React, Volt components don't have explicit lifecycle hooks. Instead:

1. **Construction**: Initialize state in constructor
2. **Render**: Called by parent when needed
3. **State Updates**: Call `invalidate()` to trigger parent re-render
4. **Destruction**: Automatic when parent app is destroyed

```cpp
class LifecycleExample : public VoltRuntime::ComponentBase {
public:
    LifecycleExample(VoltRuntime::IInvalidator* parent) 
        : ComponentBase(parent) {
        // "componentDidMount" equivalent - runs once
        initializeData();
    }
    
    ~LifecycleExample() {
        // "componentWillUnmount" equivalent - cleanup
        cleanupResources();
    }
    
    VNode render() {
        // "render" - called on every parent re-render
        return div("Content");
    }
    
private:
    void initializeData() { /* setup */ }
    void cleanupResources() { /* cleanup */ }
};
```

## Performance Tips

### 1. Prefer Stateless for Simple Components

```cpp
// ✅ Fast: No instance overhead
Card(this).render("Title", "Content");

// ❌ Slower: Unnecessary instance for stateless component
Card card(this);  // Don't store if you don't need state
card.render("Title", "Content");
```

### 2. Store Components When State is Needed

```cpp
// ✅ Correct: State persists across renders
class App : public VoltRuntime::AppBase {
private:
    Counter counter;  // Member variable
    
public:
    App(VoltRuntime* rt) : AppBase(rt), counter(this) {}
    
    VNode render() override {
        return counter.render("Count");  // State maintained
    }
};
```

### 3. Use Move Semantics

```cpp
VNode render() {
    std::vector<VNode> children;
    children.push_back(Component(this).render());
    children.push_back(Component(this).render());
    
    return div({}, std::move(children));  // Move, don't copy
}
```

## Common Patterns

### Using Fragments and map()

Volt provides `fragment()` and `map()` helpers for cleaner list rendering:

```cpp
// Without map - manual loop
VNode render() {
    std::vector<VNode> items;
    for (const auto& name : names) {
        items.push_back(li({}, name));
    }
    return ul({}, items);
}

// With map - concise and clean!
VNode render() {
    return ul({},
        map(names, [](const std::string& name) {
            return li({}, name);
        })
    );
}
```

**Fragments for grouping without extra DOM:**
```cpp
// Return multiple elements from a component
class Header : public VoltRuntime::ComponentBase {
public:
    Header(VoltRuntime::IInvalidator* parent) : ComponentBase(parent) {}
    
    VNode render(const std::string& title, const std::string& subtitle) {
        return fragment({
            h1({}, title),
            h2({style("color: gray;")}, subtitle),
            hr()
        });
    }
};

// Usage - no wrapper div!
VNode render() override {
    return div({},
        Header(this).render("Welcome", "To my app"),
        p({}, "Content here...")
    );
}
```

**Complex mapping with fragments:**
```cpp
VNode render() {
    std::vector<Product> products = getProducts();
    
    return div(
        h1("Products"),
        map(products, [this](const Product& product) {
            return fragment(
                h3(product.name),
                p("$" + std::to_string(product.price)),
                button({onClick([this, product]() { 
                    addToCart(product); 
                    invalidate();
                })}, "Add"),
                hr()
            );
        })
    );
}
```

### Conditional Rendering

```cpp
VNode render(bool showAdvanced) {
    if (showAdvanced) {
        return AdvancedPanel(this).render();
    }
    return BasicPanel(this).render();
}
```

### List Rendering

```cpp
VNode render(const std::vector<std::string>& items) {
    std::vector<VNode> nodes;
    for (const auto& item : items) {
        nodes.push_back(ListItem(this).render(item));
    }
    return ul({}, nodes);
}
```

### Composition

```cpp
VNode render() {
    return Layout(this).render(
        Header(this).render("Title"),
        Content(this).render("Body"),
        Footer(this).render("© 2025")
    );
}
```

## Migrating from AppBase Pattern

**Before (No components):**
```cpp
class App : public VoltRuntime::AppBase {
    VNode render() override {
        return div({},
            button({onClick(...)}, "Button 1"),
            button({onClick(...)}, "Button 2"),
            button({onClick(...)}, "Button 3")
        );
    }
};
```

**After (With components):**
```cpp
class App : public VoltRuntime::AppBase {
    VNode render() override {
        return div({},
            Button(this).render("Button 1", [this]() { action1(); }),
            Button(this).render("Button 2", [this]() { action2(); }),
            Button(this).render("Button 3", [this]() { action3(); })
        );
    }
};
```

---

**Next Steps:**
- See [QUICKSTART.md](QUICKSTART.md) for basic component examples
- Check [app-template/src/components/](app-template/src/components/) for working examples
- Read [CONTRIBUTING.md](CONTRIBUTING.md) for component development guidelines

**Questions?** Open an issue on [GitHub](https://github.com/vdcoder/volt/issues).