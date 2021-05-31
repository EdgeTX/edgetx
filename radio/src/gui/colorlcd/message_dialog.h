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

#ifndef _MESSAGE_DIALOG_H_
#define _MESSAGE_DIALOG_H_

#include "dialog.h"
#include "static.h"

class MessageDialog: public Dialog {
  public:
    MessageDialog(Window * parent, const char * title, const char * message, const char * info = "");

    void setInfoText(std::string text)
    {
      infoWidget->setText(std::move(text));
    }

  protected:
    StaticText * messageWidget;
    StaticText * infoWidget;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MessageDialog";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif
};

#endif // _MESSAGE_DIALOG_H_
