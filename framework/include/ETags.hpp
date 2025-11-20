#pragma once

namespace volt {

namespace tag {

// ============================================================================
// HTML Tag Enum - For performance
// ============================================================================
enum class ETag {
    _TEXT,       // Special tag for text nodes
    _FRAGMENT,   // Special tag for fragment containers (invisible wrapper)
    DIV,
    SPAN,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    P,
    A,
    BUTTON,
    INPUT,
    TEXTAREA,
    SELECT,
    OPTION,
    UL,
    OL,
    LI,
    TABLE,
    THEAD,
    TBODY,
    TR,
    TD,
    TH,
    FORM,
    LABEL,
    IMG,
    BR,
    HR,
    // Add more as needed
};

} // namespace tag

} // namespace volt