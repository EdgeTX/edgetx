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
  if (fields) { 
    fields->setHeight(oldHeight);
    fields = nullptr;
  }
  if (fieldContainer) {
    fieldContainer->setHeight(LCD_H - 0 - fieldContainer->top());
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


coord_t calcScrollOffsetForField(FormField *newField, Window *topWindow)
{
  // now we need to calculate the offset of the field in the fields scroll container
  // start with the current fields top and walk the hierarchy to calculate positon
  coord_t offsetY = newField->top();

  Window* parentWindow = newField->getParent();
  while (parentWindow && parentWindow != topWindow) {
    offsetY += parentWindow->top();
    parentWindow = parentWindow->getParent();
  }

  // try and place it in the middle of the screen.  The containing window MUST have 
  // already been resized
  return offsetY - topWindow->height() * 2 / 7;
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


// this routine finds the window that is a FormWindow.  This is the window
// that contains all of the editable fields.  This is the window that needs
// to be scrolled into view.
FormWindow *Keyboard::findFormWindow(Window *parent)
{
  if (parent) {
    auto children = parent->getChildren();
    auto childIterator = children.begin();
    while (childIterator != children.end())
    {
#if defined(DEBUG_WINDOWS)
      std::string windowName;
      Window *window = dynamic_cast<Window *>(*childIterator);
      if (window)
        windowName = window->getName();
#endif
        
      FormWindow *formWindow = dynamic_cast<FormWindow*>(*childIterator);
      if (formWindow)
        return formWindow;

      childIterator++;
    }
  }

  return nullptr;
}

void Keyboard::setField(FormField* newField)
{
  TRACE("SET FIELD");

  if (!attachKeyboard())
    return;

  fieldContainer = getFieldContainer(newField);
  if (fieldContainer) {
#if defined(DEBUG_WINDOWS)
    auto windowName = fieldContainer->getName();
#endif
    coord_t newHeight = LCD_H - height();
    fieldContainer->setHeight(newHeight);

    fields = findFormWindow(fieldContainer);

    if (fields) {
#if defined(DEBUG_WINDOWS)
      windowName = fields->getName();
#endif      
      // scroll the header of the window out of view to get more space to
      // see the field being edited
      
      // TODO: this no longer works. we need to figure out something else
      //lv_obj_scroll_to_y(fieldContainer->getLvObj(), fields->top(), LV_ANIM_OFF);

      oldHeight = fields->height();
      fields->setHeight(newHeight);

      coord_t offsetY = calcScrollOffsetForField(newField, fields);
      lv_obj_scroll_to_y(fields->getLvObj(), offsetY, LV_ANIM_OFF);
    } else {
      // TODO: what happens if field container is null. 
      // we will need to interrogate the fieldContainer for its
      // FormGroup.  That is really how it should work anyway.
    }

    invalidate();
    newField->setEditMode(true);
    lv_keyboard_set_textarea(keyboard, newField->getLvObj());

    field = newField;
  } else {
    clearField();
  }
}

Window* Keyboard::getFieldContainer(FormField* field)
{
  return field->getFullScreenWindow();
}
