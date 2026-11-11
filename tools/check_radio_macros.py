#!/usr/bin/env python3
"""Find referenced RADIO_* macros that no target can define."""

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "radio" / "src"
TARGETS = SRC / "targets"
TOKEN = r"RADIO_[A-Z0-9_]+"
SOURCE_EXTENSIONS = {".c", ".cpp", ".h", ".hpp", ".inc"}
SKIP_DIRS = {"thirdparty", "build"}


def skipped(path: Path) -> bool:
    return any(part in SKIP_DIRS or part.startswith(".") for part in path.parts)


def source_files():
    for path in SRC.rglob("*"):
        if (
            path.is_file()
            and path.suffix in SOURCE_EXTENSIONS
            and not skipped(path.relative_to(ROOT))
        ):
            yield path


def definable_macros() -> set[str]:
    macros = set()

    for cmake_file in TARGETS.rglob("CMakeLists.txt"):
        if skipped(cmake_file.relative_to(ROOT)):
            continue
        text = cmake_file.read_text(errors="ignore")
        for flavour in re.findall(
            r"set\s*\(\s*FLAVOUR\s+([A-Za-z0-9+._-]+)\s*\)", text
        ):
            macros.add("RADIO_" + flavour.upper().replace("+", "P"))

    cmake_files = []
    for base in (ROOT / "radio", ROOT / "cmake"):
        cmake_files.extend(base.rglob("CMakeLists.txt"))
        cmake_files.extend(base.rglob("*.cmake"))
    for cmake_file in cmake_files:
        if skipped(cmake_file.relative_to(ROOT)):
            continue
        macros.update(
            re.findall(r"-D(" + TOKEN + r")", cmake_file.read_text(errors="ignore"))
        )

    for source_file in source_files():
        macros.update(
            re.findall(
                r"^\s*#\s*define\s+(" + TOKEN + r")",
                source_file.read_text(errors="ignore"),
                re.M,
            )
        )

    return macros


def referenced_macros() -> dict[str, list[str]]:
    references: dict[str, list[str]] = {}
    condition = re.compile(r"^\s*#\s*(?:if|elif|ifdef|ifndef)\b(.*)$")

    for source_file in source_files():
        for line_number, line in enumerate(
            source_file.read_text(errors="ignore").splitlines(), 1
        ):
            match = condition.match(line)
            if match:
                for macro in re.findall(TOKEN, match.group(1)):
                    references.setdefault(macro, []).append(
                        f"{source_file.relative_to(ROOT)}:{line_number}"
                    )

    for script in (ROOT / "radio" / "util").rglob("*.py"):
        if skipped(script.relative_to(ROOT)):
            continue
        for line_number, line in enumerate(
            script.read_text(errors="ignore").splitlines(), 1
        ):
            if "hw_defs" in line:
                for macro in re.findall(r"['\"](" + TOKEN + r")['\"]", line):
                    references.setdefault(macro, []).append(
                        f"{script.relative_to(ROOT)}:{line_number}"
                    )

    return references


def main() -> int:
    definable = definable_macros()
    referenced = referenced_macros()
    missing = {name: locations for name, locations in referenced.items() if name not in definable}

    print(f"{len(definable)} definable RADIO_* macros, {len(referenced)} referenced")
    if not missing:
        print("OK: every referenced RADIO_* macro can be defined.")
        return 0

    print(f"\nERROR: {len(missing)} RADIO_* macro(s) cannot be defined:", file=sys.stderr)
    for name in sorted(missing):
        print(f"  {name}", file=sys.stderr)
        for location in missing[name]:
            print(f"      {location}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
