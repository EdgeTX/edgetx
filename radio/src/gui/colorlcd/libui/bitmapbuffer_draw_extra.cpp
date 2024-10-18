/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

// #include <math.h>

#include "bitmapbuffer.h"
#include "bitmaps.h"
#include "board.h"
#include "dma2d.h"
#include "edgetx.h"
#include "strhelpers.h"
#include "telemetry/telemetry_sensors.h"
#include "translations.h"

#include <cmath>

// Drawing functions used by Lua API

void BitmapBuffer::invertRect(coord_t x, coord_t y, coord_t w, coord_t h,
                              LcdFlags flags)
{
  APPLY_OFFSET();

  if (!applyClippingRect(x, y, w, h)) return;

  // No 'opacity' here, only 'color'
  pixel_t color = COLOR_VAL(flags);
  RGB_SPLIT(color, red, green, blue);
  pixel_t bg_color = COLOR_VAL(COLOR_THEME_PRIMARY2);
  RGB_SPLIT(bg_color, bgRed, bgGreen, bgBlue);

  DMAWait();
#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  for (int i = y; i < y + h; i++) {
    pixel_t *p = getPixelPtrAbs(x, i);
    for (int j = 0; j < w; j++) {
      // TODO ASSERT_IN_DISPLAY(p);
      RGB_SPLIT(*p, pRed, pGreen, pBlue);
      drawPixel(p, RGB_JOIN(bgRed + red - pRed, bgGreen + green - pGreen,
                            bgBlue + blue - pBlue));
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }
}

void BitmapBuffer::drawValueWithUnit(coord_t x, coord_t y, int val,
                                     uint8_t unit, LcdFlags flags)
{
  if ((flags & NO_UNIT) || unit == UNIT_RAW) {
    drawNumber(x, y, val, flags & (~NO_UNIT));
  } else {
    drawNumber(x, y, val, flags & (~NO_UNIT), 0, nullptr, STR_VTELEMUNIT[unit]);
  }
}

point_t BitmapBuffer::drawTextLines(coord_t left, coord_t top, coord_t width,
                                 coord_t height, const char *str,
                                 LcdFlags flags)
{
  coord_t x = left;
  coord_t y = top;
  coord_t line = getFontHeightCondensed(flags & 0xFFFF);
  coord_t space = getTextWidth(" ", 1, flags);
  coord_t word;
  point_t maxP={0,0};
  const char *nxt = str;
  flags &= ~(VCENTERED | CENTERED | RIGHT);

  while (true) {
    for (bool done = false; !done; nxt++) {
      switch (nxt[0]) {
        case '-':
        case '/':
        case ':':
        case '(':
        case '{':
        case '[':
          nxt++;
        case ' ':
        case '\n':
        case '\0':
          done = true;
      }
    }
    nxt--;
    word = getTextWidth(str, nxt - str, flags);
    if (x + word > left + width && x > left) {
      x = left;
      y += line;
      maxP.y= y + line;
    }
    else
    {
      maxP.x = max( maxP.x, x+word);
    }
    if (y + line > top + height) return maxP;
    drawSizedText(x, y, str, nxt - str, flags);
    x += word;
    switch (nxt[0]) {
      case '\0':
        return maxP;
      case '\n':
        x = left;
        y += line;
        maxP.y= y + line;
        nxt++;
        break;
      case ' ':
        x += space;
        nxt++;
    }
    str = nxt;
  }
}

void BitmapBuffer::drawSource(coord_t x, coord_t y, mixsrc_t idx,
                              LcdFlags flags)
{
  char s[16];
  getSourceString(s, idx);
  drawText(x, y, s, flags);
}

coord_t BitmapBuffer::drawSwitch(coord_t x, coord_t y, int32_t idx,
                                 LcdFlags flags)
{
  return drawText(x, y, getSwitchPositionName(idx), flags);
}

void BitmapBuffer::drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags flags)
{
  char str[LEN_TIMER_STRING];
  TimerOptions timerOptions;
  timerOptions.options = (flags & TIMEHOUR) != 0 ? SHOW_TIME : SHOW_TIMER;
  getTimerString(str, tme, timerOptions);
  drawText(x, y, str, flags);
}

