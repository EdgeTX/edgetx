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

#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define INDENT_WIDTH                   (FW/2)

#define FW                             6
#define FWNUM                          5
#define FH                             8

#define LCD_LINES                      (LCD_H/FH)
#define LCD_COLS                       (LCD_W/FW)

#define NUM_BODY_LINES                 (LCD_LINES-1)
#define TEXT_VIEWER_LINES              NUM_BODY_LINES
#define MENU_HEADER_HEIGHT             FH

#define CURVE_SIDE_WIDTH               (LCD_H/2-2)
#define CURVE_CENTER_X                 (LCD_W-CURVE_SIDE_WIDTH-3)
#define CURVE_CENTER_Y                 (LCD_H/2)

/* lcdDrawText flags */
#define BLINK                          0x01
#define INVERS                         0x02
#define BOLD                           0x40
#define LEFT                           0x00
#define RIGHT                          0x04
#define CENTERED                       0x20
#define CONDENSED                      0x08
#define FIXEDWIDTH                     0x10
/* no 0x80 here because of "GV"1 which is aligned LEFT */
/* no 0x10 here because of "MODEL"01 which uses LEADING0 */

/* lcdDrawNumber additional flags */
#define LEADING0                       0x10
#define PREC1                          0x20
#define PREC2                          0x30
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

#define IS_LEFT_ALIGNED(att)           !((att) & RIGHT)
#define IS_RIGHT_ALIGNED(att)          (!IS_LEFT_ALIGNED(att))

/* line, rect, square flags */
#define FORCE                          0x02
#define ERASE                          0x04
#define ROUND                          0x08
#define FILL_WHITE                     0x10

/* telemetry flags */
#define NO_UNIT                        0x40

#define FONTSIZE_MASK                0x0700
#define FONTSIZE(x)                  ((x) & FONTSIZE_MASK)
#define TINSIZE                      0x0100
#define SMLSIZE                      0x0200
#define MIDSIZE                      0x0300
#define DBLSIZE                      0x0400
#define XXLSIZE                      0x0500
#define ERASEBG                      0x8000
#define VERTICAL                     0x0800

#define TIMEBLINK                    0x1000
#define TIMEHOUR                     0x2000
#define STREXPANDED                  0x4000

extern pixel_t displayBuf[DISPLAY_BUFFER_SIZE];
extern coord_t lcdLastRightPos;
extern coord_t lcdLastLeftPos;
extern coord_t lcdNextPos;

#define DISPLAY_END                    (displayBuf + DISPLAY_BUFFER_SIZE)
#define IS_IN_DISPLAY(p)               ((p) >= displayBuf && (p) < DISPLAY_END)
#define ASSERT_IN_DISPLAY(p)           assert((p) >= displayBuf && (p) < DISPLAY_END)

#define SOLID                          0xff
#define DOTTED                         0x55

#if defined(BOOT)
  #define BLINK_ON_PHASE               (0)
#else
  #define SLOW_BLINK_ON_PHASE          (g_blinkTmr10ms & (1<<7))
  #define BLINK_ON_PHASE               (g_blinkTmr10ms & (1<<6))
  #define FAST_BLINK_ON_PHASE          (g_blinkTmr10ms & (1<<4))
#endif

struct PatternData
{
  uint8_t width;
  uint8_t height;
  const uint8_t * data;
};

void lcdClear();

void lcdDrawChar(coord_t x, coord_t y, uint8_t c);
void lcdDrawChar(coord_t x, coord_t y, uint8_t c, LcdFlags flags);
void lcdDrawCenteredText(coord_t y, const char * s, LcdFlags flags = 0);
void lcdDrawText(coord_t x, coord_t y, const char * s, LcdFlags flags);
void lcdDrawTextAtIndex(coord_t x, coord_t y, const char *const *s, uint8_t idx, LcdFlags flags);
void lcdDrawSizedText(coord_t x, coord_t y, const char * s, unsigned char len, LcdFlags flags);
void lcdDrawText(coord_t x, coord_t y, const char * s);
void lcdDrawSizedText(coord_t x, coord_t y, const char * s, unsigned char len);
void lcdDrawTextAlignedLeft(coord_t y, const char * s);
void lcdDrawTextIndented(coord_t y, const char * s);

void lcdDrawHexNumber(coord_t x, coord_t y, uint32_t val, LcdFlags mode=0);

void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags mode, uint8_t len);
void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags mode=0);

void drawModelName(coord_t x, coord_t y, char * name, uint8_t id, LcdFlags att);
void drawSwitch(coord_t x, coord_t y, swsrc_t swtch, LcdFlags att=0, bool autoBold = true);
void drawCurveName(coord_t x, coord_t y, int8_t idx, LcdFlags att=0);
void drawTimerMode(coord_t x, coord_t y, swsrc_t mode, LcdFlags att=0);

void putsChn(coord_t x, coord_t y, uint8_t idx, LcdFlags attr);
void putsChnLetter(coord_t x, coord_t y, uint8_t idx, LcdFlags attr);

void putsVolts(coord_t x, coord_t y, uint16_t volts, LcdFlags att);
void putsVBat(coord_t x, coord_t y, LcdFlags att);

void lcdDrawPoint(coord_t x, coord_t y, LcdFlags att=0);
void lcdMaskPoint(uint8_t *p, uint8_t mask, LcdFlags att=0);
void lcdDrawSolidHorizontalLine(coord_t x, coord_t y, coord_t w, LcdFlags att=0);
void lcdDrawHorizontalLine(coord_t x, coord_t y, coord_t w, uint8_t pat, LcdFlags att=0);
void lcdDrawSolidVerticalLine(coord_t x, coord_t y, coord_t h, LcdFlags att=0);
void lcdDrawVerticalLine(coord_t x, coord_t y, coord_t h, uint8_t pat, LcdFlags att=0);
void lcdDrawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat=SOLID, LcdFlags att=0);

void lcdDrawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat=SOLID, LcdFlags att=0);
void lcdDrawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat=SOLID, LcdFlags att=0);

inline void lcdDrawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags att=0)
{
  lcdDrawFilledRect(x, y, w, h, SOLID, att);
}

inline void lcdDrawSquare(coord_t x, coord_t y, coord_t w, LcdFlags att=0)
{
  lcdDrawRect(x, y, w, w, SOLID, att);
}

void lcdInvertLine(int8_t line);
#define lcdInvertLastLine() lcdInvertLine(LCD_LINES-1)

void drawTelemetryTopBar();

#define V_BAR(xx, yy, ll)    \
  lcdDrawSolidVerticalLine(xx-1,yy-ll,ll);  \
  lcdDrawSolidVerticalLine(xx  ,yy-ll,ll);  \
  lcdDrawSolidVerticalLine(xx+1,yy-ll,ll)

void lcdDraw1bitBitmap(coord_t x, coord_t y, const unsigned char * img, uint8_t idx, LcdFlags att=0);

uint8_t getTextWidth(const char * s, uint8_t len=0, LcdFlags flags=0);
