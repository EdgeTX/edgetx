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

#pragma once

#include "mainwindow.h"
#include "form.h"

class Keyboard: public FormWindow
{
  public:
    explicit Keyboard(coord_t height);
    void clearField();
    static void hide();

  protected:
    static Keyboard * activeKeyboard;
    static lv_obj_t * keyboard;

    FormField *field = nullptr;
    Window *fieldContainer = nullptr;
    FormWindow *fields = nullptr;
    coord_t oldHeight = 0;

    void setField(FormField *newField);
    Window *getFieldContainer(FormField * field);
    bool attachKeyboard();
    FormWindow *findFormWindow(Window *parent);
};

