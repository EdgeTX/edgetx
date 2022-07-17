#!/usr/bin/python3

import argparse
import datetime
import os
from builtins import NotADirectoryError
import shutil
import tempfile


boards = {
    "TX16S_1": {
        "PCB": "X10",
        "PCBREV": "TX16S",
        "DEFAULT_MODE": "1",
    },
    "TX16S_2": {
        "PCB": "X10",
        "PCBREV": "TX16S",
        "DEFAULT_MODE": "2",
    },
    "TX12_1": {
        "PCB": "X7",
        "PCBREV": "TX12",
        "DEFAULT_MODE": "1",
    },
    "TX12_2": {
        "PCB": "X7",
        "PCBREV": "TX12",
        "DEFAULT_MODE": "2",
    },
    "TX12MK2_1": {
        "PCB": "X7",
        "PCBREV": "TX12MK2",
        "DEFAULT_MODE": "1",
    },
    "TX12MK2_2": {
        "PCB": "X7",
        "PCBREV": "TX12MK2",
        "DEFAULT_MODE": "2",
    },
    "ZORRO_1": {
        "PCB": "X7",
        "PCBREV": "ZORRO",
        "DEFAULT_MODE": "1",
    },
    "ZORRO_2": {
        "PCB": "X7",
        "PCBREV": "ZORRO",
        "DEFAULT_MODE": "2",
    },
    "T8_1": {
        "PCB": "X7",
        "PCBREV": "T8",
        "DEFAULT_MODE": "1",
        "RADIOMASTER_RTF_RELEASE": "YES",
    },
    "T8_2": {
        "PCB": "X7",
        "PCBREV": "T8",
        "DEFAULT_MODE": "2",
        "RADIOMASTER_RTF_RELEASE": "YES",
    }
}

translations = [
    "EN",
]


def timestamp():
    return datetime.datetime.now().strftime("%y%m%d")


def build(board, translation, srcdir):
    cmake_options = " ".join(["-D%s=%s" % (key, value) for key, value in boards[board].items()])
    cwd = os.getcwd()
    if not os.path.exists("output"):
        os.mkdir("output")
    path = tempfile.mkdtemp()
    os.chdir(path)
    command = "cmake %s -DTRANSLATIONS=%s -DRADIOMASTER_RELEASE=YES %s" % (cmake_options, translation, srcdir)
    print(command)
    os.system(command)
    os.system("make firmware -j6")
    os.chdir(cwd)
    index = 0
    while 1:
        suffix = "" if index == 0 else "_%d" % index
        filename = "output/firmware_%s_%s_%s%s.bin" % (board.lower(), translation.lower(), timestamp(), suffix)
        if not os.path.exists(filename):
            shutil.copy("%s/firmware.bin" % path, filename)
            break
        index += 1
    shutil.rmtree(path)


def dir_path(string):
    if os.path.isdir(string):
        return string
    else:
        raise NotADirectoryError(string)


def main():
    parser = argparse.ArgumentParser(description="Build Radiomaster firmware")
    parser.add_argument("-b", "--boards", action="append", help="Destination boards", required=True)
    parser.add_argument("-t", "--translations", action="append", help="Translations", required=True)
    parser.add_argument("srcdir", type=dir_path)

    args = parser.parse_args()

    for board in (boards.keys() if "ALL" in args.boards else args.boards):
        for translation in (translations if "ALL" in args.translations else args.translations):
            build(board, translation, args.srcdir)


if __name__ == "__main__":
    main()
