#!/usr/bin/env python
# -*- coding: utf-8 -*-

# used ? Δ~\n\t

import os

standard_chars = """ !"#$%&'()*+,-./0123456789:;<=>?°ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz~|≥"""

extra_chars = "".join([chr(0x10000+i) for i in range(21)])

def is_special_char(c):
    # only 'our' special chars and CJK Unified Ideographs
    return 192 <= ord(c) <= 383 or 0x4E00 <= ord(c) <= 0x9FFF

def get_special_chars():
  result = {}
  for lang in["cn", "cz", "da", "de", "en", "es", "fi", "fr", "he", "it", "jp", "nl", "pl", "pt", "ru", "se", "tw"]:
    charset = set()
    tools_path = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(tools_path, "../radio/src/translations/%s.h" % lang), encoding='utf-8') as f:
        data = f.read()
        for c in data:
            if is_special_char(c):
                charset.add(c)
    data = list(charset)
    data.sort()
    result[lang] = data

  return result

special_chars = get_special_chars()

def get_chars(subset):
    result = standard_chars + extra_chars
    result += "".join([char for char in special_chars[subset]])
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
    return result

special_chars_BW = {
    "en": "",
    "fr": "éèàîç",
    "da": "åæøÅÆØ",
    "de": "ÄäÖöÜüß",
    "cz": "áčéěíóřšúůýÁÍŘÝžÉ",
    "nl": "",
    "es": "ÑñÁáÉéÍíÓóÚú",
    "fi": "åäöÅÄÖ",
    "it": "àù",
    "pl": "ąćęłńóśżźĄĆĘŁŃÓŚŻŹ",
    "pt": "ÁáÂâÃãÀàÇçÉéÊêÍíÓóÔôÕõÚú",
    "ru": "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя",
    "se": "åäöÅÄÖ",
    "cn": "",
    "tw": "",
}

subset_lowercase_BW = {
    "Č": "č",
    "Ě": "ě",
    "Š": "š",
    "Ú": "ú",
    "Ů": "ů",
    "Ž": "ž"
}

def get_chars_BW(subset):
    result = standard_chars + extra_chars
    if subset in special_chars_BW:
        if (subset == "cz"):
            result += "".join([char for char in special_chars_BW[subset] if char not in subset_lowercase_BW])
        else:
            result += "".join([char for char in special_chars_BW[subset]])
    return result

def get_chars_encoding_BW(subset):
    result = {}
    offset = 128 - len(standard_chars)
    chars = get_chars_BW(subset)
    for char in chars:
        if char not in standard_chars:
            result[char] = "\\%03o" % (offset + chars.index(char))
    if (subset == "cz"):            
        for upper, lower in subset_lowercase_BW.items():
            if lower in result:
                result[upper] = result[lower]
    return result
