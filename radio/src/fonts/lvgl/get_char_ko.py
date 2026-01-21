import sys

def is_ko_char(char):
    # Hangul Compatibility Jamo: 3130 ~ 318F
    if '\u3130' <= char <= '\u318F':
        return True
    # Hangul Jamo: 1100 ~ 11FF
    if '\u1100' <= char <= '\u11FF':
        return True
    # Hangul Syllables: AC00 ~ D7AF (가 ~ 힣)
    if '\uAC00' <= char <= '\uD7AF':
        return True
    return False

def extract_ko_characters(file_path):
    ko_characters = set()
    with open(file_path, mode='r', encoding='utf-8') as file:
        for line in file:
            for char in line:
                if is_ko_char(char):
                    ko_characters.add(ord(char))
    return sorted(ko_characters)

def format_unicode_ranges(code_points):
    """
    정렬된 유니코드 리스트를 range 형식으로 압축 (예: 0xAC00-0xAC10,0xAC15, ...)
    """
    if not code_points:
        return ""

    ranges = []
    start = prev = code_points[0]

    for cp in code_points[1:]:
        if cp == prev + 1:
            prev = cp
        else:
            if start == prev:
                ranges.append(f"0x{start:X}")
            else:
                ranges.append(f"0x{start:X}-0x{prev:X}")
            start = prev = cp

    if start == prev:
        ranges.append(f"0x{start:X}")
    else:
        ranges.append(f"0x{start:X}-0x{prev:X}")

    return ",".join(ranges)

if __name__ == "__main__":
    file_path = sys.argv[1]
    code_points = extract_ko_characters(file_path)
    formatted_output = format_unicode_ranges(code_points)
    print(formatted_output)
