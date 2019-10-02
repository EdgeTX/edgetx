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
#include "libopenui_types.h"

class Page;
class TabsGroup;

template <class T>
class Keyboard: public Window {
  public:
    explicit Keyboard(coord_t height):
      Window(nullptr, {0, LCD_H - height, LCD_W, height}, OPAQUE)
    {
    }

    void setField(T * newField)
    {
      field = newField;
      attach(&mainWindow);
      Window * w = getPageBody();
      w->setHeight(LCD_H - height() - w->top());
      w->scrollTo(newField);
      invalidate();
    }

    void disable(bool resizePage)
    {
      detach();
      if (field) {
        Window * w = getPageBody();
        w->setHeight(LCD_H - 0 - w->top());
        field = nullptr;
      }
    }

    T * getField() const
    {
      return field;
    }

  protected:
    T * field = nullptr;
    Window * getPageBody()
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