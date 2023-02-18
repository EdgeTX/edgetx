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

#include "opentx.h"


#define ANIMATIONS_BITMAP_WIDTH       60
#define ANIMATIONS_BITMAP_HEIGHT      60


const unsigned char bmp_lock[] = {
#include "lock.lbm"
};

const unsigned char bmp_shutdown[] = {
#include "shutdown.lbm"
};

const unsigned char bmp_sleep[] = {
#include "sleep.lbm"
};

void drawStartupAnimation(uint32_t duration, uint32_t totalDuration)
{
  if (totalDuration == 0)
    return;

  uint8_t index = limit<uint8_t>(0, duration / (totalDuration / 5), 4);

  lcdRefreshWait();
  lcdClear();

  if (index == 0)
    lcdDrawRleBitmap((LCD_W - ANIMATIONS_BITMAP_WIDTH) / 2, (LCD_H - ANIMATIONS_BITMAP_HEIGHT) / 2, bmp_lock, 0, ANIMATIONS_BITMAP_WIDTH);
  else
    lcdDrawRleBitmap((LCD_W - ANIMATIONS_BITMAP_WIDTH) / 2, (LCD_H - ANIMATIONS_BITMAP_HEIGHT) / 2, bmp_shutdown, (index - 1) * ANIMATIONS_BITMAP_WIDTH, ANIMATIONS_BITMAP_WIDTH);

  lcdRefresh();
}

void drawShutdownAnimation(uint32_t duration, uint32_t totalDuration, const char * message)
{
  if (totalDuration == 0)
    return;

  uint8_t index = limit<uint8_t>(0, duration / (totalDuration / 4), 3);

  lcdRefreshWait();
  lcdClear();

  lcdDrawRleBitmap((LCD_W - ANIMATIONS_BITMAP_WIDTH) / 2, (LCD_H - ANIMATIONS_BITMAP_HEIGHT) / 2, bmp_shutdown, (3 - index) * ANIMATIONS_BITMAP_WIDTH, ANIMATIONS_BITMAP_WIDTH);
  if (message) {
    lcdDrawText((LCD_W - getTextWidth(message)) / 2, LCD_H - 2*FH, message);
  }

  lcdRefresh();
}

void drawSleepBitmap()
{
  lcdRefreshWait();
  lcdClear();
  lcdDrawRleBitmap((LCD_W - ANIMATIONS_BITMAP_WIDTH) / 2, (LCD_H - ANIMATIONS_BITMAP_HEIGHT) / 2, bmp_sleep, 0);
  lcdRefresh();
  lcdRefreshWait();
}
