#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "Attrs.hpp"
#include "StableKeyManager.hpp"
#include "Tags.hpp"

namespace volt {

class VNode;

// ============================================================================
// VNodeHandle - Virtual DOM Node Handle
// ============================================================================

typedef std::variant<std::string, std::function<void(emscripten::val)>> PropValueType;

// Node wrapper, so C++ compiler allows for adding text nodes conveniently
class VNodeHandle {
public:
    VNodeHandle(tag::ETag a_nTag, std::vector<std::pair<short, PropValueType>> a_props = {}, std::vector<VNodeHandle> a_children = {});
    VNodeHandle(tag::ETag a_nTag, StableKeyManager::StableKey a_stableKey, std::vector<std::pair<short, PropValueType>> a_props = {}, std::vector<VNodeHandle> a_children = {});
    VNodeHandle(std::string a_sTextContent);
    VNodeHandle(VNode * a_pNode);
    VNode * getNodePtr() const { return m_pNode; }
private:
    VNode * m_pNode;
};

} // namespace volt