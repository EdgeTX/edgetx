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

#include "input_mix_button.h"
#include "opentx.h"

// icon: 17 x 17
static const uint8_t _mask_textline_fm[] = {
#include "mask_textline_fm.lbm"
};
STATIC_LZ4_BITMAP(mask_textline_fm);

// total: 92 x 17
#define FM_CANVAS_HEIGHT 17
#define FM_CANVAS_WIDTH  92

#if LCD_W > LCD_H // Landscape
static const lv_coord_t col_dsc[] = {
  LV_GRID_FR(7),   // weight
  LV_GRID_FR(12),   // source
  LV_GRID_FR(29),   // opts
  FM_CANVAS_WIDTH, // flight modes
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#else // Portrait
static const lv_coord_t col_dsc[] = {
  LV_GRID_FR(13),   // weight
  LV_GRID_FR(21),   // source
  LV_GRID_FR(32),   // opts
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

InputMixButton::InputMixButton(Window* parent, uint8_t index) :
    ListLineButton(parent, index)
{
  lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(lvobj, col_dsc, row_dsc);

  weight = lv_label_create(lvobj);
  lv_obj_set_grid_cell(weight, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  
  source = lv_label_create(lvobj);
  lv_obj_set_grid_cell(source, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);

  opts = lv_label_create(lvobj);
#if LCD_W > LCD_H // Landscape
  lv_obj_set_grid_cell(opts, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 0, 1);
#else
  lv_obj_set_grid_cell(opts, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 0, 2);
#endif
}

InputMixButton::~InputMixButton()
{
  if (fm_buffer) free(fm_buffer);
}

void InputMixButton::setWeight(gvar_t value, gvar_t min, gvar_t max)
{
  char s[32];
  getValueOrGVarString(s, sizeof(s), value, min, max, 0, "%");
  lv_label_set_text(weight, s);
}

void InputMixButton::setSource(mixsrc_t idx)
{
  char* s = getSourceString(idx);
  lv_label_set_text(source, s);
}

void InputMixButton::setFlightModes(uint16_t modes)
{
  if (!modelFMEnabled()) return;
  if (modes == fm_modes) return;
  fm_modes = modes;

  if (!fm_modes) {
    if (!fm_canvas) return;
    lv_obj_del(fm_canvas);
    free(fm_buffer);
    fm_canvas = nullptr;
    fm_buffer = nullptr;
    return;
  }

  if (!fm_canvas) {
    fm_canvas = lv_canvas_create(lvobj);
    fm_buffer = malloc(FM_CANVAS_WIDTH * FM_CANVAS_HEIGHT);
    lv_canvas_set_buffer(fm_canvas, fm_buffer, FM_CANVAS_WIDTH,
                         FM_CANVAS_HEIGHT, LV_IMG_CF_ALPHA_8BIT);

#if LCD_W > LCD_H // Landscape
    lv_obj_set_grid_cell(fm_canvas, LV_GRID_ALIGN_START, 3, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
#else
    lv_obj_set_grid_cell(fm_canvas, LV_GRID_ALIGN_CENTER, 0, 2,
                         LV_GRID_ALIGN_CENTER, 1, 1);
#endif

    lv_obj_set_style_img_recolor(fm_canvas, makeLvColor(COLOR_THEME_SECONDARY1), 0);
    lv_obj_set_style_img_recolor_opa(fm_canvas, LV_OPA_COVER, 0);
  }

  lv_canvas_fill_bg(fm_canvas, lv_color_black(), LV_OPA_TRANSP);

  auto mask = (const uint8_t*)mask_textline_fm;
  auto mask_hdr = (const uint16_t*)mask;
  lv_coord_t w = mask_hdr[0];
  lv_coord_t h = mask_hdr[1];

  coord_t x = 0;
  lv_canvas_copy_buf(fm_canvas, mask + 4, x, 0, w, h);
  x += 20;

  lv_draw_label_dsc_t label_dsc;
  lv_draw_label_dsc_init(&label_dsc);

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = LV_OPA_COVER;
 
  const lv_font_t* font = getFont(FONT(XS));
  label_dsc.font = font;
  
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    char s[] = " ";
    s[0] = '0' + i;
    if (fm_modes & (1 << i)) {
      label_dsc.color = lv_color_make(0x7f, 0x7f, 0x7f);
    } else {
      lv_canvas_draw_rect(fm_canvas, x, 0, 8, 3, &rect_dsc);
      label_dsc.color = lv_color_white();
    }
    lv_canvas_draw_text(fm_canvas, x, 0, 8, &label_dsc, s);
    x += 8;
  }
}
