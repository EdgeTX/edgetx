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

/*********************
 *      INCLUDES
 *********************/
#include "etx_lv_theme.h"

#include "../colors.h"
#include "font.h"

extern lv_color_t makeLvColor(uint32_t colorFlags);

/*********************
 *      DEFINES
 *********************/
#define BORDER_WIDTH 2

#if LCD_W > LCD_H
#define PAD_SMALL 10
#define PAD_TINY 3
#else
#define PAD_SMALL 8
#define PAD_TINY 2
#endif

static lv_theme_t theme;

/**********************
 *   Constant Styles
 **********************/

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

// Opacity
LV_STYLE_CONST_SINGLE_INIT(bg_opacity_transparent, LV_STYLE_BG_OPA,
                           LV_OPA_TRANSP);
LV_STYLE_CONST_SINGLE_INIT(bg_opacity_20, LV_STYLE_BG_OPA, LV_OPA_20);
LV_STYLE_CONST_SINGLE_INIT(bg_opacity_50, LV_STYLE_BG_OPA, LV_OPA_50);
LV_STYLE_CONST_SINGLE_INIT(bg_opacity_75, LV_STYLE_BG_OPA, 187);
LV_STYLE_CONST_SINGLE_INIT(bg_opacity_cover, LV_STYLE_BG_OPA, LV_OPA_COVER);
LV_STYLE_CONST_SINGLE_INIT(fg_opacity_transparent, LV_STYLE_OPA, LV_OPA_TRANSP);
LV_STYLE_CONST_SINGLE_INIT(fg_opacity_cover, LV_STYLE_OPA, LV_OPA_COVER);

// Corner rounding (button, edit box, etc)
LV_STYLE_CONST_SINGLE_INIT(rounded, LV_STYLE_RADIUS, 6);

// Toggle switch and slider knob rounding
LV_STYLE_CONST_SINGLE_INIT(circle, LV_STYLE_RADIUS, LV_RADIUS_CIRCLE);

// Animation
LV_STYLE_CONST_SINGLE_INIT(anim_fast, LV_STYLE_ANIM_TIME, 120);

// Text align
LV_STYLE_CONST_SINGLE_INIT(text_align_right, LV_STYLE_TEXT_ALIGN,
                           LV_TEXT_ALIGN_RIGHT);

