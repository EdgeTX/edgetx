#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os
import sys
from PIL import Image, ImageDraw, ImageFont
from charset import get_chars, special_chars, extra_chars, standard_chars

EXTRA_BITMAP_MAX_WIDTH = 297

class FontBitmap:
    def __init__(self, language, font_size, font_name, foreground, background):
        self.language = language
        self.chars = get_chars(language)
        self.font_size = font_size
        self.foreground = foreground
        self.background = background
        self.font = self.load_font(font_name)
        self.extra_bitmap = self.load_extra_bitmap()
        self.extra_bitmap_added = False
        self.extra_bitmap_width = EXTRA_BITMAP_MAX_WIDTH
        if self.extra_bitmap is not None:
            self.extra_bitmap_width = self.extra_bitmap.width

    def load_extra_bitmap(self):
        try:
            tools_path = os.path.dirname(os.path.realpath(__file__))
            filename = "extra_%dpx.png" % self.font_size
            path = os.path.join(tools_path, "../radio/src/fonts", filename)
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

    def get_text_dimensions(self, text_string):
        # https://stackoverflow.com/a/46220683/9263761
        _, descent = self.font.getmetrics()
        text_width = self.font.getmask(text_string).getbbox()[2]
        text_height = self.font.getmask(text_string).getbbox()[3] + descent

        return (text_width, text_height)

    def draw_char(self, draw, x, y, c):

        # default width for space
        width = 4
        if c != ' ':
            if "0123456789".find(c) >= 0 :
              width = self.font.getmask("4").getbbox()[2]
              _, (offset_x, offset_y) = self.font.font.getsize(c)
              _, (offset_x2, offset_y2) = self.font.font.getsize("4")
              offset_x = (offset_x + offset_x2) / 2
            else :
              width = self.font.getmask(c).getbbox()[2]
              _, (offset_x, offset_y) = self.font.font.getsize(c)

            draw.text((x - offset_x, y), c, fill=self.foreground, font=self.font)

        return width

    def generate(self, filename, generate_coords_file=True):
        coords = []

        (_,baseline), (offset_x, offset_y) = self.font.font.getsize(self.chars)
        (text_width, text_height) = self.get_text_dimensions(self.chars)
        y_shifts = 0
        if offset_y < 0:
            y_shifts = -offset_y
            text_height += y_shifts
            offset_y = 0

        img_width = text_width + self.extra_bitmap_width
        image = Image.new("RGB", (img_width, text_height), self.background)
        draw = ImageDraw.Draw(image)

        width = 0
        for c in self.chars:
            if c in extra_chars:
                if not self.extra_bitmap_added:
                    # append same width for non-existing characters
                    for i in range(128 - 32 - len(standard_chars)):
                        coords.append(width)

                    if self.extra_bitmap:

                        # copy extra_bitmap at once
                        image.paste(self.extra_bitmap, (width, offset_y + y_shifts))

                        # append width for extra_bitmap symbols
                        for coord in [14, 14, 12, 12, 13, 13, 13, 13, 13] + [15] * 12:
                            coords.append(width)
                            width += coord
                    else:
                        for coord in range(21):
                            coords.append(width)                    

                    # once inserted, disable insert again
                    self.extra_bitmap_added = True

                # skip
                continue

            # normal characters + CJK
            w = self.draw_char(draw, width, y_shifts, c)

            coords.append(width)
            width += w

        # append the last computed width
        coords.append(width)

        # trim image to correct size
        image = image.crop((0, offset_y, width, baseline + offset_y))

        # insert the size of the image
        coords.insert(0, image.height)

        # convert to 8-bit and save
        image = image.convert("L")
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
