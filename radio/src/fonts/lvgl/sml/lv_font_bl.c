/*******************************************************************************
 * Size: 11 px
 * Bpp: 1
 * Opts: --no-prefilter --bpp 1 --size 11 --no-compress --font ../Roboto/Roboto-Regular.ttf -r 0x20-0x7F --font ../../thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff -r 61671,63650,63426,61453,61787,61452,61931,62087 --format lvgl -o sml/lv_font_bl.c --force-fast-kern-format
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
    0xf9,

    /* U+0022 "\"" */
    0xf0,

    /* U+0023 "#" */
    0x29, 0x2f, 0xd4, 0x53, 0xe5, 0x24,

    /* U+0024 "$" */
    0x23, 0xa3, 0x14, 0x18, 0x31, 0x71, 0x0,

    /* U+0025 "%" */
    0xe1, 0x4a, 0xa7, 0x81, 0xe5, 0x52, 0x87,

    /* U+0026 "&" */
    0x71, 0x45, 0x18, 0x62, 0x59, 0x9d,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x29, 0x49, 0x24, 0x89, 0x10,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x29, 0x0,

    /* U+002A "*" */
    0x25, 0x5c, 0xa1, 0x0,

    /* U+002B "+" */
    0x21, 0x9, 0xf2, 0x10,

    /* U+002C "," */
    0x50,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x11, 0x22, 0x24, 0x48, 0x80,

    /* U+0030 "0" */
    0x74, 0x63, 0x18, 0xc6, 0x2e,

    /* U+0031 "1" */
    0x3c, 0x92, 0x49,

    /* U+0032 "2" */
    0x74, 0x62, 0x31, 0x11, 0x1f,

    /* U+0033 "3" */
    0x74, 0x42, 0x60, 0x86, 0x2e,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0xf0, 0x82,

    /* U+0035 "5" */
    0x7a, 0x21, 0xe0, 0x86, 0x2e,

    /* U+0036 "6" */
    0x32, 0x21, 0xe8, 0xc6, 0x2e,

    /* U+0037 "7" */
    0xf8, 0x44, 0x22, 0x11, 0x8,

    /* U+0038 "8" */
    0x74, 0x62, 0xe8, 0xc6, 0x2e,

    /* U+0039 "9" */
    0x74, 0x63, 0x17, 0x84, 0x4c,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x40, 0x54,

    /* U+003C "<" */
    0x1b, 0xa0, 0xe0, 0x80,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0x83, 0x6, 0xc8, 0x0,

    /* U+003F "?" */
    0x72, 0x42, 0x23, 0x10, 0x4,

    /* U+0040 "@" */
    0x1e, 0x30, 0x93, 0x72, 0x9a, 0x4d, 0x26, 0x93,
    0xb6, 0x40, 0x1e, 0x0,

    /* U+0041 "A" */
    0x10, 0x30, 0xa1, 0x44, 0x4f, 0x91, 0x41,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc6, 0x3e,

    /* U+0043 "C" */
    0x7b, 0x38, 0x60, 0x82, 0x1c, 0xde,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0xbc,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x42, 0x1f,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf, 0x42, 0x10,

    /* U+0047 "G" */
    0x7b, 0x38, 0x20, 0x9e, 0x14, 0x5e,

    /* U+0048 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+0049 "I" */
    0xff,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x86, 0x2e,

    /* U+004B "K" */
    0x8a, 0x4b, 0x38, 0xe2, 0x48, 0xa3,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x1f,

    /* U+004D "M" */
    0xc3, 0xc3, 0xc3, 0xa5, 0xa5, 0x99, 0x99, 0x99,

    /* U+004E "N" */
    0x87, 0x1e, 0x69, 0x96, 0x78, 0xe1,

    /* U+004F "O" */
    0x79, 0x28, 0x61, 0x86, 0x14, 0xde,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20,

    /* U+0051 "Q" */
    0x79, 0x28, 0x61, 0x86, 0x14, 0x9e, 0xc,

    /* U+0052 "R" */
    0xf4, 0x63, 0x1f, 0x4a, 0x51,

    /* U+0053 "S" */
    0x7a, 0x18, 0x18, 0x18, 0x18, 0x5e,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0x5e,

    /* U+0056 "V" */
    0x82, 0x89, 0x12, 0x22, 0x85, 0xa, 0x8,

    /* U+0057 "W" */
    0x8c, 0x93, 0x24, 0xc9, 0x52, 0x53, 0xc, 0xc3,
    0x30, 0x84,

    /* U+0058 "X" */
    0x44, 0xd8, 0xa0, 0x83, 0x5, 0x13, 0x22,

    /* U+0059 "Y" */
    0x44, 0x88, 0xa1, 0x41, 0x2, 0x4, 0x8,

    /* U+005A "Z" */
    0xfc, 0x21, 0xc, 0x21, 0xc, 0x3f,

    /* U+005B "[" */
    0xea, 0xaa, 0xac,

    /* U+005C "\\" */
    0x84, 0x44, 0x22, 0x11, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x5c,

    /* U+005E "^" */
    0x26, 0x59,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0xc8,

    /* U+0061 "a" */
    0x74, 0x5f, 0x18, 0xbc,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc6, 0x3e,

    /* U+0063 "c" */
    0x74, 0x61, 0x8, 0xb8,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc6, 0x2f,

    /* U+0065 "e" */
    0x74, 0x7f, 0xc, 0xb8,

    /* U+0066 "f" */
    0x34, 0x4e, 0x44, 0x44, 0x40,

    /* U+0067 "g" */
    0x7c, 0x63, 0x18, 0xbc, 0x2e,

    /* U+0068 "h" */
    0x84, 0x3d, 0x18, 0xc6, 0x31,

    /* U+0069 "i" */
    0xbf,

    /* U+006A "j" */
    0x45, 0x55, 0x70,

    /* U+006B "k" */
    0x84, 0x25, 0x4c, 0x52, 0x52,

    /* U+006C "l" */
    0xff,

    /* U+006D "m" */
    0xf7, 0x44, 0x62, 0x31, 0x18, 0x8c, 0x44,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xf4, 0x63, 0x18, 0xfa, 0x10,

    /* U+0071 "q" */
    0x7c, 0x63, 0x18, 0xbc, 0x21,

    /* U+0072 "r" */
    0xf2, 0x49, 0x0,

    /* U+0073 "s" */
    0x69, 0xc3, 0x9e,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x93,

    /* U+0075 "u" */
    0x8c, 0x63, 0x18, 0xbc,

    /* U+0076 "v" */
    0x8a, 0x54, 0xa3, 0x10,

    /* U+0077 "w" */
    0x89, 0x59, 0x5a, 0x66, 0x66, 0x24,

    /* U+0078 "x" */
    0x4a, 0x8c, 0x65, 0x24,

    /* U+0079 "y" */
    0x8a, 0x54, 0xa2, 0x10, 0x98,

    /* U+007A "z" */
    0xf8, 0x88, 0x8c, 0x7c,

    /* U+007B "{" */
    0x29, 0x25, 0x12, 0x48, 0x80,

    /* U+007C "|" */
    0xff, 0xc0,

    /* U+007D "}" */
    0x89, 0x24, 0x52, 0x4a, 0x0,

    /* U+007E "~" */
    0xe6, 0x60,

    /* U+F00C "" */
    0x0, 0x60, 0x1c, 0x7, 0x71, 0xcf, 0x70, 0xfc,
    0xf, 0x0, 0xc0,

    /* U+F00D "" */
    0xc3, 0xe7, 0x7e, 0x3c, 0x7e, 0xe7, 0xc3,

    /* U+F0E7 "" */
    0x78, 0xf3, 0xc7, 0xff, 0xff, 0x86, 0xc, 0x30,
    0x60, 0x80,

    /* U+F15B "" */
    0xfa, 0xfb, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff,

    /* U+F1EB "" */
    0x0, 0x0, 0x7f, 0x87, 0x87, 0xb8, 0x7, 0xf,
    0xc0, 0x61, 0x80, 0x0, 0x0, 0xc0, 0x7, 0x0,
    0xc, 0x0,

    /* U+F287 "" */
    0x1, 0x80, 0x1e, 0xe, 0x80, 0xbf, 0xff, 0xe2,
    0x8, 0x8, 0x0, 0x16, 0x0, 0x38,

    /* U+F7C2 "" */
    0x3f, 0x55, 0xd5, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff,

    /* U+F8A2 "" */
    0x0, 0x24, 0xd, 0x81, 0xff, 0xf6, 0x0, 0x40,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 44, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 45, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 56, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 3, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 16, .adv_w = 129, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 31, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 30, .adv_w = 60, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 35, .adv_w = 61, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 40, .adv_w = 76, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 44, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 48, .adv_w = 35, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 49, .adv_w = 49, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 50, .adv_w = 46, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 73, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 56, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 99, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 43, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 37, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 108, .adv_w = 89, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 112, .adv_w = 97, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 114, .adv_w = 92, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 83, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 158, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 135, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 110, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 100, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 97, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 125, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 48, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 97, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 95, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 125, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 121, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 121, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 231, .adv_w = 108, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 104, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 105, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 112, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 156, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 110, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 106, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 105, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 47, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 295, .adv_w = 72, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 300, .adv_w = 47, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 303, .adv_w = 74, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 305, .adv_w = 79, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 306, .adv_w = 54, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 307, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 92, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 93, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 61, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 339, .adv_w = 97, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 43, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 42, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 348, .adv_w = 89, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 43, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 154, .box_w = 9, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 97, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 374, .adv_w = 100, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 379, .adv_w = 60, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 91, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 57, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 97, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 85, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 396, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 83, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 411, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 60, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 420, .adv_w = 43, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 422, .adv_w = 60, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 427, .adv_w = 120, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 429, .adv_w = 176, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 121, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 447, .adv_w = 110, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 457, .adv_w = 132, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 467, .adv_w = 220, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 485, .adv_w = 220, .box_w = 14, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 132, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 509, .adv_w = 177, .box_w = 11, .box_h = 6, .ofs_x = 0, .ofs_y = 1}
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
    0, 1, 0, 2, 0, 0, 0, 0,
    2, 3, 0, 0, 0, 4, 0, 4,
    5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 6, 7, 8, 9, 10, 11,
    0, 12, 12, 13, 14, 15, 12, 12,
    9, 16, 17, 18, 0, 19, 13, 20,
    21, 22, 23, 24, 25, 0, 0, 0,
    0, 0, 26, 27, 28, 0, 29, 30,
    0, 31, 0, 0, 32, 0, 31, 31,
    33, 27, 0, 34, 0, 35, 0, 36,
    37, 38, 36, 39, 40, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 1, 0, 2, 0, 0, 0, 3,
    2, 0, 4, 5, 0, 6, 7, 6,
    8, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    9, 0, 10, 0, 11, 0, 0, 0,
    11, 0, 0, 12, 0, 0, 0, 0,
    11, 0, 11, 0, 13, 14, 15, 16,
    17, 18, 19, 20, 0, 0, 21, 0,
    0, 0, 22, 0, 23, 23, 23, 24,
    23, 0, 0, 0, 0, 0, 25, 25,
    26, 25, 23, 27, 28, 29, 30, 31,
    32, 33, 31, 34, 0, 0, 35, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, -10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -4, -5, 0, -2, -5, 0, -7, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -15, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -19, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -10, 0, 0, 0, 0, 0, 0, -5,
    0, -1, 0, 0, -11, -1, -7, -6,
    0, -8, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, -1, -1, -4, -3, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    0, -2, 0, 0, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -1,
    0, 0, 0, 0, 0, -9, 0, 0,
    0, -2, 0, 0, 0, -2, 0, -2,
    0, -2, -4, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, -2, -2, 0, -2, 0, 0, 0,
    -1, -2, -2, 0, 0, 0, 0, 0,
    0, 0, 0, -20, 0, 0, 0, -15,
    0, -23, 0, 2, 0, 0, 0, 0,
    0, 0, 0, -3, -2, 0, 0, -2,
    -2, 0, 0, -2, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, -2, 0,
    0, 0, 1, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -5, 0, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, -2,
    -2, 0, 0, 0, -2, -3, -5, 0,
    0, 0, 0, -29, 0, 0, 0, 0,
    0, 0, 0, 2, -6, 0, 0, -24,
    -5, -15, -12, 0, -21, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -4,
    -11, -8, 0, 0, 0, 0, 0, 0,
    0, 0, -28, 0, 0, 0, -12, 0,
    -17, 0, 0, 0, 0, 0, -3, 0,
    -2, 0, -1, -1, 0, 0, -1, 0,
    0, 1, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -4, 0, -2,
    -2, 0, -3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -7, 0, -2, 0, 0, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    0, 0, 0, -19, -20, 0, 0, -7,
    -2, -21, -1, 1, 0, 1, 1, 0,
    1, 0, 0, -10, -9, 0, -9, -9,
    -6, -10, 0, -8, -6, -5, -7, -5,
    0, 0, 0, 0, 2, 0, -19, -3,
    0, 0, -6, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, -4, -4,
    0, 0, -4, -3, 0, 0, -2, -1,
    0, 0, 0, 2, 0, 0, 0, 1,
    0, -11, -5, 0, 0, -4, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    1, -3, -3, 0, 0, -3, -2, 0,
    0, -2, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, -4, 0, 0,
    0, -2, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    -2, 0, 0, 0, -2, -3, 0, 0,
    0, 0, 0, 0, -3, 2, -4, -18,
    -4, 0, 0, -8, -2, -8, -1, 1,
    -8, 2, 1, 1, 2, 0, 2, -6,
    -6, -2, -3, -6, -3, -5, -2, -3,
    -2, 0, -2, -3, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -2, 0, 0, -2, 0,
    0, 0, -2, -2, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -2, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, -1, -1,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -1, 0, 0, 0, 0, 0,
    1, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, -9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    -2, -1, 0, 0, 1, 0, 0, 0,
    -11, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, -2, 1, 0, -2, 0, 0, 4,
    0, 2, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, -1,
    1, 0, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -11, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 40,
    .right_class_cnt     = 35,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
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
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t lv_font_bl = {
#else
lv_font_t lv_font_bl = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if LV_FONT_BL*/

