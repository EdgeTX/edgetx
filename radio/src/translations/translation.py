#!/usr/bin/env python

import os
import glob
import argparse


TRANSLATIONS_PATH = os.path.dirname(os.path.realpath(__file__))
SRC_PATH          = os.path.dirname(TRANSLATIONS_PATH)

def add_line(filename, newline, args):
    print(filename, newline)
    with open(filename, encoding='utf-8', mode='r') as f:
        lines = f.readlines()
    newline += "\n"
    if args.before:
        for i, line in enumerate(lines):
            if args.before in line:
                lines.insert(i - 1, newline)
                break
    elif args.after:
        for i, line in enumerate(lines):
            if args.after in line:
                lines.insert(i + 1, newline)
                break
    else:
        lines.append(newline)
    with open(filename, encoding='utf-8', mode='w') as f:
        f.writelines(lines)


def modify_translations(args):
    for filename in glob.glob(TRANSLATIONS_PATH + os.sep + "*.h"):
        if os.path.basename(filename) == "untranslated.h":
            continue
        newline = '#define TR_%s%s"%s"' % (args.name, " " * max(1, 28 - len(args.name)), args.value)
        add_line(filename, newline, args)


def modify_declaration(args):
    newline = 'extern const char STR_%s[];' % args.name
    filename = SRC_PATH + os.sep+ "translations.h"
    add_line(filename, newline, args)


def modify_definition(args):
    newline = 'const char STR_%s[] = TR_%s;' % (args.name, args.name)
    filename = SRC_PATH + os.sep + "translations.cpp"
    add_line(filename, newline, args)


def main():
    parser = argparse.ArgumentParser(description="Helper to manage translations")
    parser.add_argument("--after", required=False, help="String just before the new one")
    parser.add_argument("--before", required=False, help="String just after the new one")
    parser.add_argument("--name", required=True, help="String name")
    parser.add_argument("value", help="String value")
    args = parser.parse_args()

    modify_translations(args)
    modify_declaration(args)
    modify_definition(args)


if __name__ == "__main__":
    main()
