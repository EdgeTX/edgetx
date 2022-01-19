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
      x -= 5*8-8;
    else
      x -= 5*FWNUM+1;
  }

  if (tme < 0) {
    lcdDrawChar(x - ((att & DBLSIZE) ? FW+2 : ((att & MIDSIZE) ? FW+0 : FWNUM)), y, '-', att);
    tme = -tme;
  }

  qr = div((int)tme, 60);

  char separator = ':';
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
  if (FONTSIZE(att) == MIDSIZE) {
    lcdLastRightPos--;
  }
  if (separator == CHR_HOUR)
    att &= ~DBLSIZE;
  lcdDrawNumber(x, y, qr.quot, att|LEADING0|LEFT, 2);
#if defined(RTCLOCK)
  if (att & TIMEBLINK)
    lcdDrawChar(lcdLastRightPos, y, separator, BLINK);
  else
#endif
  lcdDrawChar(lcdLastRightPos, y, separator, att&att2);
  lcdDrawNumber(lcdNextPos, y, qr.rem, (att2|LEADING0|LEFT) & (~RIGHT), 2);
}

void drawTimerWithMode(coord_t x, coord_t y, uint8_t index, LcdFlags att)
{
  const TimerData &timer = g_model.timers[index];

  if (timer.mode) {
    const TimerState &timerState = timersStates[index];
    const uint8_t negative = (timerState.val < 0 ? BLINK | INVERS : 0);
    if (timerState.val < 60 * 60) { // display MM:SS
      div_t qr = div((int) abs(timerState.val), 60);
      lcdDrawNumber(x - 5, y, qr.rem, att | LEADING0 | negative, 2);
      lcdDrawText(lcdLastLeftPos, y, ":", att | BLINK | negative);
      lcdDrawNumber(lcdLastLeftPos, y, qr.quot, att | negative);
      if (negative)
        lcdDrawText(lcdLastLeftPos, y, "-", att | negative);
    }
    else if (timerState.val < (99 * 60 * 60) + (59 * 60)) { // display HHhMM
      div_t qr = div((int) (abs(timerState.val) / 60), 60);
      lcdDrawNumber(x - 5, y, qr.rem, att | LEADING0, 2);
      lcdDrawText(lcdLastLeftPos, y, "h", att);
      lcdDrawNumber(lcdLastLeftPos, y, qr.quot, att);
      if (negative)
        lcdDrawText(lcdLastLeftPos, y, "-", att);
    }
    else {  //display HHHH for crazy large persistent timers
      lcdDrawText(x - 5, y, "h", att);
      lcdDrawNumber(lcdLastLeftPos, y, timerState.val / 3600, att);
    }
    uint8_t xLabel = (negative ? x - 56 : x - 49);
    uint8_t len = zlen(timer.name, LEN_TIMER_NAME);
    if (len > 0) {
      lcdDrawSizedText(xLabel, y + FH, timer.name, len, RIGHT);
    }
    else {
      drawTimerMode(xLabel, y + FH, timer.mode, RIGHT);
    }
  }
}


void drawTelemScreenDate(coord_t x, coord_t y, source_t sensor, LcdFlags att)
{
  y+=3;
  sensor = (sensor-MIXSRC_FIRST_TELEM) / 3;
  TelemetryItem & telemetryItem = telemetryItems[sensor];

  lcdDrawNumber(x, y, telemetryItem.datetime.hour, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y, ":", att);
  lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.min, att|LEADING0, 2);
  lcdDrawText(lcdNextPos, y, ":", att);
  lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.sec, att|LEADING0, 2);

  lcdDrawNumber(x-29, y, telemetryItem.datetime.month, att|LEADING0|LEFT, 2);
  lcdDrawChar(lcdNextPos, y, '-', att);
  lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.day, att|LEADING0|LEFT, 2);
}