// Toggle switch
const lv_style_const_prop_t switch_knob_props[] = {
    LV_STYLE_CONST_PAD_TOP(-4),  LV_STYLE_CONST_PAD_BOTTOM(-4),
    LV_STYLE_CONST_PAD_LEFT(-4), LV_STYLE_CONST_PAD_RIGHT(-4),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(switch_knob, switch_knob_props);

// Scrollbar
const lv_style_const_prop_t scrollbar_props[] = {
    LV_STYLE_CONST_PAD_TOP(6),  LV_STYLE_CONST_PAD_BOTTOM(6),
    LV_STYLE_CONST_PAD_LEFT(6), LV_STYLE_CONST_PAD_RIGHT(6),
    LV_STYLE_CONST_WIDTH(4),    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(scrollbar, scrollbar_props);

// Padding
const lv_style_const_prop_t pad_zero_props[] = {
    LV_STYLE_CONST_PAD_TOP(0),  LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PAD_LEFT(0), LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_ROW(0),  LV_STYLE_CONST_PAD_COLUMN(0),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(pad_zero, pad_zero_props);

const lv_style_const_prop_t pad_tiny_props[] = {
    LV_STYLE_CONST_PAD_TOP(PAD_TINY),
    LV_STYLE_CONST_PAD_BOTTOM(PAD_TINY),
    LV_STYLE_CONST_PAD_LEFT(PAD_TINY),
    LV_STYLE_CONST_PAD_RIGHT(PAD_TINY),
    LV_STYLE_CONST_PAD_ROW(PAD_TINY),
    LV_STYLE_CONST_PAD_COLUMN(PAD_TINY),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(pad_tiny, pad_tiny_props);

const lv_style_const_prop_t pad_small_props[] = {
    LV_STYLE_CONST_PAD_TOP(PAD_SMALL),
    LV_STYLE_CONST_PAD_BOTTOM(PAD_SMALL),
    LV_STYLE_CONST_PAD_LEFT(PAD_SMALL),
    LV_STYLE_CONST_PAD_RIGHT(PAD_SMALL),
    LV_STYLE_CONST_PAD_ROW(PAD_SMALL),
    LV_STYLE_CONST_PAD_COLUMN(PAD_SMALL),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(pad_small, pad_small_props);

// Border
const lv_style_const_prop_t border_props[] = {
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_BORDER_WIDTH(BORDER_WIDTH),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(border, border_props);

// Border
const lv_style_const_prop_t border_transparent_props[] = {
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_BORDER_WIDTH(BORDER_WIDTH),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(border_transparent, border_transparent_props);

// Button
const lv_style_const_prop_t btn_props[] = {
    LV_STYLE_CONST_PAD_TOP(PAD_SMALL),
    LV_STYLE_CONST_PAD_BOTTOM(PAD_SMALL),
    LV_STYLE_CONST_PAD_LEFT(PAD_SMALL),
    LV_STYLE_CONST_PAD_RIGHT(PAD_SMALL),
    LV_STYLE_CONST_PAD_ROW(4),
    LV_STYLE_CONST_PAD_COLUMN(4),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(btn, btn_props);

// Edit box
const lv_style_const_prop_t field_props[] = {
    LV_STYLE_CONST_PAD_TOP(4),  LV_STYLE_CONST_PAD_BOTTOM(3),
    LV_STYLE_CONST_PAD_LEFT(4), LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(field, field_props);

// Slider
const lv_style_const_prop_t slider_knob_props[] = {
    LV_STYLE_CONST_PAD_TOP(9),  LV_STYLE_CONST_PAD_BOTTOM(9),
    LV_STYLE_CONST_PAD_LEFT(4), LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_CONST_RADIUS(4),   LV_STYLE_CONST_BORDER_WIDTH(2),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(slider_knob, slider_knob_props);

// Choice
const lv_style_const_prop_t choice_main_props[] = {
    LV_STYLE_CONST_PAD_TOP(3),
    LV_STYLE_CONST_PAD_BOTTOM(4),
    LV_STYLE_CONST_PAD_RIGHT(5),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(choice_main, choice_main_props);

// Modal overlay (for dimming background)
const lv_style_const_prop_t modal_title_props[] = {
    LV_STYLE_CONST_PAD_TOP(PAGE_PADDING),
    LV_STYLE_CONST_PAD_BOTTOM(PAGE_PADDING),
    LV_STYLE_CONST_PAD_LEFT(PAGE_PADDING),
    LV_STYLE_CONST_PAD_RIGHT(PAGE_PADDING),
    LV_STYLE_CONST_WIDTH(LV_PCT(100)),
    LV_STYLE_CONST_MAX_HEIGHT((int)(LCD_H * 0.8)),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(modal_title, modal_title_props);

// Check Box
const lv_style_const_prop_t cb_marker_props[] = {
    LV_STYLE_CONST_PAD_TOP(2),  LV_STYLE_CONST_PAD_BOTTOM(2),
    LV_STYLE_CONST_PAD_LEFT(2), LV_STYLE_CONST_PAD_RIGHT(2),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(cb_marker, cb_marker_props);

const lv_style_const_prop_t cb_marker_checked_props[] = {
    LV_STYLE_CONST_BG_IMG_SRC(LV_SYMBOL_OK),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(cb_marker_checked, cb_marker_checked_props);

// Table
const lv_style_const_prop_t table_cell_props[] = {
    LV_STYLE_CONST_BORDER_WIDTH(1),
    LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(table_cell, table_cell_props);

// Bubble popup
const lv_style_const_prop_t bubble_popup_props[] = {
    LV_STYLE_CONST_PAD_TOP(4),
    LV_STYLE_CONST_PAD_BOTTOM(4),
    LV_STYLE_CONST_PAD_LEFT(4),
    LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_BORDER_WIDTH(3),
    LV_STYLE_CONST_RADIUS(10),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(bubble_popup, bubble_popup_props);

// States (pressed, disabled, etc)
static lv_color_t dark_color_filter_cb(const lv_color_filter_dsc_t* f,
                                       lv_color_t c, lv_opa_t opa)
{
  LV_UNUSED(f);
  return lv_color_darken(c, opa);
}

const lv_color_filter_dsc_t dark_filter = {.filter_cb = dark_color_filter_cb,
                                           .user_data = 0};

const lv_style_const_prop_t pressed_props[] = {
    LV_STYLE_CONST_COLOR_FILTER_DSC(&dark_filter),
    LV_STYLE_CONST_COLOR_FILTER_OPA(35),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(pressed, pressed_props);

static lv_color_t grey_filter_cb(const lv_color_filter_dsc_t* f,
                                 lv_color_t color, lv_opa_t opa)
{
  LV_UNUSED(f);
  return lv_color_mix(lv_palette_lighten(LV_PALETTE_GREY, 2), color, opa);
}

const lv_color_filter_dsc_t grey_filter = {.filter_cb = grey_filter_cb,
                                           .user_data = 0};

const lv_style_const_prop_t disabled_props[] = {
    LV_STYLE_CONST_COLOR_FILTER_DSC(&grey_filter),
    LV_STYLE_CONST_COLOR_FILTER_OPA(LV_OPA_50),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(disabled, disabled_props);

/**********************
 *   Variable Styles
 **********************/

class EdgeTxStyles
{
 public:
  // Colors
  lv_style_t bg_color[LCD_COLOR_COUNT];
  lv_style_t txt_color[LCD_COLOR_COUNT];
  lv_style_t border_color_secondary1;
  lv_style_t border_color_secondary2;
  lv_style_t border_color_black;
  lv_style_t border_color_white;
  lv_style_t border_color_focus;
  lv_style_t bg_color_grey;
  lv_style_t bg_color_white;
  lv_style_t bg_color_black;
  lv_style_t fg_color_black;
  lv_style_t fg_color_white;

  // Fonts
  lv_style_t font_std;
  lv_style_t font_bold;

  EdgeTxStyles() {}

  void init()
  {
    if (!initDone) {
      initDone = true;

      // Colors
      for (int i = DEFAULT_COLOR_INDEX; i < LCD_COLOR_COUNT; i += 1) {
        lv_style_init(&bg_color[i]);
        lv_style_init(&txt_color[i]);
      }
      lv_style_init(&border_color_secondary1);
      lv_style_init(&border_color_secondary2);
      lv_style_init(&border_color_focus);

      lv_style_init(&bg_color_grey);
      lv_style_set_bg_color(&bg_color_grey, lv_palette_main(LV_PALETTE_GREY));
      lv_style_init(&bg_color_white);
      lv_style_set_bg_color(&bg_color_white, lv_color_white());
      lv_style_init(&bg_color_black);
      lv_style_set_bg_color(&bg_color_black, lv_color_black());
      lv_style_init(&fg_color_black);
      lv_style_set_text_color(&fg_color_black, lv_color_black());
      lv_style_init(&fg_color_white);
      lv_style_set_text_color(&fg_color_white, lv_color_white());
      lv_style_init(&border_color_black);
      lv_style_set_border_color(&border_color_black, lv_color_black());
      lv_style_init(&border_color_white);
      lv_style_set_border_color(&border_color_white, lv_color_white());

      // Fonts
      lv_style_init(&font_std);
      lv_style_set_text_font(&font_std, getFont(FONT(STD)));
      lv_style_init(&font_bold);
      lv_style_set_text_font(&font_bold, getFont(FONT(BOLD)));
    }

    applyColors();
  }

  void applyColors()
  {
    // Always update colors in case theme changes

    for (int i = DEFAULT_COLOR_INDEX; i < LCD_COLOR_COUNT; i += 1) {
      lv_style_set_bg_color(&bg_color[i], makeLvColor(COLOR(i)));
      lv_style_set_text_color(&txt_color[i], makeLvColor(COLOR(i)));
    }

    lv_style_set_border_color(&border_color_secondary1,
                              makeLvColor(COLOR_THEME_SECONDARY1));
    lv_style_set_border_color(&border_color_secondary2,
                              makeLvColor(COLOR_THEME_SECONDARY2));

    lv_style_set_border_color(&border_color_focus,
                              makeLvColor(COLOR_THEME_FOCUS));
  }

 protected:
  bool initDone = false;
};

static EdgeTxStyles mainStyles;
static EdgeTxStyles* previewStyles;
static EdgeTxStyles* styles = &mainStyles;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_theme_t* etx_lv_theme_init(lv_disp_t* disp, lv_color_t color_primary,
                              lv_color_t color_secondary, const lv_font_t* font)
{
  theme.disp = disp;
  theme.color_primary = color_primary;
  theme.color_secondary = color_secondary;
  theme.font_small = font;
  theme.font_normal = font;
  theme.font_large = font;
  theme.flags = 0;

  styles->init();

  if (disp == NULL || lv_disp_get_theme(disp) == &theme)
    lv_obj_report_style_change(NULL);

  return (lv_theme_t*)&theme;
}

void usePreviewStyle()
{
  if (!previewStyles) previewStyles = new EdgeTxStyles();
  styles = previewStyles;
  styles->init();
}

void useMainStyle() { styles = &mainStyles; }

/**********************
 *   Custom object creation
 **********************/

// Event handler forward references
static void window_base_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void table_event(const lv_obj_class_t* class_p, lv_event_t* e);

// Object constructors
extern "C" {

void etx_add_border(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN)
{
  lv_obj_add_style(obj, (lv_style_t*)&border, selector);
  lv_obj_add_style(obj, &styles->border_color_secondary2, selector);
}

void etx_add_border_rounded(lv_obj_t* obj,
                            lv_style_selector_t selector = LV_PART_MAIN)
{
  etx_add_border(obj, selector);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, selector);
}

void etx_add_colors(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN,
                    LcdColorIndex bg_color = COLOR_THEME_PRIMARY2_INDEX,
                    LcdColorIndex txt_color = COLOR_THEME_SECONDARY1_INDEX)
{
  lv_obj_add_style(obj, &styles->bg_color[bg_color], selector);
  lv_obj_add_style(obj, &styles->txt_color[txt_color], selector);
}

void etx_add_colors_and_opacity(
    lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN,
    LcdColorIndex bg_color = COLOR_THEME_PRIMARY2_INDEX,
    LcdColorIndex txt_color = COLOR_THEME_SECONDARY1_INDEX)
{
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, selector);
  etx_add_colors(obj, selector, bg_color, txt_color);
}

void input_mix_line_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border_rounded(obj);
  etx_add_colors_and_opacity(obj);
  lv_obj_add_style(obj, (lv_style_t*)&pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->font_std, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->border_color_focus, LV_STATE_FOCUSED);
}

void input_mix_group_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border_rounded(obj);
  etx_add_colors_and_opacity(obj);
  lv_obj_add_style(obj, (lv_style_t*)&pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->font_bold, LV_PART_MAIN);
}

void field_edit_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border_rounded(obj);
  etx_add_colors_and_opacity(obj);
  lv_obj_add_style(obj, (lv_style_t*)&field, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_MAIN | LV_STATE_FOCUSED);

  etx_add_colors(obj, LV_PART_MAIN | LV_STATE_EDITED, COLOR_THEME_EDIT_INDEX,
                 COLOR_THEME_PRIMARY2_INDEX);

  lv_obj_add_style(obj, (lv_style_t*)&fg_opacity_transparent, LV_PART_CURSOR);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_50,
                   LV_PART_CURSOR | LV_STATE_EDITED);
  lv_obj_add_style(obj, (lv_style_t*)&fg_opacity_cover,
                   LV_PART_CURSOR | LV_STATE_EDITED);

  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(obj, false);
  lv_textarea_set_one_line(obj, true);
}

void number_edit_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  field_edit_constructor(class_p, obj);
  lv_obj_add_style(obj, (lv_style_t*)&text_align_right, LV_PART_MAIN);
}

void window_base_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_50, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, (lv_style_t*)&scrollbar, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->bg_color_grey, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover,
                   LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
}

void form_window_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_transparent, LV_PART_MAIN);
}

void table_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&pad_zero, LV_PART_MAIN);

  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_50, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, (lv_style_t*)&scrollbar, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->bg_color_grey, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover,
                   LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

  etx_add_colors_and_opacity(obj, LV_PART_ITEMS, COLOR_THEME_PRIMARY2_INDEX,
                             COLOR_THEME_PRIMARY1_INDEX);
  lv_obj_add_style(obj, (lv_style_t*)&table_cell, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_ITEMS);
  lv_obj_add_style(obj, (lv_style_t*)&pad_small, LV_PART_ITEMS);
  lv_obj_set_style_pad_ver(obj, 7, LV_PART_ITEMS);

  lv_obj_add_style(obj, (lv_style_t*)&pressed,
                   LV_PART_ITEMS | LV_STATE_PRESSED);

  etx_add_colors(obj, LV_PART_ITEMS | LV_STATE_EDITED, COLOR_THEME_FOCUS_INDEX,
                 COLOR_THEME_PRIMARY2_INDEX);
}

void etx_keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY3_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_MAIN);

  etx_add_colors_and_opacity(obj, LV_PART_ITEMS, COLOR_THEME_PRIMARY2_INDEX,
                             COLOR_THEME_PRIMARY1_INDEX);
  lv_obj_add_style(obj, (lv_style_t*)&btn, LV_PART_ITEMS);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_ITEMS);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_ITEMS | LV_STATE_DISABLED);

  lv_obj_add_style(obj, (lv_style_t*)&pressed,
                   LV_PART_ITEMS | LV_STATE_PRESSED);

  etx_add_colors(obj, LV_PART_ITEMS | LV_STATE_CHECKED,
                 COLOR_THEME_ACTIVE_INDEX, COLOR_THEME_PRIMARY1_INDEX);
  etx_add_colors(obj, LV_PART_ITEMS | LV_STATE_FOCUS_KEY,
                 COLOR_THEME_FOCUS_INDEX, COLOR_THEME_PRIMARY2_INDEX);
  etx_add_colors(obj, LV_PART_ITEMS | LV_STATE_EDITED, COLOR_THEME_FOCUS_INDEX,
                 COLOR_THEME_PRIMARY2_INDEX);
}

void etx_switch_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border(obj);
  lv_obj_add_style(obj, (lv_style_t*)&circle, LV_PART_MAIN);
  etx_add_colors_and_opacity(obj, LV_PART_MAIN, COLOR_THEME_PRIMARY2_INDEX,
                             COLOR_THEME_PRIMARY1_INDEX);
  lv_obj_add_style(obj, (lv_style_t*)&anim_fast, LV_PART_MAIN);

  lv_obj_add_style(obj, (lv_style_t*)&disabled, LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->border_color_focus, LV_STATE_FOCUSED);

  etx_add_border(obj, LV_PART_INDICATOR);
  lv_obj_add_style(obj, (lv_style_t*)&circle, LV_PART_INDICATOR);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_INDICATOR | LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_INDICATOR | LV_STATE_FOCUSED);

  etx_add_colors_and_opacity(obj, LV_PART_INDICATOR | LV_STATE_CHECKED,
                             COLOR_THEME_ACTIVE_INDEX,
                             COLOR_THEME_PRIMARY1_INDEX);

  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_KNOB);
  lv_obj_add_style(obj, (lv_style_t*)&circle, LV_PART_KNOB);
  lv_obj_add_style(obj, (lv_style_t*)&switch_knob, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_KNOB);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_KNOB | LV_STATE_DISABLED);
}

