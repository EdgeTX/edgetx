/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
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

#ifndef _KEYBOARD_BASE_H_
#define _KEYBOARD_BASE_H_

#include "mainwindow.h"
#include "form.h"

class Keyboard: public FormWindow {
  public:
    explicit Keyboard(coord_t height):
      FormWindow(nullptr, {0, LCD_H - height, LCD_W, height}, OPAQUE)
    {
    }

    static void hide()
    {
      if (activeKeyboard) {
        activeKeyboard->clearField();
        activeKeyboard = nullptr;
      }
    }

  protected:
    static Keyboard * activeKeyboard;
    FormField * field = nullptr;
    Window * fieldContainer = nullptr;

    void setField(FormField * newField)
    {
      if (activeKeyboard) {
        if (activeKeyboard == this)
          return;
        activeKeyboard->clearField();
      }
      activeKeyboard = this;
      attach(&mainWindow);
      fieldContainer = getFieldContainer(newField);
      fieldContainer->setHeight(LCD_H - height() - fieldContainer->top());
      fieldContainer->scrollTo(newField);
      invalidate();
      newField->setEditMode(true);
      field = newField;
    }

    void clearField()
    {
      detach();
      if (fieldContainer) {
        fieldContainer->setHeight(LCD_H - 0 - fieldContainer->top());
        fieldContainer = nullptr;
      }
      if (field) {
        field->setEditMode(false);
        field = nullptr;
      }
    }

    Window * getFieldContainer(FormField * field)
    {
      Window * parent = field;
      while (true) {
        Window * tmp = parent->getParent();
        if ((tmp->getWindowFlags() & OPAQUE) && tmp->width() == LCD_W && tmp->height() == LCD_H) {
          return parent;
        }
        parent = tmp;
      }
    }
};

#endif // _KEYBOARD_BASE_H_