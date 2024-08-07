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

#define MAXIMUM_SPLASH_IMAGE_SIZE 3072
const struct {
  const uint8_t __magic_prefix[4] = { 'S','P','S',0 };
  const uint8_t splashdata[MAXIMUM_SPLASH_IMAGE_SIZE] = {
    #include "bitmaps/212x64/splash.lbm"
  };
  const uint8_t __magic_suffix[4] = { 'S','P','E',0 };
} splash_struct;
static_assert(sizeof(splash_struct.splashdata) <= MAXIMUM_SPLASH_IMAGE_SIZE, "");
const unsigned char * const splash_lbm = splash_struct.splashdata;

void drawSplash()
{
  lcdClear();
  lcdDrawRleBitmap(0, 0, splash_lbm);
  lcdRefresh();
}
