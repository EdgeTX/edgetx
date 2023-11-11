import sys

def is_ru_char(char):
  if '\u0400' <= char <= '\u04FF':
    return True
  
  return False

def extract_ru_characters(file_path):
  ru_characters = set()
    
  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_ru_char(char):
          ru_characters.add(char)
    
  return ru_characters

def convert_to_unicode(ru_characters):
  unicode_list = []
    
  for char in ru_characters:
    unicode_list.append(hex(ord(char)))
    
  return unicode_list

def format_output(unicode_list):
  formatted_output = ','.join(sorted(unicode_list))
  return formatted_output


file_path = sys.argv[1]
ru_chars = extract_ru_characters(file_path)
unicode_list = convert_to_unicode(ru_chars)
formatted_output = format_output(unicode_list)

print(formatted_output)
