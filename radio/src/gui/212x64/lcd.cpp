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

inline bool lcdIsPointOutside(coord_t x, coord_t y)
{
  return (x<0 || x>=LCD_W || y<0 || y>=LCD_H);
}

void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags)
{
  const coord_t orig_x = x;
  const uint8_t orig_len = len;
  uint32_t fontsize = FONTSIZE(flags);
  bool setx = false;
  uint8_t width = 0;

  if (flags & RIGHT) {
    width = getTextWidth(s, len, flags);
    x -= width;
  }
  else if (flags & CENTERED) {
    width = getTextWidth(s, len, flags);
    x -= width / 2;
  }

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
#if !defined(BOOT)
      // UTF8 detection
      c = map_utf8_char(s, len);
      if (!c) break;
      
      if (c == 46 && FONTSIZE(flags) == TINSIZE) { // '.' handling
        if (((flags & BLINK) && BLINK_ON_PHASE) || ((!(flags & BLINK) && (flags & INVERS)))) {
          lcdDrawSolidVerticalLine(x, y-1, 5);
          lcdDrawPoint(x, y + 5);
        }
        else {
          lcdDrawPoint(x, y + 5 -1 , flags);
        }
        x += 2;
      } else {
        lcdDrawChar(x, y, c, flags);
        x = lcdNextPos;
      }
#else
      lcdDrawChar(x, y, c, flags);
      x = lcdNextPos;
#endif
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
      x += (c * FW/2); // EXTENDED SPACE
    }
    s++;
  }

  lcdLastRightPos = x;
  lcdNextPos = x;
  if (fontsize == MIDSIZE) {
    lcdLastRightPos += 1;
  }

  if (flags & RIGHT) {
    lcdLastRightPos -= width;
    lcdNextPos -= width;
    lcdLastLeftPos = lcdLastRightPos;
    lcdLastRightPos = orig_x;
  }
  else {
    lcdLastLeftPos = orig_x;
  }
}

#if !defined(BOOT)
void lcdDrawHexNumber(coord_t x, coord_t y, uint32_t val, LcdFlags flags)
{
  for (int i=12; i>=0; i-=4) {
    char c = (val >> i) & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcdDrawChar(x, y, c, flags);
    x = lcdNextPos;
  }
}
#endif

#if !defined(BOOT)
void drawTelemetryTopBar()
{
  drawModelName(0, 0, g_model.header.name, g_eeGeneral.currModel, 0);
  uint8_t att = (IS_TXBATT_WARNING() ? BLINK : 0);
  putsVBat(12*FW, 0, att);
  if (g_model.timers[0].mode) {
    TimerData *timer =  &g_model.timers[0];
    int32_t val = 0;
    if (g_model.timers[0].showElapsed)
      val = timer->start - timersStates[0].val;
    else
      val = timersStates[0].val;
    LcdFlags att = (val < 0 ? BLINK : 0) | TIMEHOUR;
    drawTimer(18*FW, 0, val, att, att);
    lcdDrawText(18*FW, 0, "T1:", RIGHT);
  }
  if (g_model.timers[1].mode) {
    TimerData *timer =  &g_model.timers[1];
    int32_t val = 0;
    if (g_model.timers[1].showElapsed)
      val = timer->start - timersStates[1].val;
    else
      val = timersStates[1].val;
    LcdFlags att = (val < 0 ? BLINK : 0) | TIMEHOUR;
    drawTimer(28*FW, 0, val, att, att);
    lcdDrawText(28*FW, 0, "T2:", RIGHT);
  }
  lcdInvertLine(0);
}

void drawMainControlLabel(coord_t x, coord_t y, uint8_t idx, LcdFlags att)
{
  lcdDrawSizedText(x, y, getMainControlLabel(idx), UINT8_MAX, att);
}

void drawGPSCoord(coord_t x, coord_t y, int32_t value, const char * direction, LcdFlags att, bool seconds=true)
{
  uint32_t absvalue = abs(value);
  att &= ~RIGHT;
  if (x > 10) x-=10;
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
  att &= ~FONTSIZE_MASK;
  if (BLINK_ON_PHASE) {
    lcdDrawNumber(x, y, telemetryItem.datetime.hour, att|LEADING0, 2);
    lcdDrawText(lcdNextPos, y, ":", att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.min, att|LEADING0, 2);
    lcdDrawText(lcdNextPos, y, ":", att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.sec, att|LEADING0, 2);
  }
  else {
    lcdDrawNumber(x, y, telemetryItem.datetime.year, att|LEADING0, 4);
    lcdDrawText(lcdNextPos, y, "-", att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.month, att|LEADING0, 2);
    lcdDrawText(lcdNextPos, y, "-", att);
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.day, att|LEADING0,2);
  }
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

#endif // BOOT

void lcdSetContrast(bool useDefault)
{
#if defined(BOOT)
  lcdSetRefVolt(LCD_CONTRAST_DEFAULT);
#else
  lcdSetRefVolt(useDefault ? LCD_CONTRAST_DEFAULT : g_eeGeneral.contrast);
#endif
}

