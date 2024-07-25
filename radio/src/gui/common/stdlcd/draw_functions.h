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

#include "lcd.h"
void lcdDrawMultiProtocolString(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t protocol, LcdFlags flags = 0);
void lcdDrawMultiSubProtocolString(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t subType, LcdFlags flags = 0);
void drawStringWithIndex(coord_t x, coord_t y, const char * str, uint8_t idx, LcdFlags att=0);
void drawValueWithUnit(coord_t x, coord_t y, int32_t val, uint8_t unit, LcdFlags att=0);

void drawPower(coord_t x, coord_t y, int8_t dBm, LcdFlags att = 0);
void drawGVarName(coord_t x, coord_t y, int8_t index, LcdFlags flags=0);
void drawReceiverName(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t receiverIdx, LcdFlags flags=0);

void drawRtcTime(coord_t x, coord_t y, LcdFlags att);
void drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags att, LcdFlags att2);
inline void drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags att = 0)
{
  drawTimer(x, y, tme, att, att);
}

void drawFlightMode(coord_t x, coord_t y, int8_t idx, LcdFlags att = 0);

void drawStartupAnimation(uint32_t duration, uint32_t totalDuration);
void drawShutdownAnimation(uint32_t duration, uint32_t totalDuration, const char * message);
void drawSleepBitmap();
void drawFatalErrorScreen(const char * message);
void runFatalErrorScreen(const char * message);

void lcdDrawMMM(coord_t x, coord_t y, LcdFlags flags=0);
void drawTrimMode(coord_t x, coord_t y, uint8_t flightMode, uint8_t idx, LcdFlags att=0);

typedef int (*FnFuncP) (int x);
void drawFunction(FnFuncP fn, uint8_t offset = 0);
void drawCursor(FnFuncP fn, uint8_t offset = 0);
void drawCurve(coord_t offset = 0);

void drawSource(coord_t x, coord_t y, mixsrc_t idx, LcdFlags att=0);
