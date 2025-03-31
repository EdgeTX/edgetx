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

#include <limits.h>
#include <memory.h>

#include "lcd.h"
#include "bitfield.h"
#include "gui/common/stdlcd/fonts.h"
#include "gui/common/stdlcd/utf8.h"

#if !defined(SIMU)
  #define assert(x)
#else
  #include <assert.h>
#endif

#if !defined(BOOT)
  #include "edgetx.h"
  #include "hal/switch_driver.h"
  #include "hal/adc_driver.h"
  #include "switches.h"
#endif

pixel_t displayBuf[DISPLAY_BUFFER_SIZE] __DMA;

void lcdClear()
{
  memset(displayBuf, 0, DISPLAY_BUFFER_SIZE);
}

coord_t lcdLastRightPos;
coord_t lcdNextPos;
coord_t lcdLastLeftPos;

void lcdPutPattern(coord_t x, coord_t y, const uint8_t * pattern, uint8_t width, uint8_t height, LcdFlags flags)
{
  bool blink = false;
  bool inv = false;
  if (flags & BLINK) {
    if (BLINK_ON_PHASE) {
      if (flags & INVERS)
        inv = true;
      else {
        blink = true;
      }
    }
  }
  else if (flags & INVERS) {
    inv = true;
  }

  uint8_t lines = (height+7)/8;
  assert(lines <= 5);

  for (int8_t i=0; i<width+2; i++) {
    if (x >= 0 && x < LCD_W) {
      uint8_t b[5] = { 0 };
      if (i==0) {
        if (x==0 || !inv) {
          lcdNextPos++;
          continue;
        }
        else {
          // we need to work on the previous x when INVERS
          x--;
        }
      }
      else if (i<=width) {
        uint8_t skip = true;
        for (uint8_t j=0; j<lines; j++) {
          b[j] = *(pattern++); /*top byte*/
          if (b[j] != 0xff) {
            skip = false;
          }
        }
        if (skip) {
          if (flags & FIXEDWIDTH) {
            for (uint8_t j=0; j<lines; j++) {
              b[j] = 0;
            }
          }
          else {
            continue;
          }
        }
        if ((flags & CONDENSED) && i==2) {
          /*condense the letter by skipping column 3 */
          continue;
        }
      }

      for (int8_t j=-1; j<=height; j++) {
        bool plot;
        if (j < 0 || ((j == height) && !(FONTSIZE(flags) == SMLSIZE))) {
          plot = false;
          if (height >= 12) continue;
          if (j<0 && !inv) continue;
          if (y+j < 0) continue;
        }
        else {
          uint8_t line = (j / 8);
          uint8_t pixel = (j % 8);
          plot = b[line] & (1 << pixel);
        }
        if (inv) plot = !plot;
        if (!blink) {
          if (flags & VERTICAL)
            lcdDrawPoint(y+j, LCD_H-x, plot ? FORCE : ERASE);
          else
            lcdDrawPoint(x, y+j, plot ? FORCE : ERASE);
        }
      }
    }
    x++;
    lcdNextPos++;
  }
}

struct PatternData
{
  uint8_t width;
  uint8_t height;
  const uint8_t * data;
};

uint8_t getPatternWidth(const PatternData * pattern)
{
  uint8_t result = 0;
  uint8_t lines = (pattern->height+7)/8;
  const uint8_t * data = pattern->data;
  for (int8_t i=0; i<pattern->width; i++) {
    for (uint8_t j=0; j<lines; j++) {
      if (data[j] != 0xff) {
        result += 1;
        break;
      }
    }
    data += lines;
  }
  return result;
}

