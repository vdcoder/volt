#!/usr/bin/env python3
"""
Volt preprocessor

Transforms these DSL forms inside C/C++ code:

1) Expression form:

    <:=( some_cpp_expression )/>

   becomes:

    (some_cpp_expression).track(__COUNTER__)

2) Render-call form (functions ending in 'render' returning VNodeHandle):

    MyComponent(this).<render(1, 2)/>
    this_is_my_function_<render(3)/>

   becomes:

    MyComponent(this).render(1, 2).track(__COUNTER__)
    this_is_my_function_render(3).track(__COUNTER__)

3) Map form:

    <map(container, callback)/>

   becomes:

    volt::map(container, callback).track(__COUNTER__)

   The callback body is itself processed by the same DSL transformer,
   so you can write <.../> inside the map callback.

4) Fragment form:

    <( child1, child2, ... )/>

   becomes:

    volt::tag::_fragment(child1, child2, ...).track(__COUNTER__)

   The inner content is also recursively processed as DSL.

5) Tag form (TAGNAMES):

    <div({ style:=("color:red") }, <:=(child)/>) />

   becomes:

    volt::tag::div(
        { volt::attr::style("color:red") },
        (child).track(__COUNTER__)
    ).track(__COUNTER__)

   Props are rewritten from:

       propname:=(...)

   to:

       volt::attr::propname(...)

   when propname is in PROPNAMES.
"""

from __future__ import annotations

import sys
import string
from enum import Enum, auto
from typing import Callable, Optional, Tuple, List


# ---------------------------------------------------------------------------
#  Config: tag names & prop names
# ---------------------------------------------------------------------------

TAGNAMES = {
    # basic
    "div", "span", "p", "a", "img", "br", "hr",
    # headings
    "h1", "h2", "h3", "h4", "h5", "h6",
    # structural / semantic
    "header", "footer", "main", "nav", "section", "article", "aside",
    # lists
    "ul", "ol", "li", "dl", "dt", "dd",
    # forms
    "form", "input", "textarea", "button", "label", "select", "option",
    # tables
    "table", "thead", "tbody", "tfoot", "tr", "td", "th",
    # media
    "video", "audio", "source", "canvas",
    # text semantics
    "pre", "code", "strong", "em", "small", "blockquote", "cite",
    # misc
    "figure", "figcaption",
}

PROPNAMES = {
    # common html-ish
    "id", "class", "style", "href", "src", "title", "key", "alt", "type", "value",
    "placeholder", "disabled", "checked", "selected", "name", "width", "height",
    "maxlength", "minlength", "readonly", "multiple", "size", "tabindex",
    # Volt / event handlers
    "onClick",
    "onInput",
    "onChange",
    "onAddElement",
    "onBeforeMoveElement",
    "onMoveElement",
    "onRemoveElement",
}

IDENT_START = string.ascii_letters + "_"
IDENT_BODY = IDENT_START + string.digits


# ---------------------------------------------------------------------------
#  Scanning utilities (comment/string aware)
# ---------------------------------------------------------------------------

class ScanState(Enum):
    NORMAL = auto()
    LINE_COMMENT = auto()
    BLOCK_COMMENT = auto()
    STRING_LITERAL = auto()
    CHAR_LITERAL = auto()


def _skip_escape(code: str, i: int) -> int:
    """
    Skip over an escape sequence starting at index i, where code[i] == '\\'.
    Returns the new index *after* the escape sequence.
    """
    if i + 1 < len(code):
        return i + 2
    return i + 1


