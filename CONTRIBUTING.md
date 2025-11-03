# Contributing to Volt ⚡

Thank you for your interest in contributing to Volt! We welcome contributions from everyone.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Project Structure](#project-structure)
- [Testing](#testing)

## 📜 Code of Conduct

This project adheres to a code of conduct that promotes a welcoming and inclusive environment. By participating, you are expected to uphold this code:

- Be respectful and inclusive
- Welcome newcomers and help them get started
- Focus on what is best for the community
- Show empathy towards other community members

## 🚀 Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/volt.git
   cd volt
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/vdcoder/volt.git
   ```

## 🛠️ Development Setup

### Prerequisites

- Emscripten SDK (emsdk)
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Python 3.6+ (for development server)
- Git

### Install Emscripten

```bash
# Clone emsdk
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate latest
./emsdk install latest
./emsdk activate latest

# Set environment variables
source ./emsdk_env.sh
```

### Test the Setup

```bash
# Create a test app
./framework/user-scripts/create-volt-app.sh test-app

# Build it
cd test-app
./build.sh

# Run it
cd output
python3 -m http.server 8001
# Open http://localhost:8001
```

## 🤝 How to Contribute

### Reporting Bugs

Before creating a bug report, please check existing issues. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the behavior
- **Expected behavior**
- **Actual behavior**
- **Screenshots** (if applicable)
- **Environment details**: OS, browser, Emscripten version
- **Code samples** demonstrating the issue

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, include:

- **Clear title and description**
- **Use case**: Why would this enhancement be useful?
- **Possible implementation** (if you have ideas)
- **Examples** from other frameworks (if applicable)

### Contributing Code

1. **Find or create an issue** describing what you want to work on
2. **Comment on the issue** to let others know you're working on it
3. **Create a branch** from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Make your changes** following our coding standards
5. **Test your changes** thoroughly
6. **Commit your changes**:
   ```bash
   git commit -m "Add feature: brief description"
   ```
7. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```
8. **Open a Pull Request**

## 🔄 Pull Request Process

1. **Update documentation** for any changed functionality
2. **Add tests** for new features
3. **Ensure all tests pass**
4. **Update CHANGELOG.md** with your changes
5. **Request review** from maintainers
6. **Address review feedback** promptly
7. **Squash commits** if requested before merge

### PR Title Format

Use conventional commits style:

- `feat: add new feature`
- `fix: resolve bug in diffing algorithm`
- `docs: update API documentation`
- `refactor: simplify event handling`
- `perf: optimize DOM patching`
- `test: add tests for VNode`
- `chore: update build scripts`

## 📐 Coding Standards

### C++ Style

- **C++17 standard**: Use modern C++ features
- **Naming conventions**:
  - Classes: `PascalCase` (e.g., `VoltRuntime`)
  - Functions: `camelCase` (e.g., `scheduleRender`)
  - Variables: `camelCase` (e.g., `currentNode`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_DEPTH`)
  - Macros: `UPPER_SNAKE_CASE` (e.g., `VNODE_TAG_HELPER`)

- **Formatting**:
  - Indentation: 4 spaces (no tabs)
  - Line length: 100 characters maximum
  - Braces: Opening brace on same line
  - Pointers: `Type* ptr` (asterisk with type)

- **Best practices**:
  - Use `const` and `constexpr` where applicable
  - Prefer references over pointers when ownership is not transferred
  - Use RAII for resource management
  - Avoid raw `new`/`delete`, use smart pointers if needed
  - Keep functions focused and small

### Example

```cpp
class VoltRuntime {
private:
    std::string elementId;
    VNode* currentTree = nullptr;
    
public:
    explicit VoltRuntime(const std::string& id) 
        : elementId(id) {}
    
    template<typename TApp>
    void mount() {
        // Implementation
    }
};
```

### Comments

- Use `//` for single-line comments
- Use `/* */` for multi-line explanatory comments
- Document public APIs with clear descriptions
- Explain "why", not "what" (code should be self-documenting)

```cpp
// Good: Explains why
// Sanitize GUID to ensure valid JavaScript identifier
std::string sanitizeForJs(const std::string& guid) { ... }

// Bad: Explains what (obvious from code)
// Loop through all characters
for (char c : guid) { ... }
```

## 🏗️ Project Structure

```
volt/
├── LICENSE                        # MIT License
├── README.md                      # Main documentation
├── CONTRIBUTING.md                # This file
├── CHANGELOG.md                   # Version history
├── QUICKSTART.md                  # Quick reference
├── framework/                     # Core framework
│   ├── include/                   # Public headers
│   │   ├── Volt.hpp              # Single include
│   │   ├── VoltRuntime.hpp       # Runtime engine
│   │   ├── VoltConfig.hpp        # Configuration
│   │   ├── VNode.hpp             # Virtual DOM
│   │   ├── Attrs.hpp             # HTML attributes
│   │   ├── VoltEvents.hpp        # Event handlers
│   │   ├── Diff.hpp              # Diffing algorithm
│   │   ├── Patch.hpp             # DOM patching
│   │   └── String.hpp            # String utilities
│   ├── src/
│   │   └── VoltRuntime.cpp       # Runtime implementation
│   └── user-scripts/
│       ├── create-volt-app.sh    # App generator
│       └── update-framework.sh   # Framework updater
└── app-template/                  # Template for new apps
    ├── src/
    │   ├── App.hpp               # Template app
    │   └── main.cpp              # Boilerplate
    ├── build.sh                  # Build script
    ├── index.html                # HTML entry point
    ├── README.md                 # App documentation
    └── dependencies/             # Populated by create-volt-app
        └── volt/
```

### Key Components

- **VoltRuntime**: Core engine managing lifecycle and rendering
- **VNode**: Virtual DOM node representation
- **Diff**: Virtual DOM diffing algorithm
- **Patch**: DOM patching operations
- **VoltEvents**: Event handler helpers
- **Attrs**: HTML attribute definitions

## 🧪 Testing

### Manual Testing

1. Test the framework:
   ```bash
   ./framework/user-scripts/create-volt-app.sh test-app
   cd test-app
   ./build.sh
   cd output
   python3 -m http.server 8001
   ```

2. Test in browser:
   - Open http://localhost:8001
   - Check console for errors
   - Test event handlers
   - Verify rendering

### Testing Checklist

Before submitting a PR, verify:

- [ ] Code compiles without warnings
- [ ] `create-volt-app.sh` creates working apps
- [ ] Generated apps compile successfully
- [ ] Apps run correctly in browser
- [ ] Event handlers work (click, input, etc.)
- [ ] No console errors in browser
- [ ] Code follows style guidelines
- [ ] Documentation is updated

## 🔍 Areas to Contribute

### High Priority

- **Testing framework**: Automated tests for core functionality
- **Performance benchmarks**: Measure and optimize rendering speed
- **Documentation**: More examples and tutorials
- **Error handling**: Better error messages and validation

### Medium Priority

- **Component library**: Common UI components
- **DevTools**: Browser extension for debugging
- **Router**: Client-side routing solution
- **State management**: Built-in state management patterns

### Low Priority

- **TypeScript definitions**: For better IDE support
- **Package manager**: npm/CDN distribution
- **Build tools**: Webpack/Vite plugins
- **SSR support**: Server-side rendering

## 📞 Getting Help

- **GitHub Discussions**: Ask questions, share ideas
- **GitHub Issues**: Report bugs, request features
- **Discord**: (Coming soon) Real-time chat with community

## 🙏 Recognition

Contributors will be recognized in:

- README.md acknowledgments section
- CHANGELOG.md for each release
- GitHub contributors page

Thank you for making Volt better! ⚡

---

**Questions?** Open an issue or discussion on GitHub.
