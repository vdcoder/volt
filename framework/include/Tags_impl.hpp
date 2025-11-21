#include "Tags.hpp"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>

namespace volt {

namespace tag {

// Helper to convert Tag enum to string
inline const char* tagToString(ETag a_nTag) {
    switch (a_nTag) {
        case ETag::_TEXT: return "#text";
        case ETag::_FRAGMENT: return "#fragment";
        case ETag::DIV: return "div";
        case ETag::SPAN: return "span";
        case ETag::H1: return "h1";
        case ETag::H2: return "h2";
        case ETag::H3: return "h3";
        case ETag::H4: return "h4";
        case ETag::H5: return "h5";
        case ETag::H6: return "h6";
        case ETag::P: return "p";
        case ETag::A: return "a";
        case ETag::BUTTON: return "button";
        case ETag::INPUT: return "input";
        case ETag::TEXTAREA: return "textarea";
        case ETag::SELECT: return "select";
        case ETag::OPTION: return "option";
        case ETag::UL: return "ul";
        case ETag::OL: return "ol";
        case ETag::LI: return "li";
        case ETag::TABLE: return "table";
        case ETag::THEAD: return "thead";
        case ETag::TBODY: return "tbody";
        case ETag::TR: return "tr";
        case ETag::TD: return "td";
        case ETag::TH: return "th";
        case ETag::FORM: return "form";
        case ETag::LABEL: return "label";
        case ETag::IMG: return "img";
        case ETag::BR: return "br";
        case ETag::HR: return "hr";
        case ETag::ARTICLE: return "article";
        default: return "div";
    }
}

} // namespace tag

} // namespace volt