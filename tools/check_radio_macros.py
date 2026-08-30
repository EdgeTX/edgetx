#!/usr/bin/env python3
"""Detect RADIO_* macros that are referenced but can never be defined.

These exist only as CMake -D flags, so a stale '#if defined(RADIO_FOO)' is
silently always-false. Both sets are derived from the tree.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "radio" / "src"
TARGETS = SRC / "targets"

TOKEN = r"RADIO_[A-Z0-9_]+"
SRC_EXT = {".c", ".cpp", ".h", ".hpp", ".inc"}
# dot-dirs skipped too: a stray git worktree must not widen the definable set
SKIP_DIRS = {"thirdparty", "build"}


def skipped(path):
    return any(p in SKIP_DIRS or p.startswith(".") for p in path.parts)


def flavour_macro(flavour):
    # mirrors radio/src/CMakeLists.txt: TOUPPER then '+' -> 'P'
    return "RADIO_" + flavour.upper().replace("+", "P")


def definable():
    """Every RADIO_* the build can possibly define."""
    names, why = set(), {}

    def add(name, reason):
        names.add(name)
        why.setdefault(name, reason)

    for cml in TARGETS.glob("*/CMakeLists.txt"):
        text = cml.read_text(errors="ignore")
        for flav in re.findall(r"set\s*\(\s*FLAVOUR\s+([A-Za-z0-9+._-]+)\s*\)", text):
            add(flavour_macro(flav), f"FLAVOUR {flav} ({cml.relative_to(ROOT)})")

    # hand-written -DRADIO_* anywhere in the CMake layer
    cmake_files = []
    for base in (ROOT / "radio", ROOT / "cmake"):
        cmake_files += list(base.rglob("CMakeLists.txt")) + list(base.rglob("*.cmake"))
    for cml in cmake_files:
        if skipped(cml.relative_to(ROOT)):
            continue
        for name in re.findall(r"-D(" + TOKEN + ")", cml.read_text(errors="ignore")):
            add(name, f"add_definitions ({cml.relative_to(ROOT)})")

    # plain #define in sources
    for f in walk_sources():
        for name in re.findall(r"^\s*#\s*define\s+(" + TOKEN + ")", f.read_text(errors="ignore"), re.M):
            add(name, f"#define ({f.relative_to(ROOT)})")

    return names, why


def walk_sources():
    for f in SRC.rglob("*"):
        if f.suffix in SRC_EXT and f.is_file() and not skipped(f.relative_to(ROOT)):
            yield f


def referenced():
    """Every RADIO_* the tree tests, with locations."""
    hits = {}
    cond = re.compile(r"^\s*#\s*(?:if|elif|ifdef|ifndef)\b(.*)$")
    for f in walk_sources():
        for n, line in enumerate(f.read_text(errors="ignore").splitlines(), 1):
            m = cond.match(line)
            if m:
                for name in re.findall(TOKEN, m.group(1)):
                    hits.setdefault(name, []).append(f"{f.relative_to(ROOT)}:{n}")

    # hw_defs codegen reads these back out of the preprocessed dump
    for f in (ROOT / "radio" / "util").rglob("*.py"):
        if skipped(f.relative_to(ROOT)):
            continue
        for n, line in enumerate(f.read_text(errors="ignore").splitlines(), 1):
            if "hw_defs" in line:
                for name in re.findall(r"['\"](" + TOKEN + r")['\"]", line):
                    hits.setdefault(name, []).append(f"{f.relative_to(ROOT)}:{n}")
    return hits


def main():
    can, _ = definable()
    used = referenced()
    orphans = {k: v for k, v in used.items() if k not in can}

    print(f"{len(can)} definable RADIO_* macros, {len(used)} referenced")
    if not orphans:
        print("OK: every referenced RADIO_* macro can be defined.")
        return 0

    print(f"\nERROR: {len(orphans)} RADIO_* macro(s) referenced but never definable:\n", file=sys.stderr)
    for name in sorted(orphans):
        print(f"  {name}", file=sys.stderr)
        for loc in orphans[name]:
            print(f"      {loc}", file=sys.stderr)
    print("\nEither the name is stale, or its target no longer defines it.", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
