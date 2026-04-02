#!/usr/bin/env python3
# pyright: reportUnknownMemberType=false
"""Checker to ensure functions in src/fl/ are marked FL_NOEXCEPT.

FastLED compiles with C++ exceptions disabled (-fno-exceptions) on all
embedded platforms. Marking functions FL_NOEXCEPT communicates this intent
and allows the compiler to skip generating unwind tables.

Uses the same fast regex-based detection as NoexceptEsp32Checker but
scoped to src/fl/ (the core library).

Scope: src/fl/** (all .h / .hpp / .cpp / .cpp.hpp files)

Exemptions:
  - src/fl/stl/noexcept.h (the macro definition itself)
  - Third-party code (third_party/)
  - Lines with suppression comment "// ok no noexcept", "// ok no FL_NOEXCEPT",
    "// noexcept not required", or "// nolint"
  - Destructors (implicitly noexcept since C++11)
  - operator overloads
  - = delete / = default / = 0
  - Macro invocations (ALL_CAPS names)
"""

import re

from ci.util.check_files import FileContent, FileContentChecker


# ============================================================================
# Configuration
# ============================================================================

# Return types / qualifiers that distinguish declarations from calls
_QUALIFIERS = frozenset(
    {
        "static",
        "virtual",
        "inline",
        "explicit",
        "constexpr",
        "consteval",
        "extern",
        "friend",
        "FASTLED_FORCE_INLINE",
        "FL_OPTIMIZE_O2",
    }
)

_TYPE_KEYWORDS = frozenset(
    {
        "void",
        "bool",
        "char",
        "short",
        "int",
        "long",
        "float",
        "double",
        "signed",
        "unsigned",
        "auto",
        "size_t",
        "u8",
        "u16",
        "u32",
        "u64",
        "i8",
        "i16",
        "i32",
        "i64",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
    }
)

_STATEMENT_KEYWORDS = frozenset(
    {
        "if",
        "else",
        "while",
        "for",
        "switch",
        "catch",
        "do",
        "return",
        "throw",
        "delete",
        "new",
        "goto",
        "case",
        "sizeof",
        "alignof",
        "decltype",
        "typeof",
        "static_assert",
        "static_cast",
        "dynamic_cast",
        "const_cast",
        "reinterpret_cast",
    }
)

# ============================================================================
# Patterns
# ============================================================================

# Matches: [qualifiers] [return_type] func_name(
_FUNC_DECL_PATTERN = re.compile(r"^((?:[\w:*&<>,\s]+\s+)?)(~?\w[\w:]*)\s*\(")

# Matches noexcept or FL_NOEXCEPT already present
_HAS_NOEXCEPT = re.compile(r"\b(?:noexcept|FL_NOEXCEPT)\b")

# Matches = delete, = default, = 0
_SPECIAL_SUFFIX = re.compile(r"=\s*(?:delete|default|0)\s*;")

# Matches operator overloads including operator()
_OPERATOR_PATTERN = re.compile(r"\boperator\s*(?:[^\w\s]|\(\))")

# Matches end of function signature: ) [const] [volatile] [override] [final] [;|{]
_FUNC_END_PATTERN = re.compile(
    r"\)\s*"
    r"(?:const\s*)?(?:volatile\s*)?(?:override\s*)?(?:final\s*)?"
    r"(?:;|\{)\s*$"
)

# Suppression comments
_SUPPRESS = re.compile(
    r"//\s*(?:ok\s+no\s+(?:noexcept|FL_NOEXCEPT)|noexcept\s+not\s+required|nolint)\b",
    re.IGNORECASE,
)


# ============================================================================
# Detection
# ============================================================================