void etx_slider_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_colors_and_opacity(obj, LV_PART_MAIN, COLOR_THEME_SECONDARY1_INDEX,
                             COLOR_THEME_PRIMARY2_INDEX);
  lv_obj_add_style(obj, (lv_style_t*)&circle, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_EDIT_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED | LV_STATE_EDITED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_INDICATOR | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_EDIT_INDEX],
                   LV_PART_INDICATOR | LV_STATE_FOCUSED | LV_STATE_EDITED);

  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_KNOB);
  lv_obj_add_style(obj, (lv_style_t*)&slider_knob, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->border_color_secondary1, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_KNOB | LV_STATE_FOCUSED);
}

void etx_btnmatrix_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_20, LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);

  etx_add_border_rounded(obj, LV_PART_ITEMS);
  etx_add_colors_and_opacity(obj, LV_PART_ITEMS);
  lv_obj_add_style(obj, (lv_style_t*)&btn, LV_PART_ITEMS);

  etx_add_colors(obj, LV_PART_ITEMS | LV_STATE_CHECKED,
                 COLOR_THEME_ACTIVE_INDEX, COLOR_THEME_PRIMARY1_INDEX);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_ITEMS | LV_STATE_DISABLED);

  lv_obj_add_style(obj, (lv_style_t*)&pressed,
                   LV_PART_ITEMS | LV_STATE_PRESSED);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_ITEMS | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_ITEMS | LV_STATE_EDITED);
}

