/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "form.h"

#include "etx_lv_theme.h"

void FlexGridLayout::apply(Window* w)
{
  w->padAll(padding);

  // layout
  lv_obj_set_layout(w->getLvObj(), LV_LAYOUT_GRID);
  if (col_dsc && row_dsc) {
    lv_obj_set_grid_dsc_array(w->getLvObj(), col_dsc, row_dsc);
  }
}

void FlexGridLayout::add(Window* w)
{
  if (col_dsc && row_dsc) {
    lv_obj_set_grid_cell(w->getLvObj(), LV_GRID_ALIGN_START, col_pos, col_span,
                         LV_GRID_ALIGN_CENTER, row_pos, row_span);
  }
}

FormField::FormField(Window* parent, const rect_t& rect, LvglCreate objConstruct) :
    Window(parent, rect, objConstruct)
{
  setTextFlag(textFlags);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}

void FormField::setEditMode(bool newEditMode)
{
  editMode = newEditMode;

  if (lvobj != nullptr) {
    lv_group_t* grp = (lv_group_t*)lv_obj_get_group(lvobj);
    if (grp != nullptr) lv_group_set_editing(grp, editMode);
  }
}

void FormField::onClicked()
{
  lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
  if (indev_type != LV_INDEV_TYPE_POINTER) {
    setEditMode(!editMode);
  }
}

void FormField::onCancel()
{
  if (isEditMode()) {
    setEditMode(false);
  } else {
    Window::onCancel();
  }
}

void FormField::deleteLater(bool detach, bool trash)
{
  if (isEditMode()) setEditMode(false);
  Window::deleteLater(detach, trash);
}

FormLine::FormLine(Window* parent, FlexGridLayout& layout) :
    Window(parent, rect_t{}), layout(layout)
{
  setWindowFlag(NO_FOCUS);

  layout.resetPos();
  layout.apply(this);

  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
}

void FormLine::addChild(Window* window)
{
  Window::addChild(window);
  layout.add(window);
  layout.nextCell();
}
