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

#include <stack>

#include "../colors.h"
#include "font.h"

extern lv_color_t makeLvColor(uint32_t colorFlags);

/*********************
 *      DEFINES
 *********************/
#define BORDER_WIDTH lv_disp_dpx(theme.disp, 2)

#if LCD_W > LCD_H
#define PAD_SMALL (lv_disp_dpx(theme.disp, 12))
#define PAD_TINY (lv_disp_dpx(theme.disp, 4))
#else
#define PAD_SMALL (lv_disp_dpx(theme.disp, 10))
#define PAD_TINY (lv_disp_dpx(theme.disp, 2))
#endif

static lv_theme_t theme;

static lv_color_t dark_color_filter_cb(const lv_color_filter_dsc_t* f,
                                       lv_color_t c, lv_opa_t opa)
{
  LV_UNUSED(f);
  return lv_color_darken(c, opa);
}

static lv_color_t grey_filter_cb(const lv_color_filter_dsc_t* f,
                                 lv_color_t color, lv_opa_t opa)
{
  LV_UNUSED(f);
  return lv_color_mix(lv_palette_lighten(LV_PALETTE_GREY, 2), color, opa);
}

class EdgeTxStyles
{
 public:
  lv_style_t scrollbar;
  lv_style_t btn;
  lv_style_t field;

  // opacity
  lv_style_t bg_opacity_transparent;
  lv_style_t bg_opacity_20;
  lv_style_t bg_opacity_50;
  lv_style_t bg_opacity_cover;
  lv_style_t fg_opacity_transparent;
  lv_style_t fg_opacity_cover;

  // Colors
  lv_style_t bg_color[LCD_COLOR_COUNT];
  lv_style_t txt_color[LCD_COLOR_COUNT];
  lv_style_t border_color_secondary1;
  lv_style_t border_color_secondary2;

  // Utility
  lv_style_t pressed;
  lv_style_t disabled;
  lv_style_t focussed;
  lv_style_t pad_zero;
  lv_style_t pad_tiny;
  lv_style_t pad_small;
  lv_style_t circle;
  lv_style_t rounded;
  lv_style_t border;
  lv_style_t anim_fast;
  lv_style_t font_std;
  lv_style_t font_bold;
  lv_style_t text_align_right;

  // Slider parts
  lv_style_t slider_main;
  lv_style_t slider_knob;

  // Choice
  lv_style_t choice_main;

  // Toggle switch
  lv_style_t switch_knob;

  // Table
  lv_style_t table_cell;

  // Keyboard
  lv_style_t keyboard;
  lv_style_t keyboard_btn_bg;

  // Model popup
  lv_style_t modal_overlay;
  lv_style_t modal_title;

  // Check Box
  lv_style_t cb_marker;
  lv_style_t cb_marker_checked;

  // Bubble popup
  lv_style_t bubble_popup;

