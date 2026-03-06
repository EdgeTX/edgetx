#!/usr/bin/env python3
import argparse
import difflib
import os
import re
import sys
from collections import OrderedDict


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
RADIO_TARGETS_TSV = os.path.join(ROOT, "tools", "radio-targets.tsv")
CURATED_TSV = os.path.join(ROOT, "tools", "radio-firmware-options.tsv")
OUT_DEFAULT = os.path.join(ROOT, "tools", "radio-firmware-options.generated.tsv")

OPTION_RE = re.compile(
    r'^\s*option\s*\(\s*([A-Za-z0-9_]+)\s+"([^"]*)"\s+([A-Za-z0-9_]+)\s*\)'
)
PCB_RE = re.compile(r"\bPCB\s+STREQUAL\s+([A-Za-z0-9_+.-]+)")
PCBREV_RE = re.compile(r"\bPCBREV\s+STREQUAL\s+([A-Za-z0-9_+.-]+)")

GLOBAL_OPTION_FILES = {
    "radio/src/CMakeLists.txt",
    "radio/src/targets/common/arm/CMakeLists.txt",
    "radio/src/targets/common/arm/stm32/CMakeLists.txt",
    "radio/src/gui/colorlcd/CMakeLists.txt",
}

SKIP_OPTION_NAMES = {
    "SIMU_TARGET",
    "SIMU_DISKIO",
    "SIMU_LUA_COMPILER",
    "TRACE_SIMPGMSPACE",
    "TRACE_LUA_INTERNALS",
    "TRACE_AUDIO",
}

SKIP_PATH_PARTS = (
    "/thirdparty/",
    "/targets/simu/",
)


def relpath(path):
    return os.path.relpath(path, ROOT).replace(os.sep, "/")


def norm_token(token):
    return token.strip().upper()


def load_known_radio_tokens():
    tokens = set()
    if not os.path.exists(RADIO_TARGETS_TSV):
        return tokens
    with open(RADIO_TARGETS_TSV, "r", encoding="utf-8") as f:
        for line in f:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 4:
                continue
            pcb = parts[2].strip()
            pcbrev = parts[3].strip()
            if pcb:
                tokens.add(norm_token(pcb))
            if pcbrev:
                tokens.add(norm_token(pcbrev))
    return tokens


def discover_cmake_files():
    cmake_files = []
    for base in (
        os.path.join(ROOT, "radio", "src"),
        os.path.join(ROOT, "radio", "src", "targets"),
    ):
        if not os.path.isdir(base):
            continue
        for root, _, files in os.walk(base):
            for name in files:
                if name == "CMakeLists.txt":
                    path = os.path.join(root, name)
                    rp = relpath(path)
                    if any(part in f"/{rp}" for part in SKIP_PATH_PARTS):
                        continue
                    cmake_files.append(path)
    return sorted(set(cmake_files))


def file_level_filters(rp, content, known_tokens):
    filters = set()
    for m in PCB_RE.finditer(content):
        token = norm_token(m.group(1))
        if token in known_tokens:
            filters.add(token)
    for m in PCBREV_RE.finditer(content):
        token = norm_token(m.group(1))
        if token in known_tokens:
            filters.add(token)

    if filters:
        return filters

    # Fall back to target directory name if it matches known tokens.
    m = re.search(r"radio/src/targets/([^/]+)/CMakeLists\.txt$", rp)
    if m:
        dir_token = norm_token(m.group(1))
        if dir_token in known_tokens:
            filters.add(dir_token)
    return filters


def normalize_label(option_name, description):
    label = (description or "").strip().strip(".")
    label = re.sub(r"^\s*((enable|disable)\s+)+", "", label, flags=re.IGNORECASE)
    # Remove link/vendor noise from CMake descriptions.
    label = re.sub(r"https?://\S+", "", label, flags=re.IGNORECASE)
    label = re.sub(r"\b(github|pascallanger|edgetx)\b", "", label, flags=re.IGNORECASE)
    label = label.replace("DIY Multiprotocol TX Module", "Multiprotocol TX Module")
    label = label.replace("Competition mode", "FAI mode")
    label = label.replace("Dangerous module functions (RangeCheck / Bind / Module OFF, etc.) available", "Dangerous module functions")
    label = label.replace("Trace Lua memory (de)allocations to debug port (also needs DEBUG=YES NANO=NO)", "Trace Lua memory")
    label = label.replace("Support of old FAS prototypes (different resistors)", "Old FAS prototypes")
    label = re.sub(r"\(\s*\)", "", label)
    if label.count("(") > label.count(")"):
        label = re.sub(r"\s*\($", "", label)
    elif label.count(")") > label.count("("):
        label = re.sub(r"\)\s*$", "", label)
    label = re.sub(r"\s{2,}", " ", label).strip()
    if not label:
        label = option_name
    if label:
        label = label[0].upper() + label[1:]
    return label


