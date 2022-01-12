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

void menuRadioDiagAnalogs(event_t event)
{
  SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS, 0);

  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + (i/2)*FH;
    uint8_t x = i&1 ? LCD_W/2 + FW : 0;
    lcdDrawNumber(x, y, i+1, LEADING0|LEFT, 2);
    lcdDrawChar(x+2*FW-2, y, ':');
    lcdDrawHexNumber(x+3*FW-1, y, getAnalogValue(i));
#if defined(JITTER_MEASURE)
    lcdDrawNumber(x+10*FW-1, y, rawJitter[i].get(), RIGHT);
    lcdDrawNumber(x+13*FW-1, y, avgJitter[i].get(), RIGHT);
    lcdDrawNumber(x+17*FW-1, y, (int16_t)calibratedAnalogs[CONVERT_MODE(i)]*25/256, RIGHT);
#else
    lcdDrawNumber(x+10*FW-1, y, (int16_t)calibratedAnalogs[CONVERT_MODE(i)]*25/256, RIGHT);
#endif
  }
}