coord_t BitmapBuffer::drawGPSCoord(coord_t x, coord_t y, int32_t value,
                                   const char *direction, LcdFlags flags,
                                   bool seconds)
{
  std::string s = getGPSCoord(value, direction, seconds);
  return drawText(x, y, s.c_str(), flags);
}

void BitmapBuffer::drawGPSPosition(coord_t x, coord_t y, int32_t longitude,
                                   int32_t latitude, LcdFlags flags)
{
  if (flags & PREC1) {
    drawGPSCoord(x, y, latitude, "NS", flags, true);
    drawGPSCoord(x, y + EdgeTxStyles::PAGE_LINE_HEIGHT, longitude, "EW", flags, true);
  } else {
    if (flags & RIGHT) {
      x = drawGPSCoord(x, y, longitude, "EW", flags, true);
      drawGPSCoord(x - 5, y, latitude, "NS", flags, true);
    } else {
      x = drawGPSCoord(x, y, latitude, "NS", flags, true);
      drawGPSCoord(x + 5, y, longitude, "EW", flags, true);
    }
  }
}

void BitmapBuffer::drawDate(coord_t x, coord_t y, TelemetryItem &telemetryItem,
                            LcdFlags att)
{
  bool doTwoLines = false;
  coord_t ox = x;

  if (att & FONT(XL)) {
    att &= ~FONT_MASK;
    doTwoLines = true;
  }

  std::string s = getTelemDate(telemetryItem);
  x = drawText(x, y, s.c_str(), att);

  if (doTwoLines) {
    y += EdgeTxStyles::PAGE_LINE_HEIGHT;
    x = ox;
  } else {
    x += 11;
  }

  s = getTelemTime(telemetryItem);
  drawText(x, y, s.c_str(), att);
}

void BitmapBuffer::drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor,
                                         int32_t value, LcdFlags flags)
{
  if (sensor >= MAX_TELEMETRY_SENSORS) {
    // Lua luaLcdDrawChannel() can call us with a bad value
    return;
  }

  TelemetryItem &telemetryItem = telemetryItems[sensor];
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[sensor];

  if (telemetrySensor.unit == UNIT_DATETIME) {
    drawDate(x, y, telemetryItem, flags);
  } else if (telemetrySensor.unit == UNIT_GPS) {
    drawGPSPosition(x, y, telemetryItem.gps.longitude,
                    telemetryItem.gps.latitude, flags);
  } else if (telemetrySensor.unit == UNIT_TEXT) {
    drawSizedText(x, flags & FONT(XL) ? y + 1 : y, telemetryItem.text,
                  sizeof(telemetryItem.text), flags & ~FONT(XL));
  } else {
    if (telemetrySensor.prec > 0) {
      flags |= (telemetrySensor.prec == 1 ? PREC1 : PREC2);
    }
    drawValueWithUnit(
        x, y, value,
        telemetrySensor.unit == UNIT_CELLS ? UNIT_VOLTS : telemetrySensor.unit,
        flags);
  }
}

void BitmapBuffer::drawScaledBitmap(const BitmapBuffer *bitmap, coord_t x, coord_t y,
                                    coord_t w, coord_t h)
{
  if (bitmap) {
    float vscale = float(h) / bitmap->height();
    float hscale = float(w) / bitmap->width();
    float scale = vscale < hscale ? vscale : hscale;

    int xshift = (w - (bitmap->width() * scale)) / 2;
    int yshift = (h - (bitmap->height() * scale)) / 2;
    // TRACE("  BitmapBuffer::drawScaledBitmap()---- scale %f", scale);
    drawBitmap(x + xshift, y + yshift, bitmap, 0, 0, 0, 0, scale);
  }
}

//
// Liang-barsky clipping algo
//
static float lb_max(float arr[], int n)
{
  float m = 0;
  for (int i = 0; i < n; ++i)
    if (m < arr[i]) m = arr[i];
  return m;
}

// this function gives the minimum
static float lb_min(float arr[], int n)
{
  float m = 1;
  for (int i = 0; i < n; ++i)
    if (m > arr[i]) m = arr[i];
  return m;
}

