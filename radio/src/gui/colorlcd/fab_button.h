/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _FAB_ICON_BUTTON_H_
#define _FAB_ICON_BUTTON_H_

#include "button.h"

constexpr coord_t FAB_BUTTON_SIZE = 68;

class FabButton: public Button {
  public:
    FabButton(FormGroup * parent, coord_t x, coord_t y,
              uint8_t icon, std::function<uint8_t(void)> pressHandler,
              WindowFlags windowFlags = 0);

    FabButton(FormGroup * parent,  uint8_t icon,
              std::function<uint8_t(void)> pressHandler,
              WindowFlags windowFlags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FabButton";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};

#endif
