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

/*
    Tool to convert an LVGL font (in 'lvgl' format) to an LZ4 compressed format
    that can be uncompressed and reconstituted at runtime.

    The font to be compressed must be in the file 'lv_font.inc'.

    Usage:
        Create the lv_font.inc file using the lv_font_conv program
        Compile this program with GCC
        Run this program with the font name as the first argument

    Designed to be used by the make_fonts.sh tool, not run standalone.

    Overview:
        The font to be compressed is read from the structures in the lv_font.inc
   file. Including and compiling with this files saves the effort of writing a
   parser for the 'lvgl' font data. Data that can be compressed is collected
   into a byte array and then LZ4 compressed. The remaining data, and offsets
   needed to rebuild the LVGL font structures are collected into custom
   structures (see lz4_fonts.h). Everything is then written back out to the
   argv[1].c file to be compiled into the EdgeTx firmware.

    Note:
        This is dependant on the LVGL font structure formats. If the LVGL
   version is changed re-check that the font structure are still compatible.
*/

#include <stdio.h>
#include <stdlib.h>

#include "../../gui/colorlcd/lv_conf.h"
#define LV_CONF_SKIP 1
#include "../../gui/colorlcd/lz4_fonts.h"
#include "lv_font.inc"
#include "lz4/lz4hc.h"

// Need this to allow lvgl to compile.
bool lv_font_get_glyph_dsc_fmt_txt(const lv_font_t* font,
                                   lv_font_glyph_dsc_t* dsc_out,
                                   uint32_t unicode_letter,
                                   uint32_t unicode_letter_next)
{
  return true;
}

// Need this to allow lvgl to compile.
const uint8_t* lv_font_get_bitmap_fmt_txt(const lv_font_t* font,
                                          uint32_t letter)
{
  return nullptr;
}

