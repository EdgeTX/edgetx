import sys

def is_cyrillic_char(char):
  if '\u0400' <= char <= '\u04FF':
    return True
  
  return False

def extract_cyrillic_characters(file_path):
  cyrillic_characters = set()
    
  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_cyrillic_char(char):
          cyrillic_characters.add(char)
    
  return cyrillic_characters

def convert_to_unicode(cyrillic_characters):
  unicode_list = []
    
  for char in cyrillic_characters:
    unicode_list.append(hex(ord(char)))
    
  return unicode_list

def format_output(unicode_list):
  formatted_output = ','.join(sorted(unicode_list))
  return formatted_output


file_path = sys.argv[1]
cyrillic_chars = extract_cyrillic_characters(file_path)
unicode_list = convert_to_unicode(cyrillic_chars)
formatted_output = format_output(unicode_list)

print(formatted_output)