void etx_button_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border_rounded(obj);
  etx_add_colors_and_opacity(obj);
  lv_obj_add_style(obj, (lv_style_t*)&btn, LV_PART_MAIN);

  etx_add_colors(obj, LV_PART_MAIN | LV_STATE_CHECKED, COLOR_THEME_ACTIVE_INDEX,
                 COLOR_THEME_PRIMARY1_INDEX);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_MAIN | LV_STATE_DISABLED);
}

void etx_quick_button_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&border_transparent, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->fg_color_white, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&btn, LV_PART_MAIN);

  lv_obj_add_style(obj, (lv_style_t*)&border, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->border_color_white,
                   LV_PART_MAIN | LV_STATE_FOCUSED);
}

void etx_choice_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border_rounded(obj);
  etx_add_colors_and_opacity(obj);
  lv_obj_add_style(obj, (lv_style_t*)&choice_main, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_MAIN | LV_STATE_FOCUSED);
}

void etx_bar_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_INDICATOR);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_INDICATOR);
}

void etx_checkbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_border(obj, LV_PART_INDICATOR);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_INDICATOR);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_INDICATOR);
  lv_obj_add_style(obj, (lv_style_t*)&cb_marker, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_INDICATOR);

  lv_obj_add_style(obj, (lv_style_t*)&cb_marker_checked,
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->border_color_secondary1,
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  etx_add_colors(obj, LV_PART_INDICATOR | LV_STATE_CHECKED,
                 COLOR_THEME_SECONDARY1_INDEX, COLOR_THEME_PRIMARY2_INDEX);

  lv_obj_add_style(obj, &styles->border_color_focus,
                   LV_PART_INDICATOR | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, (lv_style_t*)&disabled,
                   LV_PART_INDICATOR | LV_STATE_DISABLED);

  lv_checkbox_set_text_static(obj, "");
}

