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
#include "opentx.h"
#include "common/stdlcd/fonts.h"

#if (defined(PCBX9E) || defined(PCBX9DP)) && defined(LCD_DUAL_BUFFER)
  pixel_t displayBuf1[DISPLAY_BUFFER_SIZE] __DMA;
  pixel_t displayBuf2[DISPLAY_BUFFER_SIZE] __DMA;
  pixel_t * displayBuf = displayBuf1;
#else
  pixel_t displayBuf[DISPLAY_BUFFER_SIZE] __DMA;
#endif

inline bool lcdIsPointOutside(coord_t x, coord_t y)
{
  return x < 0 || x >= LCD_W || y < 0 || y >= LCD_H;
}

void lcdClear()
{
  memset(displayBuf, 0, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
}

coord_t lcdLastLeftPos;
coord_t lcdLastRightPos;
coord_t lcdNextPos;

struct PatternData
{
  uint8_t width;
  uint8_t height;
  const uint8_t * data;
};

uint8_t getPatternWidth(const PatternData * pattern)
{
  uint8_t result = 0;
  uint8_t lines = (pattern->height + 7) / 8;
  const uint8_t * data = pattern->data;
  for (int8_t i=0; i < pattern->width; i++) {
    for (uint8_t j=0; j < lines; j++) {
      if (data[j] != 0xff) {
        result += 1;
        break;
      }
    }
    data += lines;
  }
  return result;
}

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

  uint8_t lines = (height + 7) / 8;
  assert(lines <= 5);

  for (int8_t i = 0; i < int8_t(width + 2); i++) {
    if (x >= 0 && x < LCD_W) {
      uint8_t b[5] = { 0 };
      if (i == 0) {
        if (x==0 || !inv) {
          lcdNextPos++;
          continue;
        }
        else {
          // we need to work on the previous x when INVERS
          x--;
        }
      }
      else if (i <= width) {
        uint8_t skip = true;
        for (uint8_t j = 0; j < lines; j++) {
          b[j] = *(pattern++); /*top byte*/
          if (b[j] != 0xFF) {
            skip = false;
          }
        }
        if (skip) {
          if (flags & FIXEDWIDTH) {
            for (uint8_t j = 0; j < lines; j++) {
              b[j] = 0;
            }
          }
          else {
            continue;
          }
        }
#if LCD_W < 212
        if ((flags & CONDENSED) && i == 2) {
          /*condense the letter by skipping column 3 */
          continue;
        }
#endif
      }

      for (int8_t j = -1; j <= int8_t(height); j++) {
        bool plot;
        if (j < 0 || (j == height && FONTSIZE(flags) != SMLSIZE)) {
          plot = false;
          if (height >= 12) continue;
          if (j < 0 && !inv) continue;
          if (y + j < 0) continue;
        }
        else {
          uint8_t line = (j / 8);
          uint8_t pixel = (j % 8);
          plot = b[line] & (1 << pixel);
        }
        if (inv)
          plot = !plot;
        if (!blink) {
          if (flags & VERTICAL)
            lcdDrawPoint(y + j, LCD_H - x, plot ? FORCE : ERASE);
          else
            lcdDrawPoint(x, y + j, plot ? FORCE : ERASE);
        }
      }
    }

    x++;
    lcdNextPos++;
  }
}

void getCharPattern(PatternData * pattern, unsigned char c, LcdFlags flags)
{
#if !defined(BOOT)
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
      flags &= ~BOLD;
  }

  if (fontsize == DBLSIZE) {
    pattern->width = 10;
    pattern->height = 16;
    if (c >= 0x80) {
      pattern->data = &font_10x14_extra[((uint16_t) (c - 0x80)) * 20];
    }
    else {
      if (c >= 0x80)
        c_remapped = c - 60;
      pattern->data = &font_10x14[((uint16_t) c_remapped) * 20];
    }
  }
  else if (fontsize == XXLSIZE) {
    pattern->width = 22;
    pattern->height = 38;
    pattern->data = &font_22x38_num[((uint16_t) c - '0' + 5) * 110];
  }
  else if (fontsize == MIDSIZE) {
    pattern->width = 8;
    pattern->height = 12;
    pattern->data = &font_8x10[((uint16_t) c - 0x20) * 16];
  }
  else if (fontsize == SMLSIZE) {
    pattern->width = 5;
    pattern->height = 6;
    pattern->data = (c < 0x80 ? &font_4x6[(c - 0x20) * 5] : &font_4x6_extra[(c - 0x80) * 5]);
  }
  else if (fontsize == TINSIZE) {
    pattern->width = 3;
    pattern->height = 5;
    pattern->data = &font_3x5[((uint16_t) c - 0x20) * 3];
  }
  else if (flags & BOLD) {
    pattern->width = 5;
    pattern->height = 7;
    pattern->data = &font_5x7_B[c_remapped * 5];
  }
  else {
    pattern->width = 5;
    pattern->height = 7;
    pattern->data = &font_5x7[(c - 0x20) * 5];
  }
