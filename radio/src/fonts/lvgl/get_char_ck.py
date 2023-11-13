import sys

def is_ck_char(char):
  # CN/TW Symbols and Punctuation \u3000 - \u303f
  if '\u3000' <= char <= '\u303f':
    return True
  # CN/TW Unified Ideographs \u4e00 - \u9fff
  if '\u4e00' <= char <= '\u9fff':
    return True
  
  return False

def extract_ck_characters(file_path):
  ck_characters = set()
    
  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_ck_char(char):
          ck_characters.add(char)
    
  return ck_characters

def convert_to_unicode(ck_characters):
  unicode_list = []
    
  for char in ck_characters:
    unicode_list.append(hex(ord(char)))
    
  return unicode_list

def format_output(unicode_list):
  formatted_output = ','.join(sorted(unicode_list))
  return formatted_output


file_path = sys.argv[1]
ck_chars = extract_ck_characters(file_path)
unicode_list = convert_to_unicode(ck_chars)
formatted_output = format_output(unicode_list)

print(formatted_output)
