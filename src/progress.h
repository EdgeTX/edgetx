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

#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include "window.h"

class Progress : public Window {
  public:
    Progress(Window * parent, const rect_t & rect):
      Window(parent, rect)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Progress";
    }
#endif

    void setValue(int value)
    {
      this->value = value;
      invalidate();
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    int value = 0;
};

#endif