#else
  pattern->width = 5;
  pattern->height = 7;
  pattern->data = &font_5x7[(c - 0x20) * 5];
#endif
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
#if defined(BOOT)
  const uint8_t * data = &font_5x7[(c-0x20)*5];
  lcdPutPattern(x, y, data, 5, 7, flags);
#else
  PatternData pattern;
  getCharPattern(&pattern, c, flags);
  lcdPutPattern(x, y, pattern.data, pattern.width, pattern.height, flags);
#endif
}

void lcdDrawChar(coord_t x, coord_t y, uint8_t c)
{
  lcdDrawChar(x, y, c, 0);
}

uint8_t getTextWidth(const char * s, uint8_t len, LcdFlags flags)
{
  uint8_t width = 0;
  for (int i = 0; len == 0 || i < len; ++i) {
#if !defined(BOOT)
    unsigned char c = *s;
#else
    unsigned char c = *s;
#endif
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
#if LCD_W >= 212
      if (c == 46 && FONTSIZE(flags) == TINSIZE) { // '.' handling
        if (((flags & BLINK) && BLINK_ON_PHASE) || ((!(flags & BLINK) && (flags & INVERS)))) {
          lcdDrawSolidVerticalLine(x, y - 1, 5);
          lcdDrawPoint(x, y + 5);
        }
        else {
          lcdDrawPoint(x, y + 5 -1 , flags);
        }
        x += 2;
      }
      else {
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
      x += (c * FW / 2); // EXTENDED SPACE
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

#if !defined(BOOT)
void lcdDrawTextAtIndex(coord_t x, coord_t y, const char * s,uint8_t idx, LcdFlags flags)
{
  uint8_t length = *(s++);
  lcdDrawSizedText(x, y, s+length*idx, length, flags);
}

void lcdDrawHexNumber(coord_t x, coord_t y, uint32_t val, LcdFlags flags)
{
  for (int i = 12; i >= 0; i -= 4) {
    char c = (val >> i) & 0x0F;
    c = c > 9 ? c + 'A' - 10 : c + '0';
    lcdDrawChar(x, y, c, flags);
    x = lcdNextPos;
  }
}

void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags)
{
  lcdDrawNumber(x, y, val, flags, 0);
}

void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len)
{
  char str[16 + 1];
  char *s = str + 16;
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
    if (mode != 0 && idx == mode) {
      mode = 0;
      *--s = '.';
      if (val == 0) {
        *--s = '0';
      }
    }
  } while (val != 0 || mode > 0 || (mode == MODE(LEADING0) && idx < len));
  if (neg) {
    *--s = '-';
  }
  flags &= ~(LEADING0 | PREC1 | PREC2);
  lcdDrawText(x, y, s, flags);
}
#endif

void lcdDrawSolidHorizontalLine(coord_t x, coord_t y, coord_t w, LcdFlags att)
{
  if (w < 0) {
    x += w;
    w = -w;
  }
  lcdDrawHorizontalLine(x, y, w, 0xff, att);
}

