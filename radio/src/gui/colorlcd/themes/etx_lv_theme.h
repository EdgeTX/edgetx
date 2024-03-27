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

#if LANDSCAPE_LCD_SMALL
enum PaddingSize {
  PAD_ZERO = 0,
  PAD_TINY = 1,
  PAD_SMALL = 3,
  PAD_MEDIUM = 4,
  PAD_LARGE = 6
};
#else
enum PaddingSize {
  PAD_ZERO = 0,
  PAD_TINY = 2,
  PAD_SMALL = 4,
  PAD_MEDIUM = 6,
  PAD_LARGE = 8
};
#endif

// Macros for setting up layout values
//  LAYOUT_VAL1 - single value where portait and landscape use the same value
//                and landscape small screen = value * 2 / 3
//  LAYOUT_VAL2 - 2 value, first is landscape, second is portrait
//                and landscape small screen = landscape * 2 / 3
//  LAYOUT_VAL3 - 3 values - landscape, landscape small, portrait

#if LANDSCAPE_LCD
#define LAYOUT_VAL1(name, landscape) \
  constexpr coord_t name = landscape;
#define LAYOUT_VAL2(name, landscape, portrait) \
  constexpr coord_t name = landscape;
#define LAYOUT_VAL3(name, landscape, landscape_small, portrait) \
  constexpr coord_t name = landscape;
#elif LANDSCAPE_LCD_SMALL
#define LAYOUT_VAL1(name, landscape) \
  constexpr coord_t name = (landscape * 4 + 3) / 6;
#define LAYOUT_VAL2(name, landscape, portrait) \
  constexpr coord_t name = (landscape * 4 + 3) / 6;
#define LAYOUT_VAL3(name, landscape, landscape_small, portrait) \
  constexpr coord_t name = landscape_small;
#else
#define LAYOUT_VAL1(name, landscape) \
  constexpr coord_t name = landscape;
#define LAYOUT_VAL2(name, landscape, portrait) \
  constexpr coord_t name = portrait;
#define LAYOUT_VAL3(name, landscape, landscape_small, portrait) \
  constexpr coord_t name = portrait;
#endif

LAYOUT_VAL1(MENU_HEADER_BUTTON_WIDTH, 33)
LAYOUT_VAL1(MENU_HEADER_BUTTONS_LEFT, 47)
LAYOUT_VAL1(MENU_HEADER_HEIGHT, 45)
LAYOUT_VAL1(MENU_TITLE_TOP, 48)
LAYOUT_VAL1(MENU_TITLE_HEIGHT, 21)

constexpr coord_t MENU_BODY_TOP = MENU_TITLE_TOP + MENU_TITLE_HEIGHT;
constexpr coord_t MENU_BODY_HEIGHT = LCD_H - MENU_BODY_TOP;

LAYOUT_VAL1(MENUS_MARGIN_LEFT, 6)
LAYOUT_VAL1(PAGE_LINE_HEIGHT, 20)

constexpr coord_t FH = PAGE_LINE_HEIGHT;
constexpr coord_t NUM_BODY_LINES = MENU_BODY_HEIGHT / PAGE_LINE_HEIGHT;

constexpr coord_t PAGE_TITLE_TOP = 2;

LAYOUT_VAL1(PAGE_TITLE_LEFT, 50)

LAYOUT_VAL1(HDR_DATE_WIDTH, 45)
LAYOUT_VAL1(HDR_DATE_HEIGHT, 12)
LAYOUT_VAL1(HDR_DATE_LINE2, 15)
LAYOUT_VAL1(HDR_DATE_XO, 48)
LAYOUT_VAL1(HDR_DATE_Y, 6)
LAYOUT_VAL1(HDR_DATE_FULL_WIDTH, 51)

LAYOUT_VAL3(UI_ELEMENT_HEIGHT, 32, 24, 32)

LAYOUT_VAL1(TOOLS_BTN_H, 48)
LAYOUT_VAL1(SUBSCR_BTN_H, 62)
LAYOUT_VAL1(MENUS_LINE_HEIGHT, 35)

#if PORTRAIT_LCD
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 10);
#else
constexpr coord_t MENUS_MAX_HEIGHT = (MENUS_LINE_HEIGHT * 7) + 8;
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the theme
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param font pointer to a font to use.
 * @return a pointer to reference this theme later
 */
lv_theme_t* etx_lv_theme_init(lv_disp_t* disp, lv_color_t color_primary,
                              lv_color_t color_secondary,
                              const lv_font_t* font);

void usePreviewStyle();
void useMainStyle();

lv_obj_t* etx_create(const lv_obj_class_t* class_p, lv_obj_t* parent);
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

void etx_bg_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                  lv_style_selector_t selector = LV_PART_MAIN);

void etx_txt_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                   lv_style_selector_t selector = LV_PART_MAIN);

void etx_img_color(lv_obj_t* obj, LcdColorIndex colorIdx,
                   lv_style_selector_t selector = LV_PART_MAIN);

void etx_textarea_style(lv_obj_t* obj);

// Create a style with a single property
#define LV_STYLE_CONST_SINGLE_INIT(var_name, prop, value)               \
  const lv_style_t var_name = {.v_p = {.value1 = {.num = value}},       \
                               .prop1 = prop,                           \
                               .is_const = 0,                           \
                               .has_group = 1 << ((prop & 0x1FF) >> 4), \
                               .prop_cnt = 1}

// Create a style with multiple properties
// Copied from lv_style.h and modified to compile with ARM GCC C++
#define LV_STYLE_CONST_MULTI_INIT(var_name, prop_array)            \
  const lv_style_t var_name = {.v_p = {.const_props = prop_array}, \
                               .prop1 = 0,                         \
                               .is_const = 1,                      \
                               .has_group = 0xFF,                  \
                               .prop_cnt = 0}

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
  lv_style_t border_color_dark;
  lv_style_t border_color_normal;
  lv_style_t border_color_black;
  lv_style_t border_color_white;
  lv_style_t border_color_focus;
  lv_style_t border_color_active;
  lv_style_t outline_color_light;
  lv_style_t outline_color_normal;
  lv_style_t outline_color_focus;
  lv_style_t bg_color_grey;
  lv_style_t arc_color;
  lv_style_t graph_border;
  lv_style_t graph_dashed;
  lv_style_t graph_line;
  lv_style_t graph_position_line;
  lv_style_t div_line;
  lv_style_t div_line_edit;
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
  static const lv_style_t outline_thick;

  EdgeTxStyles();

  void init();
  void applyColors();

 protected:
  bool initDone = false;
};

extern EdgeTxStyles* styles;

#define etx_obj_add_style(obj, style, part) \
  lv_obj_add_style(obj, (lv_style_t*)&(style), part)
