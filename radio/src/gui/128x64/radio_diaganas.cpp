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
#include "../../hal/adc_driver.h"

#define HOLDANAVALUEFRAMES 4 /* 4* 50ms = 200 ms update rate */

void menuRadioDiagAnalogs(event_t event)
{
    static int8_t entryCount = 0;
    static uint16_t lastShownAnalogValue[NUM_STICKS+NUM_POTS+NUM_SLIDERS];

    enum ANAVIEWS{
       ANAVIEW_FIRST,
       ANAVIEW_CALIB=ANAVIEW_FIRST,
       ANAVIEW_RAWLOWFPS,
       ANAVIEW_LAST=ANAVIEW_RAWLOWFPS
    };

    static int viewpage = ANAVIEW_FIRST;

// TODO enum
#if defined(TX_CAPACITY_MEASUREMENT)
  #define ANAS_ITEMS_COUNT 3
#else
  #define ANAS_ITEMS_COUNT 1
#endif

  switch (event) {
    case EVT_KEY_FIRST(KEY_UP):
#if defined(KEYS_GPIO_REG_PAGEDN)
    case EVT_KEY_BREAK(KEY_PAGEDN):
#elif defined(NAVIGATION_X7)
    case EVT_KEY_BREAK(KEY_PAGE):
#endif
    {
     if (viewpage == ANAVIEW_LAST)
       viewpage = ANAVIEW_FIRST;
     else
       viewpage++;
     if (viewpage == ANAVIEW_RAWLOWFPS) entryCount = 0;
     break;
    }

    case EVT_KEY_FIRST(KEY_DOWN):
#if defined(KEYS_GPIO_REG_PAGEUP)
    case EVT_KEY_BREAK(KEY_PAGEUP):
#elif defined(NAVIGATION_X7)
    case EVT_KEY_LONG(KEY_PAGE):
#endif
    {
     if (viewpage == ANAVIEW_FIRST)
       viewpage = ANAVIEW_LAST;
     else
       viewpage--;
     if (viewpage == ANAVIEW_RAWLOWFPS) entryCount = 0;
      break;
    }
  }

  switch (viewpage) {
    case (ANAVIEW_CALIB):
      SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_CALIB,
                     HEADER_LINE+ANAS_ITEMS_COUNT);
      break;
    case (ANAVIEW_RAWLOWFPS):
      SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_RAWLOWFPS,
                     HEADER_LINE+ANAS_ITEMS_COUNT);
      break;
  }

  coord_t y = MENU_HEADER_HEIGHT + 1;

  lcdDrawTextAlignedLeft(y, STICKS_PWM_ENABLED() ? STR_PWM_STICKS_POTS_SLIDERS
                                                 : STR_STICKS_POTS_SLIDERS);

  for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
    uint8_t x;
    if (i & 1) {
      x = LCD_W / 2 + INDENT_WIDTH;
    } else {
      x = INDENT_WIDTH;
      y += FH;
    }
    drawStringWithIndex(x, y, "A", i + 1);
    lcdDrawChar(lcdNextPos, y, ':');
    switch (viewpage) {
      case (ANAVIEW_RAWLOWFPS):
        if (entryCount == 0) {
          lastShownAnalogValue[i] = getAnalogValue(i); // Update value
        }
        lcdDrawNumber(x+3*FW-1, y, lastShownAnalogValue[i],
                      LEADING0|LEFT, 4);
        break;
      case (ANAVIEW_CALIB):
      default:
        lcdDrawNumber(x+3*FW-1, y, anaIn(i), LEADING0|LEFT, 4);
        break;
    }
    lcdDrawNumber(x+10*FW-1, y,
                  (int16_t)calibratedAnalogs[CONVERT_MODE(i)]*25/256,
                  RIGHT);
  }

  if (viewpage == ANAVIEW_RAWLOWFPS) {
    if (entryCount > HOLDANAVALUEFRAMES)
      entryCount = 0;
    else
      entryCount++;
  }

#if defined(IMU)
  y += FH;
  lcdDrawTextAlignedLeft(y, STR_IMU);
  y += FH;
  uint8_t x = INDENT_WIDTH;
  lcdDrawText(x, y, "X:");
  lcdDrawNumber(x+3*FW-1, y, gyro.outputs[0] * 180 / 1024);
  lcdDrawChar(lcdNextPos, y, '@');
  lcdDrawNumber(x+10*FW-1, y, gyro.scaledX(), RIGHT);
  x = LCD_W/2 + INDENT_WIDTH;
  lcdDrawText(x, y, "Y:");
  lcdDrawNumber(x+3*FW-1, y, gyro.outputs[1] * 180 / 1024);
  lcdDrawChar(lcdNextPos, y, '@');
  lcdDrawNumber(x+10*FW-1, y, gyro.scaledY(), RIGHT);
#endif
}
