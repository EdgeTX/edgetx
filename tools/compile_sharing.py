#!/usr/bin/env python3
"""Report how many firmware compilations radios could share.

A file can only be compiled once for several radios if it preprocesses to the same
text with the same compiler flags. This counts those files per family of radios
with the same flags and LCD, and which global defines still differ inside a family.

Usage: compile_sharing.py <builds-dir>

Expects one configured and built <builds-dir>/<radio>/arm-none-eabi per radio, as
preprocessing needs the generated headers.
"""
import collections
import concurrent.futures as cf
import hashlib
import json
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

FLAGS = re.compile(r"-mcpu=|-mfpu=|-mfloat-abi=|-flto")
SKIP_ARGS = {"-MF", "-MT", "-MQ", "-o"}
DROP_ARGS = {"-MD", "-MMD", "-c"}


def preprocess_args(entry):
    args, skip = [], False
    for arg in shlex.split(entry["command"]):
        if skip:
            skip = False
        elif arg in SKIP_ARGS:
            skip = True
        elif arg not in DROP_ARGS and arg != entry["file"]:
            args.append(arg)
    return args


def firmware_entries(build):
    for entry in json.loads((build / "compile_commands.json").read_text()):
        if "CMakeFiles/firmware.dir/" in entry["command"] and "radio/src/" in entry["file"]:
            yield entry


def fingerprint(entry):
    args = preprocess_args(entry)
    out = subprocess.run(args + ["-E", "-P", entry["file"]], cwd=entry["directory"],
                         capture_output=True, check=True).stdout
    flags = " ".join(a for a in args if FLAGS.match(a))
    return (entry["file"].split("radio/src/")[-1], hashlib.sha1(out).hexdigest(), flags, len(out))


def lcd_size(entry):
    probe = '#include "edgetx.h"\nLCD_SIZE_IS LCD_W LCD_H\n'
    with tempfile.NamedTemporaryFile("w", suffix=".cpp") as src:
        src.write(probe)
        src.flush()
        args = preprocess_args(entry)
        out = subprocess.run(args + ["-E", "-P", src.name], cwd=entry["directory"],
                             capture_output=True, text=True, errors="replace").stdout
    found = re.findall(r"LCD_SIZE_IS (\S+) (\S+)", out)
    return "x".join(found[-1]) if found else "?"


def scan(pool, build):
    entries = list(firmware_entries(build))
    reference = next(e for e in entries if e["file"].endswith("radio/src/maths.cpp"))
    args = preprocess_args(reference)
    return {
        "files": list(pool.map(fingerprint, entries)),
        "cpu": " ".join(sorted({a for a in args if FLAGS.match(a)})),
        "lcd": lcd_size(reference),
        "defines": {a[2:] for a in args if a.startswith("-D")},
    }


def report(name, radios, scans):
    rows = [row for radio in radios for row in scans[radio]["files"]]
    distinct = {row[:3]: row[3] for row in rows}
    total = sum(row[3] for row in rows)
    defines = [scans[radio]["defines"] for radio in radios]
    varying = set.union(*defines) - set.intersection(*defines)
    print(f"{name}: {', '.join(radios)}")
    print(f"  {len(rows)} compiles, {len(distinct)} distinct: "
          f"{100 * (1 - len(distinct) / len(rows)):.0f}% avoidable, "
          f"{100 * (1 - sum(distinct.values()) / total):.0f}% by preprocessed size")
    print(f"  {len(varying)} of {len(set.union(*defines))} global defines differ")
    return len(rows), len(distinct), total, sum(distinct.values())


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    builds = sorted(p for p in root.glob("*/arm-none-eabi") if (p / "compile_commands.json").exists())
    if not builds:
        print(f"No builds found in {root}")
        return 1

    with cf.ThreadPoolExecutor() as pool:
        scans = {b.parent.name: scan(pool, b) for b in builds}

    families = collections.defaultdict(list)
    for radio, result in scans.items():
        families[f"{result['cpu']}, LCD {result['lcd']}"].append(radio)

    totals = [report(name, radios, scans) for name, radios in sorted(families.items())]
    compiles, distinct, size, distinct_size = (sum(column) for column in zip(*totals))
    per_radio = [len(s["defines"]) for s in scans.values()]
    print(f"\nAll {len(scans)} radios in {len(families)} families: {compiles} compiles, {distinct} distinct: "
          f"{100 * (1 - distinct / compiles):.0f}% avoidable, "
          f"{100 * (1 - distinct_size / size):.0f}% by preprocessed size")
    print(f"Global defines per radio: {min(per_radio)} to {max(per_radio)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
