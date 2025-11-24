#pragma once
#include "ETags.hpp"
#include "VNodeHandle.hpp"

namespace volt {

namespace tag {


// Helper to convert Tag enum to string
inline const char* tagToString(ETag tag);


// ============================================================================
// HTML Element Helper Functions
// ============================================================================

// Macro to generate overloads for a tag using variadic templates
// This allows natural syntax: div({props}, child1, child2, "text", ...)
#define VNODE_TAG_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName() { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {}); \
    } \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {std::forward<Children>(children)...}); \
    } \
    template<typename... Children> \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props, Children&&... children) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {std::forward<Children>(children)...}); \
    }

// Macro for self-closing tags (no children): (props), ()
#define VNODE_SELFCLOSING_HELPER(tagName, tagEnum) \
    inline VNodeHandle tagName(std::vector<std::pair<short, PropValueType>> props) { \
        return VNodeHandle(tag::ETag::tagEnum, std::move(props), {}); \
    } \
    inline VNodeHandle tagName() { \
        return VNodeHandle(tag::ETag::tagEnum, {}, {}); \
    }

// Text Node
inline VNodeHandle _text(std::string a_sTextContent) {
    return VNodeHandle(a_sTextContent);
}

// Fragment
VNODE_TAG_HELPER(_fragment, _FRAGMENT)