def find_next_unmasked(
    code: str,
    start: int,
    predicate: Callable[[str], bool],
) -> Optional[int]:
    """
    Find the next index >= start where predicate(ch) is True,
    ignoring characters inside:
      - // line comments
      - /* block comments */
      - "string literals"
      - 'char literals'
    """
    state = ScanState.NORMAL
    i = start
    n = len(code)

    while i < n:
        c = code[i]

        if state == ScanState.NORMAL:
            # Enter comments
            if c == '/' and i + 1 < n:
                nxt = code[i + 1]
                if nxt == '/':
                    state = ScanState.LINE_COMMENT
                    i += 2
                    continue
                elif nxt == '*':
                    state = ScanState.BLOCK_COMMENT
                    i += 2
                    continue

            # Enter string
            if c == '"':
                state = ScanState.STRING_LITERAL
                i += 1
                continue

            # Enter char literal
            if c == '\'':
                state = ScanState.CHAR_LITERAL
                i += 1
                continue

            if predicate(c):
                return i

            i += 1
            continue

        elif state == ScanState.LINE_COMMENT:
            if c == '\n':
                state = ScanState.NORMAL
            i += 1
            continue

        elif state == ScanState.BLOCK_COMMENT:
            if c == '*' and i + 1 < n and code[i + 1] == '/':
                state = ScanState.NORMAL
                i += 2
            else:
                i += 1
            continue

        elif state == ScanState.STRING_LITERAL:
            if c == '\\':
                i = _skip_escape(code, i)
            elif c == '"':
                state = ScanState.NORMAL
                i += 1
            else:
                i += 1
            continue

        elif state == ScanState.CHAR_LITERAL:
            if c == '\\':
                i = _skip_escape(code, i)
            elif c == '\'':
                state = ScanState.NORMAL
                i += 1
            else:
                i += 1
            continue

    return None


def find_matching_paren(code: str, open_pos: int) -> Optional[int]:
    """
    Given index of an opening '(' in C/C++-like code, return the index of the
    matching ')', skipping comments and string/char literals.

    Returns None if not found.
    """
    if open_pos < 0 or open_pos >= len(code) or code[open_pos] != '(':
        return None

    state = ScanState.NORMAL
    depth = 1
    i = open_pos + 1
    n = len(code)

    while i < n:
        c = code[i]

        if state == ScanState.NORMAL:
            # comments
            if c == '/' and i + 1 < n:
                nxt = code[i + 1]
                if nxt == '/':
                    state = ScanState.LINE_COMMENT
                    i += 2
                    continue
                elif nxt == '*':
                    state = ScanState.BLOCK_COMMENT
                    i += 2
                    continue

            # strings / chars
            if c == '"':
                state = ScanState.STRING_LITERAL
                i += 1
                continue
            if c == '\'':
                state = ScanState.CHAR_LITERAL
                i += 1
                continue

            # nested paren
            if c == '(':
                depth += 1
                i += 1
                continue
            if c == ')':
                depth -= 1
                if depth == 0:
                    return i
                i += 1
                continue

            i += 1
            continue

        elif state == ScanState.LINE_COMMENT:
            if c == '\n':
                state = ScanState.NORMAL
            i += 1
            continue

        elif state == ScanState.BLOCK_COMMENT:
            if c == '*' and i + 1 < n and code[i + 1] == '/':
                state = ScanState.NORMAL
                i += 2
            else:
                i += 1
            continue

        elif state == ScanState.STRING_LITERAL:
            if c == '\\':
                i = _skip_escape(code, i)
            elif c == '"':
                state = ScanState.NORMAL
                i += 1
            else:
                i += 1
            continue

        elif state == ScanState.CHAR_LITERAL:
            if c == '\\':
                i = _skip_escape(code, i)
            elif c == '\'':
                state = ScanState.NORMAL
                i += 1
            else:
                i += 1
            continue

    return None


# ---------------------------------------------------------------------------
#  Helpers: identifiers and props
# ---------------------------------------------------------------------------

def parse_identifier_at(code: str, pos: int) -> Tuple[Optional[str], int]:
    """
    Parse an identifier starting at pos. Returns (name or None, new_pos).
    """
    n = len(code)
    if pos >= n or code[pos] not in IDENT_START:
        return None, pos
    start = pos
    pos += 1
    while pos < n and code[pos] in IDENT_BODY:
        pos += 1
    return code[start:pos], pos


