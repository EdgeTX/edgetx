import sys

def is_jp_char(char):
  # JP Symbols and Punctuation \u3000 - \u303f
  if '\u3000' <= char <= '\u303f':
    return True
  # JP Unified Ideographs \u4e00 - \u9fff
  if '\u4e00' <= char <= '\u9fff':
    return True
  # Hiragana
  if '\u3040' <= char <= '\u309f':
    return True
  # Katakana
  if '\u30a0' <= char <= '\u30ff':
    return True

  return False

def extract_jp_characters(file_path):
  jp_characters = set()
    
  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_jp_char(char):
          jp_characters.add(char)
    
  return jp_characters

def convert_to_unicode(jp_characters):
  unicode_list = []
    
  for char in jp_characters:
    unicode_list.append(hex(ord(char)))
    
  return unicode_list

def format_output(unicode_list):
  formatted_output = ','.join(sorted(unicode_list))
  return formatted_output


file_path = sys.argv[1]
jp_chars = extract_jp_characters(file_path)
unicode_list = convert_to_unicode(jp_chars)
formatted_output = format_output(unicode_list)

print(formatted_output)
