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
        case ETag::doctype: return "article";
        case ETag::abbr: return "abbr";
        case ETag::acronym: return "acronym";
        case ETag::address: return "address";
        case ETag::a: return "a";
        case ETag::applet: return "applet";
        case ETag::area: return "area";
        case ETag::article: return "article";
        case ETag::aside: return "aside";
        case ETag::audio: return "audio";
        case ETag::base: return "base";
        case ETag::basefont: return "basefont";
        case ETag::bdi: return "bdi";
        case ETag::bdo: return "bdo";
        case ETag::bgsound: return "bgsound";
        case ETag::big: return "big";
        case ETag::blockquote: return "blockquote";
        case ETag::body: return "body";
        case ETag::b: return "b";
        case ETag::br: return "br";
        case ETag::button: return "button";
        case ETag::caption: return "caption";
        case ETag::canvas: return "canvas";
        case ETag::center: return "center";
        case ETag::cite: return "cite";
        case ETag::code: return "code";
        case ETag::colgroup: return "colgroup";
        case ETag::col: return "col";
        case ETag::comment: return "comment";
        case ETag::data: return "data";
        case ETag::datalist: return "datalist";
        case ETag::dd: return "dd";
        case ETag::dfn: return "dfn";
        case ETag::del: return "del";
        case ETag::details: return "details";
        case ETag::dialog: return "dialog";
        case ETag::dir: return "dir";
        case ETag::div: return "div";
        case ETag::dl: return "dl";
        case ETag::dt: return "dt";
        case ETag::embed: return "embed";
        case ETag::em: return "em";
        case ETag::fieldset: return "fieldset";
        case ETag::figcaption: return "figcaption";
        case ETag::figure: return "figure";
        case ETag::font: return "font";
        case ETag::footer: return "footer";
        case ETag::form: return "form";
        case ETag::frame: return "frame";
        case ETag::frameset: return "frameset";
        case ETag::head: return "head";
        case ETag::header: return "header";
        case ETag::h1: return "h1";
        case ETag::h2: return "h2";
        case ETag::h3: return "h3";
        case ETag::h4: return "h4";
        case ETag::h5: return "h5";
        case ETag::h6: return "h6";
        case ETag::hgroup: return "hgroup";
        case ETag::hr: return "hr";
        case ETag::html: return "html";
        case ETag::iframe: return "iframe";
        case ETag::img: return "img";
        case ETag::input: return "input";
        case ETag::ins: return "ins";
        case ETag::isindex: return "isindex";
        case ETag::i: return "i";
        case ETag::kbd: return "kbd";
        case ETag::keygen: return "keygen";
        case ETag::label: return "label";
        case ETag::legend: return "legend";
        case ETag::link: return "link";
        case ETag::li: return "li";
        case ETag::main: return "main";
        case ETag::mark: return "mark";
        case ETag::marquee: return "marquee";
        case ETag::menuitem: return "menuitem";
        case ETag::meta: return "meta";
        case ETag::meter: return "meter";
        case ETag::nav: return "nav";
        case ETag::nobr: return "nobr";
        case ETag::noembed: return "noembed";
        case ETag::noscript: return "noscript";
        case ETag::object: return "object";
        case ETag::optgroup: return "optgroup";
        case ETag::option: return "option";
        case ETag::output: return "output";
        case ETag::p: return "p";
        case ETag::param: return "param";
        case ETag::pre: return "pre";
        case ETag::progress: return "progress";
        case ETag::q: return "q";
        case ETag::rp: return "rp";
        case ETag::rt: return "rt";
        case ETag::ruby: return "ruby";
        case ETag::s: return "s";
        case ETag::samp: return "samp";
        case ETag::script: return "script";
        case ETag::section: return "section";
        case ETag::select: return "select";
        case ETag::small: return "small";
        case ETag::source: return "source";
        case ETag::spacer: return "spacer";
        case ETag::span: return "span";
        case ETag::strike: return "strike";
        case ETag::strong: return "strong";
        case ETag::sub: return "sub";
        case ETag::sup: return "sup";
        case ETag::summary: return "summary";
        case ETag::svg: return "svg";
        case ETag::table: return "table";
        case ETag::tbody: return "tbody";
        case ETag::td: return "td";
        case ETag::templatetag: return "template";
        case ETag::textarea: return "textarea";
        case ETag::tfoot: return "tfoot";
        case ETag::th: return "th";
        case ETag::thead: return "thead";
        case ETag::time: return "time";
        case ETag::title: return "title";
        case ETag::tr: return "tr";
        case ETag::track: return "track";
        case ETag::tt: return "tt";
        case ETag::u: return "u";
        case ETag::ul: return "ul";
        case ETag::var: return "var";
        case ETag::video: return "video";
        case ETag::wbr: return "wbr";
        case ETag::xmp: return "xmp";
        default: return "div";
    }
}

} // namespace tag

} // namespace volt