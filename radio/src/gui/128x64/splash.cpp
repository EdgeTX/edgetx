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

#include "edgetx.h"

const unsigned char splashdata[]  = {
  'S','P','S',0,
  #include "bitmaps/128x64/splash.lbm"
  'S','P','E',0 };

const unsigned char * const splash_lbm = splashdata+4;

void drawSplash()
{
  lcdClear();
  lcdDraw1bitBitmap(0, 0, splash_lbm, 0, 0);
  lcdRefresh();
}