int main(int argc, char* argv[])
{
  int uncomp_size = 0;
  int comp_size = 0;

  // Pointers to base LVGL font strucures
  const lv_font_t* font = &lv_font;
  lv_font_fmt_txt_dsc_t* dsc = (lv_font_fmt_txt_dsc_t*)font->dsc;

  // Calculate size of data to compress
  uncomp_size = sizeof(glyph_bitmap) + sizeof(glyph_dsc);

  if (dsc->kern_classes) {
    uncomp_size += sizeof(kern_class_values) + sizeof(kern_left_class_mapping) +
                   sizeof(kern_right_class_mapping);
  }

  for (int i = 0; i < dsc->cmap_num; i += 1) {
    if (dsc->cmaps[i].unicode_list)
      uncomp_size += dsc->cmaps[i].list_length * sizeof(uint16_t);
    if (dsc->cmaps[i].glyph_id_ofs_list)
      uncomp_size += dsc->cmaps[i].list_length * sizeof(uint8_t);
  }

  // Create custom strucutre to hold font data
  etxLz4Font etx_font;
  memset(&etx_font, 0, sizeof(etxLz4Font));

  // Copy font propeties
  etx_font.line_height = font->line_height;
  etx_font.base_line = font->base_line;
  etx_font.subpx = font->subpx;
  etx_font.underline_position = font->underline_position;
  etx_font.underline_thickness = font->underline_thickness;

  etx_font.kern_scale = dsc->kern_scale;
  etx_font.cmap_num = dsc->cmap_num;
  etx_font.bpp = dsc->bpp;
  etx_font.kern_classes = dsc->kern_classes;
  etx_font.bitmap_format = dsc->bitmap_format;

  if (dsc->kern_classes) {
    etx_font.left_class_cnt = kern_classes.left_class_cnt;
    etx_font.right_class_cnt = kern_classes.right_class_cnt;
  }

  // Create custom cmap structure array
  etxFontCmap* etx_cmaps = nullptr;
  if (dsc->cmap_num > 0) {
    etx_cmaps = (etxFontCmap*)malloc(dsc->cmap_num * sizeof(etxFontCmap));
    memset(etx_cmaps, 0, dsc->cmap_num * sizeof(etxFontCmap));

    // Copy properties
    for (int i = 0; i < dsc->cmap_num; i += 1) {
      etx_cmaps[i].range_start = dsc->cmaps[i].range_start;
      etx_cmaps[i].range_length = dsc->cmaps[i].range_length;
      etx_cmaps[i].glyph_id_start = dsc->cmaps[i].glyph_id_start;
      etx_cmaps[i].list_length = dsc->cmaps[i].list_length;
      etx_cmaps[i].type = dsc->cmaps[i].type;
    }
  }

  // Allocate data blob
  uint8_t* data = (uint8_t*)malloc(uncomp_size);
  // Pointer to next available space in data blob
  uint8_t* next = data;

  memset(data, 0, uncomp_size);

  // Copy compressable data and save offsets to each section
  // glyph_dsc is always first at offset 0 - no offset needed
  memcpy(next, glyph_dsc, sizeof(glyph_dsc));

  next += sizeof(glyph_dsc);

  // Copy cmaps array
  for (int i = 0; i < dsc->cmap_num; i += 1) {
    if (dsc->cmaps[i].unicode_list) {
      memcpy(next, dsc->cmaps[i].unicode_list,
             dsc->cmaps[i].list_length * sizeof(uint16_t));
      etx_cmaps[i].unicode_list = next - data;

      next += dsc->cmaps[i].list_length * sizeof(uint16_t);
    }
    if (dsc->cmaps[i].glyph_id_ofs_list) {
      memcpy(next, dsc->cmaps[i].glyph_id_ofs_list,
             dsc->cmaps[i].list_length * sizeof(uint8_t));
      etx_cmaps[i].glyph_id_ofs_list = next - data;

      next += dsc->cmaps[i].list_length * sizeof(uint8_t);
    }
  }

  // Copy glyph_bitmap
  memcpy(next, glyph_bitmap, sizeof(glyph_bitmap));
  etx_font.glyph_bitmap = next - data;

  next += sizeof(glyph_bitmap);

  // Copy kern_classes (optional)
  if (dsc->kern_classes) {
    memcpy(next, kern_class_values, sizeof(kern_class_values));
    etx_font.class_pair_values = next - data;

    next += sizeof(kern_class_values);

    memcpy(next, kern_left_class_mapping, sizeof(kern_left_class_mapping));
    etx_font.left_class_mapping = next - data;

    next += sizeof(kern_left_class_mapping);

    memcpy(next, kern_right_class_mapping, sizeof(kern_right_class_mapping));
    etx_font.right_class_mapping = next - data;

    next += sizeof(kern_right_class_mapping);
  }

  // Allocate array and LZ4 compress data
  uint8_t* lz4_data = (uint8_t*)malloc(uncomp_size);
  comp_size = LZ4_compress_HC((const char*)data, (char*)lz4_data, uncomp_size,
                              uncomp_size, 12);

  // Write data to file
  int i;
  char filename[100];
  snprintf(filename, 100, "%s.c", argv[1]);
  FILE* fp = fopen(filename, "w");

  fprintf(fp, "#include \"definitions.h\"\n");
  fprintf(fp, "#include \"lz4_fonts.h\"\n\n");

  // Compressed data
  fprintf(fp, "static const uint8_t lz4FontData[] ={\n");
  for (i = 0; i < comp_size; i += 1) {
    fprintf(fp, "0x%02x,", lz4_data[i]);
    if ((i & 0x0F) == 0x0F) fprintf(fp, "\n");
  }
  if ((i & 0x0F) != 0) fprintf(fp, "\n");
  fprintf(fp, "};\n\n");

  // Cmaps
  if (dsc->cmap_num > 0) {
    fprintf(fp, "static const etxFontCmap cmaps[] = {\n");
    for (int i = 0; i < dsc->cmap_num; i += 1) {
      fprintf(fp,
              "{ .range_start = %d, .range_length = %d, .glyph_id_start = %d, "
              ".list_length = %d, .type = %d, .unicode_list = %d, "
              ".glyph_id_ofs_list = %d },\n",
              etx_cmaps[i].range_start, etx_cmaps[i].range_length,
              etx_cmaps[i].glyph_id_start, etx_cmaps[i].list_length,
              etx_cmaps[i].type, etx_cmaps[i].unicode_list,
              etx_cmaps[i].glyph_id_ofs_list);
    }
    fprintf(fp, "};\n\n");
  }

  // SDRAM buffer to decompress data into
  int size = uncomp_size + sizeof(lv_font_t) +
             sizeof(lv_font_fmt_txt_dsc_t) +
             sizeof(lv_font_fmt_txt_glyph_cache_t) +
             dsc->cmap_num * sizeof(lv_font_fmt_txt_cmap_t);
  if (dsc->kern_classes) size += sizeof(lv_font_fmt_txt_kern_classes_t);
  fprintf(fp, "static uint8_t etxUncompBuf[%d] __SDRAMFONTS;\n\n", size);

  // Custom font structure
  fprintf(fp, "const etxLz4Font %s = {\n", argv[1]);
  fprintf(fp, ".uncomp_size = %d,\n", uncomp_size);
  fprintf(fp, ".comp_size = %d,\n", comp_size);
  fprintf(fp, ".line_height = %d,\n", etx_font.line_height);
  fprintf(fp, ".base_line = %d,\n", etx_font.base_line);
  fprintf(fp, ".subpx = %d,\n", etx_font.subpx);
  fprintf(fp, ".underline_position = %d,\n", etx_font.underline_position);
  fprintf(fp, ".underline_thickness = %d,\n", etx_font.underline_thickness);
  fprintf(fp, ".kern_scale = %d,\n", etx_font.kern_scale);
  fprintf(fp, ".cmap_num = %d,\n", etx_font.cmap_num);
  fprintf(fp, ".bpp = %d,\n", etx_font.bpp);
  fprintf(fp, ".kern_classes = %d,\n", etx_font.kern_classes);
  fprintf(fp, ".bitmap_format = %d,\n", etx_font.bitmap_format);
  fprintf(fp, ".left_class_cnt = %d,\n", etx_font.left_class_cnt);
  fprintf(fp, ".right_class_cnt = %d,\n", etx_font.right_class_cnt);
  fprintf(fp, ".glyph_bitmap = %d,\n", etx_font.glyph_bitmap);
  fprintf(fp, ".class_pair_values = %d,\n", etx_font.class_pair_values);
  fprintf(fp, ".left_class_mapping = %d,\n", etx_font.left_class_mapping);
  fprintf(fp, ".right_class_mapping = %d,\n", etx_font.right_class_mapping);
  fprintf(fp, ".cmaps = cmaps,\n");
  fprintf(fp, ".compressed = lz4FontData,\n");
  fprintf(fp, ".lvglFontBuf = etxUncompBuf,\n");
  fprintf(fp, ".lvglFontBufSize = %d,\n", size);
  fprintf(fp, "};\n");

  fclose(fp);

  printf("%s %d %d %d%%\n", argv[1], uncomp_size, comp_size,
         (comp_size * 100) / uncomp_size);
}