def transform_props(text: str) -> str:
    """
    Transform propname:=(...) into volt::attr::propname(...)
    where propname is in PROPNAMES.

    This is used on the *argument list* of a tag, after nested DSL
    has already been expanded.
    """
    out: List[str] = []
    i = 0
    n = len(text)

    while i < n:
        # Try to match any propname at this position
        if text[i] in IDENT_START:
            name, j = parse_identifier_at(text, i)
            if name in PROPNAMES and j + 2 <= n and text[j:j+3] == ':=(':
                # We have name:=(...
                open_pos = j + 2  # index of '('
                if open_pos >= n or text[open_pos] != '(':
                    # malformed, just copy and move on
                    out.append(text[i])
                    i += 1
                    continue
                close_pos = find_matching_paren(text, open_pos)
                if close_pos is None:
                    # malformed, copy char and move
                    out.append(text[i])
                    i += 1
                    continue

                arg = text[open_pos + 1 : close_pos]
                out.append(f"volt::attr::{name}({arg})")
                i = close_pos + 1
                continue

        # default: copy one char
        out.append(text[i])
        i += 1

    return "".join(out)


# ---------------------------------------------------------------------------
#  DSL classification & expansion
# ---------------------------------------------------------------------------

def classify_dsl_start(code: str, lt_pos: int) -> Optional[str]:
    """
    Classify what kind of DSL starts at code[lt_pos] == '<'.

    We support:
      - '<:=('       → 'expr'
      - '<render('   → 'render'
      - '<map('      → 'map'
      - '<('         → 'fragment'
      - '<tagname('  → 'tag'  (tagname in TAGNAMES)
    No whitespace is allowed between '<' and the token for now.
    """
    n = len(code)

    # Expression
    if lt_pos + 3 <= n and code.startswith("<:=(", lt_pos):
        return "expr"

    # Render call
    if lt_pos + 8 <= n and code.startswith("<render(", lt_pos):
        return "render"

    # Map
    if lt_pos + 5 <= n and code.startswith("<map(", lt_pos):
        return "map"

    # Fragment
    if lt_pos + 2 <= n and code.startswith("<(", lt_pos):
        return "fragment"

    # Tag: <tagname(
    i = lt_pos + 1
    if i < n and code[i] in IDENT_START:
        ident, j = parse_identifier_at(code, i)
        if ident in TAGNAMES:
            # next non-space must be '('
            while j < n and code[j].isspace():
                j += 1
            if j < n and code[j] == '(':
                return "tag"

    return None


def expand_expr_dsl(code: str, lt_pos: int, transform_nested) -> Optional[Tuple[str, int]]:
    """
    Expand <:=( expr )/> starting at lt_pos.

    Returns (replacement_text, end_index_of_'>') or None on failure.
    """
    if not code.startswith("<:=(", lt_pos):
        return None

    open_pos = lt_pos + 3  # index of '('
    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    expr = code[open_pos + 1 : close_pos]
    expr = transform_nested(expr)

    # Find '/>' after close_pos
    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"({expr}).track(__COUNTER__)"
    return replacement, end_idx


def expand_render_dsl(code: str, lt_pos: int, transform_nested) -> Optional[Tuple[str, int]]:
    """
    Expand <render(args)/> starting at lt_pos.

    Returns (replacement_text, end_index_of_'>') or None on failure.

    The prefix before '<' is preserved by the caller, so we only
    generate 'render(args).track(__COUNTER__)'.
    """
    if not code.startswith("<render(", lt_pos):
        return None

    open_pos = code.find('(', lt_pos)
    if open_pos == -1:
        return None

    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    args = code[open_pos + 1 : close_pos]
    args = transform_nested(args)

    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"render({args}).track(__COUNTER__)"
    return replacement, end_idx


def expand_map_dsl(code: str, lt_pos: int, transform_nested) -> Optional[Tuple[str, int]]:
    """
    Expand <map(args)/> starting at lt_pos:

        <map(container, callback)/>

    →   volt::map(container, callback).track(__COUNTER__)

    'args' is recursively transformed so callback bodies can contain DSL.
    """
    if not code.startswith("<map(", lt_pos):
        return None

    open_pos = code.find('(', lt_pos)
    if open_pos == -1:
        return None

    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    args = code[open_pos + 1 : close_pos]
    args = transform_nested(args)

    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"volt::map({args}).track(__COUNTER__)"
    return replacement, end_idx


