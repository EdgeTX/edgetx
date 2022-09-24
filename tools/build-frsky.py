#!/usr/bin/python3

import argparse
import datetime
import os
from builtins import NotADirectoryError
import shutil
import tempfile


boards = {
    "XLITE_FCC": {
        "PCB": "XLITE",
        "MODULE_SIZE_STD": "NO",
        "PPM": "NO",
        "DSM2": "NO",
        "SBUS": "NO",
    },
    "XLITE_LBT": {
        "PCB": "XLITE",
        "MODULE_PROTOCOL_D8": "NO",
        "MODULE_SIZE_STD": "NO",
        "PPM": "NO",
        "DSM2": "NO",
        "SBUS": "NO",
    },
    "XLITES": {
        "PCB": "XLITES",
        "AUTOUPDATE": "YES",
        "PXX1": "YES",
        "XJT": "NO",
        "MODULE_SIZE_STD": "NO",
        "PPM": "NO",
        "DSM2": "NO",
        "SBUS": "NO",
    },
    "X9LITE": {
        "PCB": "X9LITE",
        "AUTOUPDATE": "YES",
        "PXX1": "YES",
        "XJT": "NO",
        "MODULE_SIZE_STD": "NO",
        "PPM": "NO",
        "DSM2": "NO",
        "SBUS": "NO",
        "DEFAULT_MODE": "2",
    },
    "X9LITES": {
        "PCB": "X9LITES",
        "AUTOUPDATE": "YES",
        "PXX1": "YES",
        "XJT": "NO",
        "MODULE_SIZE_STD": "NO",
        "PPM": "NO",
        "DSM2": "NO",
        "SBUS": "NO",
        "DEFAULT_MODE": "2",
    },
    "X9D+2019": {
        "PCB": "X9D+",
        "PCBREV": "2019",
        "AUTOUPDATE": "YES",
        "PXX1": "YES",
        "DEFAULT_MODE": "2",
    },
    "X9D+": {
        "PCB": "X9D+",
        "DEFAULT_MODE": "2",
    },
    "X9E": {
        "PCB": "X9E",
        "DEFAULT_MODE": "2",
    },
    "X9EHall": {
        "PCB": "X9E",
        "STICKS": "HORUS",
        "DEFAULT_MODE": "2",
    },
    "X7_FCC": {
        "PCB": "X7",
        "DEFAULT_MODE": "2",
    },
    "X7_LBT": {
        "PCB": "X7",
        "MODULE_PROTOCOL_D8": "NO",
        "DEFAULT_MODE": "2",
    },
    "X7ACCESS": {
        "PCB": "X7",
        "PCBREV": "ACCESS",
        "AUTOUPDATE": "YES",
        "PXX1": "YES",
        "DEFAULT_MODE": "2",
    },
    "X10S": {
        "PCB": "X10",
        "DEFAULT_MODE": "2",
    },
    "X10SExpress": {
        "PCB": "X10",
        "PCBREV": "EXPRESS",
        "DEFAULT_MODE": "2",
    },
    "X12S": {
        "PCB": "X12S",
        "DEFAULT_MODE": "2",
    },
}

translations = [
    "EN",
    "CZ"
]

common_options = {
    "MULTIMODULE": "NO",
    "CROSSFIRE": "NO",
    "AFHDS3": "NO",
    "GVARS": "YES",
    "LUA": "NO_MODEL_SCRIPTS",
}


def timestamp():
    return datetime.datetime.now().strftime("%y%m%d")


def build(board, translation, srcdir):
    cmake_options = " ".join(["-D%s=%s" % (key, value) for key, value in list(boards[board].items()) + list(common_options.items())])
    cwd = os.getcwd()
    if not os.path.exists("output"):
        os.mkdir("output")
    path = tempfile.mkdtemp()
    os.chdir(path)
    command = "cmake %s -DTRANSLATIONS=%s -DFRSKY_RELEASE=YES -DDEFAULT_TEMPLATE_SETUP=17 %s" % (cmake_options, translation, srcdir)
    print(command)
    os.system(command)
    os.system("make firmware -j6")
    os.chdir(cwd)
    index = 0
    while 1:
        suffix = "" if index == 0 else "_%d" % index
        filename = "output/firmware_%s_%s_%s%s.bin" % (board.lower(), translation.lower(), timestamp(), suffix)
        if not os.path.exists(filename):
            shutil.copy("%s/arm-none-eabi/firmware.bin" % path, filename)
            break
        index += 1
    shutil.rmtree(path)


def dir_path(string):
    if os.path.isdir(string):
        return string
    else:
        raise NotADirectoryError(string)


def main():
    parser = argparse.ArgumentParser(description="Build FrSky firmware")
    parser.add_argument("-b", "--boards", action="append", help="Destination boards", required=True)
    parser.add_argument("-t", "--translations", action="append", help="Translations", required=True)
    parser.add_argument("srcdir", type=dir_path)

    args = parser.parse_args()

    for board in (boards.keys() if "ALL" in args.boards else args.boards):
        for translation in (translations if "ALL" in args.translations else args.translations):
            build(board, translation, args.srcdir)


if __name__ == "__main__":
    main()
