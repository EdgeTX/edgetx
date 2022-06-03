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

#include "input_mix_group.h"
#include "opentx.h"

static const lv_coord_t col_dsc[] = {
  LV_DPI_DEF / 2, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};

static const lv_coord_t row_dsc[] = {
  LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST,
};

InputMixGroup::InputMixGroup(Window* parent, mixsrc_t idx) :
  Window(parent, rect_t{}), idx(idx)
{
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, 0);
  lv_obj_set_style_radius(lvobj, lv_dpx(16), 0);
  lv_obj_set_style_pad_hor(lvobj, lv_dpx(8), 0);
  lv_obj_set_style_pad_ver(lvobj, lv_dpx(8), 0);

  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);

  lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(lvobj, col_dsc, row_dsc);
  
  label = lv_label_create(lvobj);
  lv_label_set_text(label, getSourceString(idx));
  lv_obj_set_grid_cell(label,
                       LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  auto box = window_create(lvobj);
  lv_obj_set_size(box, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(box,
                       LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_START, 0, 1);
  
  line_container = window_create(box);
  lv_obj_set_size(line_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(line_container, lv_dpx(8), LV_PART_MAIN);
}

void InputMixGroup::addLine(Window* line, const uint8_t* symbol)
{
  lines.emplace_back(line, symbol);
  lv_obj_t* line_obj = line->getLvObj();
  lv_obj_set_parent(line_obj, line_container);
}

bool InputMixGroup::removeLine(Window* line)
{
  auto l = std::find_if(lines.begin(), lines.begin(), [=](const Line& l) -> bool {
      return l.win == line;
    });

  if (l != lines.end()) {
    lines.erase(l);
    return true;
  }

  return false;
}
