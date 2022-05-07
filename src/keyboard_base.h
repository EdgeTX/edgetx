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

#include "form.h"

class Keyboard: public FormWindow
{
  public:
    explicit Keyboard(coord_t height);
    void clearField();
    static void hide();

  protected:
    static Keyboard * activeKeyboard;
    lv_obj_t * keyboard = nullptr;

    FormField *field = nullptr;
    Window *fieldContainer = nullptr;
    lv_coord_t scroll_pos = 0;

    void setField(FormField *newField);
    bool attachKeyboard();
};

