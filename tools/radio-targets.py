#!/usr/bin/env python3
import argparse
import difflib
import os
import re
import sys


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
CURATED_TSV = os.path.join(ROOT, "tools", "radio-targets.tsv")
GENERATED_TSV = os.path.join(ROOT, "tools", "radio-targets.generated.tsv")

TARGET_FILES = [
    "radio/src/targets/horus/CMakeLists.txt",
    "radio/src/targets/taranis/CMakeLists.txt",
    "radio/src/targets/pl18/CMakeLists.txt",
    "radio/src/targets/tx16smk3/CMakeLists.txt",
    "radio/src/targets/tx15/CMakeLists.txt",
]

FILE_PCB_HINTS = {
    "radio/src/targets/tx16smk3/CMakeLists.txt": "TX16SMK3",
    "radio/src/targets/pl18/CMakeLists.txt": "PL18",
    "radio/src/targets/tx15/CMakeLists.txt": "TX15",
}

PCB_RE = re.compile(r"^\s*(if|elseif)\s*\(\s*PCB\s+STREQUAL\s+([A-Za-z0-9_+.-]+)")
PCBREV_RE = re.compile(r"\bPCBREV\s+STREQUAL\s+([A-Za-z0-9_+.-]+)")
SET_PCBREV_RE = re.compile(r"^\s*set\s*\(\s*PCBREV\s+\"?([A-Za-z0-9_+.-]+)\"?")
SET_GUI_RE = re.compile(r"^\s*set\s*\(\s*GUI_DIR\s+([A-Za-z0-9_+.-]+)")
SET_BITMAPS_RE = re.compile(r"^\s*set\s*\(\s*BITMAPS_DIR\s+([A-Za-z0-9_+.-]+)")


def ordered_add(dct, key, value):
    if key not in dct:
        dct[key] = []
    if value not in dct[key]:
        dct[key].append(value)


def parse_targets():
    pcb_map = {}
    display_map = {}
    for relpath in TARGET_FILES:
        path = os.path.join(ROOT, relpath)
        if not os.path.exists(path):
            continue
        file_default_gui = None
        file_default_bitmaps = None
        current_pcb = None
        current_pcbrev = None
        current_gui = None
        current_bitmaps = None
        saw_pcb = False
        file_revs = []
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                if current_pcb is None:
                    m = SET_GUI_RE.search(line)
                    if m:
                        file_default_gui = m.group(1)
                    m = SET_BITMAPS_RE.search(line)
                    if m:
                        file_default_bitmaps = m.group(1)
                    m = SET_PCBREV_RE.search(line)
                    if m:
                        if m.group(1) not in file_revs:
                            file_revs.append(m.group(1))
                    for m in PCBREV_RE.finditer(line):
                        if m.group(1) not in file_revs:
                            file_revs.append(m.group(1))

                m = PCB_RE.search(line)
                if m:
                    current_pcb = m.group(2)
                    saw_pcb = True
                    current_pcbrev = None
                    current_gui = file_default_gui
                    current_bitmaps = file_default_bitmaps
                    pcb_map.setdefault(current_pcb, [])
                    continue

                if not current_pcb:
                    continue

                m = PCBREV_RE.search(line)
                if m:
                    current_pcbrev = m.group(1)
                    ordered_add(pcb_map, current_pcb, current_pcbrev)

                m = SET_PCBREV_RE.search(line)
                if m:
                    current_pcbrev = m.group(1)
                    ordered_add(pcb_map, current_pcb, current_pcbrev)

                m = SET_GUI_RE.search(line)
                if m:
                    current_gui = m.group(1)

                m = SET_BITMAPS_RE.search(line)
                if m:
                    current_bitmaps = m.group(1)

                if current_gui or current_bitmaps:
                    key = (current_pcb, current_pcbrev)
                    info = display_map.get(key, {})
                    if current_gui:
                        info["gui"] = current_gui
                    if current_bitmaps:
                        info["bitmaps"] = current_bitmaps
                    display_map[key] = info
        if not saw_pcb and relpath in FILE_PCB_HINTS:
            hinted = FILE_PCB_HINTS[relpath]
            if file_revs:
                for rev in file_revs:
                    ordered_add(pcb_map, hinted, rev)
                    display_map[(hinted, rev)] = {
                        "gui": file_default_gui,
                        "bitmaps": file_default_bitmaps,
                    }
            else:
                pcb_map.setdefault(hinted, [])
                display_map[(hinted, None)] = {
                    "gui": file_default_gui,
                    "bitmaps": file_default_bitmaps,
                }
    return pcb_map, display_map


