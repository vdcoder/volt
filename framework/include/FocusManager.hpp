#pragma once

#include <unordered_set>
#include <emscripten/val.h>

namespace volt {

struct ValHash {
    std::size_t operator()(const emscripten::val& v) const noexcept {
        return std::hash<uintptr_t>{}(reinterpret_cast<uintptr_t>(v.as_handle())); 
    }
};

class FocusManager {
public:
    FocusManager() {}
    ~FocusManager() = default;

    bool isFocused(emscripten::val a_hElement) {
        return m_focusedElements.count(a_hElement) != 0;
    }

    void clear() {
        m_focusedElements.clear();
    }

    void add(emscripten::val a_hElement) {
        m_focusedElements.insert(a_hElement);
    }

private:
    // MEMBERS
    std::unordered_set<emscripten::val, ValHash> m_focusedElements;
};

} // namespace volt