  EdgeTxStyles()
  {
    // Opacity
    lv_style_init(&bg_opacity_cover);
    lv_style_set_bg_opa(&bg_opacity_cover, LV_OPA_COVER);
    lv_style_init(&bg_opacity_20);
    lv_style_set_bg_opa(&bg_opacity_20, LV_OPA_20);
    lv_style_init(&bg_opacity_50);
    lv_style_set_bg_opa(&bg_opacity_50, LV_OPA_50);
    lv_style_init(&bg_opacity_transparent);
    lv_style_set_bg_opa(&bg_opacity_transparent, LV_OPA_TRANSP);
    lv_style_init(&fg_opacity_cover);
    lv_style_set_opa(&fg_opacity_cover, LV_OPA_COVER);
    lv_style_init(&fg_opacity_transparent);
    lv_style_set_opa(&fg_opacity_transparent, LV_OPA_TRANSP);

    // Colors
    for (int i = DEFAULT_COLOR_INDEX; i < LCD_COLOR_COUNT; i += 1) {
      lv_style_init(&bg_color[i]);
      lv_style_init(&txt_color[i]);
    }
    lv_style_init(&border_color_secondary1);
    lv_style_init(&border_color_secondary2);

    // Scrollbar
    lv_style_init(&scrollbar);
    lv_style_set_pad_all(&scrollbar, lv_disp_dpx(theme.disp, 7));
    lv_style_set_width(&scrollbar, lv_disp_dpx(theme.disp, 5));

    // Keyboard
    lv_style_init(&keyboard);
    lv_style_set_pad_row(&keyboard, PAD_SMALL);
    lv_style_set_pad_column(&keyboard, PAD_SMALL);

    // Keyboard buttons
    lv_style_init(&keyboard_btn_bg);
    lv_style_set_shadow_width(&keyboard_btn_bg, 0);
    lv_style_set_radius(&keyboard_btn_bg, 5);

    // Border
    lv_style_init(&border);
    lv_style_set_border_opa(&border, LV_OPA_COVER);
    lv_style_set_border_width(&border, BORDER_WIDTH);

    // Button
    lv_style_init(&btn);
    lv_style_set_pad_hor(&btn, PAD_SMALL);
    lv_style_set_pad_ver(&btn, PAD_SMALL);
    lv_style_set_pad_column(&btn, lv_disp_dpx(theme.disp, 5));
    lv_style_set_pad_row(&btn, lv_disp_dpx(theme.disp, 5));

    // Edit box
    lv_style_init(&field);
    lv_style_set_pad_top(&field, 4);
    lv_style_set_pad_bottom(&field, 3);
    lv_style_set_pad_left(&field, 4);
    lv_style_set_pad_right(&field, 4);

    // States (pressed, disabled, etc)
    static lv_color_filter_dsc_t dark_filter;
    lv_color_filter_dsc_init(&dark_filter, dark_color_filter_cb);

    static lv_color_filter_dsc_t grey_filter;
    lv_color_filter_dsc_init(&grey_filter, grey_filter_cb);

    lv_style_init(&pressed);
    lv_style_set_color_filter_dsc(&pressed, &dark_filter);
    lv_style_set_color_filter_opa(&pressed, 35);

    lv_style_init(&disabled);
    lv_style_set_color_filter_dsc(&disabled, &grey_filter);
    lv_style_set_color_filter_opa(&disabled, LV_OPA_50);

    lv_style_init(&focussed);
    lv_style_set_border_opa(&focussed, LV_OPA_COVER);
    lv_style_set_border_width(&focussed, BORDER_WIDTH);

    // Padding
    lv_style_init(&pad_small);
    lv_style_set_pad_all(&pad_small, PAD_SMALL);
    lv_style_set_pad_gap(&pad_small, PAD_SMALL);
    lv_style_set_pad_column(&pad_small, PAD_SMALL);

    lv_style_init(&pad_zero);
    lv_style_set_pad_all(&pad_zero, 0);
    lv_style_set_pad_row(&pad_zero, 0);
    lv_style_set_pad_column(&pad_zero, 0);

    lv_style_init(&pad_tiny);
    lv_style_set_pad_all(&pad_tiny, PAD_TINY);
    lv_style_set_pad_row(&pad_tiny, PAD_TINY);
    lv_style_set_pad_column(&pad_tiny, PAD_TINY);

    // Toggle switch and slider knob rounding
    lv_style_init(&circle);
    lv_style_set_radius(&circle, LV_RADIUS_CIRCLE);

    // Corner rounding (button, edit box, etc)
    lv_style_init(&rounded);
    lv_style_set_radius(&rounded, 6);

    // Slider
    lv_style_init(&slider_main);
    lv_style_set_pad_all(&slider_main, 6);
    lv_style_init(&slider_knob);
    lv_style_set_border_width(&slider_knob, 1);
    lv_style_set_pad_all(&slider_knob, lv_disp_dpx(theme.disp, 6));

    // Choice
    lv_style_init(&choice_main);
    lv_style_set_pad_top(&choice_main, 3);
    lv_style_set_pad_bottom(&choice_main, 4);
    lv_style_set_pad_right(&choice_main, 5);

    // Animation
    lv_style_init(&anim_fast);
    lv_style_set_anim_time(&anim_fast, 120);

    // Toggle switch
    lv_style_init(&switch_knob);
    lv_style_set_pad_all(&switch_knob, -4);

    // Table
    lv_style_init(&table_cell);
    lv_style_set_border_width(&table_cell, lv_disp_dpx(theme.disp, 1));
    lv_style_set_border_side(&table_cell,
                             LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM);

    // Modal overlay (for dimming background)
    lv_style_init(&modal_overlay);
    lv_style_set_bg_color(&modal_overlay, lv_color_black());
    lv_style_init(&modal_title);
    lv_style_set_pad_all(&modal_title, PAGE_PADDING);
    lv_style_set_width(&modal_title, lv_pct(100));
    lv_style_set_max_height(&modal_title, LCD_H * 0.8);

    // Fonts
    lv_style_init(&font_std);
    lv_style_set_text_font(&font_std, getFont(FONT(STD)));
    lv_style_init(&font_bold);
    lv_style_set_text_font(&font_bold, getFont(FONT(BOLD)));

    // Text align
    lv_style_init(&text_align_right);
    lv_style_set_text_align(&text_align_right, LV_TEXT_ALIGN_RIGHT);

    // Check Box
    lv_style_init(&cb_marker);
    lv_style_set_pad_all(&cb_marker, lv_disp_dpx(theme.disp, 3));
    lv_style_init(&cb_marker_checked);
    lv_style_set_bg_img_src(&cb_marker_checked, LV_SYMBOL_OK);
    lv_style_set_text_font(&cb_marker_checked, theme.font_small);

    // Bubble popup
    lv_style_init(&bubble_popup);
    lv_style_set_pad_all(&bubble_popup, 4);
    lv_style_set_border_opa(&bubble_popup, LV_OPA_COVER);
    lv_style_set_border_width(&bubble_popup, 3);
    lv_style_set_radius(&bubble_popup, 10);
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

    lv_style_set_bg_color(&scrollbar, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_border_color(&focussed, makeLvColor(COLOR_THEME_FOCUS));

    lv_style_set_bg_color(&bubble_popup, makeLvColor(COLOR2FLAGS(WHITE)));
    lv_style_set_border_color(&bubble_popup, makeLvColor(COLOR2FLAGS(BLACK)));
    lv_style_set_text_color(&bubble_popup, makeLvColor(COLOR2FLAGS(BLACK)));
  }

 protected:
};

static EdgeTxStyles* styles;
static EdgeTxStyles* previewStyles;
static EdgeTxStyles* mainStyles;
static std::stack<EdgeTxStyles*> styleStack;

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

