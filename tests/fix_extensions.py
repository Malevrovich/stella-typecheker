#!/usr/bin/env python3
"""
Automatically fixes golden test files (and matching .stella sources) by injecting
missing `extend with` declarations until the reference typechecker accepts the test.

Reference model: docker run -i fizruk/stella typecheck
  - Writes everything to stdout (not stderr).
  - If an extension is needed it says:
      "Perhaps, you would like to enable '#foo' or '#bar' extension?"
  - Otherwise may produce a generic "syntax error" or other error.

Algorithm per golden file:
  1. Run reference on the current INPUT source.
  2. If the result already matches what we expect → skip (already good).
  3. Collect extension hints from the reference output.
  4. Inject hinted extensions and re-run. Repeat up to MAX_ITERATIONS.
  5. If no hints but still broken → try ALL_KNOWN_EXTENSIONS one by one.
  6. When a working set is found, rewrite both the .golden INPUT and the .stella file.

Usage:
    python3 tests/fix_extensions.py [--golden-dir tests/golden] [--stella-dir stella-examples]
                                    [--filter SUBSTR] [--dry-run] [-v]
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

DOCKER_CMD = ["docker", "run", "-i", "fizruk/stella", "typecheck"]
TIMEOUT = 30  # seconds per reference call

MAX_ITERATIONS = 12  # guard against infinite loops

# All extensions the reference understands; tried exhaustively when no hint given.
ALL_KNOWN_EXTENSIONS = [
    "#natural-literals",
    "#unit-type",
    "#pairs",
    "#tuples",
    "#records",
    "#let-bindings",
    "#let-patterns",
    "#type-ascriptions",
    "#sum-types",
    "#lists",
    "#variants",
    "#fixpoint-combinator",
    "#sequencing",
    "#references",
    "#panic",
    "#exceptions",
    "#exception-type-declaration",
    "#structural-subtyping",
    "#type-cast",
    "#top-type",
    "#bottom-type",
    "#ambiguous-type-as-bottom",
]

# ---------------------------------------------------------------------------
# Reference runner
# ---------------------------------------------------------------------------

def sanitize_source(source: str) -> str:
    """Strip non-ASCII from // comments (reference uses Latin-1 locale)."""
    return re.sub(r"//[^\n]*", "//", source)


def run_reference(source: str) -> tuple[str, int]:
    """Returns (stdout_output, exit_code)."""
    try:
        result = subprocess.run(
            DOCKER_CMD,
            input=sanitize_source(source),
            capture_output=True,
            text=True,
            timeout=TIMEOUT,
        )
        return result.stdout, result.returncode
    except subprocess.TimeoutExpired:
        return f"TIMEOUT after {TIMEOUT}s", -1


def check_docker() -> None:
    try:
        r = subprocess.run(["docker", "info"], capture_output=True, timeout=10)
        if r.returncode != 0:
            sys.exit("Docker daemon is not running.")
    except FileNotFoundError:
        sys.exit("'docker' not found in PATH.")


# ---------------------------------------------------------------------------
# Source manipulation
# ---------------------------------------------------------------------------

def get_current_extensions(source: str) -> list[str]:
    """Return list of extensions already declared in the source."""
    exts: list[str] = []
    for m in re.finditer(r"extend\s+with\s+([^;]+);", source):
        for e in re.findall(r"#[\w-]+", m.group(1)):
            exts.append(e)
    return exts


def inject_extensions(source: str, new_exts: list[str]) -> str:
    """
    Add new_exts to the source.

    Strategy: find the last `extend with ...;` block and append to it.
    If none exists, insert a new `extend with ...;` line after `language core;`.
    """
    if not new_exts:
        return source

    # Find an existing extend block to append to
    last_match = None
    for m in re.finditer(r"(extend\s+with\s+)([^;]+)(;)", source):
        last_match = m

    if last_match:
        existing = last_match.group(2).rstrip()
        appended = existing + ",\n  " + ",\n  ".join(new_exts)
        new_block = last_match.group(1) + appended + last_match.group(3)
        return source[: last_match.start()] + new_block + source[last_match.end() :]

    # No extend block — insert after `language core;`
    lang_match = re.search(r"language\s+\w+\s*;", source)
    if lang_match:
        insert_pos = lang_match.end()
        ext_line = "\nextend with " + ", ".join(new_exts) + ";"
        return source[:insert_pos] + ext_line + source[insert_pos:]

    # Fallback: prepend
    return "extend with " + ", ".join(new_exts) + ";\n" + source


