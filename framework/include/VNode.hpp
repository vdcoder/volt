#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include "Attrs.hpp"
#include "ETags.hpp"
#include "Tags.hpp"
#include "VNodeHandle.hpp"

namespace volt {

// ============================================================================
// VNode - Virtual DOM Node
// ============================================================================

class VNode {
public:
    // Constructor
    VNode(tag::ETag a_nTag);

    // Runtime functions
    // -----

    bool bubbleCallback(std::string a_eventName, emscripten::val a_event) {
        auto it = m_bubbleEvents.find(a_eventName);
        if (it != m_bubbleEvents.end()) {
            it->second(a_event);
            return true;
        }
        return false;
    }
    void onAddElement(emscripten::val a_element) {
        m_onAddElementEvent(a_element);
    }
    void onBeforeMoveElement(emscripten::val a_element) {
        m_onBeforeMoveElementEvent(a_element);
    }
    void onMoveElement(emscripten::val a_element) {
        m_onMoveElementEvent(a_element);
    }
    void onRemoveElement(emscripten::val a_element) {
        m_onRemoveElementEvent(a_element);
    }

    // Render functions
    // -----
    tag::ETag getTag() { return m_nTag; }
    std::string getTagName() { return tag::tagToString(m_nTag); }
    std::vector<std::pair<short, std::string>>& getProps() { return m_props; }
    std::string getKeyProp() { return m_sKeyProp; }
    std::string getIdProp() { return m_sIdProp; }
    std::string getStableKeyPrefix() const { return m_sStableKeyPrefix; }
    std::string getId() { 
        std::string str = getIdProp();
        if (!str.empty()) {
            return "D" + str + "_";
        }
        str = getKeyProp();
        if (!str.empty()) {
            return "S" + str + "_";
        }
        if (m_nStableKeyPosition >= 0) {
            return "I" + std::to_string(m_nStableKeyPosition) + "_";
        }
        return "UNKNOWN_";
     }
    int getStableKeyPosition() { return m_nStableKeyPosition; }
    std::unordered_map<std::string, std::function<void(emscripten::val)>>& getBubbleEvents() { return m_bubbleEvents; }
    std::vector<std::pair<short, std::function<void(emscripten::val)>>>& getNonBubbleEvents() { return m_nonBubbleEvents; }
    std::vector<VNode*>& getChildren() { return m_children; }

    // Setup VNode data
    void reuse(tag::ETag a_nTag);
    void setStableKeyPrefix(const std::string& a_sStableKeyPrefix) { m_sStableKeyPrefix = a_sStableKeyPrefix; }
    void setStableKeyPosition(int a_stableKeyPosition) { m_nStableKeyPosition = a_stableKeyPosition; }
    void setIdProp(const std::string& a_sIdProp) { m_sIdProp = a_sIdProp; }
    void setKeyProp(const std::string& a_sKeyProp) { m_sKeyProp = a_sKeyProp; }
    void setProps(std::vector<std::pair<short, std::string>> a_props);
    void setBubbleEvents(std::unordered_map<std::string, std::function<void(emscripten::val)>> a_events);
    void setOnAddElementEvent(std::function<void(emscripten::val)> a_fn) { m_onAddElementEvent = a_fn; }
    void setOnBeforeMoveElementEvent(std::function<void(emscripten::val)> a_fn) { m_onBeforeMoveElementEvent = a_fn; }
    void setOnMoveElementEvent(std::function<void(emscripten::val)> a_fn) { m_onMoveElementEvent = a_fn; }
    void setOnRemoveElementEvent(std::function<void(emscripten::val)> a_fn) { m_onRemoveElementEvent = a_fn; }
    void setNonBubbleEvents(std::vector<std::pair<short, std::function<void(emscripten::val)>>> a_events);
    void setChildren(std::vector<VNode*> a_children);

    // Set as text node
    void setAsText(std::string a_sTextContent);

    // Check if this is a text node
    bool isText() const { return m_nTag == tag::ETag::_TEXT; }
    
    // Check if this is a fragment node
    bool isFragment() const { return m_nTag == tag::ETag::_FRAGMENT; }

    // Get text content (only valid for TEXT nodes)
    std::string getText() const;

    // Intrusive
    emscripten::val getMatchingElement() const { return m_matchingElement; }
    void setMatchingElement(emscripten::val a_element) { m_matchingElement = a_element; }
    void setParent(VNode* a_pParent) { m_pParent = a_pParent; }
    VNode* getParent() const { return m_pParent; }
    void unlink() {
        if (m_pParent) {
            bool found = m_pParent->m_children[0] == this;
            for (size_t i = 1; i < m_pParent->m_children.size(); ++i) {
                if (found)
                    m_pParent->m_children[i - 1] = m_pParent->m_children[i];
                found = found || (m_pParent->m_children[i] == this);
            }
            if (found)
                m_pParent->m_children.pop_back();
            m_pParent = nullptr;
        }
    }

private:
    std::unordered_map<std::string, std::function<void(emscripten::val)>> m_bubbleEvents;
    std::function<void(emscripten::val)> m_onAddElementEvent;
    std::function<void(emscripten::val)> m_onBeforeMoveElementEvent;
    std::function<void(emscripten::val)> m_onMoveElementEvent;
    std::function<void(emscripten::val)> m_onRemoveElementEvent;

    tag::ETag m_nTag;
    std::vector<std::pair<short, std::string>> m_props; // Kept sorted for efficient diffing
    std::vector<std::pair<short, std::function<void(emscripten::val)>>> m_nonBubbleEvents; // Kept sorted for efficient diffing
    std::vector<VNode*> m_children;
    std::string m_sStableKeyPrefix; // This is transferred from fragment parents to children when flattening, sometimes multiple levels deep
    int m_nStableKeyPosition; // Positional key token
    std::string m_sIdProp; // Cached id prop for quick access
    std::string m_sKeyProp; // Cached key prop for quick access

    // Intrusive storage for efficient reconciliation
    emscripten::val m_matchingElement = emscripten::val::undefined();  // Associated DOM element handle when available
    VNode* m_pParent = nullptr; // Needed to remove from parent during diff/patch
};

// ============================================================================
// Helpers
// ============================================================================

template<typename Renderer>
inline VNodeHandle code(Renderer a_fnRenderer);

template<typename PropsGenerator>
inline std::vector<std::pair<short, PropValueType>> code(PropsGenerator a_fnPropsGenerator);

template<typename Renderer>
inline VNodeHandle loop(int a_nRepeat, Renderer a_fnRenderer);

template<typename Container, typename Renderer>
inline VNodeHandle map(const Container& a_container, Renderer a_fnRenderer);

} // namespace volt