  if (!styles) {
    styles = new EdgeTxStyles();
    mainStyles = styles;
  }
  styles->applyColors();

  if (disp == NULL || lv_disp_get_theme(disp) == &theme)
    lv_obj_report_style_change(NULL);

  return (lv_theme_t*)&theme;
}

void usePreviewStyle()
{
  if (!previewStyles) previewStyles = new EdgeTxStyles();
  styles = previewStyles;
  styles->applyColors();
}

void useMainStyle() { styles = mainStyles; }

/**********************
 *   Custom object creation
 **********************/

// Event handler forward references
static void field_edit_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void window_base_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void table_event(const lv_obj_class_t* class_p, lv_event_t* e);

// Object constructors
extern "C" {

void input_mix_line_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->font_std, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->focussed, LV_STATE_FOCUSED);
}

void input_mix_group_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->font_bold, LV_PART_MAIN);
}

void field_edit_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->field, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->focussed, LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_EDIT_INDEX],
                   LV_STATE_EDITED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_STATE_EDITED);

  lv_obj_add_style(obj, &styles->fg_opacity_transparent, LV_PART_CURSOR);
  lv_obj_add_style(obj, &styles->bg_opacity_50,
                   LV_PART_CURSOR | LV_STATE_EDITED);
  lv_obj_add_style(obj, &styles->fg_opacity_cover,
                   LV_PART_CURSOR | LV_STATE_EDITED);

  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(obj, false);
  lv_textarea_set_one_line(obj, true);
}

void number_edit_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  field_edit_constructor(class_p, obj);
  lv_obj_add_style(obj, &styles->text_align_right, LV_PART_MAIN);
}

void window_base_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_50, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->bg_opacity_cover,
                   LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
}

void form_window_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, &styles->bg_opacity_transparent, LV_PART_MAIN);
}

void table_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->pad_zero, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_opacity_50, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
  lv_obj_add_style(obj, &styles->bg_opacity_cover,
                   LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->table_cell, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->pad_small, LV_PART_ITEMS);
  lv_obj_set_style_pad_ver(obj, 7, LV_PART_ITEMS);

  lv_obj_add_style(obj, &styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_ITEMS | LV_STATE_EDITED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS | LV_STATE_EDITED);
}

