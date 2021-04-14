#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os
import sys
from PIL import Image, ImageDraw, ImageFont
from charset import get_chars, special_chars, extra_chars, standard_chars, is_cjk_char


class FontBitmap:
    def __init__(self, language, font_size, font_name, foreground, background):
        self.language = language
        self.chars = get_chars(language)
        self.font_size = font_size
        self.foreground = foreground
        self.background = background
        self.font = self.load_font(font_name)
        self.extra_bitmap = self.load_extra_bitmap()

    def load_extra_bitmap(self):
        try:
            tools_path = os.path.dirname(os.path.realpath(__file__))
            path = os.path.join(tools_path, "../radio/src/fonts", "extra_%dpx.png" % self.font_size)
            extra_image = Image.open(path)
            return extra_image.convert('RGB')
        except IOError:
            return None

    def load_font(self, font_name):
        # print(font_name)
        for ext in (".ttf", ".otf"):
            tools_path = os.path.dirname(os.path.realpath(__file__))
            path = os.path.join(tools_path, "../radio/src/fonts", font_name + ext)
            if os.path.exists(path):
                return ImageFont.truetype(path, self.font_size)
        print("Font file %s not found" % font_name)

    @staticmethod
    def is_row_needed(px, y, width):
        for x in range(width):
            if px[x, y] != (255, 255, 255):
                return True
        return False

    @staticmethod
    def is_column_needed(px, x, height, debug=False):
        for y in range(height):
            if debug:
                print(x, y, px[x, y])
            if px[x, y] != (255, 255, 255):
                return True
        return False

    def get_real_size(self, image, debug=False):
        px = image.load()
        left = 0
        while left < image.width:
            if self.is_column_needed(px, left, image.height, debug):
                break
            left += 1
        right = image.width - 1
        while right > left:
            if self.is_column_needed(px, right, image.height, debug):
                break
            right -= 1
        top = 0
        while top < image.height:
            if self.is_row_needed(px, top, image.width):
                break
            top += 1
        bottom = image.height - 1
        while bottom > top:
            if self.is_row_needed(px, bottom, image.width):
                break
            bottom -= 1
        return left, top, right, bottom

    def draw_char(self, image, x, c, offset_y=0):
        font=self.font
        size = font.font.getsize(c)
        width = size[0][0]
        offset_x = size[1][0]
        char_image = Image.new("RGB", (width + 10, image.height), self.background)
        draw = ImageDraw.Draw(char_image)
        draw.text((-offset_x, offset_y), c, fill=self.foreground, font=font)
        if image:
            image.paste(char_image.crop((0, 0, width, image.height)), (x, 0))
        return width

    def generate(self, filename, generate_coords_file=True):
        coords = []
        image = Image.new("RGB", (len(self.chars) * self.font_size + 200, self.font_size + 20), self.background)

        width = 0
        for c in self.chars:
            if c == " ":
                w = 4
            elif c in extra_chars:
                if self.extra_bitmap:
                    for i in range(128 - 32 - len(standard_chars)):
                        coords.append(width)
                    image.paste(self.extra_bitmap.copy(), (width, 0))
                    for coord in [14, 14, 12, 12, 13, 13, 13, 13, 13] + [15] * 12:
                        coords.append(width)
                        width += coord
                    self.extra_bitmap = None
                continue
            elif is_cjk_char(c):
                w = self.draw_char(image, width, c, -3)
            else:
                w = self.draw_char(image, width, c)

            coords.append(width)
            width += w

        coords.append(width)

        _, top, _, bottom = self.get_real_size(image)

        top = 1
        # bottom = self.font_size
        image = image.crop((0, top, width - 1, bottom))
        coords.insert(0, bottom - top + 1)

        image.save(filename + ".png")
        if generate_coords_file:
            with open(filename + ".specs", "w") as f:
                f.write(",".join(str(tmp) for tmp in coords))


def main():
    if sys.version_info < (3, 0, 0):
        print("%s requires Python 3. Terminating." % __file__)
        sys.exit(1)

    parser = argparse.ArgumentParser(description="Builder for OpenTX font files")
    parser.add_argument('--output', help="Output file name")
    parser.add_argument('--subset', help="Subset")
    parser.add_argument('--size', type=int, help="Font size")
    parser.add_argument('--font', help="Font name")
    args = parser.parse_args()

    font = FontBitmap(args.subset, args.size, args.font, (0, 0, 0), (255, 255, 255))
    font.generate(args.output)


if __name__ == "__main__":
    main()
