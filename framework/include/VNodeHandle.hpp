#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "Attrs.hpp"
#include "ETags.hpp"

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
    VNodeHandle(std::string a_sTextContent);

    inline VNodeHandle track(int a_nStableKeyPosition) const;
    inline VNode * getNodePtr() const { return m_pNode; }

    static VNodeHandle wrap(VNode * a_pNode);
    
private:
    VNodeHandle(VNode * a_pNode);
    VNode * m_pNode;
};

} // namespace volt