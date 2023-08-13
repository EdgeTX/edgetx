
#from os import path

import argparse

from hal_json import parse_defines
from generator import generate_from_template

# def eprint(*args, **kwargs):
#     print(*args, file=sys.stderr, **kwargs)

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Process hardware definitions')
    parser.add_argument('filename', metavar='filename', nargs='+')
    parser.add_argument('-i', metavar='input', choices=['json','defines'], default='json')
    parser.add_argument('-t', metavar='template')
    parser.add_argument('-T', metavar='target')

    args = parser.parse_args()

    if args.i == 'defines':
        for filename in args.filename:
            parse_defines(filename, args.T)

    elif args.i == 'json':
        for filename in args.filename:
            generate_from_template(filename, args.t, args.T)
