def extract_chinese_characters(file_path):
    chinese_characters = set()
    
    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            for char in line:
                if '\u4e00' <= char <= '\u9fff':
                    chinese_characters.add(char)
    
    return chinese_characters


def convert_to_unicode(chinese_characters):
    unicode_list = []
    
    for char in chinese_characters:
        unicode_list.append(hex(ord(char)))
    
    return unicode_list


def format_output(unicode_list):
    formatted_output = ','.join(sorted(unicode_list))
    return formatted_output


file_path = '../../translations/tw.h'  # Replace with the actual file path
chinese_chars = extract_chinese_characters(file_path)
unicode_list = convert_to_unicode(chinese_chars)
formatted_output = format_output(unicode_list)

formatted_output = '0x3001,' + formatted_output  # Add 0x3001

print(formatted_output)