def build_rows():
    known_tokens = load_known_radio_tokens()
    rows = OrderedDict()

    for cmake in discover_cmake_files():
        rp = relpath(cmake)
        with open(cmake, "r", encoding="utf-8") as f:
            content = f.read()

        file_filters = file_level_filters(rp, content, known_tokens)
        is_global_file = rp in GLOBAL_OPTION_FILES

        for line in content.splitlines():
            m = OPTION_RE.match(line)
            if not m:
                continue
            opt_name, opt_desc, _opt_default = m.groups()
            if opt_name in SKIP_OPTION_NAMES:
                continue
            if opt_name.startswith("SIMU_") or opt_name.startswith("LV_"):
                continue

            label = normalize_label(opt_name, opt_desc)
            if opt_name not in rows:
                rows[opt_name] = {
                    "label": label,
                    "filters": set(),
                    "has_global": False,
                }
            else:
                if rows[opt_name]["label"] == opt_name and label != opt_name:
                    rows[opt_name]["label"] = label

            if not is_global_file:
                rows[opt_name]["filters"].update(file_filters)
            else:
                rows[opt_name]["has_global"] = True

    return rows


def rows_to_tsv(rows):
    sorted_items = sorted(rows.items(), key=lambda kv: kv[0])
    lines = []
    for option_name, row in sorted_items:
        label = row["label"]
        filters = row["filters"]
        has_global = row["has_global"]
        filt = "" if has_global else ",".join(sorted(filters))
        lines.append(f"{label}\t{option_name}\t{filt}\n")
    return "".join(lines)


def write_tsv(text, out_path, force=False):
    out_abs = os.path.abspath(out_path)
    if os.path.exists(out_abs) and not force:
        print(
            f"Refusing to overwrite existing file: {out_abs}\n"
            f"Re-run with --force to overwrite.",
            file=sys.stderr,
        )
        return 1
    os.makedirs(os.path.dirname(out_abs), exist_ok=True)
    with open(out_abs, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"Wrote {out_abs}")
    return 0


def check_tsv(path, generated_text):
    check_abs = os.path.abspath(path)
    if not os.path.exists(check_abs):
        print(f"Check target does not exist: {check_abs}", file=sys.stderr)
        return 1
    with open(check_abs, "r", encoding="utf-8") as f:
        current = f.read()
    if current == generated_text:
        print(f"OK: {check_abs} is up to date.")
        return 0
    print(f"Mismatch: {check_abs} differs from generated output.", file=sys.stderr)
    diff = difflib.unified_diff(
        current.splitlines(keepends=True),
        generated_text.splitlines(keepends=True),
        fromfile=check_abs,
        tofile="generated",
        n=2,
    )
    for line in diff:
        sys.stderr.write(line)
    return 1


def main():
    parser = argparse.ArgumentParser(description="Generate tools/radio-firmware-options.tsv from CMake option() definitions.")
    parser.add_argument("--out", default=OUT_DEFAULT, help="Output TSV path (default: tools/radio-firmware-options.generated.tsv)")
    parser.add_argument("--force", action="store_true", help="Allow overwriting an existing output TSV.")
    parser.add_argument(
        "--check",
        nargs="?",
        const=CURATED_TSV,
        help="Compare generated output with an existing TSV (default: tools/radio-firmware-options.tsv).",
    )
    args = parser.parse_args()

    rows = build_rows()
    generated_tsv = rows_to_tsv(rows)
    if args.check:
        return check_tsv(args.check, generated_tsv)
    result = write_tsv(generated_tsv, args.out, force=args.force)
    if result == 0:
        print(f"Generated {len(rows)} options")
    return result


if __name__ == "__main__":
    raise SystemExit(main())