void drawGPSPosition(coord_t x, coord_t y, int32_t longitude, int32_t latitude, LcdFlags flags)
{
  if (flags & DBLSIZE) {
    x -= (g_eeGeneral.gpsFormat == 0 ? 62 : 61);
    flags &= ~0x0F00; // TODO constant
    drawGPSCoord(x, y, latitude, "NS", flags);
    drawGPSCoord(x, y+FH, longitude, "EW", flags);
  }
  else {
    drawGPSCoord(x, y, latitude, "NS", flags, false);
    drawGPSCoord(lcdNextPos+FWNUM, y, longitude, "EW", flags, false);
  }
}

void drawGPSSensorValue(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags)
{
  drawGPSPosition(x, y, telemetryItem.gps.longitude, telemetryItem.gps.latitude, flags);
}

void drawTelemetryTopBar()
{
  drawModelName(0, 0, g_model.header.name, g_eeGeneral.currModel, 0);
  uint8_t att = (IS_TXBATT_WARNING() ? BLINK : 0);
  putsVBat(14*FW,0,att);
  if (g_model.timers[0].mode) {
    att = (timersStates[0].val<0 ? BLINK : 0);
    drawTimer(17*FW+5*FWNUM+1, 0, timersStates[0].val, att, att);
  }
  lcdInvertLine(0);
}

#define VBATT_W         (LCD_W - 40)
#define VBATT_H         (45)
#define VBATT_X         (((LCD_W - VBATT_W) / 2) - 2)
#define VBATT_Y         (((LCD_H - VBATT_H) / 2) + 5)
#define GET_BAT_BARS()  (limit<int8_t>(2, 20 * (g_vbat100mV + 56 - 90) / (30 - 78 + 56), 20))
#define MAX_BARS()      (limit<int8_t>(2, 20 * (BATTERY_MAX + 56 - 90) / (30 - 78 + 56), 20))

void drawChargingState(void)
{
  static uint8_t old_count = 0;
  static uint8_t blink_count = 0;
  static uint32_t charging_time = g_tmr10ms;

  putsVBat(VBATT_X + 54, VBATT_Y - 13, RIGHT|MIDSIZE);
  lcdDrawSolidFilledRect(VBATT_X - 4, VBATT_Y, VBATT_W + 4, VBATT_H);
  uint8_t count = GET_BAT_BARS();

  for (uint8_t i = 0; i < count; i += 2) {
    lcdDrawSolidFilledRect(VBATT_X + 2  + (VBATT_W / (MAX_BARS() - 1) * (i )), VBATT_Y + 7,  VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
  }
  lcdDrawSolidFilledRect(VBATT_X + VBATT_W , VBATT_Y + 10, 9, VBATT_H - 10 * 2);

  if (old_count != count) {
    old_count = count;
    blink_count = (count % 2) ? count + 1: count;
  }
  for (uint8_t i = (old_count % 2) ? old_count + 1: old_count; i < blink_count; i += 2) {
    lcdDrawSolidFilledRect(VBATT_X + 2 + (VBATT_W / (MAX_BARS() - 1) * (i )), VBATT_Y + 7,  VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
  }
  if ((g_tmr10ms - charging_time) > 6) {
    if ((blink_count ++) >= MAX_BARS()) {
      blink_count = old_count;
    }
    charging_time = g_tmr10ms;
  }
}

void drawFullyCharged(void)
{
  putsVolts(VBATT_X + 54, VBATT_Y - 13, 42, RIGHT|MIDSIZE);

  lcdDrawSolidFilledRect(VBATT_X - 4, VBATT_Y, VBATT_W + 4, VBATT_H);

  for (uint8_t i = 0; i < MAX_BARS(); i += 2) {
    lcdDrawSolidFilledRect(VBATT_X + 2  + (VBATT_W / (MAX_BARS() - 1) * (i )), VBATT_Y + 7,  VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
  }
  lcdDrawSolidFilledRect(VBATT_X + VBATT_W , VBATT_Y + 10, 9, VBATT_H - 10 * 2);

  lcdDrawText(VBATT_X - 8 , VBATT_Y + VBATT_H + 3, "FULLY CHARGED", MIDSIZE);
}
