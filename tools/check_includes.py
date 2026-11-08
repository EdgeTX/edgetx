#!/usr/bin/env python3
"""Stop edgetx.h from spreading further into headers and hardware code.

Existing includes are listed in check_includes_baseline.txt, which can only
shrink: a new include fails, and so does a baseline entry that has been fixed.
See docs/development/header-guidelines.md.

Usage: check_includes.py [--update]
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "radio" / "src"
BASELINE = Path(__file__).with_name("check_includes_baseline.txt")

INCLUDE = re.compile(r'^\s*#\s*include\s*"(?:\.\./)*edgetx\.h"', re.M)
SRC_EXT = {".c", ".cpp", ".h", ".hpp"}
SKIP_DIRS = {"thirdparty", "tests"}
HARDWARE_DIRS = {"boards", "drivers", "hal", "os", "targets"}


def includers():
    for path in sorted(SRC.rglob("*")):
        rel = path.relative_to(SRC)
        if path.suffix not in SRC_EXT or path.is_dir():
            continue
        if SKIP_DIRS.intersection(rel.parts) or any(p.startswith(".") for p in rel.parts):
            continue
        if INCLUDE.search(path.read_text(errors="replace")):
            yield rel


def restricted(rel):
    return rel.suffix in {".h", ".hpp"} or rel.parts[0] in HARDWARE_DIRS


def main():
    found = list(includers())
    current = {r.as_posix() for r in found if restricted(r)}

    if "--update" in sys.argv[1:]:
        BASELINE.write_text("".join(f"{r}\n" for r in sorted(current)))
        print(f"{BASELINE.name}: {len(current)} entries")
        return 0

    baseline = set(BASELINE.read_text().split())
    new = sorted(current - baseline)
    fixed = sorted(baseline - current)

    for rel in new:
        print(f"{rel}: must not include edgetx.h")
    for rel in fixed:
        print(f"{rel}: no longer includes edgetx.h, remove it from {BASELINE.name}")

    print(f"edgetx.h includes: {len(found)} in total, {len(current)} in headers and hardware code")
    return 1 if new or fixed else 0


if __name__ == "__main__":
    sys.exit(main())
