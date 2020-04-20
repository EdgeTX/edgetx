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

class Dialog;

class DialogWindowContent: public ModalWindowContent {
  friend class Dialog;

  public:
    DialogWindowContent(Dialog * parent, const rect_t & rect);

    ~DialogWindowContent() override
    {
      form.detach();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "DialogWindowContent";
    }
#endif

  public:
    FormGroup form;
};

class Dialog: public ModalWindow {
  public:
    Dialog(Window * parent, std::string title, const rect_t & rect);

  protected:
    DialogWindowContent * content;
};

#endif // _DIALOG_H_
