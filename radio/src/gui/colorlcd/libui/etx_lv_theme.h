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

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#include "colors.h"
#include "fonts.h"

/*********************
 *      Layout
 *********************/

#if LCD_W > LCD_H
  #define LANDSCAPE true
  #define PORTRAIT false
#else
  #define LANDSCAPE false
  #define PORTRAIT true
#endif

#if PORTRAIT || LCD_W == 320
  #define NARROW_LAYOUT true
#else
  #define NARROW_LAYOUT false
#endif

#if LANDSCAPE
  #if LCD_W == 320
    #define LAYOUT_SCALE(x) (((x) * 4 + 3) / 5)
  #elif LCD_W == 800
    #define LAYOUT_SCALE(x) (((x) * 3 + 1) / 2)
  #endif
#endif

#if !defined(LAYOUT_SCALE)
  #define LAYOUT_SCALE(x) (x)
#endif

// Macros for setting up layout values
#if NARROW_LAYOUT
  #define LV(standard, narrow) LAYOUT_SCALE(narrow)
#else
  #define LV(standard, narrow) LAYOUT_SCALE(standard)
#endif

enum PaddingSize {
  PAD_ZERO = 0,
  PAD_TINY = LAYOUT_SCALE(2),
  PAD_SMALL = LAYOUT_SCALE(4),
  PAD_MEDIUM = LAYOUT_SCALE(6),
  PAD_LARGE = LAYOUT_SCALE(8),
  PAD_SCROLL = LAYOUT_SCALE(3),
  PAD_THREE = PAD_SCROLL,
  PAD_TABLE_V = LAYOUT_SCALE(7),
  PAD_TABLE_H = PAD_SMALL,
  PAD_OUTLINE = PAD_TINY,
  PAD_BORDER = 2,
};

#define LAYOUT_VAL_SCALED(name, value) \
  constexpr coord_t name = LAYOUT_SCALE(value);
#define LAYOUT_VAL_SCALED_EVEN(name, value) \
  constexpr coord_t name = (LAYOUT_SCALE(value) + 1) & 0xFFFE;
#define LAYOUT_VAL_SCALED_ODD(name, value) \
  constexpr coord_t name = (LAYOUT_SCALE(value) & 0xFFFE) + 1;

// Layout values based on available width (standard or narrow)
#define LAYOUT_SIZE_SCALED(name, standard, narrow) \
  constexpr coord_t name = LV(standard, narrow);
#define LAYOUT_SIZE_SCALED_EVEN(name, standard, narrow) \
  constexpr coord_t name = (LV(standard, narrow) + 1) & 0xFFFE;
#define LAYOUT_SIZE_SCALED_ODD(name, standard, narrow) \
  constexpr coord_t name = (LV(standard, narrow) & 0xFFFE) + 1;

// Macro for value which only differ by whether layout is normal or narrow (no scaling)
#if NARROW_LAYOUT
#define LAYOUT_SIZE(name, standard, narrow) \
  constexpr int name = narrow;
#else
#define LAYOUT_SIZE(name, standard, narrow) \
  constexpr int name = standard;
#endif

// Layout values based on orientation (landscape or portrait)
#if LANDSCAPE
  #define LAYOUT_ORIENTATION_SCALED(name, landscape, portrait) \
    constexpr coord_t name = LAYOUT_SCALE(landscape);
  #define LAYOUT_ORIENTATION(name, landscape, portrait) \
    constexpr int name = landscape;
#else
  #define LAYOUT_ORIENTATION_SCALED(name, landscape, portrait) \
    constexpr coord_t name = LAYOUT_SCALE(portrait);
  #define LAYOUT_ORIENTATION(name, landscape, portrait) \
    constexpr int name = portrait;
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void usePreviewStyle();
void useMainStyle();

lv_obj_t* etx_create(const lv_obj_class_t* class_p, lv_obj_t* parent);
lv_obj_t* etx_textarea_create(lv_obj_t* parent);
lv_obj_t* window_create(lv_obj_t* parent);

void etx_std_style(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN,
                   PaddingSize padding = PAD_ZERO);

void etx_btn_style(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);

void etx_std_ctrl_colors(lv_obj_t* obj,
                         lv_style_selector_t selector = LV_PART_MAIN);

void etx_scrollbar(lv_obj_t* obj);

void etx_padding(lv_obj_t* obj, PaddingSize padding,
                 lv_style_selector_t selector = LV_PART_MAIN);

void etx_solid_bg(lv_obj_t* obj,
                  LcdColorIndex bg_color = COLOR_THEME_SECONDARY3_INDEX,
                  lv_style_selector_t selector = LV_PART_MAIN);

void etx_font(lv_obj_t* obj, FontIndex fontIdx,
              lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_bg_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_bg_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                  lv_style_selector_t selector = LV_PART_MAIN);
