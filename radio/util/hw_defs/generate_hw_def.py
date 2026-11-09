import argparse

from generator import generate_from_template

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Generate code from hardware definitions')
    parser.add_argument('filename', metavar='filename', nargs='+')
    parser.add_argument('-t', metavar='template')
    parser.add_argument('-T', metavar='target')

    args = parser.parse_args()

    for filename in args.filename:
        generate_from_template(filename, args.t, args.T)