void etx_keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->keyboard, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY3_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->pad_tiny, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->btn, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->keyboard_btn_bg, LV_PART_ITEMS);

  lv_obj_add_style(obj, &styles->disabled, LV_PART_ITEMS | LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_PART_ITEMS | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_ITEMS | LV_STATE_CHECKED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS | LV_STATE_FOCUS_KEY);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_ITEMS | LV_STATE_EDITED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS | LV_STATE_EDITED);
}

void etx_switch_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->circle, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->anim_fast, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->disabled, LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->focussed, LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->border, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->circle, LV_PART_INDICATOR);

  lv_obj_add_style(obj, &styles->disabled,
                   LV_PART_INDICATOR | LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->focussed,
                   LV_PART_INDICATOR | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->bg_opacity_cover,
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->circle, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->switch_knob, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_KNOB);

  lv_obj_add_style(obj, &styles->disabled, LV_PART_KNOB | LV_STATE_DISABLED);
}

void etx_slider_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->slider_main, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->circle, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->slider_knob, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->border_color_secondary1, LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_KNOB);
  lv_obj_add_style(obj, &styles->circle, LV_PART_KNOB);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_EDIT_INDEX],
                   LV_PART_KNOB | LV_STATE_FOCUSED | LV_STATE_EDITED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_KNOB | LV_STATE_FOCUSED | LV_STATE_EDITED);
}

void etx_btnmatrix_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_opacity_20,
                   LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->bg_opacity_20,
                   LV_PART_MAIN | LV_STATE_FOCUSED | LV_STATE_EDITED);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_FOCUS_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->btn, LV_PART_ITEMS);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_ITEMS);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_PART_ITEMS | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_PART_ITEMS | LV_STATE_CHECKED);

  lv_obj_add_style(obj, &styles->disabled, LV_PART_ITEMS | LV_STATE_DISABLED);

  lv_obj_add_style(obj, &styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

  lv_obj_add_style(obj, &styles->focussed, LV_PART_ITEMS | LV_STATE_EDITED);
}

void etx_button_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->btn, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_ACTIVE_INDEX],
                   LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY1_INDEX],
                   LV_STATE_CHECKED);

  lv_obj_add_style(obj, &styles->focussed, LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->disabled, LV_STATE_DISABLED);
}

void etx_choice_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->choice_main, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->focussed, LV_PART_MAIN | LV_STATE_FOCUSED);
}

void etx_bar_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY2_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_MAIN);

  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_INDICATOR);
}

void etx_checkbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->rounded, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->pad_zero, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->cb_marker, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->border, LV_PART_INDICATOR);
  lv_obj_add_style(obj, &styles->border_color_secondary2, LV_PART_INDICATOR);

  lv_obj_add_style(obj, &styles->cb_marker_checked,
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->border_color_secondary1,
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_obj_add_style(obj, &styles->focussed,
                   LV_PART_INDICATOR | LV_STATE_FOCUSED);

  lv_obj_add_style(obj, &styles->disabled,
                   LV_PART_INDICATOR | LV_STATE_DISABLED);
}

void bubble_popup_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bubble_popup, LV_PART_MAIN);
}

void modal_window_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, &styles->bg_opacity_50, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->modal_overlay, LV_PART_MAIN);
}

void modal_content_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  window_base_constructor(class_p, obj);
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY3_INDEX],
                   LV_PART_MAIN);
}

void modal_title_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  lv_obj_add_style(obj, &styles->bg_opacity_cover, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->modal_title, LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->bg_color[COLOR_THEME_SECONDARY1_INDEX],
                   LV_PART_MAIN);
  lv_obj_add_style(obj, &styles->txt_color[COLOR_THEME_PRIMARY2_INDEX],
                   LV_PART_MAIN);
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
    .event_cb = field_edit_event,
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
    .event_cb = field_edit_event,
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
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
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
static void field_edit_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  /* Call the ancestor's event handler */
  lv_obj_event_base(&field_edit_class, e);
}

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

lv_obj_t* etx_modal_content_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_content_class, parent);
}

lv_obj_t* etx_modal_title_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_title_class, parent);
}
