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

#include "modal_window.h"
#include "form.h"

class Dialog: public ModalWindow {
  public:
    Dialog(std::string title, const rect_t & rect);

    ~Dialog() override
    {
      form.detach();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Dialog";
    }
#endif

  protected:
    ModalWindowContent * content;
    FormGroup form;
};

#endif // _DIALOG_H_
