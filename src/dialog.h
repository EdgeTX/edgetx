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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include "form.h"

class Dialog: public FormGroup {
  public:
    Dialog(std::string title, const rect_t rect);

    void paint(BitmapBuffer * dc) override;

    void deleteLater();

  protected:
    std::string title;
    Window * previousFocus;
};

#endif // _CONFIRMATION_H_
