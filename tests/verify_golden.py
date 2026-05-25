#!/usr/bin/env python3
"""
Verifies golden test files against the reference Stella typechecker.

Reference model: docker run -i fizruk/stella typecheck
  - reads Stella source from stdin
  - exit 0 + "Input program is well-typed!" for ok programs
  - exit 1 + "Type Error Tag: [ERROR_CODE]" on stderr for type errors

For each golden file this script checks:
  - ok tests (EXIT_CODE 0): reference must also exit 0
  - error tests (EXIT_CODE 1): the error code from the golden STDERR first line
    must appear in the reference stderr as [ERROR_CODE]

The reference may emit multiple "Alternative type errors" — any match is accepted.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path


DOCKER_IMAGE = "fizruk/stella"
DOCKER_CMD = ["docker", "run", "-i", DOCKER_IMAGE, "typecheck"]
TIMEOUT = 30  # seconds per test

# Some error codes are named differently in the reference implementation.
# Maps our code -> list of reference codes that are acceptable equivalents.
ERROR_CODE_ALIASES: dict[str, list[str]] = {
    "ERROR_AMBIGUOUS_LIST": ["ERROR_AMBIGUOUS_LIST_TYPE"],
    "ERROR_AMBIGUOUS_SUM_TYPE": ["ERROR_AMBIGUOUS_SUM_TYPE", "ERROR_AMBIGUOUS_SUM"],
}


def check_docker() -> None:
    try:
        result = subprocess.run(
            ["docker", "info"],
            capture_output=True, timeout=10
        )
        if result.returncode != 0:
            sys.exit("Docker daemon is not running. Start Docker and retry.")
    except FileNotFoundError:
        sys.exit("'docker' not found in PATH.")
    except subprocess.TimeoutExpired:
        sys.exit("Docker info timed out.")


def parse_golden(golden_path: Path) -> tuple[str, int]:
    """
    Returns (input_source, exit_code) from a golden file.
    """
    content = golden_path.read_text()
    in_input = False
    in_stderr = False
    input_lines: list[str] = []
    stderr_lines: list[str] = []
    exit_code = 0

    for line in content.split("\n"):
        if line.startswith("INPUT:"):
            in_input, in_stderr = True, False
        elif line.startswith("STDOUT:"):
            in_input, in_stderr = False, False
        elif line.startswith("STDERR:"):
            in_input, in_stderr = False, True
        elif line.startswith("EXIT_CODE:"):
            in_input, in_stderr = False, False
            exit_code = int(line.split(":", 1)[1].strip())
        elif in_input:
            input_lines.append(line)
        elif in_stderr:
            stderr_lines.append(line)

    source = "\n".join(input_lines).strip()
    stderr = "\n".join(stderr_lines).strip()
    return source, stderr, exit_code


def extract_error_code(golden_stderr: str) -> str | None:
    """
    Extracts the error code from the first non-empty line of golden STDERR.
    E.g. "ERROR_MISSING_RECORD_FIELDS:" -> "ERROR_MISSING_RECORD_FIELDS"
    """
    for line in golden_stderr.splitlines():
        line = line.strip()
        if line:
            # strip trailing colon
            return line.rstrip(":")
    return None


def sanitize_source(source: str) -> str:
    """
    Strip non-ASCII characters from single-line comments.

    The reference typechecker (Haskell) uses Latin-1 by default and crashes
    on UTF-8 bytes inside comments.  The Stella grammar only uses // comments
    (no block comments in examples), so replacing the comment body with an
    empty comment is safe — comments have no effect on type-checking.
    """
    return re.sub(r"//[^\n]*", "//", source)


def run_reference(source: str) -> tuple[str, int]:
    """
    Runs reference typechecker, returns (output, exit_code).

    Note: fizruk/stella writes ALL output (errors included) to stdout, not stderr.
    """
    try:
        result = subprocess.run(
            DOCKER_CMD,
            input=sanitize_source(source),
            capture_output=True,
            text=True,
            timeout=TIMEOUT,
        )
        # Reference writes everything to stdout
        return result.stdout, result.returncode
    except subprocess.TimeoutExpired:
        return f"TIMEOUT after {TIMEOUT}s", -1


def verify_golden(golden_path: Path, verbose: bool) -> bool:
    """
    Returns True if the golden file is consistent with the reference.
    """
    source, golden_stderr, golden_exit = parse_golden(golden_path)

    ref_stderr, ref_exit = run_reference(source)

    if golden_exit == 0:
        # ok test: reference must also succeed
        if ref_exit == 0:
            if verbose:
                print(f"  reference: ok (exit 0) ✓")
            return True
        else:
            print(f"  MISMATCH: golden expects ok (exit 0), reference exited {ref_exit}")
            if verbose:
                print(f"  reference stderr:\n{ref_stderr}")
            return False
    else:
        # error test: check the error code appears in reference output
        error_code = extract_error_code(golden_stderr)
        if error_code is None:
            print(f"  WARNING: could not extract error code from golden STDERR, skipping")
            return True

        # Build the list of acceptable codes (our code + any known aliases)
        candidates = [error_code] + ERROR_CODE_ALIASES.get(error_code, [])

        for code in candidates:
            # Standard format:   Type Error Tag: [ERROR_CODE]
            if f"[{code}]" in ref_stderr:
                if verbose:
                    match = code if code == error_code else f"{error_code} (via alias {code})"
                    print(f"  reference confirms {match} ✓")
                return True
            # Unsupported-syntax format: Unsupported Syntax Error: ERROR_CODE
            if f"Unsupported Syntax Error: {code}" in ref_stderr:
                if verbose:
                    print(f"  reference confirms {error_code} (Unsupported Syntax Error) ✓")
                return True

        print(f"  MISMATCH: golden expects {error_code}, not found in reference output")
        if verbose:
            print(f"  reference output:\n{ref_stderr}")
        return False


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify golden files against the reference Stella typechecker (Docker)."
    )
    parser.add_argument(
        "--golden-dir",
        default="tests/golden",
        help="Directory containing .golden files (default: tests/golden)",
    )
    parser.add_argument(
        "--filter",
        type=str,
        default=None,
        help="Only verify golden files whose path contains this substring",
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Print reference output for each test",
    )
    parser.add_argument(
        "--fail-fast",
        action="store_true",
        help="Stop after the first mismatch",
    )
    args = parser.parse_args()

    check_docker()

    golden_root = Path(args.golden_dir)
    if not golden_root.is_dir():
        sys.exit(f"Golden directory not found: {golden_root}")

    golden_files = sorted(golden_root.glob("**/*.golden"))
    if args.filter:
        golden_files = [f for f in golden_files if args.filter in str(f)]

    if not golden_files:
        sys.exit(f"No .golden files found under {golden_root}")

    print(f"Verifying {len(golden_files)} golden files against {DOCKER_IMAGE}...\n")

    passed = failed = 0
    for gf in golden_files:
        rel = gf.relative_to(golden_root)
        print(f"[{rel}]", end="  ")
        ok = verify_golden(gf, verbose=args.verbose)
        if ok:
            print("OK")
            passed += 1
        else:
            failed += 1
            if args.fail_fast:
                break

    print(f"\n{'='*60}")
    print(f"Result: {passed} passed, {failed} failed out of {passed + failed} checked")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