void bubble_popup_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&bubble_popup, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color_white, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->fg_color_black, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_black, LV_PART_MAIN);
}

void modal_window_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_50, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color_black, LV_PART_MAIN);
}

void modal_dialog_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, (lv_style_t*)&rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_75, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color_black, LV_PART_MAIN);
}

void modal_content_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, (lv_style_t*)&bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY3_INDEX],
                   LV_PART_MAIN);
}

void modal_title_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_add_colors_and_opacity(obj, LV_PART_MAIN, COLOR_THEME_SECONDARY1_INDEX,
                             COLOR_THEME_PRIMARY2_INDEX);
  lv_obj_add_style(obj, (lv_style_t*)&modal_title, LV_PART_MAIN);
}
}

// Object classes
const lv_obj_class_t input_mix_line_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = input_mix_line_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_btn_t),
};

const lv_obj_class_t input_mix_group_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = input_mix_group_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t),
};

const lv_obj_class_t field_edit_class = {
    .base_class = &lv_textarea_class,
    .constructor_cb = field_edit_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_DPI_DEF,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_textarea_t)};

const lv_obj_class_t number_edit_class = {
    .base_class = &lv_textarea_class,
    .constructor_cb = number_edit_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_DPI_DEF,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_textarea_t)};

const lv_obj_class_t window_base_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = window_base_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = window_base_event,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t form_window_class = {
    .base_class = &window_base_class,
    .constructor_cb = form_window_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t table_class = {
    .base_class = &lv_table_class,
    .constructor_cb = table_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = table_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_table_t),
};

