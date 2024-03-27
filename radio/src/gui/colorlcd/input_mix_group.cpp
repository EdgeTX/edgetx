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

#include <algorithm>

#include "opentx.h"
#include "themes/etx_lv_theme.h"

static void input_mix_group_constructor(const lv_obj_class_t* class_p,
                                        lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_MAIN, PAD_TINY);
}

static const lv_obj_class_t input_mix_group_class = {
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

static lv_obj_t* input_mix_group_create(lv_obj_t* parent)
{
  return etx_create(&input_mix_group_class, parent);
}

static const lv_coord_t col_dsc[] = {
    71,
    LV_GRID_FR(1),
    LV_GRID_TEMPLATE_LAST,
};

static const lv_coord_t row_dsc[] = {
    LV_GRID_CONTENT,
    LV_GRID_TEMPLATE_LAST,
};

InputMixGroupBase::InputMixGroupBase(Window* parent, mixsrc_t idx,
                                     const lv_coord_t* gridCols) :
    Window(parent, rect_t{}, input_mix_group_create), idx(idx)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
  padAll(PAD_ZERO);

  if (!gridCols) gridCols = col_dsc;
  lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(lvobj, gridCols, row_dsc);

  label = lv_label_create(lvobj);
  etx_font(label, FONT_STD_INDEX);
  lv_obj_set_style_pad_left(label, PAD_TINY, 0);
  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
  refresh();

  line_container = window_create(lvobj);
  lv_obj_set_size(line_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_flex_cross_place(line_container, LV_FLEX_ALIGN_END, 0);
  lv_obj_set_style_pad_all(line_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_style_pad_row(line_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_grid_cell(line_container, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_START, 0, 1);
}

void InputMixGroupBase::refresh()
{
  lv_label_set_text(label, getSourceString(idx));
}

void InputMixGroupBase::addLine(Window* line)
{
  lines.emplace_back(line);
  lv_obj_set_parent(line->getLvObj(), line_container);
}

bool InputMixGroupBase::removeLine(Window* line)
{
  auto l = std::find_if(lines.begin(), lines.begin(),
                        [=](const Window* l) -> bool { return l == line; });

  if (l != lines.end()) {
    lines.erase(l);
    return true;
  }

  return false;
}

InputMixGroupBase* InputMixPageBase::getGroupBySrc(mixsrc_t src)
{
  auto g = std::find_if(
      groups.begin(), groups.end(),
      [=](InputMixGroupBase* g) -> bool { return g->getMixSrc() == src; });

  if (g != groups.end()) return *g;

  return nullptr;
}

void InputMixPageBase::removeGroup(InputMixGroupBase* g)
{
  auto group = std::find_if(groups.begin(), groups.end(),
                            [=](InputMixGroupBase* lh) -> bool { return lh == g; });
  if (group != groups.end()) groups.erase(group);
}
