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

#include "keyboard_base.h"

#include "form.h"
#include "mainwindow.h"
#include "etx_lv_theme.h"

static void keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_solid_bg(obj, COLOR_THEME_SECONDARY3_INDEX);
  etx_obj_add_style(obj, styles->pad_tiny, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);

  etx_std_style(obj, LV_PART_ITEMS, PAD_SMALL);
  etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX, LV_PART_ITEMS);
  etx_txt_color(obj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_ITEMS | LV_STATE_FOCUSED);
  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX, LV_PART_ITEMS | LV_STATE_EDITED);
}

static const lv_obj_class_t keyboard_class = {
    .base_class = &lv_keyboard_class,
    .constructor_cb = keyboard_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_keyboard_t),
};

static lv_obj_t* keyboard_create(lv_obj_t* parent)
{
  return etx_create(&keyboard_class, parent);
}

Keyboard* Keyboard::activeKeyboard = nullptr;

static void keyboard_event_cb(lv_event_t* e)
{
  auto code = lv_event_get_code(e);
  if (code == LV_EVENT_READY) {
    Keyboard::hide(false);
  } else if (code == LV_EVENT_CANCEL) {
    Keyboard::hide(true);
  } else if (code == LV_EVENT_KEY) {
    int32_t c = *((int32_t*)lv_event_get_param(e));
    if (c == LV_KEY_ESC) {
      Keyboard::hide(false);
    }
  }
}

static void field_focus_leave(lv_event_t* e) { Keyboard::hide(false); }

static void _assign_lv_group(lv_group_t* g)
{
  // associate it with all input devices
  lv_indev_t* indev = lv_indev_get_next(NULL);
  while (indev) {
    lv_indev_set_group(indev, g);
    indev = lv_indev_get_next(indev);
  }
}

Keyboard::Keyboard(coord_t height) :
    NavWindow(MainWindow::instance(), {0, LCD_H - height, LCD_W, height})
{
  lv_obj_set_parent(lvobj, lv_layer_top());  // the keyboard is always on top

  // use a separate group for the keyboard
  group = lv_group_create();
  lv_group_set_editing(group, true);

  auto old_g = lv_group_get_default();
  lv_group_set_default(group);

  keyboard = keyboard_create(lvobj);
  lv_group_set_default(old_g);

  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, this);

  lv_obj_set_pos(keyboard, 0, 0);
  lv_obj_set_size(keyboard, LCD_W, height);

  // TODO: really needed ???
  lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

Keyboard::~Keyboard()
{
  if (group) lv_group_del(group);
}

void Keyboard::clearField(bool wasCancelled)
{
  TRACE("CLEAR FIELD");
  if (keyboard != nullptr) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }

  detach();
  lv_obj_set_parent(lvobj, lv_layer_top());

  if (fieldContainer) {
    lv_obj_scroll_to_y(fieldContainer->getLvObj(), scroll_pos, LV_ANIM_OFF);
    fieldContainer = nullptr;
  }

  if (field) {
    // restore field's group
    auto obj = field->getLvObj();
    if (obj) {
      lv_obj_remove_event_cb(obj, field_focus_leave);
    }

    if (!wasCancelled)
      field->setEditMode(false);
    field->changeEnd();
    field = nullptr;

    if (fieldGroup) {
      _assign_lv_group(fieldGroup);
      lv_group_set_editing(fieldGroup, false);
      fieldGroup = nullptr;
    }
  }
}

void Keyboard::hide(bool wasCancelled)
{
  if (activeKeyboard) {
    activeKeyboard->clearField(wasCancelled);
    lv_obj_add_flag(activeKeyboard->lvobj, LV_OBJ_FLAG_HIDDEN);
    activeKeyboard = nullptr;
  }
}

bool Keyboard::attachKeyboard()
{
  if (activeKeyboard) {
    if (activeKeyboard == this) return false;
    hide(false);
  }

  activeKeyboard = this;
  return true;
}

void Keyboard::setField(FormField* newField)
{
  if (!attachKeyboard()) return;

  lv_obj_t* obj = newField->getLvObj();
  if (obj) {
    fieldContainer = newField->getFullScreenWindow();
    if (fieldContainer) {
      attach(fieldContainer);

      lv_area_t coords;
      lv_obj_get_coords(obj, &coords);

      // place keyboard bellow the field with some margin
      setTop(max(coords.y2 + 21, LCD_H - height()));

      // save scroll position
      scroll_pos = lv_obj_get_scroll_y(fieldContainer->getLvObj());
      lv_obj_scroll_to_view(lvobj, LV_ANIM_OFF);

      newField->setEditMode(true);

      lv_keyboard_set_textarea(keyboard, obj);
      lv_obj_add_event_cb(obj, field_focus_leave, LV_EVENT_DEFOCUSED, nullptr);
      _assign_lv_group(group);

      field = newField;
      fieldGroup = (lv_group_t*)lv_obj_get_group(obj);
    }
  }
}
