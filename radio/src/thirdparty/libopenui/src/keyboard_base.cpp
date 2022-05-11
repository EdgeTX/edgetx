/*
 * Copyright (C) EdgeTX
 *
 * Source:
 *  https://github.com/edgetx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
#include "keyboard_base.h"
#include "mainwindow.h"

Keyboard * Keyboard::activeKeyboard = nullptr;

static void keyboard_event_cb(lv_event_t * e)
{
  auto code = lv_event_get_code(e);
  Keyboard *kb = (Keyboard *) lv_event_get_user_data(e);
  if (code == LV_EVENT_READY) {
    pushEvent(EVT_VIRTUAL_KEY('\n'));
    kb->hide();
  }
}

Keyboard::Keyboard(coord_t height) : 
  FormWindow(MainWindow::instance(), {0, LCD_H - height, LCD_W, height}, OPAQUE)
{
  // set the background of the window and opacity to 100%
  lv_obj_set_parent(lvobj, lv_layer_top());  // the keyboard is always on top
  lv_obj_set_style_bg_color(lvobj, lv_color_make(0xE0, 0xE0, 0xE0), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);

  keyboard = lv_keyboard_create(this->getLvObj());
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, this);
  lv_obj_set_pos(keyboard, 0, 0);
  lv_obj_set_size(keyboard, LCD_W, height);
  lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

void Keyboard::clearField()
{
  TRACE("CLEAR FIELD");
  if (keyboard != nullptr) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }

  detach();

  if (fieldContainer) {
    lv_obj_scroll_to_y(fieldContainer->getLvObj(), scroll_pos, LV_ANIM_OFF);
    fieldContainer = nullptr;
  }

  if (field) {
    field->setEditMode(false);
    field->changeEnd();
    field = nullptr;
  }  
}

void Keyboard::hide()
{
  if (activeKeyboard) {
    activeKeyboard->clearField();
    lv_obj_add_flag(activeKeyboard->lvobj, LV_OBJ_FLAG_HIDDEN);
    activeKeyboard = nullptr;
  }
}

bool Keyboard::attachKeyboard()
{
  if (activeKeyboard) {
    if (activeKeyboard == this) return false;
    activeKeyboard->clearField();
  }

  activeKeyboard = this;
  return true;
}

void Keyboard::setField(FormField* newField)
{
  if (!attachKeyboard())
    return;

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

      invalidate();
      newField->setEditMode(true);
      lv_keyboard_set_textarea(keyboard, newField->getLvObj());
     
      field = newField;
    }
  }
}