void etx_bg_color_from_flags(lv_obj_t* obj, LcdFlags colorFlags,
                             lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_txt_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_txt_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                   lv_style_selector_t selector = LV_PART_MAIN);
void etx_txt_color_from_flags(lv_obj_t* obj, LcdFlags colorFlags,
                              lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_border_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_border_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                   lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_arc_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_arc_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                  lv_style_selector_t selector = LV_PART_MAIN);
void etx_arc_color_from_flags(lv_obj_t* obj, LcdFlags colorFlags,
                             lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_line_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_line_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                  lv_style_selector_t selector = LV_PART_MAIN);
void etx_line_color_from_flags(lv_obj_t* obj, LcdFlags colorFlags,
                             lv_style_selector_t selector = LV_PART_MAIN);

void etx_remove_img_color(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN);
void etx_img_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                   lv_style_selector_t selector = LV_PART_MAIN);

// Create a style with a single property
#define LV_STYLE_CONST_SINGLE_INIT(var_name, prop, value)               \
  const lv_style_t var_name = {.v_p = {.value1 = {.num = value}},       \
                               .prop1 = prop,                           \
                               .has_group = 1 << ((prop & 0x1FF) >> 4), \
                               .prop_cnt = 1}

// Create a style with multiple properties
// Copied from lv_style.h and modified to compile with ARM GCC C++
#define LV_STYLE_CONST_MULTI_INIT(var_name, prop_array)            \
  const lv_style_t var_name = {.v_p = {.const_props = prop_array}, \
                               .prop1 = LV_STYLE_PROP_ANY,         \
                               .has_group = 0xFF,                  \
                               .prop_cnt = (sizeof(prop_array) / sizeof((prop_array)[0]))}

extern const lv_obj_class_t window_base_class;
extern const lv_obj_class_t field_edit_class;
extern const lv_obj_class_t button_class;

// Color and font styles
class EdgeTxStyles
{
 public:
  // Colors
  lv_style_t bg_color[TOTAL_COLOR_COUNT];
  lv_style_t txt_color[TOTAL_COLOR_COUNT];
  lv_style_t img_color[TOTAL_COLOR_COUNT];
  lv_style_t border_color[TOTAL_COLOR_COUNT];
  lv_style_t arc_color[TOTAL_COLOR_COUNT];
  lv_style_t line_color[TOTAL_COLOR_COUNT];
  lv_style_t outline_color_light;
  lv_style_t outline_color_normal;
  lv_style_t outline_color_focus;
  lv_style_t outline_color_edit;
  lv_style_t graph_border;
  lv_style_t graph_dashed;
  lv_style_t graph_line;
  lv_style_t graph_position_line;
  lv_style_t div_line;
  lv_style_t div_line_edit;
  lv_style_t div_line_warn;
  lv_style_t div_line_black;
  lv_style_t div_line_white;

  // Fonts
  lv_style_t font[FONTS_COUNT];

  // Static styles
  static const lv_style_t pad_zero;
  static const lv_style_t pad_tiny;
  static const lv_style_t pad_small;
  static const lv_style_t pad_medium;
  static const lv_style_t pad_large;
  static const lv_style_t pad_left_2;
  static const lv_style_t pad_button;
  static const lv_style_t pad_textarea;
  static const lv_style_t text_align_left;
  static const lv_style_t text_align_right;
  static const lv_style_t text_align_center;
  static const lv_style_t bg_opacity_transparent;
  static const lv_style_t bg_opacity_20;
  static const lv_style_t bg_opacity_50;
  static const lv_style_t bg_opacity_75;
  static const lv_style_t bg_opacity_cover;
  static const lv_style_t fg_opacity_transparent;
  static const lv_style_t fg_opacity_cover;
  static const lv_style_t rounded;
  static const lv_style_t circle;
  static const lv_style_t disabled;
  static const lv_style_t pressed;
  static const lv_style_t scrollbar;
  static const lv_style_t border;
  static const lv_style_t border_transparent;
  static const lv_style_t border_thin;
  static const lv_style_t outline;
  
  EdgeTxStyles();

  void init();
  void applyColors();

  static LAYOUT_VAL_SCALED(STD_FONT_HEIGHT, 21)
  static LAYOUT_VAL_SCALED(UI_ELEMENT_HEIGHT, 32)
  static LAYOUT_VAL_SCALED(MENU_HEADER_HEIGHT, 45)
  static LAYOUT_VAL_SCALED(EDIT_FLD_WIDTH_NARROW, 70)
  static LAYOUT_VAL_SCALED(EDIT_FLD_WIDTH, 100)

 protected:
  bool initDone = false;
};

extern EdgeTxStyles* styles;

#define etx_obj_add_style(obj, style, part) \
  lv_obj_add_style(obj, (lv_style_t*)&(style), part)
