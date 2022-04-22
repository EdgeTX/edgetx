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
#ifndef _CHOICEEX_H_
#define _CHOICEEX_H_
#include "choice.h"

class ChoiceEx : public Choice
{
  public:
   ChoiceEx(Window* parent, const rect_t& rect, int16_t vmin, int16_t vmax,
            std::function<int16_t()> getValue,
            std::function<void(int16_t)> setValue = nullptr,
            WindowFlags windowFlags = 0);

   void setLongPressHandler(std::function<void(event_t)> handler);

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
    bool onTouchStart(coord_t x, coord_t y) override;
    void checkEvents(void) override;
    bool isLongPress();
#endif

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ChoceEx";
    }
#endif

protected:
    std::function<void(event_t)> longPressHandler = nullptr;

#if defined(HARDWARE_TOUCH)
    uint32_t duration10ms;
    bool longPressed = false;
#endif
};

#endif
