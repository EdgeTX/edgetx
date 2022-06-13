#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import codecs
import sys
from charset import special_chars, get_chars_encoding, special_chars_BW, get_chars_encoding_BW

def main():
    parser = argparse.ArgumentParser(description='Encoder for OpenTX translations')
    parser.add_argument('input', action="store", help="Input file name")
    parser.add_argument('output', action="store", help="Output file name")
    parser.add_argument('language', action="store", help="Two letter language identifier", default=None)
    parser.add_argument('bwlcd', action="store", help="Black White LCD", default="F")
    parser.add_argument("--reverse", help="Reversed char conversion (from number to char)", action="store_true")
    args = parser.parse_args()

    if args.bwlcd == "F":
      if args.language not in special_chars:
          parser.error(args.language + ' is not a supported language. Try one of the supported ones: %s' % list(special_chars.keys()))
          sys.exit()
    else:
      if args.language not in special_chars_BW:
          parser.error(args.language + ' is not a supported language. Try one of the supported ones: %s' % list(special_chars_BW.keys()))
          sys.exit()

    # if args.reverse:
    #     for translation in special_chars:
    #         translations[translation] = [(after, before) for (before, after) in translations[translation]]

    # Read the input file into a buffer
    in_file = codecs.open(args.input, "r", "utf-8")

    # Write the result to a temporary file
    out_file = codecs.open(args.output, 'w', 'utf-8')

    for line in in_file.readlines():
        # # Do the special chars replacements
        # if args.bwlcd == "F":        
        #     for before, after in get_chars_encoding(args.language).items():
        #         line = line.replace(before, after)
        # else:
        #     for before, after in get_chars_encoding_BW(args.language).items():
        #         line = line.replace(before, after)

        # if line.startswith("#define ZSTR_"):
        #     before = line[32:-2]
        #     after = ""
        #     for c in before:
        #         if ord('A') <= ord(c) <= ord('Z'):
        #             c = "\\%03o" % (ord(c) - ord('A') + 1)
        #         elif ord('a') <= ord(c) <= ord('z'):
        #             c = "\\%03o" % (-ord(c) + ord('a') + 255)
        #         elif ord('0') <= ord(c) <= ord('9'):
        #             c = "\\%03o" % (ord(c) - ord('0') + 27)
        #         after = after + c
        #     line = line[:32] + after + line[-2:]
        out_file.write(line)

    out_file.close()
    in_file.close()


if __name__ == "__main__":
    main()
