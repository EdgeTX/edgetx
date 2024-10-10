#!/usr/bin/env python3

import argparse
import glob
from os import path

def main(jsondir, template, placeholder, output):
    if not path.isdir(jsondir):
        raise Exception(jsondir + " is not a directory")

    try:
        tmplt = open(template, "r")
    except:
        raise Exception("Unable to open template file " + template)

    try:
        qrc = open(output, "w")
    except:
        raise Exception("Unable to open " + output + " for writing")

    line = tmplt.readline()

    while placeholder not in line and line != '':
        qrc.write(line)
        line = tmplt.readline()

    for f in sorted(glob.iglob(path.join(jsondir, '*.json'), recursive = False)):
        fname = path.basename(f)
        qrc.write('    <file alias="' + fname + '">' + f + '</file>' + '\n')

    line = tmplt.readline()

    while line != '':
        qrc.write(line)
        line = tmplt.readline()

    tmplt.close()
    qrc.close()


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Generate hardware definitions resource file')
    parser.add_argument('-d', metavar='json files directory', required=True)
    parser.add_argument('-t', metavar='template file', required=True)
    parser.add_argument('-p', metavar='placeholder', required=True)
    parser.add_argument('-o', metavar='qrc file', required=True)

    args = parser.parse_args()

    main(args.d, args.t, args.p, args.o)