def expand_fragment_dsl(code: str, lt_pos: int, transform_nested) -> Optional[Tuple[str, int]]:
    """
    Expand <( children )/> fragment starting at lt_pos:

        <( child1, child2, ... )/>

    →   volt::tag::_fragment(child1, child2, ...).track(__COUNTER__)

    Inner content is recursively transformed as DSL.
    """
    if not code.startswith("<(", lt_pos):
        return None

    open_pos = lt_pos + 1  # index of '('
    if open_pos >= len(code) or code[open_pos] != '(':
        return None

    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    inner = code[open_pos + 1 : close_pos]
    inner = transform_nested(inner)

    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"volt::tag::_fragment({inner}).track(__COUNTER__)"
    return replacement, end_idx


def expand_tag_dsl(code: str, lt_pos: int, transform_nested) -> Optional[Tuple[str, int]]:
    """
    Expand <tagname(args)/> where tagname is in TAGNAMES:

        <div({ style:=("x") }, <:=(child)/>) />

    →   volt::tag::div({ volt::attr::style("x") }, (child).track(__COUNTER__))
         .track(__COUNTER__)

    The 'args' are recursively transformed (for nested DSL) and then
    props (propname:=) are rewritten to volt::attr::propname().
    """
    # Parse tag name
    n = len(code)
    i = lt_pos + 1
    ident, j = parse_identifier_at(code, i)
    if ident not in TAGNAMES:
        return None

    # find '(' after ident
    while j < n and code[j].isspace():
        j += 1
    if j >= n or code[j] != '(':
        return None

    open_pos = j
    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    args = code[open_pos + 1 : close_pos]
    # First pass DSL on args (children, map, fragments, etc.)
    args = transform_nested(args)
    # Then rewrite props
    args = transform_props(args)

    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"volt::tag::{ident}({args}).track(__COUNTER__)"
    return replacement, end_idx


# ---------------------------------------------------------------------------
#  Main transformation
# ---------------------------------------------------------------------------

def transform_code(code: str) -> str:
    """
    Main transformation loop.

    Walk the code, find '<...>' sequences in NORMAL C++ (not comments/strings),
    classify them as DSL, and expand where appropriate.
    """

    def _transform_nested(sub: str) -> str:
        # recursive call for inner expressions (map args, fragment children, expr, tag args)
        return transform_code(sub)

    out: List[str] = []
    pos = 0
    n = len(code)

    while pos < n:
        lt = find_next_unmasked(code, pos, lambda ch: ch == '<')
        if lt is None:
            out.append(code[pos:])
            break

        # Emit everything before the '<' as-is
        out.append(code[pos:lt])

        kind = classify_dsl_start(code, lt)
        if not kind:
            # Not DSL, emit '<' literally and continue
            out.append('<')
            pos = lt + 1
            continue

        if kind == "expr":
            res = expand_expr_dsl(code, lt, _transform_nested)
        elif kind == "render":
            res = expand_render_dsl(code, lt, _transform_nested)
        elif kind == "map":
            res = expand_map_dsl(code, lt, _transform_nested)
        elif kind == "fragment":
            res = expand_fragment_dsl(code, lt, _transform_nested)
        elif kind == "tag":
            res = expand_tag_dsl(code, lt, _transform_nested)
        else:
            res = None

        if res is None:
            # Failed to parse as DSL, be conservative
            out.append('<')
            pos = lt + 1
            continue

        replacement, end_idx = res
        out.append(replacement)
        pos = end_idx + 1

    return "".join(out)


# ---------------------------------------------------------------------------
#  CLI
# ---------------------------------------------------------------------------

def main(argv: list[str]) -> None:
    if len(argv) > 1 and argv[1] not in ("-", ""):
        # Read from file
        with open(argv[1], "r", encoding="utf-8") as f:
            code = f.read()
    else:
        # Read from stdin
        code = sys.stdin.read()

    transformed = transform_code(code)
    sys.stdout.write(transformed)


if __name__ == "__main__":
    main(sys.argv)
