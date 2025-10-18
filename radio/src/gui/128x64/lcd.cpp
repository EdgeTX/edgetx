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

#include "lcd.h"
#include "bitfield.h"
#include "utf8.h"

#if !defined(SIMU)
  #define assert(x)
#else
  #include <assert.h>
#endif

#if !defined(BOOT)
  #include "edgetx.h"
#endif

extern LcdFlags getCharPattern(PatternData * pattern, unsigned char c, LcdFlags flags);

void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags)
{
  const coord_t orig_x = x;

  const uint8_t orig_len = len;
  uint32_t fontsize = FONTSIZE(flags);

  uint8_t width = 0;
  if (flags & RIGHT) {
    width = getTextWidth(s, len, flags);
    x -= width;
  }
  else if (flags & CENTERED) {
    width = getTextWidth(s, len, flags);
    x -= width / 2;
  }

  bool setx = false;
  while (len--) {
    unsigned char c = *s;

    if (setx) {
      x = c;
      setx = false;
    }
    else if (!c) {
      break;
    }
    else if (c >= 0x20) {
      // UTF8 detection
      c = map_utf8_char(s, len);
      if (!c) break;

      lcdDrawChar(x, y, c, flags);
      x = lcdNextPos;
    }
    else if (c == 0x1F) {  //X-coord prefix
      setx = true;
    }
    else if (c == 0x1E) {  //NEWLINE
      len = orig_len;
      x = orig_x;
      y += FH;
      if (fontsize == DBLSIZE)
        y += FH;
      else if (fontsize == MIDSIZE)
        y += 4;
      else if (fontsize == SMLSIZE)
        y--;
      if (y >= LCD_H) break;
    }
    else if (c == 0x1D) {  // TAB
      x |= 0x3F;
      x += 1;
    }
    else {
      x += (c*FW/2); // EXTENDED SPACE
    }
    s++;
  }
  lcdLastRightPos = x;
  lcdNextPos = x;
#if !defined(BOOT)
  if (fontsize == MIDSIZE) {
    lcdLastRightPos += 1;
  }
  if (flags & RIGHT) {
    lcdLastRightPos -= width;
    lcdNextPos -= width;
    lcdLastLeftPos = lcdLastRightPos;
    lcdLastRightPos =  orig_x;
  }
  else {
    lcdLastLeftPos = orig_x;
  }
#endif
}

#if !defined(BOOT)
void lcdDrawHexNumber(coord_t x, coord_t y, uint32_t val, LcdFlags flags)
{
  x += FWNUM*4+1;
  for (int i=0; i<4; i++) {
    x -= FWNUM;
    char c = val & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcdDrawChar(x, y, c, flags|(c>='A' ? CONDENSED : 0));
    val >>= 4;
  }
}
#endif

#if !defined(BOOT)
void drawTelemetryTopBar()
{
  if (g_model.timers[0].mode) {
    TimerData *timer =  &g_model.timers[0];
    int32_t val = 0;
    if (g_model.timers[0].showElapsed)
      val = timer->start - timersStates[0].val;
    else
      val = timersStates[0].val;
    LcdFlags att = (val < 0 ? BLINK : 0) | TIMEHOUR;
    drawTimer(0, 0, val, att, att);
  } else {
    drawModelName(0, 0, g_model.header.name, g_eeGeneral.currModel, 0);
  }
  uint8_t att = (IS_TXBATT_WARNING() ? BLINK : 0);
  putsVBat(10*FW-1,0,att);
#if defined(RTCLOCK)
  drawRtcTime(17*FW+3, 0, LEFT|TIMEBLINK);
#endif
  lcdInvertLine(0);
}

