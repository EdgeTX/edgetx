#!/usr/bin/env python

import os
import re
import sys
import glob
import argparse


TRANSLATIONS_PATH = os.path.dirname(os.path.realpath(__file__))
I18N_PATH         = os.path.join(TRANSLATIONS_PATH, "i18n")
STRING_LIST       = os.path.join(TRANSLATIONS_PATH, "string_list.h")


def find_line(lines, pattern):
    for i, line in enumerate(lines):
        if re.match(pattern, line):
            return i
    return None


def add_line(filename, newline, pattern, args):
    print(filename, newline)
    with open(filename, encoding='utf-8', mode='r') as f:
        lines = f.readlines()
    newline += "\n"
    if args.before or args.after:
        i = find_line(lines, pattern)
        if not args.before:
            # Skip the continuation lines of a multi-line define
            while i + 1 < len(lines) and lines[i].rstrip().endswith("\\"):
                i += 1
            i += 1
        lines.insert(i, newline)
    else:
        lines.append(newline)
    with open(filename, encoding='utf-8', mode='w') as f:
        f.writelines(lines)


def translation_pattern(name):
    return r'\s*#\s*define\s+TR_%s\b' % re.escape(name)


def string_list_pattern(name):
    return r'\s*STR(ARRAY)?\(%s\)' % re.escape(name)


def check_position(filenames, pattern):
    # Check every file first, so that nothing is modified if one of them
    # does not contain the reference string.
    for filename in filenames:
        with open(filename, encoding='utf-8', mode='r') as f:
            if find_line(f.readlines(), pattern) is None:
                sys.exit("Error: reference string not found in %s" % filename)


def main():
    parser = argparse.ArgumentParser(description="Helper to manage translations")
    parser.add_argument("--after", required=False, help="String just before the new one")
    parser.add_argument("--before", required=False, help="String just after the new one")
    parser.add_argument("--name", required=True, help="String name")
    parser.add_argument("value", help="String value")
    args = parser.parse_args()

    languages = sorted(glob.glob(os.path.join(I18N_PATH, "*.h")))
    ref = args.before or args.after
    if ref:
        check_position(languages, translation_pattern(ref))
        check_position([STRING_LIST], string_list_pattern(ref))

    newline = '#define TR_%s%s"%s"' % (args.name, " " * max(1, 28 - len(args.name)), args.value)
    for filename in languages:
        add_line(filename, newline, translation_pattern(ref) if ref else None, args)
    add_line(STRING_LIST, 'STR(%s)' % args.name, string_list_pattern(ref) if ref else None, args)

    print("Now regenerate sim_string_list.h with 'just gen-simstr'")


if __name__ == "__main__":
    main()