bool BitmapBuffer::liangBarskyClipper(coord_t &x1, coord_t &y1, coord_t &x2,
                                      coord_t &y2)
{
  // defining variables
  float p1 = -(x2 - x1);
  float p2 = -p1;
  float p3 = -(y2 - y1);
  float p4 = -p3;

  float q1 = x1 - xmin;
  float q2 = xmax - x1;
  float q3 = y1 - ymin;
  float q4 = ymax - y1;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) ||
      (p4 == 0 && q4 < 0)) {
    return false;
  }

  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1;  // for negative p1, add it to negative array
      posarr[posind++] = r2;  // and add p2 to positive array
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }

  float xn1, yn1, xn2, yn2;
  float rn1, rn2;
  rn1 = lb_max(negarr, negind);  // maximum of negative array
  rn2 = lb_min(posarr, posind);  // minimum of positive array

  if (rn1 > rn2) {  // reject
    return false;
  }

  xn1 = x1 + p2 * rn1;
  yn1 = y1 + p4 * rn1;  // computing new points

  xn2 = x1 + p2 * rn2;
  yn2 = y1 + p4 * rn2;

  x1 = coord_t(xn1);
  y1 = coord_t(yn1);
  x2 = coord_t(xn2);
  y2 = coord_t(yn2);

  return true;
}