void drawGPSCoord(coord_t x, coord_t y, int32_t value, const char * direction, LcdFlags att, bool seconds=true)
{
  att &= ~RIGHT & ~BOLD;
  uint32_t absvalue = abs(value);
  lcdDrawNumber(x, y, absvalue / 1000000, att); // ddd
  lcdDrawChar(lcdLastRightPos, y, CHAR_BW_DEGREE, att);
  absvalue = absvalue % 1000000;
  absvalue *= 60;
  if (g_eeGeneral.gpsFormat == 0 || !seconds) {
    lcdDrawNumber(lcdNextPos, y, absvalue / 1000000, att|LEFT|LEADING0, 2); // mm before '.'
    lcdDrawSolidVerticalLine(lcdLastRightPos, y, 2);
    lcdLastRightPos += 1;
    if (seconds) {
      absvalue %= 1000000;
      absvalue *= 60;
      absvalue /= 10000;
      lcdDrawNumber(lcdLastRightPos+2, y, absvalue, att|LEFT|PREC2);
      lcdDrawSolidVerticalLine(lcdLastRightPos, y, 2);
      lcdDrawSolidVerticalLine(lcdLastRightPos+2, y, 2);
      lcdLastRightPos += 3;
    }
  }
  else {
    absvalue /= 10000;
    lcdDrawNumber(lcdLastRightPos+FW, y, absvalue, att|LEFT|PREC2); // mm.mmm
  }
  lcdDrawSizedText(lcdLastRightPos+1, y, direction + (value>=0 ? 0 : 1), 1);
}

void drawDate(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags att)
{
  if (BLINK_ON_PHASE) {
     lcdDrawNumber(x, y, telemetryItem.datetime.hour, att|LEADING0, 2);
     lcdDrawText(lcdNextPos, y, ":", att);
     lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.min, att|LEADING0, 2);
     lcdDrawText(lcdNextPos, y, ":", att);
     lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.sec, att|LEADING0, 2);
  }
  else {
    lcdDrawNumber(x, y, telemetryItem.datetime.year, att|LEADING0|LEFT, 4);
    lcdDrawChar(lcdLastRightPos, y, '-', att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.month, att|LEADING0|LEFT, 2);
    lcdDrawChar(lcdLastRightPos, y, '-', att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.day, att|LEADING0|LEFT, 2);
  }
}

