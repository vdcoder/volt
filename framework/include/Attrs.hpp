#pragma once

#include <string>
#include <utility>
#include <functional>

namespace volt {

namespace attr {

// ============================================================================
// Attribute ID Constants
// ============================================================================
// Using short (2 bytes) instead of strings for memory efficiency

// Bubble Events (0-99)
constexpr short ATTR_EVT_onafterprint = 0; const char * SATTR_EVT_onafterprint = "afterprint";
constexpr short ATTR_EVT_onbeforeprint = 1; const char * SATTR_EVT_onbeforeprint = "beforeprint";
constexpr short ATTR_EVT_oncanplay = 2; const char * SATTR_EVT_oncanplay = "canplay";
constexpr short ATTR_EVT_oncanplaythrough = 3; const char * SATTR_EVT_oncanplaythrough = "canplaythrough";
constexpr short ATTR_EVT_onchange = 4; const char * SATTR_EVT_onchange = "change";
constexpr short ATTR_EVT_onclick = 5; const char * SATTR_EVT_onclick = "click";
constexpr short ATTR_EVT_oncontextmenu = 6; const char * SATTR_EVT_oncontextmenu = "contextmenu";
constexpr short ATTR_EVT_oncopy = 7; const char * SATTR_EVT_oncopy = "copy";
constexpr short ATTR_EVT_oncuechange = 8; const char * SATTR_EVT_oncuechange = "cuechange";
constexpr short ATTR_EVT_oncut = 9; const char * SATTR_EVT_oncut = "cut";
constexpr short ATTR_EVT_ondblclick = 10; const char * SATTR_EVT_ondblclick = "dblclick";
constexpr short ATTR_EVT_ondrag = 11; const char * SATTR_EVT_ondrag = "drag";
constexpr short ATTR_EVT_ondragend = 12; const char * SATTR_EVT_ondragend = "dragend";
constexpr short ATTR_EVT_ondragenter = 13; const char * SATTR_EVT_ondragenter = "dragenter";
constexpr short ATTR_EVT_ondragleave = 14; const char * SATTR_EVT_ondragleave = "dragleave";
constexpr short ATTR_EVT_ondragover = 15; const char * SATTR_EVT_ondragover = "dragover";
constexpr short ATTR_EVT_ondragstart = 16; const char * SATTR_EVT_ondragstart = "dragstart";
constexpr short ATTR_EVT_ondrop = 17; const char * SATTR_EVT_ondrop = "drop";
constexpr short ATTR_EVT_ondurationchange = 18; const char * SATTR_EVT_ondurationchange = "durationchange";
constexpr short ATTR_EVT_onemptied = 19; const char * SATTR_EVT_onemptied = "emptied";
constexpr short ATTR_EVT_onended = 20; const char * SATTR_EVT_onended = "ended";
constexpr short ATTR_EVT_onfocusin = 21; const char * SATTR_EVT_onfocusin = "focusin";
constexpr short ATTR_EVT_onfocusout = 22; const char * SATTR_EVT_onfocusout = "focusout";
constexpr short ATTR_EVT_onhashchange = 23; const char * SATTR_EVT_onhashchange = "hashchange";
constexpr short ATTR_EVT_oninput = 24; const char * SATTR_EVT_oninput = "input";
constexpr short ATTR_EVT_oninvalid = 25; const char * SATTR_EVT_oninvalid = "invalid";
constexpr short ATTR_EVT_onkeydown = 26; const char * SATTR_EVT_onkeydown = "keydown";
constexpr short ATTR_EVT_onkeypress = 27; const char * SATTR_EVT_onkeypress = "keypress";
constexpr short ATTR_EVT_onkeyup = 28; const char * SATTR_EVT_onkeyup = "keyup";
constexpr short ATTR_EVT_onloadeddata = 29; const char * SATTR_EVT_onloadeddata = "loadeddata";
constexpr short ATTR_EVT_onloadedmetadata = 30; const char * SATTR_EVT_onloadedmetadata = "loadedmetadata";
constexpr short ATTR_EVT_onloadstart = 31; const char * SATTR_EVT_onloadstart = "loadstart";
constexpr short ATTR_EVT_onmousedown = 32; const char * SATTR_EVT_onmousedown = "mousedown";
constexpr short ATTR_EVT_onmousemove = 33; const char * SATTR_EVT_onmousemove = "mousemove";
constexpr short ATTR_EVT_onmouseout = 34; const char * SATTR_EVT_onmouseout = "mouseout";
constexpr short ATTR_EVT_onmouseover = 35; const char * SATTR_EVT_onmouseover = "mouseover";
constexpr short ATTR_EVT_onmouseup = 36; const char * SATTR_EVT_onmouseup = "mouseup";
constexpr short ATTR_EVT_onmousewheel = 37; const char * SATTR_EVT_onmousewheel = "mousewheel";
constexpr short ATTR_EVT_onoffline = 38; const char * SATTR_EVT_onoffline = "offline";
constexpr short ATTR_EVT_ononline = 39; const char * SATTR_EVT_ononline = "online";
constexpr short ATTR_EVT_onpagehide = 40; const char * SATTR_EVT_onpagehide = "pagehide";
constexpr short ATTR_EVT_onpageshow = 41; const char * SATTR_EVT_onpageshow = "pageshow";
constexpr short ATTR_EVT_onpaste = 42; const char * SATTR_EVT_onpaste = "paste";
constexpr short ATTR_EVT_onpause = 43; const char * SATTR_EVT_onpause = "pause";
constexpr short ATTR_EVT_onplay = 44; const char * SATTR_EVT_onplay = "play";
constexpr short ATTR_EVT_onplaying = 45; const char * SATTR_EVT_onplaying = "playing";
constexpr short ATTR_EVT_onpopstate = 46; const char * SATTR_EVT_onpopstate = "popstate";
constexpr short ATTR_EVT_onprogress = 47; const char * SATTR_EVT_onprogress = "progress";
constexpr short ATTR_EVT_onratechange = 48; const char * SATTR_EVT_onratechange = "ratechange";
constexpr short ATTR_EVT_onreset = 49; const char * SATTR_EVT_onreset = "reset";
constexpr short ATTR_EVT_onsearch = 50; const char * SATTR_EVT_onsearch = "search";
constexpr short ATTR_EVT_onseeked = 51; const char * SATTR_EVT_onseeked = "seeked";
constexpr short ATTR_EVT_onseeking = 52; const char * SATTR_EVT_onseeking = "seeking";
constexpr short ATTR_EVT_onselect = 53; const char * SATTR_EVT_onselect = "select";
constexpr short ATTR_EVT_onstalled = 54; const char * SATTR_EVT_onstalled = "stalled";
constexpr short ATTR_EVT_onstorage = 55; const char * SATTR_EVT_onstorage = "storage";
constexpr short ATTR_EVT_onsubmit = 56; const char * SATTR_EVT_onsubmit = "submit";
constexpr short ATTR_EVT_onsuspend = 57; const char * SATTR_EVT_onsuspend = "suspend";
constexpr short ATTR_EVT_ontimeupdate = 58; const char * SATTR_EVT_ontimeupdate = "timeupdate";
constexpr short ATTR_EVT_ontoggle = 59; const char * SATTR_EVT_ontoggle = "toggle";
constexpr short ATTR_EVT_onvolumechange = 60; const char * SATTR_EVT_onvolumechange = "volumechange";
constexpr short ATTR_EVT_onwaiting = 61; const char * SATTR_EVT_onwaiting = "waiting";
constexpr short ATTR_EVT_onwheel = 62; const char * SATTR_EVT_onwheel = "wheel";

// Non-Bubble Events (100-149)
constexpr short ATTR_EVT_NON_BUBBLE_START = 100; 
constexpr short ATTR_EVT_NON_BUBBLE_END = 200;
constexpr short ATTR_EVT_onabort = 100; const char * SATTR_EVT_onabort = "abort";
constexpr short ATTR_EVT_onbeforeunload = 101; const char * SATTR_EVT_onbeforeunload = "beforeunload";
constexpr short ATTR_EVT_onblur = 102; const char * SATTR_EVT_onblur = "blur";
constexpr short ATTR_EVT_onerror = 103; const char * SATTR_EVT_onerror = "error";
constexpr short ATTR_EVT_onfocus = 104; const char * SATTR_EVT_onfocus = "focus";
constexpr short ATTR_EVT_onload = 105; const char * SATTR_EVT_onload = "load";
constexpr short ATTR_EVT_onmouseenter = 106; const char * SATTR_EVT_onmouseenter = "mouseenter";
constexpr short ATTR_EVT_onmouseleave = 107; const char * SATTR_EVT_onmouseleave = "mouseleave";
constexpr short ATTR_EVT_onresize = 108; const char * SATTR_EVT_onresize = "resize";
constexpr short ATTR_EVT_onscroll = 109; const char * SATTR_EVT_onscroll = "scroll";
constexpr short ATTR_EVT_onunload = 110; const char * SATTR_EVT_onunload = "unload";

// Volt: Special events (150-199)
constexpr short ATTR_INTERNAL_CUSTOM_START = 150;
constexpr short ATTR_EVT_onaddelement = 150;  const char * SATTR_EVT_onaddelement = "addelement";
constexpr short ATTR_EVT_onbeforemoveelement = 151;  const char * SATTR_EVT_onbeforemoveelement = "beforemoveelement";
constexpr short ATTR_EVT_onmoveelement = 152;  const char * SATTR_EVT_onmoveelement = "moveelement";
constexpr short ATTR_EVT_onremoveelement = 153;  const char * SATTR_EVT_onremoveelement = "removeelement";
constexpr short ATTR_INTERNAL_CUSTOM_END = 200; // non-inclusive

// Common attributes (200-399)
constexpr short ATTR_accept = 200;
constexpr short ATTR_acceptcharset = 201;
constexpr short ATTR_accesskey = 202;
constexpr short ATTR_action = 203;
constexpr short ATTR_align = 204;
constexpr short ATTR_alt = 205;
constexpr short ATTR_arialabel = 206;
constexpr short ATTR_arialabelledby = 207;
constexpr short ATTR_ariahaspopup = 208;
constexpr short ATTR_ariahidden = 209;
constexpr short ATTR_ariaexpanded = 210;
constexpr short ATTR_async = 211;
constexpr short ATTR_autocomplete = 212;
constexpr short ATTR_autofocus = 213;
constexpr short ATTR_autoplay = 214;
constexpr short ATTR_bgcolor = 215;
constexpr short ATTR_border = 216;
constexpr short ATTR_charset = 217;
constexpr short ATTR_checked = 218;
constexpr short ATTR_cite = 219;
constexpr short ATTR_classname = 220;
constexpr short ATTR_color = 221;
constexpr short ATTR_cols = 222;
constexpr short ATTR_colspan = 223;
constexpr short ATTR_content = 224;
constexpr short ATTR_contenteditable = 225;
constexpr short ATTR_controls = 226;
constexpr short ATTR_coords = 227;
constexpr short ATTR_data = 228;
constexpr short ATTR_datetime = 229;
constexpr short ATTR_defaultattr = 230;
constexpr short ATTR_defer = 231;
constexpr short ATTR_dir = 232;
constexpr short ATTR_dirname = 233;
constexpr short ATTR_disabled = 234;
constexpr short ATTR_download = 235;
constexpr short ATTR_draggable = 236;
constexpr short ATTR_dropzone = 237;
constexpr short ATTR_enctype = 238;
constexpr short ATTR_enterkeyhint = 239;
constexpr short ATTR_fill = 240;
constexpr short ATTR_forattr = 241;
constexpr short ATTR_form = 242;
constexpr short ATTR_formaction = 243;
constexpr short ATTR_headers = 244;
constexpr short ATTR_height = 245;
constexpr short ATTR_hidden = 246;
constexpr short ATTR_high = 247;
constexpr short ATTR_href = 248;
constexpr short ATTR_hreflang = 249;
constexpr short ATTR_httpequiv = 250;
constexpr short ATTR_icon = 251;
constexpr short ATTR_id = 252;
constexpr short ATTR_inert = 253;
constexpr short ATTR_inputmode = 254;
constexpr short ATTR_ismap = 255;
constexpr short ATTR_kind = 256;
constexpr short ATTR_label = 257;
constexpr short ATTR_lang = 258;
constexpr short ATTR_list = 259;
constexpr short ATTR_loop = 260;
constexpr short ATTR_low = 261;
constexpr short ATTR_max = 262;
constexpr short ATTR_maxlength = 263;
constexpr short ATTR_media = 264;
constexpr short ATTR_method = 265;
constexpr short ATTR_min = 266;
constexpr short ATTR_minlength = 267;
constexpr short ATTR_multiple = 268;
constexpr short ATTR_muted = 269;
constexpr short ATTR_name = 270;
constexpr short ATTR_novalidate = 271;
constexpr short ATTR_open = 272;
constexpr short ATTR_optimum = 273;
constexpr short ATTR_pattern = 274;
constexpr short ATTR_placeholder = 275;
constexpr short ATTR_popover = 276;
constexpr short ATTR_popovertarget = 277;
constexpr short ATTR_popovertargetaction = 278;
constexpr short ATTR_poster = 279;
constexpr short ATTR_preload = 280;
constexpr short ATTR_readonly = 281;
constexpr short ATTR_rel = 282;
constexpr short ATTR_required = 283;
constexpr short ATTR_reversed = 284;
constexpr short ATTR_rows = 285;
constexpr short ATTR_rowspan = 286;
constexpr short ATTR_sandbox = 287;
constexpr short ATTR_scope = 288;
constexpr short ATTR_selected = 289;
constexpr short ATTR_shape = 290;
constexpr short ATTR_size = 291;
constexpr short ATTR_sizes = 292;
constexpr short ATTR_span = 293;
constexpr short ATTR_spellcheck = 294;
constexpr short ATTR_src = 295;
constexpr short ATTR_srcdoc = 296;
constexpr short ATTR_srclang = 297;
constexpr short ATTR_srcset = 298;
constexpr short ATTR_start = 299;
constexpr short ATTR_step = 300;
constexpr short ATTR_style = 301;
constexpr short ATTR_tabindex = 302;
constexpr short ATTR_target = 303;
constexpr short ATTR_title = 304;
constexpr short ATTR_translate = 305;
constexpr short ATTR_type = 306;
constexpr short ATTR_usemap = 307;
constexpr short ATTR_value = 308;
constexpr short ATTR_viewbox = 309;
constexpr short ATTR_width = 310;
constexpr short ATTR_wrap = 311;
constexpr short ATTR_xmlns = 312;
// SGV
constexpr short ATTR_by = 313;
constexpr short ATTR_cx = 314;
constexpr short ATTR_cy = 315;
constexpr short ATTR_d = 316;
constexpr short ATTR_fillopacity = 317;
constexpr short ATTR_filter = 318;
constexpr short ATTR_floodcolor = 319;
constexpr short ATTR_floodopacity = 320;
constexpr short ATTR_fontsize = 321;
constexpr short ATTR_fontsizeadjust = 322;
constexpr short ATTR_fontstyle = 323;
constexpr short ATTR_fr = 324;
constexpr short ATTR_from = 325;
constexpr short ATTR_fx = 326;
constexpr short ATTR_fy = 327;
constexpr short ATTR_keypoints = 328;
constexpr short ATTR_keytimes = 329;
constexpr short ATTR_lengthadjust = 330;
constexpr short ATTR_letterspacing = 331;
constexpr short ATTR_lightingcolor = 332;
constexpr short ATTR_markerheight = 333;
constexpr short ATTR_markerunits = 334;
constexpr short ATTR_markerwidth = 335;
constexpr short ATTR_mask = 336;
constexpr short ATTR_numoctaves = 337;
constexpr short ATTR_opacity = 338;
constexpr short ATTR_operatorattr = 339;
constexpr short ATTR_orient = 340;
constexpr short ATTR_path = 341;
constexpr short ATTR_pathlength = 342;
constexpr short ATTR_patterncontentunits = 343;
constexpr short ATTR_patterntransform = 344;
constexpr short ATTR_patternunits = 345;
constexpr short ATTR_pointerevents = 346;
constexpr short ATTR_points = 347;
constexpr short ATTR_pointsatx = 348;
constexpr short ATTR_pointsaty = 349;
constexpr short ATTR_pointsatz = 350;
constexpr short ATTR_preservealpha = 351;
constexpr short ATTR_r = 352;
constexpr short ATTR_radius = 353;
constexpr short ATTR_repeatcount = 354;
constexpr short ATTR_repeatdur = 355;
constexpr short ATTR_requiredextensions = 356;
constexpr short ATTR_requiredfeatures = 357;
constexpr short ATTR_restart = 358;
constexpr short ATTR_rotate = 359;
constexpr short ATTR_rx = 360;
constexpr short ATTR_ry = 361;
constexpr short ATTR_scale = 362;
constexpr short ATTR_seed = 363;
constexpr short ATTR_shaperendering = 364;
constexpr short ATTR_specularexponent = 365;
constexpr short ATTR_specularconstant = 366;
constexpr short ATTR_startoffset = 367;
constexpr short ATTR_stddeviation = 368;
constexpr short ATTR_stitchtiles = 369;
constexpr short ATTR_stopcolor = 370;
constexpr short ATTR_stopopacity = 371;
constexpr short ATTR_strikethroughposition = 372;
constexpr short ATTR_strikethroughthickness = 373;
constexpr short ATTR_stroke = 374;
constexpr short ATTR_strokedasharray = 375;
constexpr short ATTR_strokedashoffset = 376;
constexpr short ATTR_strokelinecap = 377;
constexpr short ATTR_strokelinejoin = 378;
constexpr short ATTR_strokemiterlimit = 379;
constexpr short ATTR_strokeopacity = 380;
constexpr short ATTR_strokewidth = 381;
constexpr short ATTR_surfacescale = 382;
constexpr short ATTR_systemlanguage = 383;
constexpr short ATTR_tablevalues = 384;
constexpr short ATTR_textanchor = 385;
constexpr short ATTR_textdecoration = 386;
constexpr short ATTR_textrendering = 387;
constexpr short ATTR_textlength = 388;
constexpr short ATTR_to = 389;
constexpr short ATTR_transform = 390;
constexpr short ATTR_vectoreffect = 391;
constexpr short ATTR_visibility = 392;
constexpr short ATTR_wordspacing = 393;
constexpr short ATTR_writingmode = 394;
constexpr short ATTR_x = 395;
constexpr short ATTR_x1 = 396;
constexpr short ATTR_x2 = 397;
constexpr short ATTR_xchannelselector = 398;
constexpr short ATTR_xlinkhref = 399;
constexpr short ATTR_xmllang = 400;
constexpr short ATTR_y = 401;
constexpr short ATTR_y1 = 402;
constexpr short ATTR_y2 = 403;
constexpr short ATTR_ychannelselector = 404;
constexpr short ATTR_z = 405;
constexpr short ATTR_zoomandpan = 406;

// Volt: Special attributes
constexpr short ATTR_nodevalue = 1000;
constexpr short ATTR_key = 1001;

// Custom attributes start at 20000
constexpr short ATTR_CUSTOM_START = 20000;

// ============================================================================
// Attribute Name Lookup Table
// ============================================================================
inline const char* attrIdToName(short id) {
    switch (id) {
        // Bubble Events
        case ATTR_EVT_onafterprint: return SATTR_EVT_onafterprint;
        case ATTR_EVT_onbeforeprint: return SATTR_EVT_onbeforeprint;
        case ATTR_EVT_oncanplay: return SATTR_EVT_oncanplay;
        case ATTR_EVT_oncanplaythrough: return SATTR_EVT_oncanplaythrough;
        case ATTR_EVT_onchange: return SATTR_EVT_onchange;
        case ATTR_EVT_onclick: return SATTR_EVT_onclick;
        case ATTR_EVT_oncontextmenu: return SATTR_EVT_oncontextmenu;
        case ATTR_EVT_oncopy: return SATTR_EVT_oncopy;
        case ATTR_EVT_oncuechange: return SATTR_EVT_oncuechange;
        case ATTR_EVT_oncut: return SATTR_EVT_oncut;
        case ATTR_EVT_ondblclick: return SATTR_EVT_ondblclick;
        case ATTR_EVT_ondrag: return SATTR_EVT_ondrag;
        case ATTR_EVT_ondragend: return SATTR_EVT_ondragend;
        case ATTR_EVT_ondragenter: return SATTR_EVT_ondragenter;
        case ATTR_EVT_ondragleave: return SATTR_EVT_ondragleave;
        case ATTR_EVT_ondragover: return SATTR_EVT_ondragover;
        case ATTR_EVT_ondragstart: return SATTR_EVT_ondragstart;
        case ATTR_EVT_ondrop: return SATTR_EVT_ondrop;
        case ATTR_EVT_ondurationchange: return SATTR_EVT_ondurationchange;
        case ATTR_EVT_onemptied: return SATTR_EVT_onemptied;
        case ATTR_EVT_onended: return SATTR_EVT_onended;
        case ATTR_EVT_onfocusin: return SATTR_EVT_onfocusin;
        case ATTR_EVT_onfocusout: return SATTR_EVT_onfocusout;
        case ATTR_EVT_onhashchange: return SATTR_EVT_onhashchange;
        case ATTR_EVT_oninput: return SATTR_EVT_oninput;
        case ATTR_EVT_oninvalid: return SATTR_EVT_oninvalid;
        case ATTR_EVT_onkeydown: return SATTR_EVT_onkeydown;
        case ATTR_EVT_onkeypress: return SATTR_EVT_onkeypress;
        case ATTR_EVT_onkeyup: return SATTR_EVT_onkeyup;
        case ATTR_EVT_onloadeddata: return SATTR_EVT_onloadeddata;
        case ATTR_EVT_onloadedmetadata: return SATTR_EVT_onloadedmetadata;
        case ATTR_EVT_onloadstart: return SATTR_EVT_onloadstart;
        case ATTR_EVT_onmousedown: return SATTR_EVT_onmousedown;
        case ATTR_EVT_onmousemove: return SATTR_EVT_onmousemove;
        case ATTR_EVT_onmouseout: return SATTR_EVT_onmouseout;
        case ATTR_EVT_onmouseover: return SATTR_EVT_onmouseover;
        case ATTR_EVT_onmouseup: return SATTR_EVT_onmouseup;
        case ATTR_EVT_onmousewheel: return SATTR_EVT_onmousewheel;
        case ATTR_EVT_onoffline: return SATTR_EVT_onoffline;
        case ATTR_EVT_ononline: return SATTR_EVT_ononline;
        case ATTR_EVT_onpagehide: return SATTR_EVT_onpagehide;
        case ATTR_EVT_onpageshow: return SATTR_EVT_onpageshow;
        case ATTR_EVT_onpaste: return SATTR_EVT_onpaste;
        case ATTR_EVT_onpause: return SATTR_EVT_onpause;
        case ATTR_EVT_onplay: return SATTR_EVT_onplay;
        case ATTR_EVT_onplaying: return SATTR_EVT_onplaying;
        case ATTR_EVT_onpopstate: return SATTR_EVT_onpopstate;
        case ATTR_EVT_onprogress: return SATTR_EVT_onprogress;
        case ATTR_EVT_onratechange: return SATTR_EVT_onratechange;
        case ATTR_EVT_onreset: return SATTR_EVT_onreset;
        case ATTR_EVT_onsearch: return SATTR_EVT_onsearch;
        case ATTR_EVT_onseeked: return SATTR_EVT_onseeked;
        case ATTR_EVT_onseeking: return SATTR_EVT_onseeking;
        case ATTR_EVT_onselect: return SATTR_EVT_onselect;
        case ATTR_EVT_onstalled: return SATTR_EVT_onstalled;
        case ATTR_EVT_onstorage: return SATTR_EVT_onstorage;
        case ATTR_EVT_onsubmit: return SATTR_EVT_onsubmit;
        case ATTR_EVT_onsuspend: return SATTR_EVT_onsuspend;
        case ATTR_EVT_ontimeupdate: return SATTR_EVT_ontimeupdate;
        case ATTR_EVT_ontoggle: return SATTR_EVT_ontoggle;
        case ATTR_EVT_onvolumechange: return SATTR_EVT_onvolumechange;
        case ATTR_EVT_onwaiting: return SATTR_EVT_onwaiting;
        case ATTR_EVT_onwheel: return SATTR_EVT_onwheel;

        // Non-Bubble Events
        case ATTR_EVT_onabort: return SATTR_EVT_onabort;
        case ATTR_EVT_onbeforeunload: return SATTR_EVT_onbeforeunload;
        case ATTR_EVT_onblur: return SATTR_EVT_onblur;
        case ATTR_EVT_onerror: return SATTR_EVT_onerror;
        case ATTR_EVT_onfocus: return SATTR_EVT_onfocus;
        case ATTR_EVT_onload: return SATTR_EVT_onload;
        case ATTR_EVT_onmouseenter: return SATTR_EVT_onmouseenter;
        case ATTR_EVT_onmouseleave: return SATTR_EVT_onmouseleave;
        case ATTR_EVT_onresize: return SATTR_EVT_onresize;
        case ATTR_EVT_onscroll: return SATTR_EVT_onscroll;
        case ATTR_EVT_onunload: return SATTR_EVT_onunload;

        // Volt: Special events
        case ATTR_EVT_onaddelement: return SATTR_EVT_onaddelement;
        case ATTR_EVT_onbeforemoveelement: return SATTR_EVT_onbeforemoveelement;
        case ATTR_EVT_onmoveelement: return SATTR_EVT_onmoveelement;
        case ATTR_EVT_onremoveelement: return SATTR_EVT_onremoveelement;
        
        // Common attributes
        case ATTR_accept: return "accept";
        case ATTR_acceptcharset: return "accept-charset";
        case ATTR_accesskey: return "accesskey";
        case ATTR_action: return "action";
        case ATTR_align: return "align";
        case ATTR_alt: return "alt";
        case ATTR_arialabel: return "aria-label";
        case ATTR_arialabelledby: return "aria-labelledby";
        case ATTR_ariahaspopup: return "aria-haspopup";
        case ATTR_ariahidden: return "aria-hidden";
        case ATTR_ariaexpanded: return "aria-expanded";
        case ATTR_async: return "async";
        case ATTR_autocomplete: return "autocomplete";
        case ATTR_autofocus: return "autofocus";
        case ATTR_autoplay: return "autoplay";
        case ATTR_bgcolor: return "bgcolor";
        case ATTR_border: return "border";
        case ATTR_charset: return "charset";
        case ATTR_checked: return "checked";
        case ATTR_cite: return "cite";
        case ATTR_classname: return "class";
        case ATTR_color: return "color";
        case ATTR_cols: return "cols";
        case ATTR_colspan: return "colspan";
        case ATTR_content: return "content";
        case ATTR_contenteditable: return "contenteditable";
        case ATTR_controls: return "controls";
        case ATTR_coords: return "coords";
        case ATTR_data: return "data";
        case ATTR_datetime: return "datetime";
        case ATTR_defaultattr: return "default";
        case ATTR_defer: return "defer";
        case ATTR_dir: return "dir";
        case ATTR_dirname: return "dirname";
        case ATTR_disabled: return "disabled";
        case ATTR_download: return "download";
        case ATTR_draggable: return "draggable";
        case ATTR_dropzone: return "dropzone";
        case ATTR_enctype: return "enctype";
        case ATTR_enterkeyhint: return "enterkeyhint";
        case ATTR_fill: return "fill";
        case ATTR_forattr: return "for";
        case ATTR_form: return "form";
        case ATTR_formaction: return "formaction";
        case ATTR_headers: return "headers";
        case ATTR_height: return "height";
        case ATTR_hidden: return "hidden";
        case ATTR_high: return "high";
        case ATTR_href: return "href";
        case ATTR_hreflang: return "hreflang";
        case ATTR_httpequiv: return "http-equiv";
        case ATTR_icon: return "icon";
        case ATTR_id: return "id";
        case ATTR_inert: return "inert";
        case ATTR_inputmode: return "inputmode";
        case ATTR_ismap: return "ismap";
        case ATTR_kind: return "kind";
        case ATTR_label: return "label";
        case ATTR_lang: return "lang";
        case ATTR_list: return "list";
        case ATTR_loop: return "loop";
        case ATTR_low: return "low";
        case ATTR_max: return "max";
        case ATTR_maxlength: return "maxlength";
        case ATTR_media: return "media";
        case ATTR_method: return "method";
        case ATTR_min: return "min";
        case ATTR_minlength: return "minlength";
        case ATTR_multiple: return "multiple";
        case ATTR_muted: return "muted";
        case ATTR_name: return "name";
        case ATTR_novalidate: return "novalidate";
        case ATTR_open: return "open";
        case ATTR_optimum: return "optimum";
        case ATTR_pattern: return "pattern";
        case ATTR_placeholder: return "placeholder";
        case ATTR_popover: return "popover";
        case ATTR_popovertarget: return "popovertarget";
        case ATTR_popovertargetaction: return "popovertargetaction";
        case ATTR_poster: return "poster";
        case ATTR_preload: return "preload";
        case ATTR_readonly: return "readonly";
        case ATTR_rel: return "rel";
        case ATTR_required: return "required";
        case ATTR_reversed: return "reversed";
        case ATTR_rows: return "rows";
        case ATTR_rowspan: return "rowspan";
        case ATTR_sandbox: return "sandbox";
        case ATTR_scope: return "scope";
        case ATTR_selected: return "selected";
        case ATTR_shape: return "shape";
        case ATTR_size: return "size";
        case ATTR_sizes: return "sizes";
        case ATTR_span: return "span";
        case ATTR_spellcheck: return "spellcheck";
        case ATTR_src: return "src";
        case ATTR_srcdoc: return "srcdoc";
        case ATTR_srclang: return "srclang";
        case ATTR_srcset: return "srcset";
        case ATTR_start: return "start";
        case ATTR_step: return "step";
        case ATTR_style: return "style";
        case ATTR_tabindex: return "tabindex";
        case ATTR_target: return "target";
        case ATTR_title: return "title";
        case ATTR_translate: return "translate";
        case ATTR_type: return "type";
        case ATTR_usemap: return "usemap";
        case ATTR_value: return "value";
        case ATTR_viewbox: return "viewbox";
        case ATTR_width: return "width";
        case ATTR_wrap: return "wrap";
        case ATTR_xmlns: return "xmlns";
        // SGV
        case ATTR_by: return "by";
        case ATTR_cx: return "cx";
        case ATTR_cy: return "cy";
        case ATTR_d: return "d";
        case ATTR_fillopacity: return "fill-opacity";
        case ATTR_filter: return "filter";
        case ATTR_floodcolor: return "flood-color";
        case ATTR_floodopacity: return "flood-opacity";
        case ATTR_fontsize: return "font-size";
        case ATTR_fontsizeadjust: return "font-size-adjust";
        case ATTR_fontstyle: return "font-style";
        case ATTR_fr: return "fr";
        case ATTR_from: return "from";
        case ATTR_fx: return "fx";
        case ATTR_fy: return "fy";
        case ATTR_keypoints: return "keyPoints";
        case ATTR_keytimes: return "keyTimes";
        case ATTR_lengthadjust: return "lengthAdjust";
        case ATTR_letterspacing: return "letter-spacing";
        case ATTR_lightingcolor: return "lighting-color";
        case ATTR_markerheight: return "markerHeight";
        case ATTR_markerunits: return "markerUnits";
        case ATTR_markerwidth: return "markerWidth";
        case ATTR_mask: return "mask";
        case ATTR_numoctaves: return "numOctaves";
        case ATTR_opacity: return "opacity";
        case ATTR_operatorattr: return "operator";
        case ATTR_orient: return "orient";
        case ATTR_path: return "path";
        case ATTR_pathlength: return "pathLength";
        case ATTR_patterncontentunits: return "patternContentUnits";
        case ATTR_patterntransform: return "patternTransform";
        case ATTR_patternunits: return "patternUnits";
        case ATTR_pointerevents: return "pointer-events";
        case ATTR_points: return "points";
        case ATTR_pointsatx: return "pointsAtX";
        case ATTR_pointsaty: return "pointsAtY";
        case ATTR_pointsatz: return "pointsAtZ";
        case ATTR_preservealpha: return "preserveAlpha";
        case ATTR_r: return "r";
        case ATTR_radius: return "radius";
        case ATTR_repeatcount: return "repeatCount";
        case ATTR_repeatdur: return "repeatDur";
        case ATTR_requiredextensions: return "requiredExtensions";
        case ATTR_requiredfeatures: return "requiredFeatures";
        case ATTR_restart: return "restart";
        case ATTR_rotate: return "rotate";
        case ATTR_rx: return "rx";
        case ATTR_ry: return "ry";
        case ATTR_scale: return "scale";
        case ATTR_seed: return "seed";
        case ATTR_shaperendering: return "shape-rendering";
        case ATTR_specularexponent: return "specularExponent";
        case ATTR_specularconstant: return "specularConstant";
        case ATTR_startoffset: return "startOffset";
        case ATTR_stddeviation: return "stdDeviation";
        case ATTR_stitchtiles: return "stitchTiles";
        case ATTR_stopcolor: return "stop-color";
        case ATTR_stopopacity: return "stop-opacity";
        case ATTR_strikethroughposition: return "strikethrough-position";
        case ATTR_strikethroughthickness: return "strikethrough-thickness";
        case ATTR_stroke: return "stroke";
        case ATTR_strokedasharray: return "stroke-dasharray";
        case ATTR_strokedashoffset: return "stroke-dashoffset";
        case ATTR_strokelinecap: return "stroke-linecap";
        case ATTR_strokelinejoin: return "stroke-linejoin";
        case ATTR_strokemiterlimit: return "stroke-miterlimit";
        case ATTR_strokeopacity: return "stroke-opacity";
        case ATTR_strokewidth: return "stroke-width";
        case ATTR_surfacescale: return "surfaceScale";
        case ATTR_systemlanguage: return "systemLanguage";
        case ATTR_tablevalues: return "tableValues";
        case ATTR_textanchor: return "text-anchor";
        case ATTR_textdecoration: return "text-decoration";
        case ATTR_textrendering: return "text-rendering";
        case ATTR_textlength: return "textLength";
        case ATTR_to: return "to";
        case ATTR_transform: return "transform";
        case ATTR_vectoreffect: return "vector-effect";
        case ATTR_visibility: return "visibility";
        case ATTR_wordspacing: return "word-spacing";
        case ATTR_writingmode: return "writing-mode";
        case ATTR_x: return "x";
        case ATTR_x1: return "x1";
        case ATTR_x2: return "x2";
        case ATTR_xchannelselector: return "xChannelSelector";
        case ATTR_xlinkhref: return "xlink:href";
        case ATTR_xmllang: return "xml:lang";
        case ATTR_y: return "y";
        case ATTR_y1: return "y1";
        case ATTR_y2: return "y2";
        case ATTR_ychannelselector: return "yChannelSelector";
        case ATTR_z: return "z";
        case ATTR_zoomandpan: return "zoomAndPan";

        // Volt: Special attributes
        case ATTR_nodevalue: return "nodevalue"; // Special case for keeping text nodes's string content
        case ATTR_key: return "key"; // Special stable key attribute
        
        default: return "unknown";
    }
}

// ============================================================================
// Common Attributes
// ============================================================================

#define DEFINE_ATTR_HELPER(funcAttrName) \
    inline std::pair<short, std::string> funcAttrName(std::string a_sValue) { \
        return {ATTR_##funcAttrName, a_sValue}; \
    }

DEFINE_ATTR_HELPER(accept)
DEFINE_ATTR_HELPER(acceptcharset)
DEFINE_ATTR_HELPER(accesskey)
DEFINE_ATTR_HELPER(action)
DEFINE_ATTR_HELPER(align)
DEFINE_ATTR_HELPER(alt)
DEFINE_ATTR_HELPER(arialabel)
DEFINE_ATTR_HELPER(arialabelledby)
DEFINE_ATTR_HELPER(ariahaspopup)
DEFINE_ATTR_HELPER(ariahidden)
DEFINE_ATTR_HELPER(ariaexpanded)
DEFINE_ATTR_HELPER(async)
DEFINE_ATTR_HELPER(autocomplete)
DEFINE_ATTR_HELPER(autofocus)
DEFINE_ATTR_HELPER(autoplay)
DEFINE_ATTR_HELPER(bgcolor)
DEFINE_ATTR_HELPER(border)
DEFINE_ATTR_HELPER(charset)
DEFINE_ATTR_HELPER(checked)
DEFINE_ATTR_HELPER(cite)
DEFINE_ATTR_HELPER(classname)
DEFINE_ATTR_HELPER(color)
DEFINE_ATTR_HELPER(cols)
DEFINE_ATTR_HELPER(colspan)
DEFINE_ATTR_HELPER(content)
DEFINE_ATTR_HELPER(contenteditable)
DEFINE_ATTR_HELPER(controls)
DEFINE_ATTR_HELPER(coords)
DEFINE_ATTR_HELPER(data)
DEFINE_ATTR_HELPER(datetime)
DEFINE_ATTR_HELPER(defaultattr)
DEFINE_ATTR_HELPER(defer)
DEFINE_ATTR_HELPER(dir)
DEFINE_ATTR_HELPER(dirname)
DEFINE_ATTR_HELPER(disabled)
DEFINE_ATTR_HELPER(download)
DEFINE_ATTR_HELPER(draggable)
DEFINE_ATTR_HELPER(dropzone)
DEFINE_ATTR_HELPER(enctype)
DEFINE_ATTR_HELPER(enterkeyhint)
DEFINE_ATTR_HELPER(fill)
DEFINE_ATTR_HELPER(forattr)
DEFINE_ATTR_HELPER(form)
DEFINE_ATTR_HELPER(formaction)
DEFINE_ATTR_HELPER(headers)
DEFINE_ATTR_HELPER(height)
DEFINE_ATTR_HELPER(hidden)
DEFINE_ATTR_HELPER(high)
DEFINE_ATTR_HELPER(href)
DEFINE_ATTR_HELPER(hreflang)
DEFINE_ATTR_HELPER(httpequiv)
DEFINE_ATTR_HELPER(icon)
DEFINE_ATTR_HELPER(id)
DEFINE_ATTR_HELPER(inert)
DEFINE_ATTR_HELPER(inputmode)
DEFINE_ATTR_HELPER(ismap)
DEFINE_ATTR_HELPER(kind)
DEFINE_ATTR_HELPER(label)
DEFINE_ATTR_HELPER(lang)
DEFINE_ATTR_HELPER(list)
DEFINE_ATTR_HELPER(loop)
DEFINE_ATTR_HELPER(low)
DEFINE_ATTR_HELPER(max)
DEFINE_ATTR_HELPER(maxlength)
DEFINE_ATTR_HELPER(media)
DEFINE_ATTR_HELPER(method)
DEFINE_ATTR_HELPER(min)
DEFINE_ATTR_HELPER(minlength)
DEFINE_ATTR_HELPER(multiple)
DEFINE_ATTR_HELPER(muted)
DEFINE_ATTR_HELPER(name)
DEFINE_ATTR_HELPER(novalidate)
DEFINE_ATTR_HELPER(open)
DEFINE_ATTR_HELPER(optimum)
DEFINE_ATTR_HELPER(pattern)
DEFINE_ATTR_HELPER(placeholder)
DEFINE_ATTR_HELPER(popover)
DEFINE_ATTR_HELPER(popovertarget)
DEFINE_ATTR_HELPER(popovertargetaction)
DEFINE_ATTR_HELPER(poster)
DEFINE_ATTR_HELPER(preload)
DEFINE_ATTR_HELPER(readonly)
DEFINE_ATTR_HELPER(rel)
DEFINE_ATTR_HELPER(required)
DEFINE_ATTR_HELPER(reversed)
DEFINE_ATTR_HELPER(rows)
DEFINE_ATTR_HELPER(rowspan)
DEFINE_ATTR_HELPER(sandbox)
DEFINE_ATTR_HELPER(scope)
DEFINE_ATTR_HELPER(selected)
DEFINE_ATTR_HELPER(shape)
DEFINE_ATTR_HELPER(size)
DEFINE_ATTR_HELPER(sizes)
DEFINE_ATTR_HELPER(span)
DEFINE_ATTR_HELPER(spellcheck)
DEFINE_ATTR_HELPER(src)
DEFINE_ATTR_HELPER(srcdoc)
DEFINE_ATTR_HELPER(srclang)
DEFINE_ATTR_HELPER(srcset)
DEFINE_ATTR_HELPER(start)
DEFINE_ATTR_HELPER(step)
DEFINE_ATTR_HELPER(style)
DEFINE_ATTR_HELPER(tabindex)
DEFINE_ATTR_HELPER(target)
DEFINE_ATTR_HELPER(title)
DEFINE_ATTR_HELPER(translate)
DEFINE_ATTR_HELPER(type)
DEFINE_ATTR_HELPER(usemap)
DEFINE_ATTR_HELPER(value)
DEFINE_ATTR_HELPER(viewbox)
DEFINE_ATTR_HELPER(width)
DEFINE_ATTR_HELPER(wrap)
DEFINE_ATTR_HELPER(xmlns)
// SGV
DEFINE_ATTR_HELPER(by)
DEFINE_ATTR_HELPER(cx)
DEFINE_ATTR_HELPER(cy)
DEFINE_ATTR_HELPER(d)
DEFINE_ATTR_HELPER(fillopacity)
DEFINE_ATTR_HELPER(filter)
DEFINE_ATTR_HELPER(floodcolor)
DEFINE_ATTR_HELPER(floodopacity)
DEFINE_ATTR_HELPER(fontsize)
DEFINE_ATTR_HELPER(fontsizeadjust)
DEFINE_ATTR_HELPER(fontstyle)
DEFINE_ATTR_HELPER(fr)
DEFINE_ATTR_HELPER(from)
DEFINE_ATTR_HELPER(fx)
DEFINE_ATTR_HELPER(fy)
DEFINE_ATTR_HELPER(keypoints)
DEFINE_ATTR_HELPER(keytimes)
DEFINE_ATTR_HELPER(lengthadjust)
DEFINE_ATTR_HELPER(letterspacing)
DEFINE_ATTR_HELPER(lightingcolor)
DEFINE_ATTR_HELPER(markerheight)
DEFINE_ATTR_HELPER(markerunits)
DEFINE_ATTR_HELPER(markerwidth)
DEFINE_ATTR_HELPER(mask)
DEFINE_ATTR_HELPER(numoctaves)
DEFINE_ATTR_HELPER(opacity)
DEFINE_ATTR_HELPER(operatorattr)
DEFINE_ATTR_HELPER(orient)
DEFINE_ATTR_HELPER(path)
DEFINE_ATTR_HELPER(pathlength)
DEFINE_ATTR_HELPER(patterncontentunits)
DEFINE_ATTR_HELPER(patterntransform)
DEFINE_ATTR_HELPER(patternunits)
DEFINE_ATTR_HELPER(pointerevents)
DEFINE_ATTR_HELPER(points)
DEFINE_ATTR_HELPER(pointsatx)
DEFINE_ATTR_HELPER(pointsaty)
DEFINE_ATTR_HELPER(pointsatz)
DEFINE_ATTR_HELPER(preservealpha)
DEFINE_ATTR_HELPER(r)
DEFINE_ATTR_HELPER(radius)
DEFINE_ATTR_HELPER(repeatcount)
DEFINE_ATTR_HELPER(repeatdur)
DEFINE_ATTR_HELPER(requiredextensions)
DEFINE_ATTR_HELPER(requiredfeatures)
DEFINE_ATTR_HELPER(restart)
DEFINE_ATTR_HELPER(rotate)
DEFINE_ATTR_HELPER(rx)
DEFINE_ATTR_HELPER(ry)
DEFINE_ATTR_HELPER(scale)
DEFINE_ATTR_HELPER(seed)
DEFINE_ATTR_HELPER(shaperendering)
DEFINE_ATTR_HELPER(specularexponent)
DEFINE_ATTR_HELPER(specularconstant)
DEFINE_ATTR_HELPER(startoffset)
DEFINE_ATTR_HELPER(stddeviation)
DEFINE_ATTR_HELPER(stitchtiles)
DEFINE_ATTR_HELPER(stopcolor)
DEFINE_ATTR_HELPER(stopopacity)
DEFINE_ATTR_HELPER(strikethroughposition)
DEFINE_ATTR_HELPER(strikethroughthickness)
DEFINE_ATTR_HELPER(stroke)
DEFINE_ATTR_HELPER(strokedasharray)
DEFINE_ATTR_HELPER(strokedashoffset)
DEFINE_ATTR_HELPER(strokelinecap)
DEFINE_ATTR_HELPER(strokelinejoin)
DEFINE_ATTR_HELPER(strokemiterlimit)
DEFINE_ATTR_HELPER(strokeopacity)
DEFINE_ATTR_HELPER(strokewidth)
DEFINE_ATTR_HELPER(surfacescale)
DEFINE_ATTR_HELPER(systemlanguage)
DEFINE_ATTR_HELPER(tablevalues)
DEFINE_ATTR_HELPER(textanchor)
DEFINE_ATTR_HELPER(textdecoration)
DEFINE_ATTR_HELPER(textrendering)
DEFINE_ATTR_HELPER(textlength)
DEFINE_ATTR_HELPER(to)
DEFINE_ATTR_HELPER(transform)
DEFINE_ATTR_HELPER(vectoreffect)
DEFINE_ATTR_HELPER(visibility)
DEFINE_ATTR_HELPER(wordspacing)
DEFINE_ATTR_HELPER(writingmode)
DEFINE_ATTR_HELPER(x)
DEFINE_ATTR_HELPER(x1)
DEFINE_ATTR_HELPER(x2)
DEFINE_ATTR_HELPER(xchannelselector)
DEFINE_ATTR_HELPER(xlinkhref)
DEFINE_ATTR_HELPER(xmllang)
DEFINE_ATTR_HELPER(y)
DEFINE_ATTR_HELPER(y1)
DEFINE_ATTR_HELPER(y2)
DEFINE_ATTR_HELPER(ychannelselector)
DEFINE_ATTR_HELPER(z)
DEFINE_ATTR_HELPER(zoomandpan)

// Volt: Special Attributes
DEFINE_ATTR_HELPER(key)

// ============================================================================
// Events 
// ============================================================================

#define DECLARE_EVENT_HELPER(funcEvtName) \
    inline std::pair<short, std::function<void(emscripten::val)>> funcEvtName(std::function<void(emscripten::val)> a_fnCallback) { \
        return {ATTR_EVT_##funcEvtName, a_fnCallback}; \
    }

// Bubble Events
DECLARE_EVENT_HELPER(onafterprint);
DECLARE_EVENT_HELPER(onbeforeprint);
DECLARE_EVENT_HELPER(oncanplay);
DECLARE_EVENT_HELPER(oncanplaythrough);
DECLARE_EVENT_HELPER(onchange);
DECLARE_EVENT_HELPER(onclick);
DECLARE_EVENT_HELPER(oncontextmenu);
DECLARE_EVENT_HELPER(oncopy);
DECLARE_EVENT_HELPER(oncuechange);
DECLARE_EVENT_HELPER(oncut);
DECLARE_EVENT_HELPER(ondblclick);
DECLARE_EVENT_HELPER(ondrag);
DECLARE_EVENT_HELPER(ondragend);
DECLARE_EVENT_HELPER(ondragenter);
DECLARE_EVENT_HELPER(ondragleave);
DECLARE_EVENT_HELPER(ondragover);
DECLARE_EVENT_HELPER(ondragstart);
DECLARE_EVENT_HELPER(ondrop);
DECLARE_EVENT_HELPER(ondurationchange);
DECLARE_EVENT_HELPER(onemptied);
DECLARE_EVENT_HELPER(onended);
DECLARE_EVENT_HELPER(onfocusin);
DECLARE_EVENT_HELPER(onfocusout);
DECLARE_EVENT_HELPER(onhashchange);
DECLARE_EVENT_HELPER(oninput);
DECLARE_EVENT_HELPER(oninvalid);
DECLARE_EVENT_HELPER(onkeydown);
DECLARE_EVENT_HELPER(onkeypress);
DECLARE_EVENT_HELPER(onkeyup);
DECLARE_EVENT_HELPER(onloadeddata);
DECLARE_EVENT_HELPER(onloadedmetadata);
DECLARE_EVENT_HELPER(onloadstart);
DECLARE_EVENT_HELPER(onmousedown);
DECLARE_EVENT_HELPER(onmousemove);
DECLARE_EVENT_HELPER(onmouseout);
DECLARE_EVENT_HELPER(onmouseover);
DECLARE_EVENT_HELPER(onmouseup);
DECLARE_EVENT_HELPER(onmousewheel);
DECLARE_EVENT_HELPER(onoffline);
DECLARE_EVENT_HELPER(ononline);
DECLARE_EVENT_HELPER(onpagehide);
DECLARE_EVENT_HELPER(onpageshow);
DECLARE_EVENT_HELPER(onpaste);
DECLARE_EVENT_HELPER(onpause);
DECLARE_EVENT_HELPER(onplay);
DECLARE_EVENT_HELPER(onplaying);
DECLARE_EVENT_HELPER(onpopstate);
DECLARE_EVENT_HELPER(onprogress);
DECLARE_EVENT_HELPER(onratechange);
DECLARE_EVENT_HELPER(onreset);
DECLARE_EVENT_HELPER(onsearch);
DECLARE_EVENT_HELPER(onseeked);
DECLARE_EVENT_HELPER(onseeking);
DECLARE_EVENT_HELPER(onselect);
DECLARE_EVENT_HELPER(onstalled);
DECLARE_EVENT_HELPER(onstorage);
DECLARE_EVENT_HELPER(onsubmit);
DECLARE_EVENT_HELPER(onsuspend);
DECLARE_EVENT_HELPER(ontimeupdate);
DECLARE_EVENT_HELPER(ontoggle);
DECLARE_EVENT_HELPER(onvolumechange);
DECLARE_EVENT_HELPER(onwaiting);
DECLARE_EVENT_HELPER(onwheel);

// Non-Bubble Events
DECLARE_EVENT_HELPER(onabort);
DECLARE_EVENT_HELPER(onbeforeunload);
DECLARE_EVENT_HELPER(onblur);
DECLARE_EVENT_HELPER(onerror);
DECLARE_EVENT_HELPER(onfocus);
DECLARE_EVENT_HELPER(onload);
DECLARE_EVENT_HELPER(onmouseenter);
DECLARE_EVENT_HELPER(onmouseleave);
DECLARE_EVENT_HELPER(onresize);
DECLARE_EVENT_HELPER(onscroll);
DECLARE_EVENT_HELPER(onunload);

// Volt: Special events
DECLARE_EVENT_HELPER(onaddelement);
DECLARE_EVENT_HELPER(onbeforemoveelement);
DECLARE_EVENT_HELPER(onmoveelement);
DECLARE_EVENT_HELPER(onremoveelement);

} // namespace attrs

} // namespace volt