const lv_obj_class_t etx_keyboard_class = {
    .base_class = &lv_keyboard_class,
    .constructor_cb = etx_keyboard_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_keyboard_t),
};

const lv_obj_class_t etx_switch_class = {
    .base_class = &lv_switch_class,
    .constructor_cb = etx_switch_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_switch_t),
};

const lv_obj_class_t etx_slider_class = {
    .base_class = &lv_slider_class,
    .constructor_cb = etx_slider_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 8,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_slider_t),
};

const lv_obj_class_t etx_btnmatrix_class = {
    .base_class = &lv_btnmatrix_class,
    .constructor_cb = etx_btnmatrix_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_btnmatrix_t),
};

const lv_obj_class_t etx_button_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = etx_button_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_btn_t),
};

const lv_obj_class_t etx_quick_button_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = etx_quick_button_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 80,
    .height_def = 114,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_btn_t),
};

const lv_obj_class_t etx_menu_button_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = etx_button_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_btn_t),
};

const lv_obj_class_t etx_choice_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = etx_choice_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_obj_t),
};

const lv_obj_class_t etx_bar_class = {
    .base_class = &lv_bar_class,
    .constructor_cb = etx_bar_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_PCT(100),
    .height_def = 32,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_bar_t),
};

const lv_obj_class_t etx_checkbox_class = {
    .base_class = &lv_checkbox_class,
    .constructor_cb = etx_checkbox_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 25,
    .height_def = 25,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_checkbox_t),
};

