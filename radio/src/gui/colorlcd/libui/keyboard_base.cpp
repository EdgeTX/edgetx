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
#include "debug.h"
#include "keys.h"

static void keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_solid_bg(obj, COLOR_THEME_DISABLED_INDEX);
  etx_obj_add_style(obj, styles->pad_tiny, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);

  etx_obj_add_style(obj, styles->border, LV_PART_ITEMS);
  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_SECONDARY2_INDEX], LV_PART_ITEMS);
  etx_obj_add_style(obj, styles->rounded, LV_PART_ITEMS);
  etx_obj_add_style(obj, styles->disabled, LV_PART_ITEMS | LV_STATE_DISABLED);
  etx_obj_add_style(obj, styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

  etx_solid_bg(obj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_ITEMS);
  etx_bg_color(obj, COLOR_THEME_ACTIVE_INDEX, LV_PART_ITEMS | LV_STATE_CHECKED);
  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX, LV_PART_ITEMS | LV_STATE_EDITED);

  etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX, LV_PART_ITEMS);
  etx_txt_color(obj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_ITEMS | LV_STATE_EDITED);
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
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL ||
      (code == LV_EVENT_KEY && lv_event_get_key(e) == LV_KEY_ESC)) {
    Keyboard::hideKeyboard();
  }
}

Keyboard::Keyboard(coord_t height) :
    NavWindow(MainWindow::instance(), {0, LCD_H - height, LCD_W, height}, keyboard_create)
{
  setWindowFlag(IS_EDIT_WINDOW);

#if defined(USE_HATS_AS_KEYS)
  hasTwoPageKeys = true;
#else
  hasTwoPageKeys = keyIsSupported(KEY_PAGEUP);
#endif

  // Keyboard is created off screen so it can be animated into place
  // We don't use this, so just move it into position.
  lv_obj_set_pos(lvobj, 0, 0);

  // use a separate group for the keyboard
  group = lv_group_create();
  lv_group_set_editing(group, true);
  lv_group_add_obj(group, lvobj);

  lv_obj_add_event_cb(lvobj, keyboard_event_cb, LV_EVENT_ALL, this);

  onClosing([=]() {
    hideKeyboard();
    if (group) lv_group_del(group);
  });
}

void Keyboard::clearField()
{
  if (!deleted()) {
    TRACE("CLEAR FIELD");

    lv_obj_move_background(lvobj);
    hide();

    if (field) {
      bool focused = lv_obj_has_state(field->getLvObj(), LV_STATE_FOCUSED);

      // restore field's group
      if (fieldGroup) {
        assignLvGroup(fieldGroup, false);
        lv_group_set_editing(fieldGroup, false);
        fieldGroup = nullptr;
       }

      // End changes and restore focus
      field->changeEnd();
      if (focused)
        lv_group_focus_obj(field->getParent()->getLvObj());
      field = nullptr;
    }

    // Reset parent if moved
    if (fieldContainer) {
      fieldContainer->setTop(fieldContainerTop);
      fieldContainer = nullptr;
    }

    // Restore scroll position
    if (scrollWindow) {
      lv_obj_scroll_to_y(scrollWindow->getLvObj(), scrollPos, LV_ANIM_OFF);
      scrollWindow = nullptr;
    }
  }
}

void Keyboard::hideKeyboard()
{
  if (activeKeyboard) {
    activeKeyboard->clearField();
    activeKeyboard = nullptr;
  }
}

void Keyboard::setField(FormField* newField, bool useTextArea)
{
  if (activeKeyboard) {
    if (activeKeyboard == this) return;
    // Cancel previous keyboard
    hideKeyboard();
  }

  activeKeyboard = this;

  // Ensure the keyboard is on top
  lv_obj_move_foreground(lvobj);

  lv_obj_t* obj = newField->getLvObj();

  if (useTextArea)
    lv_keyboard_set_textarea(lvobj, obj);
  assignLvGroup(group, false);

  // save scroll position (scroll may get changed when focus is restored)
  scrollPos = 0;
  scrollWindow = newField->getParent();
  while (scrollWindow && scrollPos == 0) {
    scrollPos = lv_obj_get_scroll_y(scrollWindow->getLvObj());
    if (scrollPos == 0)
      scrollWindow = scrollWindow->getParent();
  }

  // Find parent window large enough so window can be moved and field is not under keyboard
  fieldContainer = newField->getParent();
  while (fieldContainer && ((fieldContainer->width() == LV_SIZE_CONTENT) ||
                            (fieldContainer->width() < LCD_W))
                        && ((fieldContainer->height() == LV_SIZE_CONTENT) ||
                            (fieldContainer->height() < LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT))) {
    fieldContainer = fieldContainer->getParent();
  }
  if (fieldContainer) {
    fieldContainerTop = fieldContainer->top();
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    // Distance of field bottom to keyboard top - if negative field is under keyboard
    lv_coord_t yo = (LCD_H - height() - PAD_MEDIUM) - coords.y2;
    if (yo < 0)
      fieldContainer->setTop(yo + fieldContainerTop);
    else
      fieldContainer = nullptr;
  }
  
  field = newField;
  fieldGroup = (lv_group_t*)lv_obj_get_group(obj);
}
