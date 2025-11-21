#!/usr/bin/env python3
"""
Volt preprocessor (phase 1)

Transforms two DSL forms inside C/C++ code:

1) Expression form:

    <:=( some_cpp_expression )/>

   becomes:

    (some_cpp_expression).track(__COUNTER__)

2) Render-call form:

    MyComponent(this).<render(1, 2)/>
    this_is_my_function_<render(3)/>

   becomes:

    MyComponent(this).render(1, 2).track(__COUNTER__)
    this_is_my_function_render(3).track(__COUNTER__)

All scanning is comment/string aware:
- // line comments
- /* block comments */
- "string literals"
- 'char literals'
"""

from __future__ import annotations

import sys
from enum import Enum, auto
from typing import Callable, Optional, Tuple


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
#  DSL classification & expansion
# ---------------------------------------------------------------------------

def classify_dsl_start(code: str, lt_pos: int) -> Optional[str]:
    """
    Classify what kind of DSL starts at code[lt_pos] == '<'.

    For phase 1 we support:
      - '<:=('    → 'expr'
      - '<render(' → 'render'

    No whitespace is allowed between '<' and the token for now.
    """
    n = len(code)
    if lt_pos + 3 <= n and code.startswith("<:=(", lt_pos):
        return "expr"
    if lt_pos + 8 <= n and code.startswith("<render(", lt_pos):
        return "render"
    return None


def expand_expr_dsl(code: str, lt_pos: int) -> Optional[Tuple[str, int]]:
    """
    Expand <:=( expr )/> starting at lt_pos.

    Returns (replacement_text, end_index_of_'>') or None on failure.
    """
    # Expect "<:=(" at lt_pos
    if not code.startswith("<:=(", lt_pos):
        return None

    open_pos = lt_pos + 3  # index of '('
    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    expr = code[open_pos + 1 : close_pos]

    # Now find '/>' after close_pos
    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"({expr}).track(__COUNTER__)"
    return replacement, end_idx


def expand_render_dsl(code: str, lt_pos: int) -> Optional[Tuple[str, int]]:
    """
    Expand <render(args)/> starting at lt_pos.

    Returns (replacement_text, end_index_of_'>') or None on failure.

    The prefix before '<' is preserved by the caller, so we only
    generate 'render(args).track(__COUNTER__)'.
    """
    if not code.startswith("<render(", lt_pos):
        return None

    # index of '(' just after 'render'
    open_pos = code.find('(', lt_pos)
    if open_pos == -1:
        return None

    close_pos = find_matching_paren(code, open_pos)
    if close_pos is None:
        return None

    args = code[open_pos + 1 : close_pos]

    # Find '/>' after close_pos
    slash_idx = find_next_unmasked(code, close_pos + 1, lambda ch: ch == '/')
    if slash_idx is None or slash_idx + 1 >= len(code) or code[slash_idx + 1] != '>':
        return None

    end_idx = slash_idx + 1
    replacement = f"render({args}).track(__COUNTER__)"
    return replacement, end_idx


def transform_code(code: str) -> str:
    """
    Main transformation loop.

    Walk the code, find '<...>' sequences in NORMAL C++ (not comments/strings),
    classify them as DSL, and expand where appropriate.
    """
    out: list[str] = []
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
            res = expand_expr_dsl(code, lt)
        else:  # "render"
            res = expand_render_dsl(code, lt)

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