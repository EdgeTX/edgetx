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

#include "page.h"
#include "form.h"
#include "curve.h"

class Window;
class NumberEdit;

class MixEditAdvanced : public Page
{
 public:
  MixEditAdvanced(int8_t channel, uint8_t index);

  static LAYOUT_VAL(NUM_EDIT_W, 100, 100, LS(100))

 protected:
  uint8_t channel;
  uint8_t index;
  NumberEdit* delayUp;
  NumberEdit* delayDn;
  NumberEdit* slowUp;
  NumberEdit* slowDn;

  void buildHeader(Window *window);
  void buildBody(Window *window);
};