LcdFlags getCharPattern(PatternData * pattern, unsigned char c, LcdFlags flags)
{
#if !defined(BOOT)
  static const uint8_t fontWidth[] = { 5, 3, 5, 8, 10, 22, 5 };
  static const uint8_t fontHeight[] = { 7, 5, 6, 12, 16, 38, 7 };

  uint32_t fontsize = FONTSIZE(flags);
  unsigned char c_remapped = 0;

  if (fontsize == DBLSIZE || (flags & BOLD)) {
    // To save space only some DBLSIZE and BOLD chars are available
    // c has to be remapped. All non existing chars mapped to 0 (space)
    if (c >= ',' && c <= ':')
      c_remapped = c - ',' + 1;
    else if (c >= 'A' && c <= 'Z')
      c_remapped = c - 'A' + 16;
    else if (c >= 'a' && c <= 'z')
      c_remapped = c - 'a' + 42;
    else if (c == '_')
      c_remapped = 4;
    else if (c != ' ')
      flags &= ~BOLD; // For BOLD use Standard font if character is not in BOLD
  }

  uint8_t fontIdx = fontsize >> 8;
  if (fontIdx == 0 && flags & BOLD) fontIdx = 6;

  pattern->width = fontWidth[fontIdx];
  pattern->height = fontHeight[fontIdx];
  int charSize = (pattern->height + 7) / 8 * pattern->width;

  switch (fontIdx) {
    case 0: // Standard
      pattern->data = &font_5x7[(c - FONT_BASE_START) * charSize];
      break;
    case 1: // TINSIZE
      pattern->data = &font_3x5[(c - FONT_BASE_START) * charSize];
      break;
    case 2: // SMLSIZE
      // Adjust language special characters offset
      if (c >= FONT_LANG_START)
        c = c - (FONT_SYMS_CNT - FONT_SYMS_CNT_4x6);
      pattern->data = &font_4x6[(c - FONT_BASE_START) * charSize];
      break;
    case 3: // MDLSIZE
      // Adjust language special characters offset
      if (c >= FONT_LANG_START)
        c = c - FONT_SYMS_CNT;
      pattern->data = &font_8x10[(c - FONT_BASE_START) * charSize];
      break;
    case 4: // DBLSIZE
      // Adjust language special characters offset and symbols offset
      if (c >= FONT_LANG_START)
        c_remapped = c - (FONT_BASE_CNT - FONT_BASE_CNT_10x14) - (FONT_SYMS_CNT - FONT_SYMS_CNT_10x14) - FONT_BASE_START;
      else if (c >= FONT_SYMS_START)
        c_remapped = c - (FONT_BASE_CNT - FONT_BASE_CNT_10x14) - FONT_BASE_START;
      pattern->data = &font_10x14[(c_remapped) * charSize];
      break;
    case 5: // XXLSIZE
      pattern->data = &font_22x38_num[(c - '0' + 5) * charSize];
      break;
    case 6: // BOLD
      pattern->data = &font_5x7_B[c_remapped * charSize];
      break;
  };
#else
  pattern->width = 5;
  pattern->height = 7;
  pattern->data = &font_5x7[(c - FONT_BASE_START) * 5];
#endif
  return flags;
}

uint8_t getCharWidth(char c, LcdFlags flags)
{
  PatternData pattern;
  getCharPattern(&pattern, c, flags);
  return getPatternWidth(&pattern);
}

void lcdDrawChar(coord_t x, coord_t y, uint8_t c, LcdFlags flags)
{
  lcdNextPos = x - 1;
  PatternData pattern;
  flags = getCharPattern(&pattern, c, flags);
  lcdPutPattern(x, y, pattern.data, pattern.width, pattern.height, flags);
}

void lcdDrawChar(coord_t x, coord_t y, uint8_t c)
{
  lcdDrawChar(x, y, c, 0);
}

uint8_t getTextWidth(const char * s, uint8_t len, LcdFlags flags)
{
  uint8_t width = 0;
  if (len == 0) len = strlen(s);
  while (len--) {
    unsigned char c = map_utf8_char(s, len);
    if (!c) {
      break;
    }
    width += getCharWidth(c, flags) + 1;
    s++;
  }
  return width;
}

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

void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len)
{
  lcdDrawSizedText(x, y, s, len, 0);
}

void lcdDrawText(coord_t x, coord_t y, const char * s, LcdFlags flags)
{
  lcdDrawSizedText(x, y, s, 255, flags);
}

