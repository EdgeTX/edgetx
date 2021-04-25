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

#ifndef _FAB_ICON_BUTTON_H_
#define _FAB_ICON_BUTTON_H_

#include "button.h"

constexpr coord_t FAB_BUTTON_SIZE = 68;

class FabButton: public Button {
  public:
    FabButton(FormGroup * parent, coord_t x, coord_t y, uint8_t icon, std::function<uint8_t(void)> pressHandler, WindowFlags windowFlags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FabButton(" + std::to_string(icon) + ")";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};

#endif
