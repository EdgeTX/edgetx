#!/usr/bin/env python
# -*- coding: utf-8 -*-

# used ? Δ~\n\t

import os

standard_chars = """ !"#$%&'()*+,-./0123456789:;<=>?°ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz~|≥"""

extra_chars = "".join([chr(0x10000+i) for i in range(21)])

def is_cjk_char(c):
    return 0x4E00 <= ord(c) <= 0x9FFF

def cjk_chars(lang):
    charset = set()
    tools_path = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(tools_path, "../radio/src/translations/%s.h.txt" % lang), encoding='utf-8') as f:
        data = f.read()
        for c in data:
            if is_cjk_char(c):
                charset.add(c)
                # print(ord(c))
    result = list(charset)
    result.sort()
    return result


special_chars = {
    "en": "",
    "fr": "éèàîç",
    "de": "ÄäÖöÜüß",
    "cz": "ěščřžýáíéňóůúďťĚŠČŘŽÝÁÍÉŇÓÚŮĎŤ",
    "nl": "",
    "es": "Ññ",
    "fi": "åäöÅÄÖ",
    "it": "àù",
    "pl": "ąćęłńóśżźĄĆĘŁŃÓŚŻŹ",
    "pt": "ÁáÂâÃãÀàÇçÉéÊêÍíÓóÔôÕõÚú",
    "se": "åäöÅÄÖ",
    "cn": "".join(cjk_chars("cn")),
    "tw": "".join(cjk_chars("tw")),
}

subset_lowercase = {
    "Č": "č",
    "Ě": "ě",
    "Š": "š",
    "Ú": "ú",
    "Ů": "ů",
    "Ž": "ž"
}

# print("CN charset: %d symbols" % len(special_chars["cn"]))


def get_chars(subset):
    result = standard_chars + extra_chars
    if False: # subset == "all":
        for key, chars in special_chars.items():
            result += "".join([char for char in chars if char not in result])
    else:
        if subset in special_chars:
            result += "".join([char for char in special_chars[subset] if char not in subset_lowercase])
    return result


def get_chars_encoding(subset):
    result = {}
    if subset in ("cn", "tw"):
        chars = get_chars(subset)
        for char in chars:
            if char in special_chars[subset]:
                index = special_chars[subset].index(char) + 1
                if index >= 0x100:
                    index += 1
                result[char] = "\\%03o\\%03o" % (0xFE + ((index >> 8) & 0x01), index & 0xFF)
            elif char not in standard_chars and char not in extra_chars:
                result[char] = "\\%03o" % (0xC0 + chars.index(char) - len(standard_chars))
    else:
        offset = 128 - len(standard_chars)
        chars = get_chars(subset)
        for char in chars:
            if char not in standard_chars:
                result[char] = "\\%03o" % (offset + chars.index(char))
        for upper, lower in subset_lowercase.items():
            if lower in result:
                result[upper] = result[lower]
    return result
