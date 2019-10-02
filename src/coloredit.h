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

#ifndef _COLOREDIT_H_
#define _COLOREDIT_H_

#include "window.h"

class ColorEdit : public Window {
  public:
    ColorEdit(Window * parent, const rect_t & rect, std::function<uint16_t()> getValue, std::function<void(uint16_t)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "ColorEdit";
    }
#endif

    ~ColorEdit() override;
};

#endif // _COLOREDIT_H_
