#!/usr/bin/env python

from __future__ import print_function

import sys
import glob


def addLine(filename, newline, after):
    print(filename, newline)
    with open(filename, 'r') as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        if after in line:
            lines.insert(i + 1, newline + '\n')
            break
    with open(filename, 'w') as f:
        f.writelines(lines)


def modifyTranslations(constant, translation, after):
    for filename in glob.glob('translations/*.h'):
        newline = "#define " + constant + " " * max(1, 31 - len(constant)) + '"' + translation + '"'
        addLine(filename, newline, after + " ")


def modifyDeclaration(constant, after):
    newline = "extern const char S" + constant + "[];"
    filename = "translations.h"
    addLine(filename, newline, after + "[];")


def modifyDefinition(constant, after):
    newline = "const char S" + constant + "[]  = " + constant + ";"
    filename = "translations.cpp"
    addLine(filename, newline, after + "[] ")


after = sys.argv[-1]
for arg in sys.argv[1:-1]:
    constant, translation = arg.split("=")
    modifyTranslations(constant, translation, after)
    modifyDeclaration(constant, after)
    modifyDefinition(constant, after)
    after = constant