void drawTimerWithMode(coord_t x, coord_t y, uint8_t index, LcdFlags att)
{
  const TimerData &timer = g_model.timers[index];

  if (timer.mode) {
    const TimerData &timerData = g_model.timers[index];
    const TimerState &timerState = timersStates[index];
    const uint8_t negative = (timerState.val < 0 ? BLINK | INVERS : 0);
    int val = timerState.val;
    if (timerData.start && timerData.showElapsed &&
        timerData.start != timerState.val)
      val = (int)timerData.start - (int)timerState.val;
    if (val < 60 * 60) { // display MM:SS
      div_t qr = div((int) abs(val), 60);
      lcdDrawNumber(x - 5, y, qr.rem, att | LEADING0 | negative, 2);
      lcdDrawText(lcdLastLeftPos, y, ":", att | BLINK | negative);
      lcdDrawNumber(lcdLastLeftPos, y, qr.quot, att | negative);
      if (negative)
        lcdDrawText(lcdLastLeftPos, y, "-", att | negative);
    }
    else if (val < (99 * 60 * 60) + (59 * 60)) { // display HHhMM
      div_t qr = div((int) (abs(val) / 60), 60);
      lcdDrawNumber(x - 5, y, qr.rem, att | LEADING0, 2);
      lcdDrawText(lcdLastLeftPos, y, "h", att);
      lcdDrawNumber(lcdLastLeftPos, y, qr.quot, att);
      if (negative)
        lcdDrawText(lcdLastLeftPos, y, "-", att);
    }
    else {  //display HHHH for crazy large persistent timers
      lcdDrawText(x - 5, y, "h", att);
      lcdDrawNumber(lcdLastLeftPos, y, val / 3600, att);
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

void lcdDraw1bitBitmap(coord_t x, coord_t y, const uint8_t * img, uint8_t idx, LcdFlags att)
{
  const uint8_t * q = img;
  uint8_t w = *q++;
  uint8_t hb = ((*q++) + 7) / 8;
  uint8_t yShift = y % 8;

  bool inv = (att & INVERS) ? true : (att & BLINK ? BLINK_ON_PHASE : false);

  q += idx*w*hb;

  for (uint8_t yb = 0; yb < hb; yb++) {

    uint8_t *p = &displayBuf[(y / 8 + yb) * LCD_W + x];

    for (coord_t i=0; i<w; i++){

      uint8_t b = inv ? ~(*q++) : *q++;
      
      if (p < DISPLAY_END) {

        if (!yShift) {
          *p = b;
        }
        else {
          *p = (*p & ((1 << yShift) - 1)) | (b << yShift);

          if (p + LCD_W < DISPLAY_END) {
            p[LCD_W] = (p[LCD_W] & (0xFF >> yShift)) | (b >> (8 - yShift));
          }
        }
      }
      p++;
    }
  }
}

#endif

void lcdSetContrast(bool useDefault)
{
#if defined(BOOT)
  lcdSetRefVolt(LCD_CONTRAST_DEFAULT);
#elif defined(OLED_SCREEN)
  // Handled by brightness feature
  if (useDefault)
    lcdSetRefVolt(LCD_CONTRAST_DEFAULT);
#else
  lcdSetRefVolt(useDefault ? LCD_CONTRAST_DEFAULT : g_eeGeneral.contrast);
#endif
}

void lcdMaskPoint(uint8_t * p, uint8_t mask, LcdFlags att)
{
  ASSERT_IN_DISPLAY(p);

  if (att & FORCE)
    *p |= mask;
  else if (att & ERASE)
    *p &= ~mask;
  else
    *p ^= mask;
}

void lcdDrawPoint(coord_t x, coord_t y, LcdFlags att)
{
  uint8_t * p = &displayBuf[ y / 8 * LCD_W + x ];
  if (IS_IN_DISPLAY(p)) {
    lcdMaskPoint(p, bfBit<uint8_t>(y % 8), att);
  }
}

void lcdInvertLine(int8_t line)
{
  if (line < 0) return;
  if (line >= LCD_LINES) return;

  uint8_t *p  = &displayBuf[line * LCD_W];
  for (coord_t x=0; x<LCD_W; x++) {
    ASSERT_IN_DISPLAY(p);
    *p++ ^= 0xff;
  }
}

void lcdDrawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags att)
{
  if (y < 0 || y >= LCD_H || w == 0) return;
  if (w < 0) { x = x + w + 1; w = -w; }
  if (x + w <= 0 || x >= LCD_W) return;
  if (x < 0) { w += x; x = 0; }
  if (x + w > LCD_W) { w = LCD_W - x; }

  uint8_t *p  = &displayBuf[ y / 8 * LCD_W + x ];
  uint8_t msk = bfBit<uint8_t>(y % 8);
  while (w--) {
    if(pat & 1) {
      lcdMaskPoint(p, msk, att);
      pat = (pat >> 1) | 0x80;
    }
    else {
      pat = pat >> 1;
    }
    p++;
  }
}

void lcdDrawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat, LcdFlags att)
{
  if (x < 0 || x >= LCD_W || h == 0) return;
  if (h < 0) { y = y + h + 1; h = -h; }
  if (y + h <= 0 || y >= LCD_H) return;
  if (y < 0) { h += y; y = 0; }
  if (y + h > LCD_H) { h = LCD_H - y; }

  if (pat==DOTTED && !(y%2))
    pat = ~pat;

  uint8_t * p = &displayBuf[ y / 8 * LCD_W + x ];
  y = (y & 0x07);
  if (y) {
    ASSERT_IN_DISPLAY(p);
    uint8_t msk = ~(bfBitmask<uint8_t>(y));
    h -= 8-y;
    if (h < 0)
      msk -= ~(bfBitmask<uint8_t>(8 + h));
    lcdMaskPoint(p, msk & pat, att);
    p += LCD_W;
  }
  while (h >= 8) {
    ASSERT_IN_DISPLAY(p);
    lcdMaskPoint(p, pat, att);
    p += LCD_W;
    h -= 8;
  }
  if (h > 0) {
    ASSERT_IN_DISPLAY(p);
    lcdMaskPoint(p, (bfBitmask<uint8_t>(h)) & pat, att);
  }
}
