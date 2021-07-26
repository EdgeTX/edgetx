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

void drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags att, LcdFlags att2)
{
  div_t qr;

  if (IS_RIGHT_ALIGNED(att)) {
    att -= RIGHT;
    if (att & DBLSIZE)
      x -= 5*(2*FWNUM)-4;
    else if (att & MIDSIZE)
      x -= 5*8-4;
    else
      x -= 5*FWNUM+1;
  }

  if (tme < 0) {
    lcdDrawChar(x - ((att & DBLSIZE) ? FW+2 : ((att & MIDSIZE) ? FW+0 : FWNUM)), y, '-', att);
    tme = -tme;
  }

  qr = div((int)tme, 60);

#define separator ':'
  if (att & TIMEHOUR) {
    div_t qr2 = div(qr.quot, 60);
    if (qr2.quot < 100) {
      lcdDrawNumber(x, y, qr2.quot, att|LEADING0|LEFT, 2);
    }
    else {
      lcdDrawNumber(x, y, qr2.quot, att|LEFT);
    }
    lcdDrawChar(lcdNextPos, y, separator, att);
    qr.quot = qr2.rem;
    x = lcdNextPos;
  }

  lcdDrawNumber(x, y, qr.quot, att|LEADING0|LEFT, 2);
  if (att & TIMEBLINK)
    lcdDrawChar(lcdNextPos, y, separator, BLINK);
  else
    lcdDrawChar(lcdNextPos, y, separator, att&att2);
  lcdDrawNumber(lcdNextPos, y, qr.rem, (att2|LEADING0) & (~RIGHT), 2);
}


void drawTelemetryTopBar()
{
  drawModelName(0, 0, g_model.header.name, g_eeGeneral.currModel, 0);
  uint8_t att = (IS_TXBATT_WARNING() ? BLINK : 0);
  putsVBat(12*FW, 0, att);
  if (g_model.timers[0].mode) {
    att = (timersStates[0].val<0 ? BLINK : 0);
    drawTimer(22*FW, 0, timersStates[0].val, att, att);
    lcdDrawText(22*FW, 0, "T1:", RIGHT);
  }
  if (g_model.timers[1].mode) {
    att = (timersStates[1].val<0 ? BLINK : 0);
    drawTimer(31*FW, 0, timersStates[1].val, att, att);
    lcdDrawText(31*FW, 0, "T2:", RIGHT);
  }
  lcdInvertLine(0);
}


void drawTelemScreenDate(coord_t x, coord_t y, source_t sensor, LcdFlags att)
{
  sensor = (sensor-MIXSRC_FIRST_TELEM) / 3;
  TelemetryItem & telemetryItem = telemetryItems[sensor];

  att &= ~FONTSIZE_MASK;
  lcdDrawNumber(x, y+1, telemetryItem.datetime.sec, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y+1, ":", att);
  lcdDrawNumber(lcdNextPos, y+1, telemetryItem.datetime.min, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y+1, ":", att);
  lcdDrawNumber(lcdNextPos, y+1, telemetryItem.datetime.hour, att|LEADING0, 2);

  lcdDrawNumber(x, y+9, telemetryItem.datetime.day, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y+9, "-", att);
  lcdDrawNumber(lcdNextPos, y+9, telemetryItem.datetime.month, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y+9, "-", att);
  lcdDrawNumber(lcdNextPos, y+9, telemetryItem.datetime.year, att|LEADING0,4);
}

void drawGPSSensorValue(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags att)
{
  if (att & DBLSIZE) {
    x -= (g_eeGeneral.gpsFormat == 0 ? 54 : 51);
    att &= ~FONTSIZE_MASK;
    drawGPSCoord(x, y, telemetryItem.gps.latitude, "NS", att, true);
    drawGPSCoord(x, y+FH, telemetryItem.gps.longitude, "EW", att, true);
  }
  else {
    drawGPSCoord(x, y, telemetryItem.gps.latitude, "NS", att, false);
    drawGPSCoord(lcdLastRightPos+2*FWNUM, y, telemetryItem.gps.longitude, "EW", att, false);
  }
}

#if !defined(BOOT)
void lcdDrawRleBitmap(coord_t x, coord_t y, const uint8_t * img, coord_t offset, coord_t width)
{
  RleBitmap pic(img, offset);

  uint8_t w = pic.getWidth();
  if (!width || width > w) {
    width = w;
  }
  if (x+width > LCD_W) {
    if (x >= LCD_W ) return;
    width = LCD_W-x;
  }
  uint8_t rows = pic.getRows();

  for (uint8_t row=0; row<rows; row++) {
    uint8_t *p = &displayBuf[(row + (y/2)) * LCD_W + x];
    for (coord_t i=0; i<width; i++) {
      if (p >= DISPLAY_END) return;
      uint8_t b = pic.getNext();
      if (y & 1) {
        *p = (*p & 0x0f) + ((b & 0x0f) << 4);
        if ((p+LCD_W) < DISPLAY_END) {
          *(p+LCD_W) = (*(p+LCD_W) & 0xf0) + ((b & 0xf0) >> 4);
        }
      }
      else {
        *p = b;
      }
      p++;
    }
    pic.goToNextRow();
    pic.skip(offset);
  }
}
#endif