def extract_hinted_extensions(ref_output: str) -> list[str]:
    """
    Parse "Perhaps, you would like to enable '#foo' or '#bar' extension?" from
    reference output and return a list of extension names like ['#foo', '#bar'].
    """
    hints: list[str] = []
    for m in re.finditer(r"Perhaps.*?enable\s+((?:'#[\w-]+'(?:\s+or\s+)?)+)", ref_output):
        for e in re.findall(r"#[\w-]+", m.group(1)):
            hints.append(e)
    return hints


# ---------------------------------------------------------------------------
# Golden file I/O
# ---------------------------------------------------------------------------

def parse_golden(path: Path) -> dict:
    """Parse a golden file into {input, stdout, stderr, exit_code}."""
    content = path.read_text()
    sections: dict = {"input": [], "stdout": [], "stderr": [], "exit_code": 0}
    current = None

    for line in content.split("\n"):
        if line.startswith("INPUT:"):
            current = "input"
        elif line.startswith("STDOUT:"):
            current = "stdout"
        elif line.startswith("STDERR:"):
            current = "stderr"
        elif line.startswith("EXIT_CODE:"):
            current = None
            sections["exit_code"] = int(line.split(":", 1)[1].strip())
        elif current:
            sections[current].append(line)

    return {
        "input": "\n".join(sections["input"]).strip(),
        "stdout": "\n".join(sections["stdout"]).strip(),
        "stderr": "\n".join(sections["stderr"]).strip(),
        "exit_code": sections["exit_code"],
    }


def write_golden(path: Path, data: dict) -> None:
    """Rewrite a golden file preserving exact format."""
    input_str  = data["input"]  or "(empty)"
    stdout_str = data["stdout"] or "(empty)"
    stderr_str = data["stderr"] or "(empty)"
    content = (
        f"INPUT:\n{input_str}\n\n"
        f"STDOUT:\n{stdout_str}\n\n"
        f"STDERR:\n{stderr_str}\n\n"
        f"EXIT_CODE: {data['exit_code']}\n"
    )
    path.write_text(content)


# ---------------------------------------------------------------------------
# Verification helpers
# ---------------------------------------------------------------------------

def is_reference_ok(ref_out: str, ref_exit: int, golden: dict) -> bool:
    """Check if reference result matches what the golden expects."""
    if golden["exit_code"] == 0:
        return ref_exit == 0
    # error test: expected error code must appear in reference output
    expected_code = extract_golden_error_code(golden["stderr"])
    if expected_code is None:
        return False
    return f"[{expected_code}]" in ref_out


def extract_golden_error_code(golden_stderr: str) -> str | None:
    for line in golden_stderr.splitlines():
        line = line.strip()
        if line:
            return line.rstrip(":")
    return None


def is_reference_broken_by_extension(ref_out: str) -> bool:
    """
    True when the reference output indicates a missing extension (not a real type error).

    Excludes cases where the reference gave a real type error (even if tagged as
    "Unsupported Syntax Error" — that just means the reference doesn't emit a
    [ERROR_CODE] tag for it, but extensions won't fix it either).
    """
    if "Perhaps, you would like to enable" in ref_out:
        return True
    if "Illegal expression" in ref_out or "Illegal type" in ref_out:
        return True
    # Generic "syntax error at line N" — only treat as fixable if there's no
    # "Unsupported Syntax Error" marker (which means it's a genuine unsupported feature).
    if "Unsupported Syntax Error" in ref_out:
        return False
    if re.search(r"syntax error at line \d+", ref_out, re.IGNORECASE):
        return True
    return False


# ---------------------------------------------------------------------------
# Core fixer
# ---------------------------------------------------------------------------

def find_stella_file(golden_path: Path, golden_root: Path, stella_root: Path) -> Path | None:
    """Derive the .stella path from a .golden path."""
    rel = golden_path.relative_to(golden_root)
    candidate = stella_root / rel.with_suffix(".stella")
    return candidate if candidate.exists() else None