void lcdMaskPoint(uint8_t *p, uint8_t mask, LcdFlags att)
{
  if ((p) >= DISPLAY_END) {
    return;
  }

  if (att&FILL_WHITE) {
    // TODO I could remove this, it's used for the top bar
    if (*p & 0x0F) mask &= 0xF0;
    if (*p & 0xF0) mask &= 0x0F;
  }

  if (att & FORCE) {
    *p |= mask;
  }
  else if (att & ERASE) {
    *p &= ~mask;
  }
  else {
    *p ^= mask;
  }
}

#define PIXEL_GREY_MASK(y, att) (((y) & 1) ? (0xF0 - (COLOUR_MASK(att) >> 12)) : (0x0F - (COLOUR_MASK(att) >> 16)))

void lcdDrawPoint(coord_t x, coord_t y, LcdFlags att)
{
  if (lcdIsPointOutside(x, y))
    return;
  uint8_t *p = &displayBuf[ y / 2 * LCD_W + x ];
  uint8_t mask = PIXEL_GREY_MASK(y, att);
  lcdMaskPoint(p, mask, att);
}

void lcdDrawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags att)
{
  if (y < 0 || y >= LCD_H || w == 0) return;
  if (w < 0) { x = x + w + 1; w = -w; }
  if (x + w <= 0 || x >= LCD_W) return;
  if (x < 0) { w += x; x = 0; }
  if (x + w > LCD_W) { w = LCD_W - x; }

  uint8_t *p  = &displayBuf[ y / 2 * LCD_W + x ];
  uint8_t mask = PIXEL_GREY_MASK(y, att);
  while (w--) {
    if (pat&1) {
      lcdMaskPoint(p, mask, att);
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

  if (pat==DOTTED && !(y%2)) {
    pat = ~pat;
  }

  while (h--) {
    if (pat & 1) {
      lcdDrawPoint(x, y, att);
      pat = (pat >> 1) | 0x80;
    }
    else {
      pat = pat >> 1;
    }
    y++;
  }
}

void lcdInvertLine(int8_t line)
{
  if (line < 0) return;
  if (line >= LCD_LINES) return;

  uint8_t *p  = &displayBuf[line * 4 * LCD_W];
  for (coord_t x=0; x<LCD_W*4; x++) {
    ASSERT_IN_DISPLAY(p);
    *p++ ^= 0xff;
  }
}

#if !defined(BOOT)
void lcdDraw1bitBitmap(coord_t x, coord_t y, const unsigned char * img, uint8_t idx, LcdFlags att)
{
  const unsigned char *q = img;
  uint8_t w    = *(q++);
  uint8_t hb   = (*(q++)+7) / 8;
  bool    inv  = (att & INVERS) ? true : (att & BLINK ? BLINK_ON_PHASE : false);
  q += idx*w*hb;
  for (uint8_t yb = 0; yb < hb; yb++) {
    for (uint8_t i=0; i<w; i++) {
      uint8_t b = *(q++);
      uint8_t val = inv ? ~b : b;
      for (int k=0; k<8; k++) {
        if (val & (1<<k)) {
          lcdDrawPoint(x+i, y+yb*8+k, 0);
        }
      }
    }
  }
}

void lcdDrawBitmap(coord_t x, coord_t y, const uint8_t * img, coord_t offset, coord_t width)
{
  const uint8_t *q = img;
  uint8_t w = *q++;
  if (!width || width > w) {
    width = w;
  }
  if (x+width > LCD_W) {
    if (x >= LCD_W ) return;
    width = LCD_W-x;
  }
  uint8_t rows = (*q++ + 1) / 2;

  for (uint8_t row=0; row<rows; row++) {
    q = img + 2 + row*w + offset;
    uint8_t *p = &displayBuf[(row + (y/2)) * LCD_W + x];
    for (coord_t i=0; i<width; i++) {
      if (p >= DISPLAY_END) return;
      uint8_t b = *q++;
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
  }
}

void lcdDrawRleBitmap(coord_t x, coord_t y, const uint8_t * img, coord_t offset, coord_t width, bool overlay)
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
    if (overlay) {
      for (coord_t i=0; i<width; i++) {
        if (p >= DISPLAY_END) return;
        uint8_t b = pic.getNext();
        if (y & 1) {
          if (b & 0x0f)
            *p = (*p & 0x0f) + ((b & 0x0f) << 4);
          if ((p+LCD_W) < DISPLAY_END && b & 0xf0) {
            *(p+LCD_W) = (*(p+LCD_W) & 0xf0) + ((b & 0xf0) >> 4);
          }
        }
        else {
          if (b & 0x0f)
            *p = (*p & 0xf0) + (b & 0x0f);
          if (b & 0xf0)
            *p = (*p & 0x0f) + (b & 0xf0);
        }
        p++;
      }
    } else {
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
    }
    pic.goToNextRow();
    pic.skip(offset);
  }
}
#endif
