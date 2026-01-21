/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --no-prefilter --bpp 1 --size 14 --no-compress --font ../Roboto/Roboto-Regular-BL.ttf -r 0x20-0x7F --font ../../thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff -r 61671,63650,63426,61453,61787,61452,61931,62087 --format lvgl -o sml/lv_font_bl.c --force-fast-kern-format --no-compress
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_BL
#define LV_FONT_BL 1
#endif

#if LV_FONT_BL

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x20,

    /* U+0022 "\"" */
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x12, 0x12, 0x14, 0x7f, 0x24, 0x24, 0x24, 0xfe,
    0x28, 0x28, 0x28,

    /* U+0024 "$" */
    0x10, 0x43, 0x93, 0x45, 0x6, 0xe, 0xc, 0x18,
    0x73, 0x78, 0x40,

    /* U+0025 "%" */
    0x60, 0x48, 0x24, 0x92, 0x86, 0x40, 0x40, 0x4c,
    0x29, 0x24, 0x92, 0x40, 0xc0,

    /* U+0026 "&" */
    0x38, 0x44, 0x44, 0x4c, 0x78, 0x70, 0xd9, 0x8d,
    0x87, 0xc6, 0x7f,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x5, 0x29, 0x24, 0x92, 0x44, 0x88,

    /* U+0029 ")" */
    0x11, 0x22, 0x49, 0x24, 0xa4, 0xa0,

    /* U+002A "*" */
    0x25, 0x5c, 0xe5, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x46, 0x21, 0x10, 0x84, 0x42, 0x31, 0x0,

    /* U+0030 "0" */
    0x79, 0x28, 0x61, 0x86, 0x18, 0x61, 0x85, 0x27,
    0x80,

    /* U+0031 "1" */
    0xe4, 0x92, 0x49, 0x24,

    /* U+0032 "2" */
    0x7b, 0x38, 0x41, 0xc, 0x21, 0xc, 0x61, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x7b, 0x38, 0x41, 0x4, 0xe0, 0xc1, 0x87, 0x37,
    0x80,

    /* U+0034 "4" */
    0x8, 0x30, 0x61, 0x46, 0x89, 0x32, 0x7f, 0x8,
    0x10, 0x20,

    /* U+0035 "5" */
    0x7f, 0xc, 0x20, 0xfa, 0x30, 0x41, 0x87, 0x37,
    0x80,

    /* U+0036 "6" */
    0x39, 0x8, 0x20, 0xfb, 0x38, 0x61, 0x85, 0x33,
    0x80,

    /* U+0037 "7" */
    0xfe, 0xc, 0x30, 0x41, 0x2, 0x8, 0x10, 0x20,
    0x40, 0x80,

    /* U+0038 "8" */
    0x7b, 0x38, 0x61, 0xcd, 0xec, 0xe1, 0x87, 0x37,
    0x80,

    /* U+0039 "9" */
    0x7b, 0x28, 0x61, 0x87, 0x37, 0xc1, 0x4, 0x27,
    0x0,

    /* U+003A ":" */
    0x81,

    /* U+003B ";" */
    0x83, 0x80,

    /* U+003C "<" */
    0xc, 0xec, 0x38, 0x38, 0x30,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0xc1, 0xc1, 0xc7, 0x73, 0x0,

    /* U+003F "?" */
    0x74, 0x62, 0x11, 0x88, 0x84, 0x0, 0x8,

    /* U+0040 "@" */
    0x1f, 0x6, 0x11, 0x1, 0x20, 0x28, 0xe3, 0x32,
    0x64, 0x4c, 0x99, 0x92, 0x32, 0x4b, 0x77, 0x20,
    0x2, 0x0, 0x3c, 0x0,

    /* U+0041 "A" */
    0x8, 0xe, 0x5, 0x2, 0x83, 0x41, 0x30, 0x88,
    0xfc, 0x43, 0x20, 0xb0, 0x40,

    /* U+0042 "B" */
    0xf9, 0x1a, 0x14, 0x28, 0xdf, 0x21, 0xc1, 0x83,
    0xf, 0xf0,

    /* U+0043 "C" */
    0x38, 0x8a, 0xc, 0x8, 0x10, 0x20, 0x41, 0x82,
    0x88, 0xe0,

    /* U+0044 "D" */
    0xf9, 0xa, 0x1c, 0x18, 0x30, 0x60, 0xc1, 0x87,
    0xb, 0xe0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0x83, 0xe8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xe8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x38, 0x8a, 0xc, 0x18, 0x10, 0x23, 0xc1, 0x82,
    0x84, 0xf0,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0049 "I" */
    0xff, 0xe0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x41, 0x87, 0x37,
    0x80,

    /* U+004B "K" */
    0x87, 0x1a, 0x24, 0x8b, 0x1c, 0x2c, 0x4c, 0x89,
    0x1a, 0x18,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xc0, 0xf0, 0x3c, 0xf, 0x87, 0xa1, 0x6c, 0xd9,
    0x26, 0x79, 0x9e, 0x63, 0x18, 0xc4,

    /* U+004E "N" */
    0x83, 0x87, 0xd, 0x1b, 0x32, 0x66, 0xc5, 0x87,
    0xe, 0x8,

    /* U+004F "O" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+0050 "P" */
    0xfd, 0xe, 0xc, 0x18, 0x30, 0xbe, 0x40, 0x81,
    0x2, 0x0,

    /* U+0051 "Q" */
    0x3c, 0x21, 0x20, 0x50, 0x28, 0x14, 0xa, 0x5,
    0x2, 0x81, 0x21, 0xf, 0xc0, 0x20,

    /* U+0052 "R" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa1, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0053 "S" */
    0x79, 0x8e, 0xc, 0xe, 0x7, 0x1, 0x81, 0x83,
    0x8c, 0xf0,

    /* U+0054 "T" */
    0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0056 "V" */
    0xc1, 0xa0, 0x90, 0xcc, 0x62, 0x21, 0x30, 0xd8,
    0x28, 0x14, 0xe, 0x2, 0x0,

    /* U+0057 "W" */
    0xc6, 0x34, 0x62, 0x46, 0x24, 0xf2, 0x6f, 0x66,
    0x96, 0x29, 0x42, 0x9c, 0x38, 0xc3, 0xc, 0x10,
    0xc0,

    /* U+0058 "X" */
    0x43, 0x63, 0x36, 0x14, 0x1c, 0x8, 0x1c, 0x34,
    0x26, 0x63, 0x41,

    /* U+0059 "Y" */
    0x82, 0xc6, 0x44, 0x6c, 0x28, 0x38, 0x10, 0x10,
    0x10, 0x10, 0x10,

    /* U+005A "Z" */
    0xfe, 0xc, 0x30, 0x41, 0x82, 0xc, 0x30, 0x41,
    0x83, 0xf8,

    /* U+005B "[" */
    0xea, 0xaa, 0xaa, 0xb0,

    /* U+005C "\\" */
    0xc1, 0x4, 0x8, 0x20, 0xc1, 0x4, 0x18, 0x20,
    0x83,

    /* U+005D "]" */
    0xd5, 0x55, 0x55, 0x70,

    /* U+005E "^" */
    0x21, 0x8c, 0xf4, 0xa4,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x7b, 0x10, 0x5f, 0xc6, 0x18, 0xdd,

    /* U+0062 "b" */
    0x82, 0x8, 0x3e, 0xca, 0x18, 0x61, 0x87, 0x3f,
    0x80,

    /* U+0063 "c" */
    0x79, 0x38, 0x60, 0x82, 0x14, 0x5e,

    /* U+0064 "d" */
    0x4, 0x10, 0x5f, 0x4e, 0x18, 0x61, 0x87, 0x37,
    0xc0,

    /* U+0065 "e" */
    0x39, 0x38, 0x7f, 0x82, 0x4, 0x5e,

    /* U+0066 "f" */
    0x3a, 0x11, 0xe4, 0x21, 0x8, 0x42, 0x10,

    /* U+0067 "g" */
    0x7d, 0x38, 0x61, 0x86, 0x1c, 0xdf, 0x4, 0x17,
    0x80,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x9f, 0xe0,

    /* U+006A "j" */
    0x20, 0x12, 0x49, 0x24, 0x93, 0x80,

    /* U+006B "k" */
    0x82, 0x8, 0x22, 0x92, 0xce, 0x28, 0x92, 0x68,
    0x80,

    /* U+006C "l" */
    0xff, 0xe0,

    /* U+006D "m" */
    0xb9, 0xd9, 0xce, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x79, 0x28, 0x61, 0x86, 0x14, 0x9e,

    /* U+0070 "p" */
    0xfa, 0x28, 0x61, 0x86, 0x18, 0xfe, 0x82, 0x8,
    0x0,

    /* U+0071 "q" */
    0x7d, 0x18, 0x61, 0x86, 0x1c, 0x5f, 0x4, 0x10,
    0x40,

    /* U+0072 "r" */
    0xf2, 0x49, 0x24,

    /* U+0073 "s" */
    0x74, 0x61, 0xc3, 0x86, 0x2e,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x43,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0xdd,

    /* U+0076 "v" */
    0xc6, 0x89, 0x11, 0x62, 0x85, 0x6, 0x8,

    /* U+0077 "w" */
    0xc4, 0x53, 0x14, 0xed, 0xaa, 0x3a, 0x8c, 0xa3,
    0x18, 0xc4,

    /* U+0078 "x" */
    0x44, 0xd8, 0xa1, 0xc3, 0x85, 0x1b, 0x22,

    /* U+0079 "y" */
    0xc6, 0x89, 0x13, 0x62, 0x85, 0xe, 0x8, 0x10,
    0x61, 0x80,

    /* U+007A "z" */
    0xfc, 0x31, 0x8c, 0x21, 0x8c, 0x3f,

    /* U+007B "{" */
    0x1, 0x22, 0x22, 0x2c, 0x22, 0x22, 0x21, 0x0,

    /* U+007C "|" */
    0xff, 0xf8,

    /* U+007D "}" */
    0x8, 0x44, 0x44, 0x43, 0x44, 0x44, 0x48, 0x0,

    /* U+007E "~" */
    0x71, 0x99, 0x8e,

    /* U+F00C "" */
    0x0, 0x1c, 0x0, 0xf0, 0x7, 0x90, 0x3c, 0xe1,
    0xe3, 0xcf, 0x7, 0xf8, 0xf, 0xc0, 0x1e, 0x0,
    0x10, 0x0,

    /* U+F00D "" */
    0x40, 0xbc, 0xf7, 0xf8, 0xfc, 0x1e, 0xf, 0xc7,
    0xfb, 0xcf, 0x40, 0x80,

    /* U+F0E7 "" */
    0x7c, 0x3e, 0x1f, 0x1f, 0xf, 0xf7, 0xff, 0xfd,
    0xfc, 0xe, 0x6, 0x7, 0x3, 0x1, 0x80, 0x80,

    /* U+F15B "" */
    0xfd, 0x9f, 0xbb, 0xf7, 0xfe, 0xf, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfe,

    /* U+F1EB "" */
    0x0, 0x0, 0x3, 0xff, 0x3, 0xff, 0xf1, 0xe0,
    0x1e, 0xe0, 0x1, 0xd0, 0xfc, 0x20, 0xff, 0xc0,
    0x78, 0x78, 0x8, 0x4, 0x0, 0x0, 0x0, 0xc,
    0x0, 0x7, 0x80, 0x0, 0xc0, 0x0,

    /* U+F287 "" */
    0x0, 0x70, 0x0, 0x7c, 0x0, 0x37, 0x0, 0x8,
    0x0, 0xf6, 0x1, 0xbf, 0xff, 0xff, 0x18, 0x18,
    0x2, 0x0, 0x0, 0xdc, 0x0, 0x1f, 0x0, 0x1,
    0xc0,

    /* U+F7C2 "" */
    0x1f, 0xc7, 0xfd, 0xa9, 0xf5, 0x3f, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xf7, 0xfc,

    /* U+F8A2 "" */
    0x0, 0x4, 0x0, 0x31, 0x80, 0xce, 0x3, 0xff,
    0xff, 0xff, 0xf3, 0x80, 0x6, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 56, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 59, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 80, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 130, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 27, .adv_w = 164, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 49, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 52, .adv_w = 74, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 58, .adv_w = 75, .box_w = 3, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 64, .adv_w = 97, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 68, .adv_w = 127, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 75, .adv_w = 44, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 76, .adv_w = 101, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 77, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 93, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 86, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 126, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 126, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 126, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 57, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 57, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 176, .adv_w = 114, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 181, .adv_w = 126, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 184, .adv_w = 117, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 189, .adv_w = 107, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 200, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 216, .adv_w = 142, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 152, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 124, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 124, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 152, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 158, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 63, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 123, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 123, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 195, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 158, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 153, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 156, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 386, .adv_w = 143, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 396, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 152, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 142, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 193, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 142, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 468, .adv_w = 142, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 479, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 60, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 493, .adv_w = 93, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 502, .adv_w = 60, .box_w = 2, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 506, .adv_w = 94, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 510, .adv_w = 102, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 511, .adv_w = 70, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 512, .adv_w = 123, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 74, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 564, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 573, .adv_w = 56, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 58, .box_w = 3, .box_h = 14, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 581, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 590, .adv_w = 56, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 592, .adv_w = 196, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 603, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 609, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 624, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 633, .adv_w = 78, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 117, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 641, .adv_w = 75, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 113, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 659, .adv_w = 170, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 113, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 113, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 686, .adv_w = 113, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 692, .adv_w = 76, .box_w = 4, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 700, .adv_w = 55, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 702, .adv_w = 76, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 710, .adv_w = 152, .box_w = 8, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 713, .adv_w = 224, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 731, .adv_w = 154, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 743, .adv_w = 140, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 759, .adv_w = 168, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 777, .adv_w = 280, .box_w = 18, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 807, .adv_w = 280, .box_w = 18, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 168, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 850, .adv_w = 225, .box_w = 14, .box_h = 8, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x1, 0xdb, 0x14f, 0x1df, 0x27b, 0x7b6, 0x896
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 61452, .range_length = 2199, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 8, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 1, 0, 0, 0, 0,
    1, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 4, 5, 6, 7, 8,
    0, 9, 9, 10, 11, 12, 9, 9,
    6, 13, 14, 15, 0, 16, 10, 17,
    18, 19, 20, 21, 22, 0, 0, 0,
    0, 0, 23, 24, 25, 0, 26, 27,
    0, 28, 0, 0, 29, 0, 28, 28,
    30, 24, 0, 31, 0, 0, 0, 32,
    33, 34, 32, 35, 36, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 1, 0, 0, 0, 2,
    1, 0, 3, 4, 0, 5, 6, 5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 5, 0, 0, 0, 0,
    7, 0, 8, 0, 9, 0, 0, 0,
    9, 0, 0, 10, 0, 0, 0, 0,
    9, 0, 9, 0, 11, 12, 13, 14,
    15, 16, 17, 18, 0, 0, 19, 0,
    0, 0, 20, 0, 21, 21, 21, 22,
    21, 0, 0, 0, 0, 0, 23, 23,
    24, 23, 21, 25, 26, 27, 28, 29,
    30, 31, 29, 32, 0, 0, 33, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    -12, 0, 0, 0, 0, 0, 0, -12,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, -6, 0, -2, -7,
    0, -5, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 2, 2,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -9, 0, 0, 0, 0, 0,
    -7, 0, -1, 0, 0, -14, -2, -10,
    -8, 0, -10, 0, 0, 0, 0, 0,
    0, -1, 0, 0, -2, -1, -5, -4,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    -3, 0, 0, -6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, 0, 0, 0, 0, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    0, -11, 0, 0, -2, 0, 0, 0,
    -3, 0, -2, 0, -2, -5, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, -2, 0, -2, 0, 0,
    0, -2, -3, -2, 0, 0, 0, 0,
    0, 0, 0, -26, 0, 0, -6, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -4, -2, 0, 0, -2, -3,
    0, 0, -2, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, -3, 0, 0, 0, 2,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -7,
    0, 0, -3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    -3, -3, 0, 0, 0, -3, -4, -7,
    0, 0, 0, -15, 0, 0, 0, 0,
    0, 0, 2, -7, 0, 0, -30, -6,
    -19, -16, 0, -26, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -5, -15,
    -10, 0, 0, 0, 0, 0, 0, 0,
    -35, 0, 0, -15, 0, 0, 0, 0,
    0, 0, 0, -3, 0, -3, 0, -1,
    -1, 0, 0, -1, 0, 0, 2, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -5, 0, -3, -2, 0, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -9, 0, -2, 0, 0, -5, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -24, -25, 0, -9, -3,
    0, -2, 2, 0, 2, 2, 0, 2,
    0, 0, -12, -11, 0, -12, -11, -8,
    -13, 0, -10, -8, -6, -8, -7, 0,
    0, 0, 2, 0, -25, -4, 0, -8,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, -5, -5, 0, 0, -5,
    -3, 0, 0, -3, -1, 0, 0, 0,
    2, 0, 0, 2, 0, -13, -7, 0,
    -5, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 1, -4, -3, 0, 0,
    -3, -2, 0, 0, -2, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, -5,
    0, 0, -3, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, -3, 0,
    0, -2, 0, 0, 0, -2, -3, 0,
    0, 0, 0, 0, -3, 2, -5, -23,
    -6, 0, -9, -3, -10, -2, 2, -10,
    2, 2, 1, 2, 0, 2, -8, -7,
    -2, -4, -7, -4, -6, -2, -4, -2,
    0, -3, -3, 2, 0, 0, 0, 0,
    0, 0, 0, 1, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, -2, 0, 0, 0, -2,
    -3, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -7, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, 0, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -1, 0, -2, -2, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, 0, 2, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -2, 0, -2, -2, 0, 2, 0,
    0, 0, -13, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 2, 0, -2, 0, 0,
    2, 0, 2, 2, 0, 0, 0, 2,
    0, 0, 0, -12, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, -1, 1, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -14, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 36,
    .right_class_cnt     = 33,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_bl = {
#else
lv_font_t lv_font_bl = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_BL*/

