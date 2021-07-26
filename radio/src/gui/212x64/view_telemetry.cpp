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

#include "opentx.h"

#define STATUS_BAR_Y     (7*FH+1)

uint8_t s_frsky_view = 0;

#define BAR_LEFT    30
#define BAR_WIDTH   152

void displayRssiLine()
{
  if (TELEMETRY_STREAMING()) {
    lcdDrawSolidHorizontalLine(0, 55, 212, 0); // separator
    uint8_t rssi = min((uint8_t)99, TELEMETRY_RSSI());
    lcdDrawSizedText(0, STATUS_BAR_Y, STR_RX, 2);
    lcdDrawNumber(4*FW, STATUS_BAR_Y, rssi, LEADING0|RIGHT, 2);
    lcdDrawRect(BAR_LEFT, 57, 78, 7);
    lcdDrawFilledRect(BAR_LEFT+1, 58, 19*rssi/25, 5, (rssi < g_model.rssiAlarms.getWarningRssi()) ? DOTTED : SOLID);
  }
  else {
    lcdDrawText(7*FW, STATUS_BAR_Y, STR_NODATA, BLINK);
    lcdInvertLastLine();
  }
}

int barCoord(int value, int min, int max)
{
  if (value <= min)
    return 0;
  else if (value >= max)
    return BAR_WIDTH-1;
  else
    return ((BAR_WIDTH-1) * (value - min)) / (max - min);
}

void displayGaugesTelemetryScreen(TelemetryScreenData & screen)
{
  // Custom Screen with gauges
  int barHeight = 5;
  for (int i=3; i>=0; i--) {
    FrSkyBarData & bar = screen.bars[i];
    source_t source = bar.source;
    getvalue_t barMin = bar.barMin;
    getvalue_t barMax = bar.barMax;
    if (source <= MIXSRC_LAST_CH) {
      barMin = calc100toRESX(barMin);
      barMax = calc100toRESX(barMax);
    }
    if (source) {
      int y = barHeight+6+i*(barHeight+6);
      drawSource(0, y+barHeight-5, source, 0);
      lcdDrawRect(BAR_LEFT, y, BAR_WIDTH+1, barHeight+2);
      getvalue_t value = getValue(source);
      drawSourceValue(BAR_LEFT+2+BAR_WIDTH, y+barHeight-5, source, LEFT);
      uint8_t thresholdX = 0;
      uint8_t width = (barMin < barMax) ? barCoord(value, barMin, barMax) : limit(0, 151 - barCoord(value, barMax, barMin), 151);
      uint8_t barShade = SOLID;
      lcdDrawFilledRect(BAR_LEFT+1, y+1, width, barHeight, barShade);
      for (uint8_t j=24; j<99; j+=25) {
        if (j>thresholdX || j>width) {
          lcdDrawSolidVerticalLine(j*BAR_WIDTH/100+BAR_LEFT+1, y+1, barHeight);
        }
      }
      if (thresholdX) {
        lcdDrawVerticalLine(BAR_LEFT+1+thresholdX, y-2, barHeight+3, DOTTED);
        lcdDrawSolidHorizontalLine(BAR_LEFT+thresholdX, y-2, 3);
      }
    }
    else {
      barHeight += 2;
    }
  }
  displayRssiLine();
}

bool displayNumbersTelemetryScreen(TelemetryScreenData & screen)
{
  // Custom Screen with numbers
  const coord_t pos[] = {0, 71, 143, 214};
  uint8_t fields_count = 0;
  lcdDrawSolidVerticalLine(69, 8, 48);
  lcdDrawSolidVerticalLine(141, 8, 48);
  for (uint8_t i=0; i<4; i++) {
    for (uint8_t j=0; j<NUM_LINE_ITEMS; j++) {
      source_t field = screen.lines[i].sources[j];
      if (field > 0) {
        fields_count++;
      }
      if (i==3) {
        if (!TELEMETRY_STREAMING()) {
          displayRssiLine();
          return fields_count;
        }
      }
      if (field) {
        coord_t x = pos[j+1]-2;
        coord_t y = (i==3 ? 1+FH+2*FH*i:FH+2*FH*i);
        LcdFlags att = RIGHT | (i==3 ? NO_UNIT : DBLSIZE|NO_UNIT);
        if (field >= MIXSRC_FIRST_TIMER && field <= MIXSRC_LAST_TIMER && i!=3) {
          // there is not enough space on LCD for displaying "Tmr1" or "Tmr2" and still see the - sign, we write "T1" or "T2" instead
          drawStringWithIndex(pos[j], 1+FH+2*FH*i, "T", field-MIXSRC_FIRST_TIMER+1, 0);
          if (timersStates[field-MIXSRC_FIRST_TIMER].val > 3600) {
            att += TIMEHOUR - DBLSIZE;
            x -= 3*FW;
            y += FH/2;
          }
        }
        else if (field >= MIXSRC_FIRST_TELEM && isGPSSensor(1+(field-MIXSRC_FIRST_TELEM)/3) && telemetryItems[(field-MIXSRC_FIRST_TELEM)/3].isAvailable()) {
          // we don't display GPS name, no space for it, but we shift x by some pixel to allow it to fit on max coord
          x -=2;
        }
        else {
          drawSource(pos[j], 1+FH+2*FH*i, field, 0);
        }

        if (field >= MIXSRC_FIRST_TELEM) {
          TelemetryItem & telemetryItem = telemetryItems[(field-MIXSRC_FIRST_TELEM)/3]; // TODO macro to convert a source to a telemetry index
          if (!telemetryItem.isAvailable()) {
            continue;
          }
          else if (telemetryItem.isOld()) {
            att |= INVERS|BLINK;
          }
        }

        if(isSensorUnit(1+(field-MIXSRC_FIRST_TELEM)/3, UNIT_DATETIME) && field >= MIXSRC_FIRST_TELEM) {
          drawTelemScreenDate(x, y, field, att);
        }
        else {
          drawSourceValue(x, y, field, att);
        }
      }
    }
  }
  lcdInvertLastLine();
  return fields_count;
}

void displayCustomTelemetryScreen(uint8_t index)
{
  TelemetryScreenData & screen = g_model.screens[index];

  if (IS_BARS_SCREEN(s_frsky_view)) {
    return displayGaugesTelemetryScreen(screen);
  }

  displayNumbersTelemetryScreen(screen);
}

bool displayTelemetryScreen()
{
#if defined(LUA)
  if (TELEMETRY_SCREEN_TYPE(s_frsky_view) == TELEMETRY_SCREEN_TYPE_SCRIPT) {
    uint8_t state = isTelemetryScriptAvailable(s_frsky_view);
    switch (state) {
      case SCRIPT_OK:
        return true;  // contents will be drawed by Lua Task
      case SCRIPT_NOFILE:
        return false;  // requested lua telemetry screen not available
      case SCRIPT_SYNTAX_ERROR:
      case SCRIPT_PANIC:
        luaError(lsScripts, state, false);
        return true;
    }
    return false;
  }
#endif

  if (TELEMETRY_SCREEN_TYPE(s_frsky_view) == TELEMETRY_SCREEN_TYPE_NONE) {
    return false;
  }

  drawTelemetryTopBar();

  if (s_frsky_view < MAX_TELEMETRY_SCREENS) {
    displayCustomTelemetryScreen(s_frsky_view);
  }

  return true;
}

