/*******************************************************************************
 * Size: 11 px
 * Bpp: 2
 * Opts: --no-prefilter --bpp 2 --size 11 --font ../Roboto/Roboto-Regular.ttf -r 0x20-0x7F --font ../../thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff -r 61671,63650,63426,61453,61787,61452,61931,62087 --format lvgl -o sml/lv_font_bl.c --force-fast-kern-format
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

    /* U+0021 "!" */
    0x30, 0x60, 0xc1, 0x83, 0x4, 0x1d, 0x80,

    /* U+0022 "\"" */
    0x54, 0xa8, 0x8,

    /* U+0023 "#" */
    0xa, 0x30, 0x94, 0x40, 0xbf, 0x20, 0x85, 0x5,
    0x90, 0x1f, 0xe0, 0xa2, 0x9, 0x44, 0x0,

    /* U+0024 "$" */
    0xf, 0xe8, 0x27, 0xc8, 0x30, 0x69, 0x81, 0x45,
    0xa1, 0x9b, 0xe, 0x93, 0x4, 0x8f, 0xa0, 0x98,
    0x0,

    /* U+0025 "%" */
    0x28, 0x86, 0x51, 0x10, 0x14, 0x4a, 0xa, 0x8c,
    0x1e, 0x9a, 0x4, 0xd0, 0x60, 0x44, 0x8, 0x3a,
    0x80,

    /* U+0026 "&" */
    0x7, 0x90, 0xa4, 0xc2, 0x95, 0x6, 0xe0, 0xdd,
    0x88, 0xc3, 0x63, 0x3, 0xc0, 0xbd, 0x68,

    /* U+0027 "'" */
    0x5e, 0x0,

    /* U+0028 "(" */
    0x2, 0x81, 0x82, 0x41, 0x85, 0x81, 0x82, 0xc2,
    0xc2, 0xc2, 0x60, 0xa0, 0xe0,

    /* U+0029 ")" */
    0x83, 0x61, 0x48, 0x18, 0x2c, 0x2c, 0x2c, 0x2c,
    0xc, 0x8, 0xc, 0x1e,

    /* U+002A "*" */
    0x6, 0x5, 0x6a, 0x2d, 0x6, 0xc0, 0x44,

    /* U+002B "+" */
    0x2, 0x83, 0x30, 0x66, 0x5, 0xfa, 0x3, 0x6,
    0x60, 0x0,

    /* U+002C "," */
    0x66, 0x40,

    /* U+002D "-" */
    0xb8,

    /* U+002E "." */
    0xd, 0x80,

    /* U+002F "/" */
    0xb, 0xd, 0x5, 0x21, 0x61, 0x28, 0x28, 0x34,
    0x14, 0x84, 0x86,

    /* U+0030 "0" */
    0x1f, 0x40, 0xc1, 0x8c, 0x9, 0x50, 0x19, 0x80,
    0xcc, 0x9, 0x30, 0x60, 0x7d, 0x0,

    /* U+0031 "1" */
    0x5, 0x9b, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85,
    0x80,

    /* U+0032 "2" */
    0x2f, 0x43, 0x83, 0x43, 0x48, 0x6c, 0x36, 0x85,
    0x21, 0x48, 0x4f, 0xe8,

    /* U+0033 "3" */
    0x2f, 0x43, 0x3, 0x43, 0x61, 0x5c, 0x1d, 0x87,
    0x4a, 0xc1, 0x82, 0xf2, 0x0,

    /* U+0034 "4" */
    0xa, 0xc3, 0x78, 0x2d, 0xc1, 0x26, 0xc, 0x19,
    0x7f, 0x6, 0xc3, 0xb0,

    /* U+0035 "5" */
    0x2f, 0x92, 0x43, 0x41, 0xdf, 0x40, 0x41, 0x40,
    0xcc, 0x60, 0xa0, 0xfa, 0x0,

    /* U+0036 "6" */
    0x5, 0xa0, 0xa0, 0x6c, 0x3b, 0xb8, 0x70, 0x49,
    0x1, 0x8c, 0x12, 0xf, 0x40,

    /* U+0037 "7" */
    0x7f, 0x41, 0xa4, 0x36, 0x1a, 0x43, 0x61, 0x98,
    0x36, 0x19, 0x82,

    /* U+0038 "8" */
    0x1f, 0x40, 0xc1, 0xa6, 0xc, 0xf, 0xa0, 0x60,
    0xd5, 0x1, 0x8c, 0x12, 0x3e, 0x80,

    /* U+0039 "9" */
    0x2f, 0x23, 0x83, 0x18, 0x12, 0xb0, 0x68, 0xf4,
    0x43, 0x61, 0x9c, 0xf, 0x0,

    /* U+003A ":" */
    0x30, 0xfe, 0xc0,

    /* U+003B ";" */
    0x60, 0xfc, 0xd1, 0x0,

    /* U+003C "<" */
    0x9, 0x87, 0x8b, 0xc, 0xf0, 0x66,

    /* U+003D "=" */
    0x3f, 0x21, 0xf7, 0xe4,

    /* U+003E ">" */
    0x60, 0xe7, 0x90, 0xce, 0x8f, 0x5, 0x6,

    /* U+003F "?" */
    0x2f, 0xc, 0x30, 0x4c, 0x16, 0x16, 0x85, 0x7,
    0xf6, 0x0,

    /* U+0040 "@" */
    0xa, 0xb2, 0x1a, 0xd, 0x20, 0x81, 0x44, 0x81,
    0xa, 0x31, 0x14, 0x60, 0x85, 0xc6, 0x8, 0x5a,
    0x31, 0x88, 0x19, 0x46, 0x88, 0x18, 0x3f, 0x9a,
    0x90, 0x80,

    /* U+0041 "A" */
    0xb, 0xf, 0x3c, 0x1d, 0x98, 0x76, 0x48, 0x52,
    0x30, 0x5f, 0xc0, 0x60, 0xa4, 0x90, 0xd8,

    /* U+0042 "B" */
    0x3f, 0x20, 0xc2, 0xc1, 0x81, 0xc1, 0xf9, 0x6,
    0x16, 0xc, 0x2c, 0x18, 0x58, 0x3f, 0x20,

    /* U+0043 "C" */
    0x5, 0xe4, 0x1a, 0xd, 0x30, 0x96, 0x83, 0x98,
    0x3d, 0x84, 0xa3, 0x41, 0xa0, 0xbc, 0x80,

    /* U+0044 "D" */
    0x3e, 0x82, 0xc0, 0xe0, 0xc2, 0x93, 0x9, 0x8c,
    0x26, 0x30, 0xa4, 0xc0, 0xe0, 0xfa, 0x0,

    /* U+0045 "E" */
    0x3f, 0x46, 0x1d, 0x87, 0x7e, 0x4c, 0x3b, 0xe,
    0xc3, 0xbf, 0x80,

    /* U+0046 "F" */
    0x3f, 0x46, 0x1d, 0x87, 0x61, 0xdf, 0x81, 0x87,
    0x61, 0xd8, 0x60,

    /* U+0047 "G" */
    0x5, 0xe4, 0x1a, 0xd, 0x30, 0xca, 0x83, 0x98,
    0x1e, 0x8c, 0x26, 0x34, 0xc, 0xb, 0xd0,

    /* U+0048 "H" */
    0x30, 0xd9, 0x86, 0xcc, 0x36, 0x7f, 0x8c, 0x36,
    0x61, 0xb3, 0xd, 0x98, 0x6c,

    /* U+0049 "I" */
    0x30, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,

    /* U+004A "J" */
    0xd, 0x87, 0x61, 0xd8, 0x76, 0x1d, 0x87, 0x65,
    0x1, 0x82, 0xf2, 0x0,

    /* U+004B "K" */
    0x30, 0xb0, 0x60, 0xd0, 0x66, 0x85, 0xd8, 0x6e,
    0xd0, 0xb0, 0x61, 0x60, 0x70, 0x61, 0x50,

    /* U+004C "L" */
    0x30, 0xec, 0x3b, 0xe, 0xc3, 0xb0, 0xec, 0x3b,
    0xe, 0xfd, 0x0,

    /* U+004D "M" */
    0x34, 0x37, 0x1e, 0xd, 0xc7, 0x82, 0xa8, 0xd8,
    0x1b, 0x19, 0x8c, 0xc6, 0x4e, 0x31, 0x83, 0xc3,
    0x18, 0x34, 0x60,

    /* U+004E "N" */
    0x34, 0x2c, 0xf0, 0x59, 0xd0, 0x19, 0x98, 0x33,
    0x1c, 0xcc, 0x14, 0xcc, 0x2f, 0x18, 0x4e,

    /* U+004F "O" */
    0x5, 0xe4, 0x1a, 0xd, 0x30, 0x9d, 0x83, 0x6c,
    0x1b, 0x30, 0x9c, 0xd0, 0x68, 0x2f, 0x20,

    /* U+0050 "P" */
    0x3f, 0x20, 0xc2, 0xd3, 0xa, 0x4c, 0x2d, 0x3f,
    0x20, 0xc3, 0xd8, 0x7b, 0xe,

    /* U+0051 "Q" */
    0x5, 0xe4, 0x1a, 0xd, 0x30, 0x99, 0x83, 0x6c,
    0x1b, 0x70, 0x98, 0xd0, 0x68, 0x2f, 0x83, 0xd4,
    0xf, 0x80,

    /* U+0052 "R" */
    0x3f, 0x20, 0xc2, 0xc1, 0x85, 0x83, 0xb, 0x7,
    0xe0, 0xb0, 0x61, 0x60, 0x60, 0x61, 0x68,

    /* U+0053 "S" */
    0x1f, 0x82, 0xc0, 0xe0, 0xc2, 0x41, 0x68, 0x7a,
    0xd0, 0xf6, 0x38, 0x58, 0x1f, 0x90,

    /* U+0054 "T" */
    0xbf, 0x90, 0xb0, 0xf6, 0x1e, 0xc3, 0xd8, 0x7b,
    0xf, 0x61, 0xec, 0x20,

    /* U+0055 "U" */
    0x70, 0xa5, 0x61, 0x4a, 0xc2, 0x95, 0x85, 0x2b,
    0xa, 0x4c, 0x29, 0x30, 0xb0, 0xbe, 0x40,

    /* U+0056 "V" */
    0x90, 0x99, 0xc2, 0xd3, 0xb, 0x4, 0x8c, 0x16,
    0x61, 0xb7, 0xc, 0xe8, 0x76, 0x10,

    /* U+0057 "W" */
    0x90, 0x50, 0x12, 0xa0, 0x78, 0x18, 0x30, 0x58,
    0x30, 0x62, 0xa5, 0x40, 0x98, 0x68, 0x81, 0xb0,
    0x70, 0x6e, 0x5, 0x86, 0xd0, 0x30, 0x0,

    /* U+0058 "X" */
    0x70, 0xb0, 0x50, 0xa0, 0x5b, 0x86, 0x74, 0x33,
    0xa1, 0xb7, 0xa, 0x4a, 0xe, 0x16, 0x80,

    /* U+0059 "Y" */
    0xa0, 0x58, 0x30, 0x30, 0x28, 0x61, 0xba, 0x6,
    0xb0, 0xf6, 0x1e, 0xc3, 0xd8, 0x40,

    /* U+005A "Z" */
    0x7f, 0x83, 0xa4, 0x33, 0x87, 0x61, 0xd2, 0x19,
    0xc3, 0xb0, 0xeb, 0xf8,

    /* U+005B "[" */
    0x38, 0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x60,
    0xc1, 0xc0,

    /* U+005C "\\" */
    0x90, 0xd0, 0x6c, 0x33, 0x6, 0xc3, 0x48, 0x4c,
    0x1b, 0xc, 0x80,

    /* U+005D "]" */
    0xf0, 0x30, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,
    0x33, 0xc0,

    /* U+005E "^" */
    0x1c, 0x2b, 0xb, 0x61, 0x46, 0x0,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0x10, 0x40,

    /* U+0061 "a" */
    0x2f, 0x20, 0x41, 0x81, 0xa6, 0x38, 0x31, 0x81,
    0x82, 0xe8, 0x80,

    /* U+0062 "b" */
    0x70, 0xce, 0x19, 0xee, 0x1c, 0x12, 0xb0, 0x33,
    0x81, 0x9c, 0x12, 0xbb, 0x80,

    /* U+0063 "c" */
    0x1f, 0x20, 0xc1, 0x8c, 0x19, 0x83, 0xb0, 0x40,
    0x7c, 0x80,

    /* U+0064 "d" */
    0xd, 0x21, 0xa4, 0x7e, 0x4c, 0x1a, 0xa0, 0x4a,
    0x81, 0x26, 0x9, 0x1e, 0xd0,

    /* U+0065 "e" */
    0x1f, 0x40, 0xc1, 0x8f, 0xe5, 0x41, 0xd8, 0x10,
    0x3e, 0x80,

    /* U+0066 "f" */
    0x7, 0x12, 0xc, 0x17, 0x93, 0xb, 0xb, 0xb,
    0xb, 0x0,

    /* U+0067 "g" */
    0x1f, 0x49, 0x83, 0x54, 0x9, 0x50, 0x24, 0xc1,
    0xa3, 0xf2, 0x20, 0xc1, 0x79, 0x0,

    /* U+0068 "h" */
    0x70, 0xce, 0x19, 0xee, 0x1c, 0x18, 0xe0, 0x95,
    0x82, 0x56, 0x9, 0x58, 0x24,

    /* U+0069 "i" */
    0x30, 0xb3, 0x33, 0x33,

    /* U+006A "j" */
    0x4, 0x1d, 0x83, 0x6, 0xc, 0x18, 0x31, 0xdd,

    /* U+006B "k" */
    0x70, 0xce, 0x19, 0xc7, 0x1d, 0xc0, 0xf8, 0x27,
    0xb0, 0x39, 0x23, 0x83, 0x0,

    /* U+006C "l" */
    0x33, 0x33, 0x33, 0x33,

    /* U+006D "m" */
    0x7b, 0x9f, 0x2b, 0x7, 0xe, 0xc0, 0x90, 0x6c,
    0x9, 0x6, 0xc0, 0x90, 0x6c, 0x9, 0x6,

    /* U+006E "n" */
    0x7b, 0x87, 0x6, 0x38, 0x25, 0x60, 0x95, 0x82,
    0x56, 0x9,

    /* U+006F "o" */
    0x1f, 0x40, 0xc1, 0x2a, 0x3, 0x30, 0x18, 0xc1,
    0x23, 0xe8,

    /* U+0070 "p" */
    0x7b, 0x87, 0x4, 0xac, 0xc, 0xe0, 0x67, 0x4,
    0xae, 0xe1, 0xc3, 0x38, 0x60,

    /* U+0071 "q" */
    0x1f, 0x49, 0x82, 0x54, 0x9, 0x50, 0x24, 0xc1,
    0x23, 0xf2, 0x1a, 0x43, 0x48,

    /* U+0072 "r" */
    0xc, 0xf9, 0x58, 0x1c, 0xe, 0x7, 0x3, 0x80,

    /* U+0073 "s" */
    0x2f, 0x20, 0xc1, 0x83, 0x90, 0xcd, 0x8c, 0xc,
    0x17, 0x90,

    /* U+0074 "t" */
    0x10, 0xb0, 0x5e, 0x6, 0x16, 0x16, 0x16, 0x15,
    0x80,

    /* U+0075 "u" */
    0x60, 0x4a, 0x81, 0x2a, 0x4, 0xa8, 0x12, 0x60,
    0xd2, 0xf9,

    /* U+0076 "v" */
    0x90, 0x63, 0xc, 0xc, 0x90, 0x4e, 0x13, 0xc1,
    0xb0, 0x80,

    /* U+0077 "w" */
    0x91, 0x81, 0x8c, 0x58, 0xc0, 0xce, 0x64, 0x1a,
    0xd6, 0x15, 0x83, 0x83, 0x60, 0x90, 0x0,

    /* U+0078 "x" */
    0x61, 0xc1, 0xb8, 0x6e, 0xd, 0xc1, 0x6e, 0x6,
    0x6, 0x0,

    /* U+0079 "y" */
    0x90, 0x63, 0xc, 0xc, 0xc2, 0x9c, 0x36, 0x86,
    0xc3, 0xa0, 0x9c, 0x30,

    /* U+007A "z" */
    0x7f, 0x6, 0xd0, 0xa8, 0x13, 0x86, 0xc3, 0x5f,
    0x80,

    /* U+007B "{" */
    0x2, 0x81, 0x81, 0x40, 0x90, 0x49, 0x61, 0x48,
    0x24, 0xa, 0x3, 0x84, 0xa0,

    /* U+007C "|" */
    0x22, 0x22, 0x22, 0x22, 0x21,

    /* U+007D "}" */
    0x83, 0x61, 0x61, 0x61, 0x48, 0x5a, 0x48, 0x30,
    0xb0, 0xb0, 0x41, 0x0,

    /* U+007E "~" */
    0x2e, 0x4, 0x43, 0xe0,

    /* U+F00C "" */
    0xf, 0xfe, 0x3b, 0xc1, 0xf3, 0xf0, 0x86, 0x7e,
    0x3d, 0x1, 0xf8, 0x17, 0xa7, 0xe0, 0xd7, 0xf0,
    0x7a, 0xf8, 0x3f, 0x50, 0x3c,

    /* U+F00D "" */
    0x60, 0x9c, 0xf8, 0x7c, 0x9f, 0xd0, 0x5f, 0x41,
    0x3f, 0x80, 0xfa, 0xf8, 0xf4, 0xf, 0x2c, 0x1a,
    0x0,

    /* U+F0E7 "" */
    0x7, 0xd0, 0x4f, 0xd0, 0x57, 0xc1, 0xaf, 0x96,
    0x3f, 0xd1, 0xfe, 0x2, 0xde, 0x83, 0xbc, 0x1c,
    0xe8, 0x75, 0x87, 0xb4, 0x3f, 0xe0,

    /* U+F15B "" */
    0x5e, 0xd, 0xfd, 0x60, 0xfe, 0xbc, 0x7f, 0xa,
    0x3f, 0xf3, 0xff, 0x9f, 0xfc, 0xff, 0xe7, 0xff,
    0x3f, 0xf9, 0xff, 0xca, 0xaf, 0x80,

    /* U+F1EB "" */
    0xf, 0xfe, 0x2b, 0x7e, 0x90, 0xcf, 0xeb, 0xf9,
    0x2f, 0x21, 0xeb, 0xca, 0x82, 0x68, 0x85, 0xa1,
    0x5f, 0xe4, 0x39, 0xf2, 0x7, 0xc1, 0xe4, 0x3f,
    0xf8, 0x9e, 0xf, 0xf9, 0xf0, 0x7f, 0xf0, 0x24,
    0x3c,

    /* U+F287 "" */
    0xf, 0xfe, 0x45, 0xe0, 0xff, 0x65, 0x87, 0x3a,
    0x41, 0xe4, 0x1f, 0x5d, 0x7b, 0xcd, 0xc0, 0x60,
    0xd4, 0xf, 0xb0, 0xff, 0xe0, 0xb7, 0xa0, 0xff,
    0xae, 0xf, 0xfe, 0x28,

    /* U+F7C2 "" */
    0x9, 0x70, 0x4f, 0xf8, 0x76, 0x23, 0x7c, 0xc4,
    0x6f, 0xfe, 0x7f, 0xf3, 0xff, 0x9f, 0xfc, 0xff,
    0xe7, 0xff, 0x3f, 0xf8, 0x15, 0xe4,

    /* U+F8A2 "" */
    0xf, 0xfe, 0x56, 0x14, 0x87, 0x58, 0x3d, 0x7,
    0x59, 0xff, 0xf0, 0xfd, 0x5f, 0x1, 0xd0, 0xff,
    0x21, 0xfc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 44, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 45, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 10, .adv_w = 108, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 99, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 42, .adv_w = 129, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 109, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 31, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 76, .adv_w = 60, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 89, .adv_w = 61, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 101, .adv_w = 76, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 108, .adv_w = 100, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 35, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 120, .adv_w = 49, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 121, .adv_w = 46, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 73, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 134, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 99, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 43, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 37, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 265, .adv_w = 89, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 271, .adv_w = 97, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 275, .adv_w = 92, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 282, .adv_w = 83, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 158, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 318, .adv_w = 115, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 110, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 100, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 120, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 125, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 428, .adv_w = 48, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 110, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 95, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 154, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 125, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 121, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 111, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 535, .adv_w = 121, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 553, .adv_w = 108, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 104, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 582, .adv_w = 105, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 609, .adv_w = 112, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 156, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 110, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 106, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 105, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 687, .adv_w = 47, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 697, .adv_w = 72, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 708, .adv_w = 47, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 718, .adv_w = 74, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 724, .adv_w = 79, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 725, .adv_w = 54, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 727, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 92, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 761, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 774, .adv_w = 93, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 61, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 794, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 808, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 43, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 825, .adv_w = 42, .box_w = 3, .box_h = 10, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 833, .adv_w = 89, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 43, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 154, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 865, .adv_w = 97, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 875, .adv_w = 100, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 885, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 898, .adv_w = 100, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 911, .adv_w = 60, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 91, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 929, .adv_w = 57, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 938, .adv_w = 97, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 948, .adv_w = 85, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 958, .adv_w = 132, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 973, .adv_w = 87, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 983, .adv_w = 83, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 995, .adv_w = 87, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1004, .adv_w = 60, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1017, .adv_w = 43, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1022, .adv_w = 60, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1034, .adv_w = 120, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1038, .adv_w = 176, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1059, .adv_w = 121, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1076, .adv_w = 110, .box_w = 8, .box_h = 12, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 1098, .adv_w = 132, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1120, .adv_w = 220, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1153, .adv_w = 220, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1181, .adv_w = 132, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1203, .adv_w = 177, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 0}
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
    .bpp = 2,
    .kern_classes = 1,
    .bitmap_format = 2,
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
    .line_height = 13,          /*The maximum line height required by the font*/
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

