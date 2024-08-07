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


#define ANIMATIONS_BITMAP_WIDTH     30
#define ANIMATIONS_BITMAP_HEIGHT    30

#define POWER_BITMAP_WIDTH          40
#define POWER_BITMAP_HEIGHT         40

#define SLEEP_BITMAP_WIDTH          60
#define SLEEP_BITMAP_HEIGHT         60


const unsigned char bmp_lock[] = {
#include "lock.lbm"
};

const unsigned char bmp_shutdown[] = {
#include "shutdown.lbm"
};
const int8_t bmp_shutdown_xo[] = {
  0, 0, -ANIMATIONS_BITMAP_WIDTH, -ANIMATIONS_BITMAP_WIDTH
};
const int8_t bmp_shutdown_yo[] = {
  -ANIMATIONS_BITMAP_HEIGHT, 0, 0, -ANIMATIONS_BITMAP_HEIGHT
};

const unsigned char bmp_power[] = {
#include "power.lbm"
};

const unsigned char bmp_sleep[] = {
#include "sleep.lbm"
};

static void drawAnimation(uint8_t idx)
{
  lcdDrawRleBitmap((LCD_W - POWER_BITMAP_WIDTH) / 2, (LCD_H - POWER_BITMAP_HEIGHT) / 2, bmp_power);
  for (uint8_t i = 0; i <=idx; i += 1)
    lcdDrawRleBitmap((LCD_W / 2) + bmp_shutdown_xo[i], (LCD_H / 2) + bmp_shutdown_yo[i], bmp_shutdown, i * ANIMATIONS_BITMAP_WIDTH, ANIMATIONS_BITMAP_WIDTH, true);
}

void drawStartupAnimation(uint32_t duration, uint32_t totalDuration)
{
  if (totalDuration == 0)
    return;

  uint8_t index = limit<uint8_t>(0, duration / (totalDuration / 5), 4);

  lcdRefreshWait();
  lcdClear();

  if (index == 0)
    lcdDrawRleBitmap((LCD_W - POWER_BITMAP_WIDTH) / 2, (LCD_H - POWER_BITMAP_HEIGHT) / 2, bmp_lock);
  else
    drawAnimation(index - 1);

  lcdRefresh();
}

void drawShutdownAnimation(uint32_t duration, uint32_t totalDuration, const char * message)
{
  if (totalDuration == 0)
    return;

  uint8_t index = limit<uint8_t>(0, duration / (totalDuration / 4), 3);

  lcdRefreshWait();
  lcdClear();

  drawAnimation(3 - index);

  if (message) {
    lcdDrawText((LCD_W - getTextWidth(message)) / 2, LCD_H - 2*FH, message);
  }

  lcdRefresh();
}

void drawSleepBitmap()
{
  lcdRefreshWait();
  lcdClear();
  lcdDrawRleBitmap((LCD_W - SLEEP_BITMAP_WIDTH) / 2, (LCD_H - SLEEP_BITMAP_HEIGHT) / 2, bmp_sleep);
  lcdRefresh();
  lcdRefreshWait();
}