void lcdDrawCenteredText(coord_t y, const char * s, LcdFlags flags)
{
  coord_t x = (LCD_W - getTextWidth(s, flags)) / 2;
  lcdDrawText(x, y, s, flags);
}

void lcdDrawText(coord_t x, coord_t y, const char * s)
{
  lcdDrawText(x, y, s, 0);
}

void lcdDrawTextAlignedLeft(coord_t y, const char * s)
{
  lcdDrawText(0, y, s);
}

void lcdDrawTextIndented(coord_t y, const char * s)
{
  lcdDrawText(INDENT_WIDTH, y, s);
}

#if !defined(BOOT)
void lcdDrawTextAtIndex(coord_t x, coord_t y, const char *const *s,uint8_t idx, LcdFlags flags)
{
  lcdDrawSizedText(x, y, s[idx], UINT8_MAX, flags);
}

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

void lcdDrawHexChar(coord_t x, coord_t y, uint8_t val, LcdFlags flags)
{
  x += FWNUM*2;
  for (int i=0; i<2; i++) {
    x -= FWNUM;
    char c = val & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcdDrawChar(x, y, c, flags|(c>='A' ? CONDENSED : 0));
    val >>= 4;
  }
}

void lcdDraw8bitsNumber(coord_t x, coord_t y, int8_t val)
{
  lcdDrawNumber(x, y, val);
}

void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags)
{
  lcdDrawNumber(x, y, val, flags, 0);
}

void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len)
{
  char str[16+1];
  char *s = str+16;
  *s = '\0';
  int idx = 0;
  int mode = MODE(flags);
  bool neg = false;

  if (val == INT_MAX) {
    flags &= ~(LEADING0 | PREC1 | PREC2);
    lcdDrawText(x, y, "INT_MAX", flags);
    return;
  }

  if (val < 0) {
    if (val == INT_MIN) {
      flags &= ~(LEADING0 | PREC1 | PREC2);
      lcdDrawText(x, y, "INT_MIN", flags);
      return;
    }
    val = -val;
    neg = true;
  }
  do {
    *--s = '0' + (val % 10);
    ++idx;
    val /= 10;
    if (mode!=0 && idx==mode) {
      mode = 0;
      *--s = '.';
      if (val==0) {
        *--s = '0';
      }
    }
  } while (val!=0 || mode>0 || (mode==MODE(LEADING0) && idx<len));
  if (neg) {
    *--s = '-';
  }
  flags &= ~(LEADING0 | PREC1 | PREC2);
  lcdDrawText(x, y, s, flags);
}
#endif

void lcdDrawSolidHorizontalLine(coord_t x, coord_t y, coord_t w, LcdFlags att)
{
  lcdDrawHorizontalLine(x, y, w, 0xff, att);
}

#if !defined(BOOT)
void lcdDrawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags att)
{
  int dx = x2-x1;      /* the horizontal distance of the line */
  int dy = y2-y1;      /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs>>1;
  int y = dxabs>>1;
  int px = x1;
  int py = y1;

  if (dxabs >= dyabs) {
    /* the line is more horizontal than vertical */
    for (int i=0; i<=dxabs; i++) {
      if ((1<<(px%8)) & pat) {
        lcdDrawPoint(px, py, att);
      }
      y += dyabs;
      if (y>=dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
    }
  }
  else {
    /* the line is more vertical than horizontal */
    for (int i=0; i<=dyabs; i++) {
      if ((1<<(py%8)) & pat) {
        lcdDrawPoint(px, py, att);
      }
      x += dxabs;
      if (x >= dyabs) {
        x -= dyabs;
        px += sdx;
      }
      py += sdy;
    }
  }
}
#endif


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

void lcdDrawSolidVerticalLine(coord_t x, coord_t y, coord_t h, LcdFlags att)
{
  lcdDrawVerticalLine(x, y, h, SOLID, att);
}

void lcdDrawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att)
{
  lcdDrawVerticalLine(x, y, h, pat, att);
  lcdDrawVerticalLine(x+w-1, y, h, pat, att);
  if (~att & ROUND) { x+=1; w-=2; }
  lcdDrawHorizontalLine(x, y+h-1, w, pat, att);
  lcdDrawHorizontalLine(x, y, w, pat, att);
}