def _is_function_declaration(code: str) -> bool:
    """Determine if a line is a function declaration/definition (not a call)."""
    match = _FUNC_DECL_PATTERN.match(code)
    if not match:
        return False

    prefix = match.group(1).strip()
    func_name = match.group(2)
    base_name = func_name.rsplit("::", 1)[-1] if "::" in func_name else func_name

    # Skip keywords, destructors, operators, macros, typedefs
    if base_name in _STATEMENT_KEYWORDS:
        return False
    if base_name.startswith("~"):
        return False
    if _OPERATOR_PATTERN.search(code):
        return False
    if code.startswith("typedef "):
        return False
    # ALL_CAPS = macro call (FL_WARN, ESP_ERROR_CHECK, etc.)
    if re.match(r"^[A-Z][A-Z0-9_]*$", base_name):
        return False

    # Has a return type prefix → declaration
    if prefix:
        tokens = prefix.split()
        for token in tokens:
            if token in _STATEMENT_KEYWORDS:
                return False
        for token in tokens:
            clean = token.rstrip("*&").lstrip("*&")
            if "<" in clean:
                clean = clean[: clean.index("<")]
            if not clean:
                continue
            if clean in _TYPE_KEYWORDS or clean in _QUALIFIERS:
                return True
            if clean[0].isupper() or "::" in clean:
                return True
            return True  # any prefix token implies return type
        return False

    # No prefix + starts uppercase = constructor
    if base_name[0].isupper():
        return True
    return False


# ============================================================================
# Checker
# ============================================================================


class NoexceptFlChecker(FileContentChecker):
    """Checker that enforces FL_NOEXCEPT on functions in src/fl/."""

    def __init__(self) -> None:
        self.violations: dict[str, list[tuple[int, str]]] = {}

    def should_process_file(self, file_path: str) -> bool:
        normalized = file_path.replace("\\", "/")
        if "/src/fl/" not in normalized and not normalized.startswith("src/fl/"):
            return False
        # Only check header files — .cpp.hpp implementation files have too many
        # variable declarations that look like function declarations to regex
        # (e.g. fl::unique_lock<fl::mutex> lock(mtx);).  Use the AST-based
        # checker (ci/tools/check_noexcept.py) for .cpp.hpp files.
        if normalized.endswith(".cpp.hpp") or normalized.endswith(".cpp"):
            return False
        if not normalized.endswith((".h", ".hpp")):
            return False
        if normalized.endswith("noexcept.h") or normalized.endswith(
            "fl/stl/noexcept.h"
        ):
            return False
        if "/third_party/" in normalized:
            return False
        return True

    def check_file_content(self, file_content: FileContent) -> list[str]:
        if "(" not in file_content.content:
            return []

        violations: list[tuple[int, str]] = []
        in_multiline_comment = False

        for line_number, line in enumerate(file_content.lines, 1):
            stripped = line.strip()

            if "/*" in stripped:
                in_multiline_comment = True
            if "*/" in stripped:
                in_multiline_comment = False
                continue
            if in_multiline_comment:
                continue
            if stripped.startswith("//"):
                continue

            code = stripped.split("//")[0].strip()
            if not code or "(" not in code:
                continue
            if _SUPPRESS.search(line):
                continue
            if code.startswith("#") or code.startswith(":") or code.startswith("?"):
                continue

            if not _is_function_declaration(code):
                continue

            # Build full signature for multi-line
            full_sig = code
            paren_depth = code.count("(") - code.count(")")
            lookahead = line_number
            while paren_depth > 0 and lookahead < len(file_content.lines):
                next_code = file_content.lines[lookahead].strip().split("//")[0].strip()
                full_sig += " " + next_code
                paren_depth += next_code.count("(") - next_code.count(")")
                lookahead += 1

            if _HAS_NOEXCEPT.search(full_sig):
                continue
            if _SPECIAL_SUFFIX.search(full_sig):
                continue
            if not _FUNC_END_PATTERN.search(full_sig):
                continue

            violations.append((line_number, stripped))

        if violations:
            self.violations[file_content.path] = violations

        return []


def main() -> None:
    from ci.util.check_files import run_checker_standalone
    from ci.util.paths import PROJECT_ROOT

    checker = NoexceptFlChecker()
    target_dir = str(PROJECT_ROOT / "src" / "fl")
    run_checker_standalone(
        checker,
        [target_dir],
        "Found functions missing FL_NOEXCEPT in src/fl/",
        extensions=[".h", ".hpp"],
    )


if __name__ == "__main__":
    main()