def display_label(display_info):
    if not display_info:
        return ""
    gui = display_info.get("gui")
    bitmaps = display_info.get("bitmaps")
    color = ""
    if gui == "colorlcd":
        color = "Color"
    elif bitmaps in ("128x64", "212x64"):
        color = "BW"
    res = bitmaps if bitmaps and re.match(r"^\d+x\d+$", bitmaps) else ""
    parts = [p for p in [color, res] if p]
    return " ".join(parts)


def display_parts(display_info):
    if not display_info:
        return "", ""
    gui = display_info.get("gui")
    bitmaps = display_info.get("bitmaps")
    display_type = ""
    if gui == "colorlcd":
        display_type = "Color"
    elif bitmaps in ("128x64", "212x64"):
        display_type = "BW"
    resolution = bitmaps if bitmaps and re.match(r"^\d+x\d+$", bitmaps) else ""
    return display_type, resolution


def build_entries(pcb_map, display_map):
    entries = []
    skip_combos = {
        ("XLITES", "MT12"),
        ("XLITE", "MT12"),
    }
    display_overrides = {
        ("TX15", None): {"bitmaps": "480x320", "gui": "colorlcd"},
    }
    labels = {
        ("X10", "TX16S"): "Radiomaster TX16S",
        ("X10", "T16"): "Jumper T16",
        ("X10", "T15"): "Jumper T15",
        ("X10", "T18"): "Jumper T18",
        ("X10", "F16"): "FATFISH F16",
        ("X10", "V16"): "HelloRadioSky V16",
        ("X10", "EXPRESS"): "FrSky X10 Express",
        ("X10", "STD"): "FrSky X10",
        ("X12S", "13"): "FrSky X12S",
        ("X12S", "EXPRESS"): "FrSky X12S Express",
        ("X9D+", "2014"): "FrSky X9D+ (2014)",
        ("X9D+", "2019"): "FrSky X9D+ (2019)",
        ("X9D", None): "FrSky X9D",
        ("X9E", None): "FrSky X9E",
        ("X7", "ACCESS"): "FrSky X7 Access",
        ("X7", "T12"): "Jumper T12",
        ("X7", "TPRO"): "Jumper T-Pro",
        ("X7", "TPROV2"): "Jumper T-Pro V2",
        ("X7", "TPROS"): "Jumper T-Pro S",
        ("X7", "T14"): "Jumper T14",
        ("X7", "T20"): "Jumper T20",
        ("X7", "T20V2"): "Jumper T20 V2",
        ("X7", "T12MAX"): "Jumper T12 Max",
        ("X7", "TLITE"): "Jumper T-Lite",
        ("X7", "TLITEF4"): "Jumper T-Lite (F4)",
        ("X7", "TX12"): "Radiomaster TX12",
        ("X7", "TX12MK2"): "Radiomaster TX12 MkII",
        ("X7", "BOXER"): "Radiomaster Boxer",
        ("X7", "ZORRO"): "Radiomaster Zorro",
        ("X7", "POCKET"): "Radiomaster Pocket",
        ("X7", "MT12"): "Radiomaster MT12",
        ("X7", "V12"): "HelloRadioSky V12",
        ("X7", "V14"): "HelloRadioSky V14",
        ("X7", "GX12"): "Radiomaster GX12",
        ("X7", "COMMANDO8"): "iFlight Commando 8",
        ("X7", "BUMBLEBEE"): "Jumper Bumblebee",
        ("X7", "LR3PRO"): "BetaFPV LiteRadio 3 Pro",
        ("X7", "T8"): "Radiomaster T8",
        ("X9LITE", None): "FrSky X9 Lite",
        ("X9LITES", None): "FrSky X9 Lite S",
        ("XLITE", None): "FrSky X-Lite",
        ("XLITES", None): "FrSky X-Lite S",
        ("TX16SMK3", None): "Radiomaster TX16S MK3",
        ("TX15", None): "Radiomaster TX15",
        ("PL18", None): "FlySky PL18",
        ("PL18", "PL18U"): "FlySky PL18",
        ("PL18", "EL18"): "FlySky EL18",
        ("PL18", "NV14"): "FlySky NV14",
        ("PL18", "NB4P"): "FlySky NB4+",
        ("PL18", "PL18EV"): "FlySky PL18EV",
    }
    for pcb, revs in pcb_map.items():
        if revs:
            for rev in revs:
                if (pcb, rev) in skip_combos:
                    continue
                pretty = labels.get((pcb, rev))
                disp_info = display_overrides.get((pcb, rev)) or display_map.get((pcb, rev)) or display_overrides.get((pcb, None)) or display_map.get((pcb, None))
                display_type, resolution = display_parts(disp_info)
                base = pretty if pretty else f"{pcb}/{rev}"
                label = base
                if base in ("XLITES/MT12", "XLITE/MT12"):
                    continue
                entries.append((label, pcb, rev, display_type, resolution))
        else:
            pretty = labels.get((pcb, None))
            disp_info = display_overrides.get((pcb, None)) or display_map.get((pcb, None))
            display_type, resolution = display_parts(disp_info)
            base = pretty if pretty else f"{pcb}"
            label = base
            entries.append((label, pcb, None, display_type, resolution))

    return entries


