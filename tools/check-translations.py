#!/usr/bin/env python3
"""
check-translations.py — EdgeTX translation completeness checker

Compares each language translation file against the English baseline and reports:
  • Strings defined in English but missing from the language file (would cause a
    compile error in the existing test_*.cpp tests — this is a sanity cross-check).
  • Strings defined in a language file but not in English (orphaned/obsolete).
  • Strings whose translated value is identical to English (likely untranslated).
  • Strings in string_list.h that are not in en.h (referenced but not defined in English).
  • Strings in en.h that are not in string_list.h (defined but never compiled in).

Usage:
    python3 tools/check-translations.py [--lang fr] [--show-identical] [--summary-only]

Run from the repository root.  Exit code is non-zero when any language has
missing strings or when --strict is given and untranslated strings are found.
"""

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
I18N_DIR  = REPO_ROOT / "radio" / "src" / "translations" / "i18n"
EN_FILE   = I18N_DIR / "en.h"
LIST_FILE = REPO_ROOT / "radio" / "src" / "translations" / "string_list.h"

# Map of language code → file (built from what's actually present)
LANG_FILES = {p.stem: p for p in I18N_DIR.glob("*.h") if p.stem != "en"}

# Preprocessor guards that can appear around #define TR_ lines — ignore them.
_GUARD_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|else|elif|endif|pragma|include)")

def parse_defines(path: Path) -> dict[str, str]:
    """Return {KEY: raw_value} for every '#define TR_KEY ...' in *path*."""
    defines: dict[str, str] = {}
    # A define can be continued on the next line with '\'; handle multi-line.
    text = path.read_text(encoding="utf-8", errors="replace")
    # Join continuation lines
    text = re.sub(r"\\\n", " ", text)
    for line in text.splitlines():
        if _GUARD_RE.match(line):
            continue
        m = re.match(r"\s*#\s*define\s+(TR_\w+)\s+(.*)", line)
        if m:
            key = m.group(1).removeprefix("TR_")
            val = m.group(2).strip()
            defines[key] = val
    return defines


def parse_string_list(path: Path) -> set[str]:
    """Return the set of keys referenced by STR(x) / STRARRAY(x) in string_list.h."""
    keys: set[str] = set()
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"\s*STR(?:ARRAY)?\((\w+)\)", line)
        if m:
            keys.add(m.group(1))
    return keys


def check_language(lang: str, lang_file: Path, en: dict[str, str],
                   listed: set[str], args) -> int:
    """Check one language file.  Returns number of missing strings."""
    lang_defs = parse_defines(lang_file)

    missing   = sorted(k for k in en if k not in lang_defs)
    orphaned  = sorted(k for k in lang_defs if k not in en)
    identical = sorted(k for k in lang_defs if k in en and lang_defs[k] == en[k])

    errors = len(missing)

    if args.summary_only:
        total    = len(en)
        translated = len([k for k in lang_defs if k in en and lang_defs[k] != en[k]])
        pct = 100 * translated // total if total else 0
        status = "OK " if not missing else "ERR"
        print(f"[{status}] {lang:4s}  {translated:4d}/{total} translated ({pct:3d}%)  "
              f"missing={len(missing):3d}  identical={len(identical):3d}  orphaned={len(orphaned):3d}")
        return errors

    # --- detailed report ---
    print(f"\n{'='*60}")
    print(f"Language: {lang}  ({lang_file.name})")
    print(f"{'='*60}")
    total = len(en)
    translated = len([k for k in lang_defs if k in en and lang_defs[k] != en[k]])
    print(f"  English baseline : {total} strings")
    print(f"  Language defines : {len(lang_defs)}")
    print(f"  Translated       : {translated}  ({100*translated//total if total else 0}%)")
    print(f"  Identical to EN  : {len(identical)}")
    print(f"  Missing          : {len(missing)}")
    print(f"  Orphaned         : {len(orphaned)}")

    if missing:
        print(f"\n  MISSING strings (defined in English, absent from {lang}):")
        for k in missing:
            print(f"    TR_{k}")

    if orphaned:
        print(f"\n  ORPHANED strings (in {lang} but not in English):")
        for k in orphaned:
            print(f"    TR_{k}")

    if identical and args.show_identical:
        print(f"\n  IDENTICAL-TO-ENGLISH strings (possibly untranslated):")
        for k in identical:
            print(f"    TR_{k}  =  {en[k][:60]}")

    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--lang", metavar="CODE",
                        help="Check only this language (e.g. fr, de). Default: all.")
    parser.add_argument("--show-identical", action="store_true",
                        help="List strings that are identical to English.")
    parser.add_argument("--summary-only", action="store_true",
                        help="Print a one-line summary per language.")
    parser.add_argument("--strict", action="store_true",
                        help="Exit non-zero if any identical-to-English strings exist.")
    args = parser.parse_args()

    if not EN_FILE.exists():
        print(f"ERROR: English file not found: {EN_FILE}", file=sys.stderr)
        return 2
    if not LIST_FILE.exists():
        print(f"ERROR: string_list.h not found: {LIST_FILE}", file=sys.stderr)
        return 2

    en      = parse_defines(EN_FILE)
    listed  = parse_string_list(LIST_FILE)

    # Cross-check en.h vs string_list.h
    not_listed = sorted(k for k in en if k not in listed)
    not_in_en  = sorted(k for k in listed if k not in en)

    if not args.summary_only:
        if not_listed:
            print(f"INFO: {len(not_listed)} string(s) in en.h but NOT in string_list.h "
                  f"(not compiled into firmware):")
            for k in not_listed[:20]:
                print(f"  TR_{k}")
            if len(not_listed) > 20:
                print(f"  ... and {len(not_listed)-20} more")
            print()
        if not_in_en:
            print(f"WARNING: {len(not_in_en)} string(s) in string_list.h but NOT in en.h:")
            for k in not_in_en:
                print(f"  TR_{k}")
            print()

    # Select languages to check
    if args.lang:
        targets = {args.lang: LANG_FILES.get(args.lang)}
        if targets[args.lang] is None:
            print(f"ERROR: no translation file found for '{args.lang}'", file=sys.stderr)
            print(f"Available: {', '.join(sorted(LANG_FILES))}", file=sys.stderr)
            return 2
    else:
        targets = dict(sorted(LANG_FILES.items()))

    total_missing = 0
    total_identical = 0
    for lang, fpath in targets.items():
        lang_defs = parse_defines(fpath)
        total_identical += len([k for k in lang_defs if k in en and lang_defs[k] == en[k]])
        total_missing += check_language(lang, fpath, en, listed, args)

    if not args.summary_only:
        print()
        if total_missing:
            print(f"RESULT: {total_missing} missing string(s) across checked languages.")
        else:
            print("RESULT: All checked languages define every English string.")

    rc = 0
    if total_missing:
        rc = 1
    if args.strict and total_identical:
        rc = max(rc, 1)
    return rc


if __name__ == "__main__":
    sys.exit(main())
