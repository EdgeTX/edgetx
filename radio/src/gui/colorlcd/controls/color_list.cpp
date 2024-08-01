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

#include "color_list.h"

#include "color_editor.h"
#include "etx_lv_theme.h"

ColorList::ColorList(Window *parent, const rect_t &rect,
                     std::vector<ColorEntry> colors) :
    ListBox(parent, rect, getColorListNames(colors)), _colorList(colors)
{
  setSelected(0);
}

std::vector<std::string> ColorList::getColorListNames(
    std::vector<ColorEntry> colors)
{
  std::vector<std::string> names;
  char **colorNames = ThemePersistance::getColorNames();
  for (auto color : colors) {
    names.emplace_back(colorNames[color.colorNumber]);
  }

  return names;
}

void ColorList::onDrawEnd(uint16_t row, uint16_t col,
                          lv_obj_draw_part_dsc_t *dsc)
{
  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);

  lv_area_t coords;
  lv_coord_t area_h = lv_area_get_height(dsc->draw_area);

  auto box_h = getFontHeight(FONT(STD));
  auto box_w = 3 * box_h / 4;
  lv_coord_t cell_right = lv_obj_get_style_pad_right(lvobj, LV_PART_ITEMS);

  coords.x2 = dsc->draw_area->x2 - cell_right;
  coords.x1 = coords.x2 - box_w;
  coords.y1 = dsc->draw_area->y1 + (area_h - box_h) / 2;
  coords.y2 = coords.y1 + box_h - 1;

  rect_dsc.border_color = makeLvColor(COLOR_THEME_PRIMARY1);
  rect_dsc.border_opa = LV_OPA_100;
  rect_dsc.border_width = lv_dpx(1);

  uint16_t color = _colorList[row].colorValue;
  rect_dsc.bg_color =
      lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
  rect_dsc.bg_opa = LV_OPA_100;

  lv_draw_rect(dsc->draw_ctx, &rect_dsc, &coords);
}

static void color_swatch_constructor(const lv_obj_class_t *class_p,
                                     lv_obj_t *obj)
{
  etx_obj_add_style(obj, styles->bg_opacity_cover, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_thin, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_color[COLOR_BLACK_INDEX], LV_PART_MAIN);
}

static const lv_obj_class_t color_swatch_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = color_swatch_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_obj_t),
};

static lv_obj_t *color_swatch_create(lv_obj_t *parent)
{
  return etx_create(&color_swatch_class, parent);
}

ColorSwatch::ColorSwatch(Window *window, const rect_t &rect, uint32_t color) :
    Window(window, rect, color_swatch_create)
{
  setWindowFlag(NO_FOCUS);
  setColor(color);
}

void ColorSwatch::setColor(uint32_t colorEntry)
{
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR2FLAGS(colorEntry)),
                            LV_PART_MAIN);
}

void ColorSwatch::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  lv_obj_set_style_bg_color(lvobj, lv_color_make(r, g, b), LV_PART_MAIN);
}