def entries_to_tsv(entries):
    lines = []
    for i, (label, pcb, rev, display_type, resolution) in enumerate(entries, 1):
        lines.append(f"{i}\t{label}\t{pcb}\t{rev or ''}\t{display_type}\t{resolution}\n")
    return "".join(lines)


def write_tsv(out_path, text, force=False):
    out_abs = os.path.abspath(out_path)
    if os.path.exists(out_abs) and not force:
        print(
            f"Refusing to overwrite existing file: {out_abs}\n"
            f"Re-run with --force to overwrite.",
            file=sys.stderr,
        )
        return 1
    out_dir = os.path.dirname(out_abs)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)
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
    parser = argparse.ArgumentParser(description="List available radio build options (PCB/PCBREV) parsed from CMake.")
    parser.add_argument("--select", type=int, help="Select an entry by number and print cmake flags.")
    parser.add_argument("--cmake", action="store_true", help="Print full cmake configure command.")
    parser.add_argument(
        "--export-tsv",
        nargs="?",
        const=GENERATED_TSV,
        help="Write entries to a TSV file (default: tools/radio-targets.generated.tsv).",
    )
    parser.add_argument("--force", action="store_true", help="Allow overwriting an existing output TSV.")
    parser.add_argument(
        "--check",
        nargs="?",
        const=CURATED_TSV,
        help="Compare generated output with an existing TSV (default: tools/radio-targets.tsv).",
    )
    args = parser.parse_args()

    pcb_map, display_map = parse_targets()
    if not pcb_map:
        print("No PCB options found. Are the target CMake files present?")
        return 1

    entries = build_entries(pcb_map, display_map)
    entries.sort(key=lambda e: e[0].lower())
    generated_tsv = entries_to_tsv(entries)

    if args.check:
        return check_tsv(args.check, generated_tsv)

    if args.export_tsv:
        return write_tsv(args.export_tsv, generated_tsv, force=args.force)

    if args.select is not None:
        idx = args.select - 1
        if idx < 0 or idx >= len(entries):
            print(f"Invalid selection: {args.select}")
            return 1
        _, pcb, rev, _, _ = entries[idx]
        if rev:
            flags = f"-DPCB={pcb} -DPCBREV={rev}"
        else:
            flags = f"-DPCB={pcb}"
        if args.cmake:
            print(f"cmake -S . -B build {flags}")
        else:
            print(flags)
        return 0

    print("Available radios:")
    for i, (label, _, _, _, _) in enumerate(entries, 1):
        print(f"{i:3d}. {label}")
    print()
    print("Select one:")
    print("  tools/radio-targets.py --select <number> --cmake")
    return 0


if __name__ == "__main__":
    sys.exit(main())
