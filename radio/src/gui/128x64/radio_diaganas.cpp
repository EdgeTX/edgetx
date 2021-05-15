/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

void menuRadioDiagAnalogs(event_t event)
{
// TODO enum
#if defined(TX_CAPACITY_MEASUREMENT)
  #define ANAS_ITEMS_COUNT 3
#elif defined(PCBSKY9X)
  #define ANAS_ITEMS_COUNT 2
#else
  #define ANAS_ITEMS_COUNT 1
#endif

  SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS, HEADER_LINE+ANAS_ITEMS_COUNT);

  coord_t y = MENU_HEADER_HEIGHT + 1;

  lcdDrawTextAlignedLeft(y, STICKS_PWM_ENABLED() ? STR_PWM_STICKS_POTS_SLIDERS : STR_STICKS_POTS_SLIDERS);

  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
    uint8_t x;
    if (i & 1) {
      x = LCD_W/2 + INDENT_WIDTH;
    }
    else {
      x = INDENT_WIDTH;
      y += FH;
    }
    drawStringWithIndex(x, y, "A", i+1);
    lcdDrawChar(lcdNextPos, y, ':');
    lcdDrawHexNumber(x+3*FW-1, y, anaIn(i));
    lcdDrawNumber(x+10*FW-1, y, (int16_t)calibratedAnalogs[CONVERT_MODE(i)]*25/256, RIGHT);
  }

#if defined(GYRO)
  y += FH;
  lcdDrawTextAlignedLeft(y, STR_GYRO);
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