def fix_golden(
    golden_path: Path,
    golden_root: Path,
    stella_root: Path,
    dry_run: bool,
    verbose: bool,
) -> str:
    """
    Returns one of: 'skip' (already ok), 'fixed', 'failed', 'no_stella'.
    """
    golden = parse_golden(golden_path)
    source = golden["input"]

    stella_path = find_stella_file(golden_path, golden_root, stella_root)
    if stella_path is None:
        return "no_stella"

    # Quick check: is it already fine?
    ref_out, ref_exit = run_reference(source)
    if is_reference_ok(ref_out, ref_exit, golden):
        return "skip"

    if verbose:
        print(f"    initial ref output: {ref_out[:120].strip()!r}")

    current_source = source
    added_exts: list[str] = []

    for iteration in range(MAX_ITERATIONS):
        if not is_reference_broken_by_extension(ref_out):
            # Reference returned a real type error, but wrong code.
            # Extensions won't help here.
            break

        hints = extract_hinted_extensions(ref_out)

        if hints:
            # Filter out extensions already present
            present = set(get_current_extensions(current_source))
            new = [e for e in hints if e not in present and e not in added_exts]
            if not new:
                # Hints exhausted with no progress
                break
            to_add = new
        else:
            # No hints: try ALL_KNOWN_EXTENSIONS one at a time
            present = set(get_current_extensions(current_source)) | set(added_exts)
            remaining = [e for e in ALL_KNOWN_EXTENSIONS if e not in present]
            if not remaining:
                break
            to_add = [remaining[0]]

        current_source = inject_extensions(current_source, to_add)
        added_exts.extend(to_add)

        if verbose:
            print(f"    iter {iteration+1}: injected {to_add}")

        ref_out, ref_exit = run_reference(current_source)
        if is_reference_ok(ref_out, ref_exit, golden):
            # Found a working extension set
            if dry_run:
                print(f"    [dry-run] would add: {added_exts}")
                return "fixed"
            # Rewrite golden INPUT
            golden["input"] = current_source
            write_golden(golden_path, golden)
            # Rewrite .stella file
            stella_path.write_text(current_source)
            return "fixed"

    return "failed"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Auto-inject missing extensions into golden files and .stella sources."
    )
    parser.add_argument("--golden-dir", default="tests/golden")
    parser.add_argument("--stella-dir", default="stella-examples")
    parser.add_argument("--filter", default=None,
                        help="Only process files whose path contains this substring")
    parser.add_argument("--dry-run", action="store_true",
                        help="Show what would change without writing files")
    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("--fail-fast", action="store_true")
    args = parser.parse_args()

    check_docker()

    golden_root = Path(args.golden_dir)
    stella_root = Path(args.stella_dir)

    golden_files = sorted(golden_root.glob("**/*.golden"))
    if args.filter:
        golden_files = [f for f in golden_files if args.filter in str(f)]

    if not golden_files:
        sys.exit(f"No golden files found under {golden_root}")

    print(f"Processing {len(golden_files)} golden files...\n")

    counts = {"skip": 0, "fixed": 0, "failed": 0, "no_stella": 0}

    for gf in golden_files:
        rel = gf.relative_to(golden_root)
        print(f"[{rel}]", end="  ", flush=True)
        result = fix_golden(gf, golden_root, stella_root,
                            dry_run=args.dry_run, verbose=args.verbose)
        counts[result] += 1
        label = {"skip": "ok (skip)", "fixed": "FIXED", "failed": "FAILED", "no_stella": "no .stella"}[result]
        print(label)
        if args.verbose and result == "failed":
            golden = parse_golden(gf)
            ref_out, ref_exit = run_reference(golden["input"])
            print(f"    ref exit={ref_exit} out={ref_out[:200].strip()!r}")
        if args.fail_fast and result == "failed":
            break

    print(f"\n{'='*60}")
    print(f"skipped (already ok): {counts['skip']}")
    print(f"fixed:                {counts['fixed']}")
    print(f"failed:               {counts['failed']}")
    print(f"no .stella file:      {counts['no_stella']}")
    return 0 if counts["failed"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
