import sys

def is_he_char(char):
  if '\u0590' <= char <= '\u05FF':
    return True
  if '\uFB1D' <= char <= '\uFB4F':
    return True
  
  return False

def extract_he_characters(file_path):
  he_characters = set()
    
  with open(file_path, mode='r', encoding='utf-8') as file:
    for line in file:
      for char in line:
        if is_he_char(char):
          he_characters.add(char)
    
  return he_characters

def convert_to_unicode(he_characters):
  unicode_list = []
    
  for char in he_characters:
    unicode_list.append(hex(ord(char)))
    
  return unicode_list

def format_output(unicode_list):
  formatted_output = ','.join(sorted(unicode_list))
  return formatted_output


file_path = sys.argv[1]
he_chars = extract_he_characters(file_path)
unicode_list = convert_to_unicode(he_chars)
formatted_output = format_output(unicode_list)

print(formatted_output)
