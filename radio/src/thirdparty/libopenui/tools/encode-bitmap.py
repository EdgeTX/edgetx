#!/usr/bin/env python

import argparse
from PIL import Image

# LZ4 compression
import lz4.block

class RawMixin:
    def encode_byte(self, byte):
        self.write(byte)

    def encode_end(self):
        pass

class RleMixin:
    RLE_BYTE = 0
    RLE_SEQ = 1

    def __init__(self):
        self.state = self.RLE_BYTE
        self.count = 0
        self.prev_byte = None

    def eq_prev_byte(self, byte):
        if self.prev_byte is None:
            return False
        return byte == self.prev_byte

    def encode_byte(self, byte):
        if self.state == self.RLE_BYTE:
            self.write(byte)
            if self.eq_prev_byte(byte):
                self.state = self.RLE_SEQ
                self.count = 0
            else:
                self.prev_byte = byte
        elif self.state == self.RLE_SEQ:
            if self.eq_prev_byte(byte):
                self.count += 1
                if self.count == 255:
                    self.write(self.count)
                    self.prev_byte = None
                    self.state = self.RLE_BYTE
            else:
                self.write(self.count)
                self.write(byte)
                self.prev_byte = byte
                self.state = self.RLE_BYTE

    def encode_end(self):
        if self.state == self.RLE_SEQ:
            self.write(self.count)

class Lz4Mixin:

    def __init__(self):
        self.buffer = []

    def encode_byte(self,byte):
        self.buffer.append(byte)

    def encode_end(self):
        compressed_data = lz4.block.compress(bytes(self.buffer), compression=12, mode='high_compression', store_size=False)
        data_len = len(compressed_data).to_bytes(4, byteorder='little')
        for b in data_len:
            self.write(b)
        for b in compressed_data:
            self.write(b)

class ImageEncoder:
    def __init__(self, filename, size_format, reverse=False):
        self.f = open(filename, "w")
        self.size_format = size_format
        self.reverse = reverse

    def write(self, value):
        if isinstance(value, bytes):
            value = int.from_bytes(value, 'little')
        self.f.write("0x%02x," % value)

    def write_size(self, width, height):
        if self.size_format == 2:
            self.f.write("%d,%d,%d,%d,\n" % (width % 256, width // 256, height % 256, height // 256))
        else:
            self.f.write("%d,%d,\n" % (width, height))

    def encode_1bit(self, image, rows):
        image = image.convert(mode='1')
        width, height = image.size
        self.write_size(width, height // rows)
        for y in range(0, height, 8):
            for x in range(width):
                value = 0
                for z in range(8):
                    if y + z < height:
                        if image.format == "XBM":
                            if image.getpixel((x, y + z)) > 0:
                                value += 1 << z
                        else:
                            if image.getpixel((x, y + z)) == 0:
                                value += 1 << z
                self.encode_byte(value)
            self.f.write("\n")
        self.encode_end()

    def encode_4bits(self, image):
        image = image.convert(mode='L')
        width, height = image.size
        self.write_size(width, height)
        for y in range(0, height, 2):
            for x in range(width):
                value = 0xFF
                gray1 = self.get_pixel(image, x, y)
                if y + 1 < height:
                    gray2 = self.get_pixel(image, x, y + 1)
                else:
                    gray2 = 255
                for i in range(4):
                    if gray1 & (1 << (4 + i)):
                        value -= 1 << i
                    if gray2 & (1 << (4 + i)):
                        value -= 1 << (4 + i)
                self.encode_byte(value)
            self.f.write("\n")
        self.encode_end()

    def encode_8bits(self, image):
        image = image.convert(mode='L')
        width, height = image.size
        self.write_size(width, height)
        for y in range(height):
            for x in range(width):
                value = self.get_pixel(image, x, y)
                value = 0xFF - value
                self.encode_byte(value)
        self.encode_end()

    def encode_5_6_5(self, image):
        width, height = image.size
        self.write_size(width, height)
        for y in range(height):
            for x in range(width):
                pixel = self.get_pixel(image, x, y)
                val = ((pixel[0] >> 3) << 11) + ((pixel[1] >> 2) << 5) + ((pixel[2] >> 3) << 0)
                self.encode_byte(val & 255)
                self.encode_byte(val >> 8)
        self.encode_end()

    def encode_4_4_4_4(self, image):
        width, height = image.size
        self.write_size(width, height)
        for y in range(height):
            for x in range(width):
                pixel = self.get_pixel(image, x, y)
                val = ((pixel[3] // 16) << 12) + ((pixel[0] // 16) << 8) + ((pixel[1] // 16) << 4) + ((pixel[2] // 16) << 0)
                self.encode_byte(val & 255)
                self.encode_byte(val >> 8)
        self.encode_end()

    def get_pixel(self, image, x, y):
        if self.reverse:
            return image.getpixel((image.width - x - 1, image.height - y - 1))
        else:
            return image.getpixel((x, y))

    @staticmethod
    def create(filename, size_format=1, reverse=False, encode_mixin=RawMixin):
        class ResultClass(ImageEncoder, encode_mixin):
            def __init__(self, *args, **kwargs):
                ImageEncoder.__init__(self, *args, **kwargs)
                encode_mixin.__init__(self)
        return ResultClass(filename, size_format, reverse)


def main():
    parser = argparse.ArgumentParser(description='Bitmaps encoder')
    parser.add_argument('input', action="store", help="Input file name")
    parser.add_argument('output', action="store", help="Output file name")
    parser.add_argument('--format', action="store", help="Output format")
    parser.add_argument("--reverse", help="Invert pixels order", action="store_true")
    parser.add_argument("--rle", help="Enable RLE compression", action="store_true")
    parser.add_argument("--lz4", help="Enable RLE compression", action="store_true")
    parser.add_argument("--rows", help="Image rows count (for 1bit format)", type=int, default=1)
    parser.add_argument("--size-format", help="Header image size format (1 or 2 bytes)", type=int, default=1)

    args = parser.parse_args()

    image = Image.open(args.input)
    output = args.output

    byte_encoder = RawMixin
    if args.rle:
        byte_encoder = RleMixin
    if args.lz4:
        byte_encoder = Lz4Mixin

    encoder = ImageEncoder.create(output, args.size_format, args.reverse, byte_encoder)

    if args.format == "1bit":
        encoder.encode_1bit(image, args.rows)
    elif args.format == "4bits":
        encoder.encode_4bits(image)
    elif args.format == "8bits":
        encoder.encode_8bits(image)
    elif args.format == "4/4/4/4":
        encoder.encode_4_4_4_4(image)
    elif args.format == "5/6/5":
        encoder.encode_5_6_5(image)


if __name__ == "__main__":
    main()
