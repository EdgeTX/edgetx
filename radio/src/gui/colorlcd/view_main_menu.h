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

#pragma once

#include "form.h"

class ViewMainMenu : public Window
{
 public:
  ViewMainMenu(Window* parent);

  void onCancel() override;
  void paint(BitmapBuffer* dc) override;
  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(HARDWARE_TOUCH)
  bool onTouchStart(coord_t /*x*/, coord_t /*y*/) override { return true; }
  bool onTouchEnd(coord_t x, coord_t y) override
  {
    onKeyPress();
    deleteLater();
    return true;
  }
  bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY,
                    coord_t slideX, coord_t slideY) override
  {
    Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
    return true;
  }
#endif

 protected:
  rect_t carouselRect;
};