void BitmapBuffer::drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
                            uint8_t pat, LcdFlags flags)
{
  // Offsets
  x1 += offsetX;
  y1 += offsetY;
  x2 += offsetX;
  y2 += offsetY;

  if (!liangBarskyClipper(x1, y1, x2, y2)) return;

  // TODO; Replace with LVGL line draw - currently does not support dotted line drawing
  //       except for vertical and horizontal.

  // No 'opacity' here, only 'color'
  pixel_t color = COLOR_VAL(flags);

  int dx = x2 - x1; /* the horizontal distance of the line */
  int dy = y2 - y1; /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs >> 1;
  int y = dxabs >> 1;
  int px = x1;
  int py = y1;

  DMAWait();
#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  if (dxabs >= dyabs) {
    /* the line is more horizontal than vertical */
    for (int i = 0; i <= dxabs; i++) {
      if ((1 << (px % 8)) & pat) {
        drawPixelAbs(px, py, color);
      }
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
    }
  } else {
    /* the line is more vertical than horizontal */
    for (int i = 0; i <= dyabs; i++) {
      if ((1 << (py % 8)) & pat) {
        drawPixelAbs(px, py, color);
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

/*
  Notice: BitmapBuffer::drawFilledTriangle use the implementation
          made for the Adafruit GFX library, which is licensed under BSD
          license.

  Software License Agreement (BSD License)

  Copyright (c) 2012 Adafruit Industries.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
void BitmapBuffer::drawFilledTriangle(coord_t x0, coord_t y0, coord_t x1,
                                      coord_t y1, coord_t x2, coord_t y2,
                                      LcdFlags flags, uint8_t opacity)
{
  coord_t a, b;

#define SWAP(a, b)   \
  {                  \
    coord_t tmp = b; \
    b = a;           \
    a = tmp;         \
  }

  if (y0 > y1) {
    SWAP(y0, y1);
    SWAP(x0, x1);
  }
  if (y1 > y2) {
    SWAP(y2, y1);
    SWAP(x2, x1);
  }
  if (y0 > y1) {
    SWAP(y0, y1);
    SWAP(x0, x1);
  }

  if (y0 == y2) {  // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    drawHorizontalLine(a, y0, b - a + 1, SOLID, flags, opacity);
    return;
  }

  coord_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  coord_t sa = 0, sb = 0;
  coord_t last = (y1 == y2) ? y1 : y1 - 1;
  coord_t y;

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    if (a > b) SWAP(a, b);
    drawHorizontalLine(a, y, b - a + 1, SOLID, flags, opacity);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);

  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    if (a > b) SWAP(a, b);
    drawHorizontalLine(a, y, b - a + 1, SOLID, flags, opacity);
  }
}

void BitmapBuffer::drawFilledCircle(coord_t x, coord_t y, coord_t radius,
                                    LcdFlags flags)
{
  APPLY_OFFSET();

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = LV_OPA_COVER;
  rect_dsc.bg_color = makeLvColor(flags);
  rect_dsc.radius = LV_RADIUS_CIRCLE;

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_area_t coords = {
      (lv_coord_t)(x - radius),
      (lv_coord_t)(y - radius),
      (lv_coord_t)(x + radius),
      (lv_coord_t)(y + radius),
  };

  if (draw_ctx) {
    lv_draw_rect(draw_ctx, &rect_dsc, &coords);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_rect(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        coords.y2 - coords.y1 + 1, &rect_dsc);
  }
#endif
}

void BitmapBuffer::drawCircle(coord_t x, coord_t y, coord_t radius,
                              LcdFlags flags, coord_t thickness)
{
  APPLY_OFFSET();

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = LV_OPA_TRANSP;
  rect_dsc.border_opa = LV_OPA_COVER;
  rect_dsc.border_color = makeLvColor(flags);
  rect_dsc.border_width = thickness;
  rect_dsc.radius = LV_RADIUS_CIRCLE;

  if (draw_ctx) {
    x += draw_ctx->buf_area->x1;
    y += draw_ctx->buf_area->y1;
  }

  lv_area_t coords = {
      (lv_coord_t)(x - radius),
      (lv_coord_t)(y - radius),
      (lv_coord_t)(x + radius),
      (lv_coord_t)(y + radius),
  };

  if (draw_ctx) {
    lv_draw_rect(draw_ctx, &rect_dsc, &coords);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_rect(canvas, coords.x1, coords.y1, coords.x2 - coords.x1 + 1,
                        coords.y2 - coords.y1 + 1, &rect_dsc);
  }
#endif
}

class Slope
{
 public:
  explicit Slope(int angle)
  {
    if (angle < 0) angle += 360;
    if (angle > 360) angle %= 360;
    float radians = float(angle) * (M_PI / 180.0f);
    if (angle == 0) {
      left = false;
      value = 100000;
    } else if (angle == 360) {
      left = true;
      value = 100000;
    } else if (angle >= 180) {
      left = true;
      value = -(cosf(radians) * 100 / sinf(radians));
    } else {
      left = false;
      value = (cosf(radians) * 100 / sinf(radians));
    }
  }

  Slope(bool left, int value) : left(left), value(value) {}

  bool isBetween(const Slope &start, const Slope &end) const
  {
    if (left) {
      if (start.left) {
        if (end.left)
          return end.value > start.value
                     ? (value <= end.value && value >= start.value)
                     : (value <= end.value || value >= start.value);
        else
          return value >= start.value;
      } else {
        if (end.left)
          return value <= end.value;
        else
          return end.value > start.value;
      }
    } else {
      if (start.left) {
        if (end.left)
          return start.value > end.value;
        else
          return value >= end.value;
      } else {
        if (end.left)
          return value <= start.value;
        else
          return end.value < start.value
                     ? (value >= end.value && value <= start.value)
                     : (value <= start.value || value >= end.value);
      }
    }
  }

  Slope &invertVertical()
  {
    value = -value;
    return *this;
  }

  Slope &invertHorizontal()
  {
    left = !left;
    return *this;
  }

 protected:
  bool left;
  int value;
};

void BitmapBuffer::drawBitmapPatternPie(coord_t x, coord_t y,
                                        const MaskBitmap *img, LcdFlags flags,
                                        int startAngle, int endAngle)
{
  // TODO: can this converted to LVGL primitives?

  if (endAngle == startAngle) {
    endAngle += 1;
  }

  Slope startSlope(startAngle);
  Slope endSlope(endAngle);

  pixel_t color = COLOR_VAL(flags);

  coord_t width = img->width;
  coord_t height = img->height;
  const uint8_t *q = img->data;

  int w2 = width / 2;
  int h2 = height / 2;

  DMAWait();
#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  for (int y1 = h2 - 1; y1 >= 0; y1--) {
    for (int x1 = w2 - 1; x1 >= 0; x1--) {
      Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
      if (slope.isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 - y1,
                       q[(h2 - y1) * width + w2 + x1] >> 4, color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 + y1,
                       q[(h2 + y1) * width + w2 + x1] >> 4, color);
      }
      if (slope.invertHorizontal().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 + y1,
                       q[(h2 + y1) * width + w2 - x1] >> 4, color);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 - y1,
                       q[(h2 - y1) * width + w2 - x1] >> 4, color);
      }
    }
  }
}

void BitmapBuffer::drawAnnulusSector(coord_t x, coord_t y,
                                     coord_t internalRadius,
                                     coord_t externalRadius, int startAngle,
                                     int endAngle, LcdFlags flags)
{
  APPLY_OFFSET();

  if (endAngle == startAngle)
    endAngle += 1;

  startAngle -= 90;
  if (startAngle < 0) startAngle += 360;
  endAngle -= 90;
  if (endAngle < 0) endAngle += 360;

  lv_draw_arc_dsc_t arc_dsc;
  lv_draw_arc_dsc_init(&arc_dsc);

  arc_dsc.width = externalRadius - internalRadius;
  arc_dsc.opa = LV_OPA_COVER;
  arc_dsc.color = makeLvColor(flags);

  if (draw_ctx) {
    lv_point_t p;
    p.x = x + draw_ctx->buf_area->x1;
    p.y = y + draw_ctx->buf_area->y1;
    lv_draw_arc(draw_ctx, &arc_dsc, &p, externalRadius, startAngle, endAngle);
  }
#if !defined(BOOT)
  else if (canvas) {
    lv_canvas_draw_arc(canvas, x, y, externalRadius, startAngle, endAngle, &arc_dsc);
  }
#endif
}

MaskBitmap *BitmapBuffer::to8bitMask(size_t *size) const
{
  static uint8_t grayScale[16] = {255, 238, 221, 204, 187, 170, 153, 136, 119, 102, 85, 68, 51, 34, 17, 0};

  *size = width() * height() + 4;
  MaskBitmap *mask = (MaskBitmap*)malloc(*size);
  mask->width = width();
  mask->height = height();

  const pixel_t *p = getPixelPtrAbs(0, 0);
  int ofs = 0;
  if (format == BMP_ARGB4444) {
    for (int i = width() * height(); i > 0; i--) {
      ARGB_SPLIT(*p, a __attribute__((unused)), r, g, b);
      int v = (r + g + b) / 3;
      mask->data[ofs++] = grayScale[v];
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  } else {  // BMP_RGB565
    for (int i = width() * height(); i > 0; i--) {
      RGB_SPLIT(*p, r, g, b);
      int v = ((r + (g >> 1) + b) / 3) >> 1;
      mask->data[ofs++] = grayScale[v];
      MOVE_TO_NEXT_RIGHT_PIXEL(p);
    }
  }

  return mask;
}

// Apply a mask ('bmp') + color ('flags') on top of current pixels:
//
//  drawAlphaPixel(bmp[x][y], pixel(x,y), color)
//
//
//
void BitmapBuffer::drawBitmapPattern(coord_t x, coord_t y, const MaskBitmap *bmp,
                                     LcdFlags flags, coord_t offset,
                                     coord_t width)
{
  APPLY_OFFSET();

  coord_t bmpw = bmp->width;
  coord_t bmph = bmp->height;

  coord_t srcx = offset;
  coord_t srcy = 0;

  coord_t srcw = (width != 0 ? width : bmpw);
  coord_t srch = bmph;
  if (srcx + srcw > bmpw) srcw = bmpw - srcx;
  if (srcy + srch > bmph) srch = bmph - srcy;

  if (x < xmin) {
    srcw += x - xmin;
    srcx -= x - xmin;
    x = xmin;
  }
  if (y < ymin) {
    srch += y - ymin;
    srcy -= y - ymin;
    y = ymin;
  }
  if (x + srcw > xmax) {
    srcw = xmax - x;
  }
  if (y + srch > ymax) {
    srch = ymax - y;
  }

  if (srcw <= 0 || srch <= 0) {
    return;
  }

  DMACopyAlphaMask(data, _width, _height, x, y, bmp->data, bmpw, bmph, srcx,
                   srcy, srcw, srch, COLOR_VAL(flags));
}

coord_t BitmapBuffer::drawNumber(coord_t x, coord_t y, int32_t val,
                                 LcdFlags flags, uint8_t len,
                                 const char *prefix, const char *suffix)
{
  char s[49];
  formatNumberAsString(s, 49, val, flags, len, prefix, suffix);
  return drawText(x, y, s, flags);
}
