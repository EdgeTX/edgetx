import sys

# Arabic (U+0600-06FF) characters used in the translation file are
# included as-is. LVGL's Arabic shaping (LV_USE_ARABIC_PERSIAN_CHARS)
# replaces letters with their positional forms from Arabic Presentation
# Forms-B at runtime, and those never appear in the source text, so the
# whole Presentation Forms-B block is always added (U+FE75 is unassigned).
PRESENTATION_FORMS_B = "0xfe70-0xfe74,0xfe76-0xfefc"


def is_ar_char(char):
  return '؀' <= char <= 'ۿ'


def extract_ar_characters(file_path):
  ar_characters = set()

  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_ar_char(char):
          ar_characters.add(char)

  return ar_characters


file_path = sys.argv[1]
ar_chars = extract_ar_characters(file_path)
unicode_list = sorted(hex(ord(c)) for c in ar_chars)
unicode_list.append(PRESENTATION_FORMS_B)
print(','.join(unicode_list))
