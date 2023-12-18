/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include <stdint.h>

/*
    Structures for handling LZ4 compressed versions of LVGL fonts
    Compressed data is uncompressed at runtime and these strucutres
    then used to rebuild the LVGL font structures.

    Note: this is dependant on the LVGL font structure formats. If
          the LVGL version changes, re-check the LVGL font structures.
*/

typedef struct {
    // Properties used to reconstitute the LVGL 'cmaps' array
    uint16_t range_start;               // lv_font_fmt_txt_cmap_t.range_start
    uint16_t range_length;              // lv_font_fmt_txt_cmap_t.range_length
    uint16_t glyph_id_start;            // lv_font_fmt_txt_cmap_t.glyph_id_start
    uint16_t list_length;               // lv_font_fmt_txt_cmap_t.list_length
    uint16_t type;                      // lv_font_fmt_txt_cmap_t.type

    // Offsets into uncompressed data to relocate pointers at runtime
    uint32_t unicode_list;              // lv_font_fmt_txt_cmap_t.unicode_list
    uint32_t glyph_id_ofs_list;         // lv_font_fmt_txt_cmap_t.glyph_id_ofs_list

} etxFontCmap;

typedef struct {
    uint32_t uncomp_size;               // Uncompressed data size
    uint32_t comp_size;                 // Compressed data size

    // Properties used to reconstitute the various LVGL font structures
    uint8_t line_height;                // lv_font_t.line_height
    uint8_t base_line;                  // lv_font_t.base_line
    uint8_t subpx;                      // lv_font_t.subpx
    int8_t underline_position;          // lv_font_t.underline_position
    int8_t underline_thickness;         // lv_font_t.underline_thickness

    uint8_t kern_scale;                 // lv_font_fmt_txt_dsc_t.kern_scale
    uint8_t cmap_num;                   // lv_font_fmt_txt_dsc_t.cmap_num
    uint8_t bpp;                        // lv_font_fmt_txt_dsc_t.bpp
    uint8_t kern_classes;               // lv_font_fmt_txt_dsc_t.kern_classes
    uint8_t bitmap_format;              // lv_font_fmt_txt_dsc_t.bitmap_format

    uint8_t left_class_cnt;             // lv_font_fmt_txt_kern_classes_t.left_class_cnt
    uint8_t right_class_cnt;            // lv_font_fmt_txt_kern_classes_t.right_class_cnt

    // Offsets into uncompressed data to relocate pointers at runtime
    // Note: lv_font_fmt_txt_dsc_t.glyph_dsc is always at the start of the uncompressed data
    uint32_t glyph_bitmap;              // lv_font_fmt_txt_dsc_t.glyph_bitmap

    uint32_t class_pair_values;         // lv_font_fmt_txt_kern_classes_t.class_pair_values
    uint32_t left_class_mapping;        // lv_font_fmt_txt_kern_classes_t.left_class_mapping
    uint32_t right_class_mapping;       // lv_font_fmt_txt_kern_classes_t.right_class_mapping

    // Pointers to cmaps, compressed data and buffer for uncompressed data
    const etxFontCmap* cmaps;
    const uint8_t* compressed;
    uint8_t* lvglFontBuf;
    uint32_t lvglFontBufSize;
} etxLz4Font;