// Common HTML elements
VNODE_TAG_HELPER(doctype, doctype)
VNODE_TAG_HELPER(abbr, abbr)
VNODE_TAG_HELPER(acronym, acronym)
VNODE_TAG_HELPER(address, address)
VNODE_TAG_HELPER(a, a)
VNODE_TAG_HELPER(applet, applet)
VNODE_TAG_HELPER(area, area)
VNODE_TAG_HELPER(article, article)
VNODE_TAG_HELPER(aside, aside)
VNODE_TAG_HELPER(audio, audio)
VNODE_TAG_HELPER(base, base)
VNODE_TAG_HELPER(basefont, basefont)
VNODE_TAG_HELPER(bdi, bdi)
VNODE_TAG_HELPER(bdo, bdo)
VNODE_TAG_HELPER(bgsound, bgsound)
VNODE_TAG_HELPER(big, big)
VNODE_TAG_HELPER(blockquote, blockquote)
VNODE_TAG_HELPER(body, body)
VNODE_TAG_HELPER(b, b)
VNODE_TAG_HELPER(button, button)
VNODE_TAG_HELPER(caption, caption)
VNODE_TAG_HELPER(canvas, canvas)
VNODE_TAG_HELPER(center, center)
VNODE_TAG_HELPER(cite, cite)
VNODE_TAG_HELPER(code, code)
VNODE_TAG_HELPER(colgroup, colgroup)
VNODE_TAG_HELPER(col, col)
VNODE_TAG_HELPER(comment, comment)
VNODE_TAG_HELPER(data, data)
VNODE_TAG_HELPER(datalist, datalist)
VNODE_TAG_HELPER(dd, dd)
VNODE_TAG_HELPER(dfn, dfn)
VNODE_TAG_HELPER(del, del)
VNODE_TAG_HELPER(details, details)
VNODE_TAG_HELPER(dialog, dialog)
VNODE_TAG_HELPER(dir, dir)
VNODE_TAG_HELPER(div, div)
VNODE_TAG_HELPER(dl, dl)
VNODE_TAG_HELPER(dt, dt)
VNODE_TAG_HELPER(embed, embed)
VNODE_TAG_HELPER(em, em)
VNODE_TAG_HELPER(fieldset, fieldset)
VNODE_TAG_HELPER(figcaption, figcaption)
VNODE_TAG_HELPER(figure, figure)
VNODE_TAG_HELPER(font, font)
VNODE_TAG_HELPER(footer, footer)
VNODE_TAG_HELPER(form, form)
VNODE_TAG_HELPER(frame, frame)
VNODE_TAG_HELPER(frameset, frameset)
VNODE_TAG_HELPER(head, head)
VNODE_TAG_HELPER(header, header)
VNODE_TAG_HELPER(h1, h1)
VNODE_TAG_HELPER(h2, h2)
VNODE_TAG_HELPER(h3, h3)
VNODE_TAG_HELPER(h4, h4)
VNODE_TAG_HELPER(h5, h5)
VNODE_TAG_HELPER(h6, h6)
VNODE_TAG_HELPER(hgroup, hgroup)
VNODE_TAG_HELPER(html, html)
VNODE_TAG_HELPER(iframe, iframe)
VNODE_TAG_HELPER(ins, ins)
VNODE_TAG_HELPER(isindex, isindex)
VNODE_TAG_HELPER(i, i)
VNODE_TAG_HELPER(kbd, kbd)
VNODE_TAG_HELPER(keygen, keygen)
VNODE_TAG_HELPER(label, label)
VNODE_TAG_HELPER(legend, legend)
VNODE_TAG_HELPER(link, link)
VNODE_TAG_HELPER(li, li)
VNODE_TAG_HELPER(main, main)
VNODE_TAG_HELPER(mark, mark)
VNODE_TAG_HELPER(marquee, marquee)
VNODE_TAG_HELPER(menuitem, menuitem)
VNODE_TAG_HELPER(meta, meta)
VNODE_TAG_HELPER(meter, meter)
VNODE_TAG_HELPER(nav, nav)
VNODE_TAG_HELPER(nobr, nobr)
VNODE_TAG_HELPER(noembed, noembed)
VNODE_TAG_HELPER(noscript, noscript)
VNODE_TAG_HELPER(object, object)
VNODE_TAG_HELPER(optgroup, optgroup)
VNODE_TAG_HELPER(option, option)
VNODE_TAG_HELPER(output, output)
VNODE_TAG_HELPER(p, p)
VNODE_TAG_HELPER(param, param)
VNODE_TAG_HELPER(pre, pre)
VNODE_TAG_HELPER(progress, progress)
VNODE_TAG_HELPER(q, q)
VNODE_TAG_HELPER(rp, rp)
VNODE_TAG_HELPER(rt, rt)
VNODE_TAG_HELPER(ruby, ruby)
VNODE_TAG_HELPER(s, s)
VNODE_TAG_HELPER(samp, samp)
VNODE_TAG_HELPER(script, script)
VNODE_TAG_HELPER(section, section)
VNODE_TAG_HELPER(select, select)
VNODE_TAG_HELPER(small, small)
VNODE_TAG_HELPER(source, source)
VNODE_TAG_HELPER(spacer, spacer)
VNODE_TAG_HELPER(span, span)
VNODE_TAG_HELPER(strike, strike)
VNODE_TAG_HELPER(strong, strong)
VNODE_TAG_HELPER(sub, sub)
VNODE_TAG_HELPER(sup, sup)
VNODE_TAG_HELPER(summary, summary)
VNODE_TAG_HELPER(svg, svg)
VNODE_TAG_HELPER(table, table)
VNODE_TAG_HELPER(tbody, tbody)
VNODE_TAG_HELPER(td, td)
VNODE_TAG_HELPER(templatetag, templatetag)
VNODE_TAG_HELPER(textarea, textarea)
VNODE_TAG_HELPER(tfoot, tfoot)
VNODE_TAG_HELPER(th, th)
VNODE_TAG_HELPER(thead, thead)
VNODE_TAG_HELPER(time, time)
VNODE_TAG_HELPER(title, title)
VNODE_TAG_HELPER(tr, tr)
VNODE_TAG_HELPER(track, track)
VNODE_TAG_HELPER(tt, tt)
VNODE_TAG_HELPER(u, u)
VNODE_TAG_HELPER(var, var)
VNODE_TAG_HELPER(video, video)
VNODE_TAG_HELPER(wbr, wbr)
VNODE_TAG_HELPER(xmp, xmp)

// Self-closing elements
VNODE_SELFCLOSING_HELPER(input, input)
VNODE_SELFCLOSING_HELPER(img, img)
VNODE_SELFCLOSING_HELPER(br, br)
VNODE_SELFCLOSING_HELPER(hr, hr)

} // namespace tag

} // namespace volt