/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --no-prefilter --bpp 1 --size 16 --no-compress --font ../Roboto/Roboto-Regular-BL.ttf -r 0x20-0x7F --font ../../thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff -r 61671,63650,63426,61453,61787,61452,61931,62087 --format lvgl -o std/lv_font_bl.c --force-fast-kern-format --no-compress
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
    0xff, 0xff, 0x3,

    /* U+0022 "\"" */
    0xff, 0xfa,

    /* U+0023 "#" */
    0xb, 0x9, 0x4, 0x8f, 0xf1, 0x20, 0x90, 0x58,
    0x68, 0xff, 0x12, 0x9, 0x4, 0x80,

    /* U+0024 "$" */
    0x10, 0x21, 0xf6, 0x7c, 0x78, 0x38, 0x1c, 0xc,
    0xf, 0x1e, 0x37, 0xc2, 0x4, 0x0,

    /* U+0025 "%" */
    0x70, 0x22, 0x8, 0x92, 0x28, 0x72, 0x1, 0x0,
    0x98, 0x29, 0x12, 0x4c, 0x92, 0x24, 0x6,

    /* U+0026 "&" */
    0x3c, 0x33, 0x19, 0x8c, 0xc6, 0xc1, 0xc0, 0xe1,
    0xd3, 0xcd, 0xe3, 0xb8, 0xc7, 0xb0,

    /* U+0027 "'" */
    0xfe,

    /* U+0028 "(" */
    0x19, 0x98, 0xc4, 0x63, 0x18, 0xc6, 0x31, 0x86,
    0x30, 0xc2, 0x0,

    /* U+0029 ")" */
    0xc3, 0x8, 0x61, 0xc, 0x63, 0x18, 0xc6, 0x33,
    0x19, 0x88, 0x0,

    /* U+002A "*" */
    0x25, 0x3e, 0xc5, 0x28,

    /* U+002B "+" */
    0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18,

    /* U+002C "," */
    0x6d, 0x20,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x4, 0x30, 0x82, 0x18, 0x43, 0x8, 0x21, 0x84,
    0x10, 0xc0,

    /* U+0030 "0" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8d, 0xb1, 0xc0,

    /* U+0031 "1" */
    0xff, 0x33, 0x33, 0x33, 0x33, 0x33,

    /* U+0032 "2" */
    0x3c, 0xcf, 0x18, 0x30, 0x61, 0x87, 0xc, 0x30,
    0xe3, 0x87, 0xf0,

    /* U+0033 "3" */
    0x3d, 0xcf, 0x18, 0x30, 0x43, 0x1, 0x3, 0x7,
    0x8d, 0xb1, 0xc0,

    /* U+0034 "4" */
    0x6, 0xe, 0x1e, 0x16, 0x36, 0x26, 0x66, 0xc6,
    0xff, 0x6, 0x6, 0x6,

    /* U+0035 "5" */
    0x7c, 0x81, 0x2, 0x7, 0xc9, 0x81, 0x83, 0x7,
    0x8f, 0xb3, 0xc0,

    /* U+0036 "6" */
    0x3c, 0xc3, 0x6, 0xf, 0xd9, 0xf1, 0xe3, 0xc7,
    0x8d, 0xb1, 0xc0,

    /* U+0037 "7" */
    0xff, 0x3, 0x6, 0xc, 0x8, 0x18, 0x18, 0x30,
    0x30, 0x30, 0x30, 0x30,

    /* U+0038 "8" */
    0x7c, 0xce, 0xc6, 0xc6, 0x6c, 0x38, 0x66, 0xc3,
    0xc3, 0xc3, 0x66, 0x3c,

    /* U+0039 "9" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xdb, 0xbf, 0x6,
    0xc, 0x33, 0xc0,

    /* U+003A ":" */
    0xc0, 0x0, 0xc0,

    /* U+003B ";" */
    0xc0, 0xf, 0xa0,

    /* U+003C "<" */
    0x6, 0x39, 0xc6, 0x7, 0x3, 0xc1, 0x80,

    /* U+003D "=" */
    0xfe, 0x0, 0x0, 0xf, 0xe0,

    /* U+003E ">" */
    0x81, 0xc0, 0xf0, 0x31, 0xde, 0x30, 0x0,

    /* U+003F "?" */
    0x7d, 0x8f, 0x18, 0x30, 0x61, 0x86, 0x18, 0x30,
    0x0, 0x1, 0x80,

    /* U+0040 "@" */
    0xf, 0x81, 0x83, 0x8, 0xc, 0x8f, 0x2c, 0xc8,
    0xc4, 0x46, 0x22, 0x31, 0x11, 0x98, 0x8c, 0x44,
    0x62, 0x64, 0x9d, 0xc6, 0x0, 0x18, 0x0, 0x3e,
    0x0,

    /* U+0041 "A" */
    0xc, 0x3, 0x0, 0xe0, 0x78, 0x12, 0x4, 0xc3,
    0x30, 0xc4, 0x3f, 0x98, 0x64, 0x9, 0x3,

    /* U+0042 "B" */
    0xfc, 0xc7, 0xc3, 0xc3, 0xc6, 0xfc, 0xc6, 0xc3,
    0xc3, 0xc3, 0xc7, 0xfe,

    /* U+0043 "C" */
    0x3e, 0x31, 0x98, 0x78, 0x3c, 0x6, 0x3, 0x1,
    0x80, 0xc1, 0xa0, 0xd8, 0xc7, 0xc0,

    /* U+0044 "D" */
    0xfc, 0x63, 0xb0, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe1, 0xb1, 0xdf, 0x80,

    /* U+0045 "E" */
    0xff, 0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1,
    0x83, 0x7, 0xf0,

    /* U+0046 "F" */
    0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0047 "G" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x6, 0x3, 0x1f,
    0x83, 0xc1, 0xe0, 0xd8, 0xe7, 0xe0,

    /* U+0048 "H" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0xff,
    0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x30,

    /* U+0049 "I" */
    0xff, 0xff, 0xff,

    /* U+004A "J" */
    0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83, 0x7,
    0x8f, 0x33, 0xc0,

    /* U+004B "K" */
    0xc1, 0x61, 0xb1, 0x99, 0x8d, 0x87, 0x83, 0x61,
    0x98, 0xc6, 0x63, 0x30, 0xd8, 0x30,

    /* U+004C "L" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xff,

    /* U+004D "M" */
    0xe0, 0xfc, 0x1f, 0x83, 0xf8, 0x7d, 0x17, 0xa2,
    0xf4, 0x5e, 0x53, 0xca, 0x79, 0x4f, 0x11, 0xe2,
    0x30,

    /* U+004E "N" */
    0xc1, 0xf0, 0xf8, 0x7e, 0x3d, 0x1e, 0xcf, 0x37,
    0x8b, 0xc7, 0xe1, 0xf0, 0xf8, 0x30,

    /* U+004F "O" */
    0x3e, 0x31, 0xb0, 0x58, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe0, 0x98, 0xc7, 0xc0,

    /* U+0050 "P" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc6, 0xfc, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0051 "Q" */
    0x3e, 0x18, 0xcc, 0x13, 0x6, 0xc1, 0xb0, 0x6c,
    0x1b, 0x6, 0xc1, 0xb0, 0x46, 0x30, 0xf6, 0x0,
    0x80,

    /* U+0052 "R" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc6, 0xfc, 0xc7,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0053 "S" */
    0x3c, 0x66, 0xc3, 0xc0, 0xe0, 0x7c, 0x1e, 0x7,
    0x3, 0xc3, 0x67, 0x3c,

    /* U+0054 "T" */
    0xff, 0x8c, 0x6, 0x3, 0x1, 0x80, 0xc0, 0x60,
    0x30, 0x18, 0xc, 0x6, 0x3, 0x0,

    /* U+0055 "U" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe0, 0xd8, 0xc7, 0xc0,

    /* U+0056 "V" */
    0xc0, 0xd0, 0x26, 0x19, 0x86, 0x21, 0xc, 0xc3,
    0x30, 0x48, 0x1e, 0x3, 0x80, 0xc0, 0x30,

    /* U+0057 "W" */
    0xc3, 0x9, 0xc, 0x64, 0x71, 0x99, 0xe6, 0x65,
    0x90, 0x92, 0x42, 0xcb, 0xf, 0x3c, 0x38, 0xf0,
    0xe1, 0x81, 0x86, 0x6, 0x18,

    /* U+0058 "X" */
    0xc3, 0x61, 0x99, 0x84, 0x83, 0xc0, 0xc0, 0x60,
    0x78, 0x26, 0x33, 0x30, 0xd8, 0x20,

    /* U+0059 "Y" */
    0xc0, 0xd8, 0x66, 0x18, 0xcc, 0x33, 0x7, 0x81,
    0xe0, 0x30, 0xc, 0x3, 0x0, 0xc0, 0x30,

    /* U+005A "Z" */
    0xff, 0x3, 0x6, 0x4, 0xc, 0x18, 0x18, 0x30,
    0x20, 0x60, 0xc0, 0xff,

    /* U+005B "[" */
    0xfb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb7,

    /* U+005C "\\" */
    0xc0, 0x81, 0x81, 0x2, 0x6, 0x4, 0xc, 0x18,
    0x10, 0x30, 0x20, 0x40,

    /* U+005D "]" */
    0xed, 0xb6, 0xdb, 0x6d, 0xb6, 0xdf,

    /* U+005E "^" */
    0x10, 0xc3, 0x9a, 0x4d, 0x10,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0xc8,

    /* U+0061 "a" */
    0x3c, 0xcd, 0x18, 0x37, 0xf8, 0xf1, 0xe7, 0x76,

    /* U+0062 "b" */
    0xc1, 0x83, 0x6, 0xf, 0xd9, 0xb1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x6f, 0xc0,

    /* U+0063 "c" */
    0x3c, 0xdf, 0x1e, 0xc, 0x18, 0x31, 0xb3, 0x3c,

    /* U+0064 "d" */
    0x6, 0xc, 0x18, 0x37, 0xec, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1b, 0x37, 0xe0,

    /* U+0065 "e" */
    0x38, 0xdb, 0x1e, 0x3f, 0xf8, 0x30, 0x30, 0x3c,

    /* U+0066 "f" */
    0x3b, 0x18, 0xcf, 0x31, 0x8c, 0x63, 0x18, 0xc6,
    0x0,

    /* U+0067 "g" */
    0x7e, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xb3, 0x7e,
    0xd, 0x1b, 0xe0,

    /* U+0068 "h" */
    0xc1, 0x83, 0x6, 0xd, 0xdc, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x3c, 0x60,

    /* U+0069 "i" */
    0xc0, 0xff, 0xff, 0xc0,

    /* U+006A "j" */
    0x30, 0x0, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0xc, 0x79, 0xb2, 0x6c, 0xf1,
    0xb3, 0x36, 0x2c, 0x60,

    /* U+006C "l" */
    0xff, 0xff, 0xff, 0xc0,

    /* U+006D "m" */
    0xdd, 0xee, 0x73, 0xc6, 0x3c, 0x63, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x63, 0xc6, 0x30,

    /* U+006E "n" */
    0xdd, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc6,

    /* U+006F "o" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0070 "p" */
    0xfd, 0x9b, 0x1e, 0x3c, 0x78, 0xf1, 0xe6, 0xfd,
    0x83, 0x6, 0x0,

    /* U+0071 "q" */
    0x7e, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xb3, 0x7e,
    0xc, 0x18, 0x30,

    /* U+0072 "r" */
    0xfe, 0x31, 0x8c, 0x63, 0x18, 0xc0,

    /* U+0073 "s" */
    0x7d, 0x8f, 0x1f, 0x7, 0xc1, 0xf1, 0xe3, 0x7c,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x66, 0x66, 0x30,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe7, 0x76,

    /* U+0076 "v" */
    0x43, 0x42, 0x66, 0x26, 0x24, 0x34, 0x1c, 0x18,
    0x18,

    /* U+0077 "w" */
    0xc6, 0x34, 0x62, 0x66, 0x26, 0xf6, 0x29, 0x62,
    0x94, 0x29, 0x43, 0x8c, 0x10, 0x80,

    /* U+0078 "x" */
    0x46, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x34, 0x66,
    0x42,

    /* U+0079 "y" */
    0xc3, 0x42, 0x66, 0x66, 0x24, 0x3c, 0x1c, 0x18,
    0x18, 0x10, 0x30, 0x60,

    /* U+007A "z" */
    0xfc, 0x18, 0x60, 0x83, 0x4, 0x18, 0x60, 0xfe,

    /* U+007B "{" */
    0x1c, 0xc3, 0xc, 0x30, 0xc3, 0x30, 0x30, 0xc3,
    0xc, 0x30, 0xc1, 0xc0,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xe1, 0x8c, 0x63, 0x18, 0xc1, 0x31, 0x8c, 0x63,
    0x1b, 0x80,

    /* U+007E "~" */
    0x70, 0xc4, 0x61, 0xc0,

    /* U+F00C "" */
    0x0, 0x6, 0x0, 0xf, 0x0, 0x1f, 0x0, 0x3e,
    0x60, 0x7c, 0xf0, 0xf8, 0xf9, 0xf0, 0x7f, 0xe0,
    0x3f, 0xc0, 0x1f, 0x80, 0xf, 0x0, 0x6, 0x0,

    /* U+F00D "" */
    0x60, 0xde, 0x3f, 0xef, 0xbf, 0xe3, 0xf8, 0x3e,
    0xf, 0xe3, 0xfe, 0xfb, 0xfe, 0x3d, 0x83, 0x0,

    /* U+F0E7 "" */
    0x7e, 0x1f, 0x87, 0xc1, 0xf0, 0xfc, 0x3f, 0xff,
    0xff, 0xfe, 0xff, 0x83, 0xc0, 0xe0, 0x38, 0xc,
    0x7, 0x1, 0x80, 0x40,

    /* U+F15B "" */
    0xfe, 0x8f, 0xec, 0xfe, 0xef, 0xef, 0xfe, 0xf,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    /* U+F1EB "" */
    0x3, 0xfc, 0x0, 0xff, 0xf0, 0x3f, 0xff, 0xc7,
    0x80, 0x1e, 0xe0, 0x0, 0x74, 0x1f, 0x82, 0xf,
    0xff, 0x0, 0xe0, 0x70, 0x4, 0x2, 0x0, 0x0,
    0x0, 0x0, 0x60, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0x6, 0x0,

    /* U+F287 "" */
    0x0, 0x38, 0x0, 0xf, 0x80, 0x1, 0xb8, 0x0,
    0x10, 0x0, 0x63, 0x0, 0xf, 0x20, 0x6, 0xff,
    0xff, 0xff, 0xc, 0x6, 0x60, 0x40, 0x0, 0x6,
    0xf0, 0x0, 0x3f, 0x0, 0x0, 0xf0,

    /* U+F7C2 "" */
    0xf, 0xe1, 0xff, 0x34, 0xb7, 0x4b, 0xf4, 0xbf,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xfe,

    /* U+F8A2 "" */
    0x0, 0x0, 0x80, 0x0, 0xc2, 0x0, 0xe3, 0x80,
    0x73, 0xc0, 0x3b, 0xff, 0xfd, 0xff, 0xfe, 0x78,
    0x0, 0x1c, 0x0, 0x4, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 67, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 92, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 160, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 149, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 34, .adv_w = 187, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 160, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 56, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 64, .adv_w = 85, .box_w = 5, .box_h = 17, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 75, .adv_w = 86, .box_w = 5, .box_h = 17, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 86, .adv_w = 111, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 90, .adv_w = 145, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 98, .adv_w = 51, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 100, .adv_w = 115, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 101, .adv_w = 69, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 106, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 112, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 144, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 65, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 66, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 226, .adv_w = 130, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 233, .adv_w = 144, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 238, .adv_w = 134, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 245, .adv_w = 122, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 229, .box_w = 13, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 281, .adv_w = 162, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 163, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 162, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 173, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 141, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 141, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 173, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 180, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 387, .adv_w = 72, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 140, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 163, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 141, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 222, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 180, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 175, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 163, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 178, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 501, .adv_w = 163, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 157, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 153, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 173, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 162, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 220, .box_w = 14, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 589, .adv_w = 162, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 603, .adv_w = 162, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 618, .adv_w = 153, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 69, .box_w = 3, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 636, .adv_w = 106, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 648, .adv_w = 69, .box_w = 3, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 654, .adv_w = 107, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 659, .adv_w = 116, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 660, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 661, .adv_w = 141, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 146, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 681, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 146, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 709, .adv_w = 84, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 718, .adv_w = 146, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 729, .adv_w = 146, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 65, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 66, .box_w = 4, .box_h = 16, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 753, .adv_w = 131, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 765, .adv_w = 65, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 769, .adv_w = 224, .box_w = 12, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 146, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 791, .adv_w = 146, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 146, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 811, .adv_w = 146, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 822, .adv_w = 90, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 828, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 836, .adv_w = 86, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 842, .adv_w = 146, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 129, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 859, .adv_w = 194, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 129, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 882, .adv_w = 129, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 894, .adv_w = 129, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 902, .adv_w = 87, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 914, .adv_w = 63, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 918, .adv_w = 87, .box_w = 5, .box_h = 15, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 928, .adv_w = 174, .box_w = 9, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 932, .adv_w = 256, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 956, .adv_w = 176, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 972, .adv_w = 160, .box_w = 10, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 992, .adv_w = 192, .box_w = 12, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1016, .adv_w = 320, .box_w = 20, .box_h = 14, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1051, .adv_w = 320, .box_w = 20, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1081, .adv_w = 192, .box_w = 12, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1105, .adv_w = 258, .box_w = 17, .box_h = 10, .ofs_x = -1, .ofs_y = 1}
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
    -13, 0, 0, 0, 0, 0, 0, -13,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, -7, 0, -2, -8,
    0, -5, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 3, 2,
    0, 3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -10, 0, 0, 0, 0, 0,
    -8, 0, -1, 0, 0, -16, -2, -11,
    -9, 0, -12, 0, 0, 0, 0, 0,
    0, -1, 0, 0, -2, -1, -6, -4,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    -3, 0, 0, -7, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, -4,
    0, 0, 0, 0, 0, 0, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    0, -13, 0, 0, -3, 0, 0, 0,
    -3, 0, -3, 0, -3, -5, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, -2, 0, -2, 0, 0,
    0, -2, -3, -3, 0, 0, 0, 0,
    0, 0, 0, -29, 0, 0, -7, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -4, -3, 0, 0, -3, -3,
    0, 0, -3, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, -4, 0, 0, 0, 2,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -8,
    0, 0, -4, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 0,
    -3, -3, 0, 0, 0, -3, -5, -8,
    0, 0, 0, -17, 0, 0, 0, 0,
    0, 0, 2, -8, 0, 0, -34, -7,
    -22, -18, 0, -30, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -5, -17,
    -11, 0, 0, 0, 0, 0, 0, 0,
    -40, 0, 0, -17, 0, 0, 0, 0,
    0, 0, 0, -4, 0, -3, 0, -1,
    -2, 0, 0, -2, 0, 0, 2, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -5, 0, -3, -2, 0, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -10, 0, -2, 0, 0, -6, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -27, -29, 0, -10, -3,
    0, -2, 2, 0, 2, 2, 0, 2,
    0, 0, -14, -12, 0, -14, -12, -9,
    -14, 0, -12, -9, -7, -10, -7, 0,
    0, 0, 3, 0, -28, -5, 0, -9,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, -6, -5, 0, 0, -6,
    -4, 0, 0, -3, -1, 0, 0, 0,
    2, 0, 0, 2, 0, -15, -7, 0,
    -5, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 2, -4, -4, 0, 0,
    -4, -3, 0, 0, -2, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, -6,
    0, 0, -3, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, -3, 0,
    0, -3, 0, 0, 0, -3, -4, 0,
    0, 0, 0, 0, -4, 3, -6, -26,
    -6, 0, -10, -4, -12, -2, 2, -12,
    2, 2, 2, 2, 0, 2, -9, -8,
    -3, -5, -8, -5, -7, -3, -5, -2,
    0, -3, -4, 2, 0, 0, 0, 0,
    0, 0, 0, 2, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, 0, -3, 0, 0, 0, -2,
    -3, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -8, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, 0, -4,
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
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 2, 0, 3, 0, 0, 0,
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
    0, -2, 0, -3, -2, 0, 2, 0,
    0, 0, -15, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 2, 0, -2, 0, 0,
    2, 0, 2, 2, 0, 0, 0, 2,
    0, 0, 0, -13, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, -2, 2, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -15, 0, 0, 0,
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
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
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