#if !defined(BOOT)
void lcdDrawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att)
{
  for (coord_t i=y; i<y+h; i++) {    // cast to coord_t needed otherwise (y+h) is promoted to int (see #5055)
    if ((att&ROUND) && (i==y || i==y+h-1))
      lcdDrawHorizontalLine(x+1, i, w-2, pat, att);
    else
      lcdDrawHorizontalLine(x, i, w, pat, att);
    pat = (pat >> 1) + ((pat & 1) << 7);
  }
}

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

#if defined(RTCLOCK)
void drawRtcTime(coord_t x, coord_t y, LcdFlags att)
{
  drawTimer(x, y, getValue(MIXSRC_TX_TIME), att, att);
}
#endif

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

// TODO to be optimized with drawValueWithUnit
void putsVolts(coord_t x, coord_t y, uint16_t volts, LcdFlags att)
{
  lcdDrawNumber(x, y, (int16_t)volts, (~NO_UNIT) & (att | ((att&PREC2)==PREC2 ? 0 : PREC1)));
  if (~att & NO_UNIT) lcdDrawChar(lcdLastRightPos, y, 'V', att);
}

void putsVBat(coord_t x, coord_t y, LcdFlags att)
{
  putsVolts(x, y, g_vbat100mV, att);
}

void putsChn(coord_t x, coord_t y, uint8_t idx, LcdFlags att)
{
  drawStringWithIndex(x, y, STR_CH, idx, att);
}

void putsChnLetter(coord_t x, coord_t y, uint8_t idx, LcdFlags att)
{
  lcdDrawText(x, y, getAnalogShortLabel(idx), att);
}

void drawModelName(coord_t x, coord_t y, char *name, uint8_t id, LcdFlags att)
{
  uint8_t len = sizeof(g_model.header.name);
  while (len>0 && !name[len-1]) --len;
  if (len==0) {
    drawStringWithIndex(x, y, STR_MODEL, id+1, att|LEADING0);
  }
  else {
    lcdDrawSizedText(x, y, name, sizeof(g_model.header.name), att);
  }
}

void drawSwitch(coord_t x, coord_t y, swsrc_t idx, LcdFlags flags, bool autoBold)
{
  char s[8];
  getSwitchPositionName(s, idx);
  if (autoBold && idx != SWSRC_NONE && getSwitch(idx))
    flags |= BOLD;
  lcdDrawText(x, y, s, flags);
}

void drawCurveName(coord_t x, coord_t y, int8_t idx, LcdFlags att)
{
  char s[8];
  getCurveString(s, idx);
  lcdDrawText(x, y, s, att);
}

void drawTimerMode(coord_t x, coord_t y, swsrc_t mode, LcdFlags att)
{
  if (mode >= 0) {
    if (mode < TMRMODE_COUNT)
      return lcdDrawTextAtIndex(x, y, STR_VTMRMODES, mode, att);
    else
      mode -= (TMRMODE_COUNT-1);
  }
  drawSwitch(x, y, mode, att);
}

void drawShortTrimMode(coord_t x, coord_t y, uint8_t fm, uint8_t idx, LcdFlags att)
{
  trim_t v = getRawTrimValue(fm, idx);
  uint8_t mode = v.mode;
  uint8_t p = v.mode >> 1;
  if (mode == TRIM_MODE_NONE) {
    putsChnLetter(x, y, idx, att);
  }
  else {
    lcdDrawChar(x, y, '0' + p, att);
  }
}

void drawGPSCoord(coord_t x, coord_t y, int32_t value, const char * direction, LcdFlags att, bool seconds=true)
{
  att &= ~RIGHT & ~BOLD;
  uint32_t absvalue = abs(value);
  lcdDrawNumber(x, y, absvalue / 1000000, att); // ddd
  lcdDrawChar(lcdLastRightPos, y, STR_CHAR_BW_DEGREE, att);
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


#define LCD_BYTE_FILTER(p, keep, add) *(p) = (*(p) & (keep)) | (add)


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

void lcdFlushed() {}