const lv_obj_class_t etx_bubble_popup_class = {
    .base_class = &window_base_class,
    .constructor_cb = bubble_popup_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t etx_modal_window_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_window_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t etx_modal_dialog_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_dialog_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t etx_modal_content_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_content_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

const lv_obj_class_t etx_modal_title_class = {
    .base_class = &lv_label_class,
    .constructor_cb = modal_title_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t)};

// Event handlers
extern "C" void window_event_cb(lv_event_t* e);

static void window_base_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  /* Call the ancestor's event handler */
  lv_res_t res = lv_obj_event_base(&window_base_class, e);
  if (res != LV_RES_OK) return;

  window_event_cb(e);
}

static void table_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  lv_res_t res;
  lv_event_code_t code = lv_event_get_code(e);
  if (code != LV_EVENT_RELEASED) {
    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(&table_class, e);
    if (res != LV_RES_OK) return;
  } else {
    lv_obj_t* obj = lv_event_get_target(e);
    lv_table_t* table = (lv_table_t*)obj;

    /*From lv_table.c: handler for LV_EVENT_RELEASED*/
    lv_obj_invalidate(obj);
    lv_indev_t* indev = lv_indev_get_act();
    lv_obj_t* scroll_obj = lv_indev_get_scroll_obj(indev);
    if (table->col_act != LV_TABLE_CELL_NONE &&
        table->row_act != LV_TABLE_CELL_NONE && scroll_obj == NULL) {
      res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
      if (res != LV_RES_OK) return;
    }
  }
}

// Object creators
static lv_obj_t* etx_create(const lv_obj_class_t* class_p, lv_obj_t* parent)
{
  lv_obj_t* obj = lv_obj_class_create_obj(class_p, parent);
  lv_obj_class_init_obj(obj);

  return obj;
}

lv_obj_t* input_mix_line_create(lv_obj_t* parent)
{
  return etx_create(&input_mix_line_class, parent);
}

lv_obj_t* input_mix_group_create(lv_obj_t* parent)
{
  return etx_create(&input_mix_group_class, parent);
}

lv_obj_t* etx_text_edit_create(lv_obj_t* parent)
{
  return etx_create(&field_edit_class, parent);
}

lv_obj_t* etx_number_edit_create(lv_obj_t* parent)
{
  return etx_create(&number_edit_class, parent);
}

lv_obj_t* window_create(lv_obj_t* parent)
{
  return etx_create(&window_base_class, parent);
}

lv_obj_t* etx_form_window_create(lv_obj_t* parent)
{
  return etx_create(&form_window_class, parent);
}

lv_obj_t* etx_table_create(lv_obj_t* parent)
{
  return etx_create(&table_class, parent);
}

lv_obj_t* etx_keyboard_create(lv_obj_t* parent)
{
  return etx_create(&etx_keyboard_class, parent);
}

lv_obj_t* etx_switch_create(lv_obj_t* parent)
{
  return etx_create(&etx_switch_class, parent);
}

lv_obj_t* etx_slider_create(lv_obj_t* parent)
{
  return etx_create(&etx_slider_class, parent);
}

lv_obj_t* etx_btnmatrix_create(lv_obj_t* parent)
{
  return etx_create(&etx_btnmatrix_class, parent);
}

lv_obj_t* etx_button_create(lv_obj_t* parent)
{
  return etx_create(&etx_button_class, parent);
}

lv_obj_t* etx_quick_button_create(lv_obj_t* parent)
{
  return etx_create(&etx_quick_button_class, parent);
}

lv_obj_t* etx_menu_button_create(lv_obj_t* parent)
{
  return etx_create(&etx_menu_button_class, parent);
}

lv_obj_t* etx_choice_create(lv_obj_t* parent)
{
  return etx_create(&etx_choice_class, parent);
}

lv_obj_t* etx_bubble_popup_create(lv_obj_t* parent)
{
  return etx_create(&etx_bubble_popup_class, parent);
}

lv_obj_t* etx_bar_create(lv_obj_t* parent)
{
  return etx_create(&etx_bar_class, parent);
}

lv_obj_t* etx_checkbox_create(lv_obj_t* parent)
{
  return etx_create(&etx_checkbox_class, parent);
}

lv_obj_t* etx_modal_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_window_class, parent);
}

lv_obj_t* etx_modal_dialog_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_dialog_class, parent);
}

lv_obj_t* etx_modal_content_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_content_class, parent);
}

lv_obj_t* etx_modal_title_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_title_class, parent);
}
