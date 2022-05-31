#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import codecs
import sys
from charset import special_chars, get_chars_encoding, special_chars_BW, get_chars_encoding_BW

all_languages = special_chars.keys()

def main():

    languages = ""
    char_list = []

    lang_args = all_languages
    if len(sys.argv) > 1:
        lang_args = sys.argv[1:]
        
    for lang in lang_args:
        if lang not in special_chars:
            print(lang + ' is not a supported language. Try one of the supported ones: %s' % list(special_chars.keys()))
            sys.exit()

        languages += ' ' + lang
        char_list.extend(c for c in get_chars_encoding(lang).keys())


    char_list = sorted(set(char_list))
    chars = ""
    for c in char_list:
        if ord(c) < 0x10000 or ord(c) > 0x10014:
            chars = chars + hex(ord(c)) + ','

    print(f"{languages}: {chars}")
        
if __name__ == "__main__":
    main()
