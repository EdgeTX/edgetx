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
#include "../../hal/adc_driver.h"

#define HOLDANAVALUEFRAMES 4 /* 4* 50ms = 200 ms update rate */

void menuRadioDiagAnalogs(event_t event)
{
    static int8_t entryCount = 0;
    static uint16_t lastShownAnalogValue[MAX_ANALOG_INPUTS];

    enum ANAVIEWS{
       ANAVIEW_FIRST,
       ANAVIEW_CALIB=ANAVIEW_FIRST,
       ANAVIEW_RAWLOWFPS,
       ANAVIEW_LAST=ANAVIEW_RAWLOWFPS
    };

    static int viewpage = ANAVIEW_FIRST;

  if (event == EVT_KEY_BREAK(KEY_PAGEDN)) {
      if (viewpage == ANAVIEW_LAST)
        viewpage = ANAVIEW_FIRST;
      else
          viewpage++;
      if (viewpage == ANAVIEW_RAWLOWFPS)
        entryCount = 0;
  }

  switch (viewpage)
  {
    case (ANAVIEW_CALIB): SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_CALIB, 0); break;
    case (ANAVIEW_RAWLOWFPS): SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_RAWLOWFPS, 0); break;
  }

  for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + (i/2)*FH;
    uint8_t x = i&1 ? LCD_W/2 + FW : 0;
    if (((adcGetInputMask() & (1 << i)) != 0) && i < adcGetMaxInputs(ADC_INPUT_MAIN)) {
      lcdDrawText(x, y, "D");
      lcdDrawNumber(lcdNextPos, y, i + 1);
    }
    else {
      lcdDrawNumber(x, y, i + 1, LEADING0 | LEFT, 2);
    }
    lcdDrawChar(x+2*FW-2, y, ':');
    
    switch (viewpage)
    {
      case (ANAVIEW_RAWLOWFPS):
        if (entryCount == 0)
        {
          lastShownAnalogValue[i] = getAnalogValue(i); // Update value
        }
        lcdDrawNumber(x+3*FW-1, y, lastShownAnalogValue[i], LEADING0|LEFT, 4);
        break;
      case (ANAVIEW_CALIB):
      default:
        lcdDrawNumber(x+3*FW-1, y, anaIn(i), LEADING0|LEFT, 4); break;
    }
#if defined(JITTER_MEASURE)
    lcdDrawNumber(x+10*FW-1, y, rawJitter[i].get(), RIGHT);
    lcdDrawNumber(x+13*FW-1, y, avgJitter[i].get(), RIGHT);
    lcdDrawNumber(x+17*FW-1, y, (int16_t)calibratedAnalogs[i]*25/256, RIGHT);
#else
    lcdDrawNumber(x+10*FW-1, y, (int16_t)calibratedAnalogs[i]*25/256, RIGHT);
#endif
  }

  if (viewpage == ANAVIEW_RAWLOWFPS)
  {
    if (entryCount > HOLDANAVALUEFRAMES)
      entryCount = 0;
    else
      entryCount++;
  }
}