#if !defined(BOOT)
void lcdDrawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags att)
{
  int dx = x2 - x1;      /* the horizontal distance of the line */
  int dy = y2 - y1;      /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs >> 1;
  int y = dxabs >> 1;
  int px = x1;
  int py = y1;

  if (dxabs >= dyabs) {
    /* the line is more horizontal than vertical */
    for (int i = 0; i <= dxabs; i++) {
      if ((1 << (px % 8)) & pat) {
        lcdDrawPoint(px, py, att);
      }
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
    }
  }
  else {
    /* the line is more vertical than horizontal */
    for (int i = 0; i <= dyabs; i++) {
      if ((1 << (py % 8)) & pat) {
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
  if (x >= LCD_W) return;
  if (y >= LCD_H) return;
  if (h < 0) { y += h; h = -h; }
  if (y < 0) { h += y; y = 0; if (h <= 0) return; }
  if (y + h > LCD_H) { h = LCD_H - y; }

  if (pat == DOTTED && !(y & 1)) {
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

void lcdDrawSolidVerticalLine(coord_t x, coord_t y, coord_t h, LcdFlags att)
{
  lcdDrawVerticalLine(x, y, h, SOLID, att);
}

void lcdDrawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att)
{
  lcdDrawVerticalLine(x, y, h, pat, att);
  lcdDrawVerticalLine(x + w - 1, y, h, pat, att);
  if (~att & ROUND) { x += 1; w -= 2; }
  lcdDrawHorizontalLine(x, y + h - 1, w, pat, att);
  lcdDrawHorizontalLine(x, y, w, pat, att);
}

#if !defined(BOOT)
void lcdDrawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att)
{
  for (coord_t i = y; i < y + h; i++) {
    if ((att&ROUND) && (i == y || i == y + h - 1))
      lcdDrawHorizontalLine(x + 1, i, w - 2, pat, att);
    else
      lcdDrawHorizontalLine(x, i, w, pat, att);
    pat = (pat >> 1) + ((pat & 1) << 7);
  }
}

void drawRtcTime(coord_t x, coord_t y, LcdFlags att)
{
  drawTimer(x, y, getValue(MIXSRC_TX_TIME), att, att);
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

void drawStickName(coord_t x, coord_t y, uint8_t idx, LcdFlags att)
{
  uint8_t length = STR_VSRCRAW[0];
  lcdDrawSizedText(x, y, STR_VSRCRAW + 2 + length * (idx + 1), length - 1, att);
}

void drawSource(coord_t x, coord_t y, uint32_t idx, LcdFlags att)
{
  if (idx == MIXSRC_NONE) {
    lcdDrawTextAtIndex(x, y, STR_VSRCRAW, 0, att); // TODO macro
  }
  else if (idx <= MIXSRC_LAST_INPUT) {
    lcdDrawChar(x + 2, y + 1, CHR_INPUT, TINSIZE);
    lcdDrawFilledRect(x, y, 7, 7);
    if (ZEXIST(g_model.inputNames[idx-MIXSRC_FIRST_INPUT]))
      lcdDrawSizedText(x+8, y, g_model.inputNames[idx-MIXSRC_FIRST_INPUT], LEN_INPUT_NAME, att);
    else
      lcdDrawNumber(x + 8, y, idx, att|LEADING0|LEFT, 2);
  }
  else if (idx <= MIXSRC_LAST_LUA) {
    div_t qr = div(idx - MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
#if defined(LUA_MODEL_SCRIPTS) && !defined(BOOT)
    if (qr.quot < MAX_SCRIPTS && qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {
      lcdDrawChar(x + 2, y + 1, '1' + qr.quot, TINSIZE);
      lcdDrawFilledRect(x, y, 7, 7);
      lcdDrawSizedText(x + 8, y, scriptInputsOutputs[qr.quot].outputs[qr.rem].name, att & STREXPANDED ? 9 : 4, att);
    }
    else
#endif
    {
      drawStringWithIndex(x, y, "LUA", qr.quot + 1, att);
      lcdDrawChar(lcdLastRightPos, y, 'a' + qr.rem, att);
    }
  }
  else if (idx <= MIXSRC_LAST_POT) {
    idx = idx-MIXSRC_Rud;
    if (g_eeGeneral.anaNames[idx][0]) {
      if (idx < MIXSRC_FIRST_POT-MIXSRC_Rud )
        lcdDrawChar(x, y, '\207', att); //stick symbol
      else if (idx < MIXSRC_FIRST_SLIDER-MIXSRC_Rud )
        lcdDrawChar(x, y, '\210', att); //pot symbol
      else
        lcdDrawChar(x, y, '\211', att); //slider symbol
      lcdDrawSizedText(lcdNextPos, y, g_eeGeneral.anaNames[idx], LEN_ANA_NAME, att);
    }
    else {
      lcdDrawTextAtIndex(x, y, STR_VSRCRAW, idx + 1, att);
    }
  }
  else if (idx >= MIXSRC_FIRST_SWITCH && idx <= MIXSRC_LAST_SWITCH) {
    idx = idx - MIXSRC_FIRST_SWITCH;
    if (g_eeGeneral.switchNames[idx][0]) {
      lcdDrawChar(x, y, '\212', att); //switch symbol
      lcdDrawSizedText(lcdNextPos, y, g_eeGeneral.switchNames[idx], LEN_SWITCH_NAME, att);
    }
    else {
      lcdDrawTextAtIndex(x, y, STR_VSRCRAW, idx + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1, att);
    }
  }
  else if (idx < MIXSRC_SW1)
    lcdDrawTextAtIndex(x, y, STR_VSRCRAW, idx - MIXSRC_Rud + 1, att);
  else if (idx <= MIXSRC_LAST_LOGICAL_SWITCH)
    drawSwitch(x, y, SWSRC_SW1 + idx - MIXSRC_SW1, att);
  else if (idx < MIXSRC_CH1)
    drawStringWithIndex(x, y, STR_PPM_TRAINER, idx - MIXSRC_FIRST_TRAINER + 1, att);
  else if (idx <= MIXSRC_LAST_CH) {
    drawStringWithIndex(x, y, STR_CH, idx - MIXSRC_CH1 + 1, att);
    if (ZEXIST(g_model.limitData[idx - MIXSRC_CH1].name) && (att & STREXPANDED)) {
      lcdDrawChar(lcdLastRightPos, y, ' ', att|SMLSIZE);
      lcdDrawSizedText(lcdLastRightPos+3, y, g_model.limitData[idx-MIXSRC_CH1].name, LEN_CHANNEL_NAME, att|SMLSIZE);
    }
  }
  else if (idx <= MIXSRC_LAST_GVAR) {
    drawStringWithIndex(x, y, STR_GV, idx - MIXSRC_GVAR1 + 1, att);
  }
  else if (idx < MIXSRC_FIRST_TIMER) {
    lcdDrawTextAtIndex(x, y, STR_VSRCRAW, idx - MIXSRC_Rud + 1 - MAX_LOGICAL_SWITCHES - MAX_TRAINER_CHANNELS - MAX_OUTPUT_CHANNELS - MAX_GVARS, att);
  }
  else if (idx <= MIXSRC_LAST_TIMER) {
    if(ZEXIST(g_model.timers[idx-MIXSRC_FIRST_TIMER].name)) {
      lcdDrawSizedText(x, y, g_model.timers[idx-MIXSRC_FIRST_TIMER].name, LEN_TIMER_NAME, att);
    }
    else {
      lcdDrawTextAtIndex(x, y, STR_VSRCRAW, idx - MIXSRC_Rud + 1 - MAX_LOGICAL_SWITCHES - MAX_TRAINER_CHANNELS - MAX_OUTPUT_CHANNELS - MAX_GVARS, att);
    }
  }
  else {
    idx -= MIXSRC_FIRST_TELEM;
    div_t qr = div(idx, 3);
    lcdDrawSizedText(x, y, g_model.telemetrySensors[qr.quot].label, TELEM_LABEL_LEN, att);
    if (qr.rem) lcdDrawChar(lcdLastRightPos, y, qr.rem==2 ? '+' : '-', att);
  }
}

void putsChnLetter(coord_t x, coord_t y, uint8_t idx, LcdFlags att)
{
  lcdDrawTextAtIndex(x, y, STR_RETA123, idx - 1, att);
}

void drawModelName(coord_t x, coord_t y, char *name, uint8_t id, LcdFlags att)
{
  uint8_t len = sizeof(g_model.header.name);
  while (len > 0 && !name[len - 1]) --len;
  if (len == 0) {
    drawStringWithIndex(x, y, STR_MODEL, id+1, att|LEADING0);
  }
  else {
    lcdDrawSizedText(x, y, name, sizeof(g_model.header.name), att);
  }
}

void drawTimerMode(coord_t x, coord_t y, swsrc_t mode, LcdFlags att)
{
  if (mode >= 0) {
    if (mode < TMRMODE_COUNT)
      return lcdDrawTextAtIndex(x, y, STR_VTMRMODES, mode, att);
    else
      mode -= (TMRMODE_COUNT - 1);
  }
  drawSwitch(x, y, mode, att);
}

void drawShortTrimMode(coord_t x, coord_t y, uint8_t fm, uint8_t idx, LcdFlags att)
{
  trim_t v = getRawTrimValue(fm, idx);
  uint8_t mode = v.mode;
  uint8_t p = v.mode >> 1;
  if (mode == TRIM_MODE_NONE) {
    putsChnLetter(x, y, idx + 1, att);
  }
  else {
    lcdDrawChar(x, y, '0' + p, att);
  }
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
    lcdDrawNumber(lcdNextPos, y, telemetryItem.datetime.day, att|LEADING0, 2);
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

void lcdMaskPoint(uint8_t * p, uint8_t mask, LcdFlags att)
{
  if ((p) >= DISPLAY_END) {
    return;
  }

  if (att & FILL_WHITE) {
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

inline uint8_t PIXEL_GREY_MASK(uint8_t x, coord_t y, LcdFlags att)
{
#if defined(RADIO_FAMILY_TBS)
  return (x & 1) ? (0x0F - (COLOUR_MASK(att) >> 16)) : (0xF0 - (COLOUR_MASK(att) >> 12));
#else
  return (y & 1) ? (0xF0 - (COLOUR_MASK(att) >> 12)) : (0x0F - (COLOUR_MASK(att) >> 16));
#endif
}

uint8_t * getPixelPtr(coord_t x, coord_t y)
{
#if defined(RADIO_FAMILY_TBS)
  return &displayBuf[y * (LCD_W / 2) + x / 2];
#else
  return &displayBuf[ y / 2 * LCD_W + x ];
#endif
}

void lcdDrawPoint(coord_t x, coord_t y, LcdFlags att)
{
#if defined(LCD_VERTICAL_INVERT)
  y = LCD_H - y - 1;
#endif
#if defined(LCD_HORIZONTAL_INVERT)
  x = LCD_W - x - 1;
#endif

  if (lcdIsPointOutside(x, y))
    return;

  uint8_t * p = getPixelPtr(x, y);
  uint8_t mask = PIXEL_GREY_MASK(x, y, att);
  lcdMaskPoint(p, mask, att);
}

void lcdDrawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags att)
{
  if (y < 0 || y >= LCD_H)
    return;
  if (x + w > LCD_W) {
    if (x >= LCD_W)
      return;
    w = LCD_W - x;
  }

  while (w--) {
    if (pat & 1) {
      lcdDrawPoint(x, y, att);
      pat = (pat >> 1) | 0x80;
    }
    else {
      pat = pat >> 1;
    }
    x++;
  }
}

void lcdInvertLine(int8_t line)
{
  if (line < 0) return;
  if (line >= LCD_LINES) return;

#if defined(LCD_VERTICAL_INVERT)
  uint8_t * p  = &displayBuf[(LCD_LINES - line - 1) * 4 * LCD_W];
#else
  uint8_t * p  = &displayBuf[line * 4 * LCD_W];
#endif
  for (coord_t x = 0; x < LCD_W * 4; x++) {
    ASSERT_IN_DISPLAY(p);
    *p++ ^= 0xff;
  }
}

void lcdDraw1bitBitmap(coord_t x, coord_t y, const uint8_t * img, uint8_t idx, LcdFlags att)
{
  const uint8_t * q = img;
  uint8_t w = *(q++);
  uint8_t hb = (*(q++) + 7) / 8;
  bool inv = (att & INVERS) ? true : (att & BLINK ? BLINK_ON_PHASE : false);

  q += idx * w * hb;

  for (uint8_t yb = 0; yb < hb; yb++) {
    for (uint8_t i = 0; i < w; i++) {
      uint8_t b = *(q++);
      uint8_t val = inv ? ~b : b;
      for (int k = 0; k < 8; k++) {
        if (val & (1 << k)) {
          lcdDrawPoint(x + i, y + yb * 8 + k, 0);
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
  if (x + width > LCD_W) {
    if (x >= LCD_W ) return;
    width = LCD_W - x;
  }
  uint8_t rows = (*q++ + 1) / 2;

  for (uint8_t row = 0; row < rows; row++) {
    q = img + 2 + row * w + offset;
    uint8_t * p = &displayBuf[(row + (y / 2)) * LCD_W + x];
    for (coord_t i = 0; i < width; i++) {
      if (p >= DISPLAY_END) return;
      uint8_t b = *q++;
      if (y & 1) {
        *p = (*p & 0x0f) + ((b & 0x0f) << 4);
        if ((p + LCD_W) < DISPLAY_END) {
          *(p + LCD_W) = (*(p + LCD_W) & 0xf0) + ((b & 0xf0) >> 4);
        }
      }
      else {
        *p = b;
      }
      p++;
    }
  }
}

