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

#define LUA_LIB

#include <cctype>
#include <cstdio>

#include "edgetx.h"
#include "libopenui.h"
#include "widget.h"

#include "lua_api.h"
#include "lua_widget.h"
#include "api_colorlcd.h"

#define BITMAP_METATABLE "BITMAP*"

BitmapBuffer* luaLcdBuffer  = nullptr;
LuaWidget *runningFS = nullptr;

/*luadoc
@function lcd.refresh()

Refresh the LCD screen

From 2.4.0 on color LCDs, this is done automatically when the screen
needs to be refreshed (on events and depending on refresh period).

@status current Only used on non-color LCDs.
*/
static int luaLcdRefresh(lua_State *L)
{
  // This one does not seem to be used by any script
  // (at least none of the opentx LUA scripts)
  // we cannot possibly force a re-draw here
  return 0;
}

/*luadoc
@function lcd.clear([color])

Clear the LCD screen

@param color (optional, only on color screens)

@status current Introduced in 2.0.0, `color` parameter introduced in 2.2.0 RC12
*/
static int luaLcdClear(lua_State * L)
{
  if (luaLcdAllowed && luaLcdBuffer) {
    LcdFlags flags = luaL_optunsigned(L, 1, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX));
    flags = colorToRGB(flags);
    luaLcdBuffer->clear(flags);
  }
  return 0;
}

/*luadoc
@function lcd.resetBacklightTimeout()

Reset the backlight timeout

@status current Introduced in 2.3.6
*/
static int luaLcdResetBacklightTimeout(lua_State * L)
{
  if (!luaLcdAllowed)
    return 0;

  resetBacklightTimeout();
  return 0;
}

/*luadoc
@function lcd.drawPoint(x, y, [flags])

Draw a single pixel at (x,y) position

@param x (positive number) x position

@param y (positive number) y position

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@notice Taranis has an LCD display width of 212 pixels and height of 64 pixels.
Position (0,0) is at top left. Y axis is negative, top line is 0,
bottom line is 63. Drawing on an existing black pixel produces white pixel (TODO check this!)

@status current Introduced in 2.0.0
*/
static int luaLcdDrawPoint(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  LcdFlags flags = luaL_optunsigned(L, 3, 0);
  flags = colorToRGB(flags);

  // drawPixel uses color value directly; hence COLOR_VAL again
  luaLcdBuffer->drawPixel(x, y, COLOR_VAL(flags));

  return 0;
}

/*luadoc
@function lcd.drawLine(x1, y1, x2, y2, pattern, [flags])

Draw a straight line on LCD

@param x1,y1 (positive numbers) starting coordinate

@param x2,y2 (positive numbers) end coordinate

@param pattern SOLID or DOTTED

@param flags (optional) lcdflags

@notice If the start or the end of the line is outside the LCD dimensions, then the
whole line will not be drawn (starting from OpenTX 2.1.5)

@status current Introduced in 2.0.0, flags introduced in 2.3.6
*/
static int luaLcdDrawLine(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x1 = luaL_checkunsigned(L, 1);
  coord_t y1 = luaL_checkunsigned(L, 2);
  coord_t x2 = luaL_checkunsigned(L, 3);
  coord_t y2 = luaL_checkunsigned(L, 4);
  uint8_t pat = luaL_checkunsigned(L, 5);
  LcdFlags flags = luaL_optunsigned(L, 6, 0);
  flags = colorToRGB(flags);

  if (x1 > LCD_W || y1 > LCD_H || x2 > LCD_W || y2 > LCD_H)
    return 0;

  if (pat == SOLID) {
    if (x1 == x2) {
      luaLcdBuffer->drawVerticalLine(x1, y1 < y2 ? y1 : y2, y1 < y2 ? (y2 - y1) + 1 : (y1 - y2) + 1, pat, flags);
      return 0;
    } else if (y1 == y2) {
      luaLcdBuffer->drawHorizontalLine(x1 < x2 ? x1 : x2, y1, x1 < x2 ? (x2 - x1) + 1 : (x1 - x2) + 1, pat, flags);
      return 0;
    }
  }

  luaLcdBuffer->drawLine(x1, y1, x2, y2, pat, flags);

  return 0;
}

// Used to draw text, numbers and timers
static void drawString(lua_State *L, const char * s, LcdFlags flags)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);

  bool invers = flags & INVERS;
  if (flags & BLINK)
    invers = invers && !BLINK_ON_PHASE;

  if (invers) {
    // Find inverse color or read from optional Lua argument
    LcdFlags color = colorToRGB(flags);
    LcdFlags invColor = luaL_optunsigned(L, 5, ~0u); // ~0 is impossible for color flag!
    if (invColor == ~0u) {
      RGB_SPLIT(COLOR_VAL(color), r, g, b);
      invColor = COLOR2FLAGS(RGB_JOIN(31 - r, 63 - g, 31 - b));
    } else
      invColor = colorToRGB(invColor);
    flags = (flags & 0xFFFF) | invColor;
    
    // Draw color box
    constexpr coord_t INVERT_BOX_MARGIN = 2;
    int height = getFontHeight(flags & 0xFFFF) + 2 * INVERT_BOX_MARGIN;
    int width = getTextWidth(s, 0, flags);
    int ix = x - INVERT_BOX_MARGIN;
    if (flags & RIGHT)
      ix -= width;
    else if (flags & CENTERED)
      ix -= width / 2;
    width += 2 * INVERT_BOX_MARGIN;
    luaLcdBuffer->drawSolidFilledRect(ix, y - INVERT_BOX_MARGIN, width, height, color);
  } else {
    if ((flags & BLINK) && !BLINK_ON_PHASE)
      return;
    if (flags & SHADOWED)
      luaLcdBuffer->drawText(x+1, y+1, s, flags & 0xFFFF); // force black
    flags = (flags & 0xFFFF) | colorToRGB(flags);
  }
  
  luaLcdBuffer->drawText(x, y, s, flags);
}

/*luadoc
@function lcd.drawText(x, y, text [, flags [, inversColor]])

Draw a text beginning at (x,y)

@param x,y (positive numbers) starting coordinate

@param text (string) text to display

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html) for drawing flags and colors, and [Appendix](../../part_vii_-_appendix/fonts.md) for available characters in each font set.

@param inversColor (optional with the INVERS flag) overrides the inverse text color for INVERS

@status current Introduced in 2.0.0, `SHADOWED` introduced in 2.2.1
*/
static int luaLcdDrawText(lua_State *L)
{
  const char * s = luaL_checkstring(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  drawString(L, s, flags);
  return 0;
}

/*luadoc
@function lcd.sizeText(text [, flags])

Get the width and height of a text string drawn with flags

@param text (string)

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@retval w,h (integers) width and height of the text

@status current Introduced in 2.5.0
*/
static int luaLcdSizeText(lua_State *L)
{
  const char * s = luaL_checkstring(L, 1);
  LcdFlags flags = luaL_optunsigned(L, 2, 0);
  lua_pushinteger(L, getTextWidth(s, 0, flags));
  lua_pushinteger(L, getFontHeight(flags & 0xFFFF));
  return 2;
}

/*luadoc
@function lcd.drawTextLines(x, y, w, h, text [, flags])

Draw text inside rectangle (x,y,w,h) with line breaks

@param x,y (positive numbers) starting coordinate

@param w,h (positive numbers) width and height of bounding rectangle

@param text (string) text to display

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html) for drawing flags and colors, and [Appendix](../../part_vii_-_appendix/fonts.md) for available characters in each font set. RIGHT, CENTER and VCENTER are not implemented.

@retval x,y (integers) point where text drawing ended

@status current Introduced in 2.5.0, return x,y added in 2.11.0
*/
static int luaLcdDrawTextLines(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  point_t maxP = {0,0};
  const char * s = luaL_checkstring(L, 5);
  LcdFlags flags = luaL_optunsigned(L, 6, 0);
  
  bool invers = flags & INVERS;
  if (flags & BLINK)
    invers = invers && !BLINK_ON_PHASE;

  if (invers) {
    // Find inverse color or read from optional Lua argument
    LcdFlags color = colorToRGB(flags);
    LcdFlags invColor = luaL_optunsigned(L, 7, ~0u); // ~0 is impossible for color flag!
    if (invColor == ~0u) {
      RGB_SPLIT(COLOR_VAL(color), r, g, b);
      invColor = COLOR2FLAGS(RGB_JOIN(31 - r, 63 - g, 31 - b));
    } else
      invColor = colorToRGB(invColor);
    flags = (flags & 0xFFFF) | invColor;
    
    // Draw color box
    luaLcdBuffer->drawSolidFilledRect(x, y, w, h, color);
  } else {
    if ((flags & BLINK) && !BLINK_ON_PHASE)
      return 0;
    if (flags & SHADOWED)
      luaLcdBuffer->drawTextLines(x+1, y+1, w, h, s, flags & 0xFFFF); // force black
    flags = (flags & 0xFFFF) | colorToRGB(flags);
  }
  
  maxP = luaLcdBuffer->drawTextLines(x, y, w, h, s, flags);
  if (!invers && flags & SHADOWED) {
    maxP.x++;
    maxP.y++;
  }
  lua_pushinteger(L, maxP.x);
  lua_pushinteger(L, maxP.y);
  return 2;
}

/*luadoc
@function lcd.drawTimer(x, y, value [, flags [, inversColor]])

Display a value formatted as time at (x,y)

@param x,y (positive numbers) starting coordinate

@param value (number) time in seconds

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@param inversColor (optional with INVERS flag) overrides the inverse text color for INVERS

@status current Introduced in 2.0.0
*/
static int luaLcdDrawTimer(lua_State *L)
{
  char s[LEN_TIMER_STRING];
  int tme = luaL_checkinteger(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  TimerOptions timerOptions;
  timerOptions.options = (flags & TIMEHOUR) != 0 ? SHOW_TIME : SHOW_TIMER;
  getTimerString(s, tme, timerOptions);
  drawString(L, s, flags);
  return 0;
}

/*luadoc
@function lcd.drawNumber(x, y, value [, flags [, inversColor]])

Display a number at (x,y)

@param x,y (positive numbers) starting coordinate

@param value (number) value to display

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@param inversColor (optional with INVERS flag) overrides the inverse text color for INVERS

@status current Introduced in 2.0.0
*/
static int luaLcdDrawNumber(lua_State *L)
{
  char s[49];
  int val = luaL_checkinteger(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  formatNumberAsString(s, 49, val, flags & 0xFFFF);
  drawString(L, s, flags);
  return 0;
}

/*luadoc
@function lcd.drawChannel(x, y, source, flags)

Display a telemetry value at (x,y)

@param x,y (positive numbers) starting coordinate

@param source can be a source identifier (number) or a source name (string).
See getValue()

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.0.6, changed in 2.1.0 (only telemetry sources are valid)
*/
static int luaLcdDrawChannel(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int channel = -1;
  if (lua_isnumber(L, 3)) {
    channel = luaL_checkinteger(L, 3);
  }
  else {
    const char * what = luaL_checkstring(L, 3);
    LuaField field;
    bool found = luaFindFieldByName(what, field);
    if (found) {
      channel = field.id;
    }
  }
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  flags = colorToRGB(flags);
  getvalue_t value = getValue(channel);
  luaLcdBuffer->drawSensorCustomValue(x, y, (channel-MIXSRC_FIRST_TELEM)/3, value, flags);

  return 0;
}

/*luadoc
@function lcd.drawSwitch(x, y, switch, flags)

Draw a text representation of switch at (x,y)

@param x,y (positive numbers) starting coordinate

@param switch (number) number of switch to display, negative number
displays negated switch

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html), only SMLSIZE, BLINK and INVERS.

@status current Introduced in 2.0.0
*/
static int luaLcdDrawSwitch(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  flags = colorToRGB(flags);
  luaLcdBuffer->drawSwitch(x, y, s, flags);

  return 0;
}

/*luadoc
@function lcd.drawSource(x, y, source [, flags])

Displays the name of the corresponding input as defined by the source at (x,y)

@param x,y (positive numbers) starting coordinate

@param source (number) source index

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.0.0
*/
static int luaLcdDrawSource(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  flags = colorToRGB(flags);
  luaLcdBuffer->drawSource(x, y, s, flags);

  return 0;
}

/*luadoc
@function Bitmap.open(name)

Loads a bitmap in memory, for later use with lcd.drawBitmap(). Bitmaps should be loaded only
once, returned object should be stored and used for drawing. If loading fails for whatever
reason the resulting bitmap object will have width and height set to zero.

Bitmap loading can fail if:
 * File is not found or contains invalid image
 * System is low on memory
 * Combined memory usage of all Lua script bitmaps exceeds certain value

@param name (string) full path to the bitmap on SD card (i.e. “/IMAGES/test.bmp”)

@retval bitmap (object) a bitmap object that can be used with other bitmap functions

@notice Only available on Horus

@status current Introduced in 2.2.0
*/
static int luaOpenBitmap(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);

  BitmapBuffer **b =
      (BitmapBuffer **)lua_newuserdata(L, sizeof(BitmapBuffer *));

  if (luaExtraMemoryUsage > LUA_MEM_EXTRA_MAX) {
    // already allocated more than max allowed, fail
    TRACE("luaOpenBitmap: Error, using too much memory %u/%u",
          luaExtraMemoryUsage, LUA_MEM_EXTRA_MAX);
    *b = 0;
  } else {
    *b = BitmapBuffer::loadBitmap(filename);
    if (*b == NULL && G(L)->gcrunning) {
      luaC_fullgc(L, 1);                       /* try to free some memory... */
      *b = BitmapBuffer::loadBitmap(filename); /* try again */
    }
  }

  if (*b) {
    uint32_t size = (*b)->getDataSize();
    luaExtraMemoryUsage += size;
    TRACE("luaOpenBitmap: %p (%u)", *b, size);
  }

  luaL_getmetatable(L, BITMAP_METATABLE);
  lua_setmetatable(L, -2);

  return 1;
}

static BitmapBuffer * checkBitmap(lua_State * L, int index)
{
  BitmapBuffer ** b = (BitmapBuffer **)luaL_checkudata(L, index, BITMAP_METATABLE);
  return *b;
}

/*luadoc
@function Bitmap.getSize(name)

Return width, height of a bitmap object

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@retval multiple returns 2 values:
 * (number) width in pixels
 * (number) height in pixels

@notice Only available on Horus

@status current Introduced in 2.2.0
*/
static int luaGetBitmapSize(lua_State * L)
{
  const BitmapBuffer * b = checkBitmap(L, 1);
  if (b) {
    lua_pushinteger(L, b->width());
    lua_pushinteger(L, b->height());
  }
  else {
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 0);
  }
  return 2;
}

/*luadoc
@function Bitmap.resize(bitmap, width, height)

Return a resized bitmap object

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@param width (number) the new bitmap width

@param height (number) the new bitmap height

@notice Only available on Horus

@status current Introduced in 2.8.0
*/
static int luaBitmapResize(lua_State * L)
{
  const BitmapBuffer * b = checkBitmap(L, 1);
  unsigned int w = luaL_checkunsigned(L, 2);
  unsigned int h = luaL_checkunsigned(L, 3);

  if (!b) {
    lua_pushnil(L);
    return 1;
  }

  BitmapBuffer **n = (BitmapBuffer**)lua_newuserdata(L, sizeof(BitmapBuffer*));

  if (luaExtraMemoryUsage > LUA_MEM_EXTRA_MAX) {
    // already allocated more than max allowed, fail
    TRACE("luaOpenBitmap: Error, using too much memory %u/%u",
          luaExtraMemoryUsage, LUA_MEM_EXTRA_MAX);
    *n = 0;
  } else {
    *n = new BitmapBuffer(BMP_ARGB4444, w, h);
    (*n)->clear();
    (*n)->drawScaledBitmap(b, 0, 0, w, h);
  }

  if (*n) {
    uint32_t size = (*n)->getDataSize();
    luaExtraMemoryUsage += size;
    TRACE("luaResizeBitmap: %p (%u)", *n, size);
  }

  luaL_getmetatable(L, BITMAP_METATABLE);
  lua_setmetatable(L, -2);

  return 1;
}

/*luadoc
@function Bitmap.toMask(bitmap)

Return a 8bit bitmap mask that can be used with lcd.drawBitmapPattern()

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@retval a bitmap mask

@notice Only available on Horus

@status current Introduced in 2.8.0
*/
static int luaBitmapTo8bitMask(lua_State * L)
{
  const BitmapBuffer * b = checkBitmap(L, 1);
  if (b) {
    size_t size;
    auto mask = b->to8bitMask(&size);
    lua_pushlstring(L, (char*)mask, size);
  }
  else {
    lua_pushlstring(L, "\x00\x00\x00\x00", 4);
  }
  return 1;
}

static int luaDestroyBitmap(lua_State * L)
{
  BitmapBuffer * b = checkBitmap(L, 1);
  if (b) {
    uint32_t size = b->getDataSize();
    TRACE("luaDestroyBitmap: %p (%u)", b, size);
    if (luaExtraMemoryUsage >= size) {
      luaExtraMemoryUsage -= size;
    }
    else {
      luaExtraMemoryUsage = 0;
    }
    delete b;
  }
  return 0;
}


/*luadoc
@function lcd.drawBitmap(bitmap, x, y [, scale])

Displays a bitmap at (x,y)

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@param x,y (positive numbers) starting coordinates

@param scale (positive numbers) scale in %, 50 divides size by two, 100 is unchanged, 200 doubles size.
Omitting scale draws image in 1:1 scale and is faster than specifying 100 for scale.

@notice Only available on Horus

@status current Introduced in 2.2.0
*/
static int luaLcdDrawBitmap(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  const BitmapBuffer * b = checkBitmap(L, 1);

  if (b) {
    unsigned int x = luaL_checkunsigned(L, 2);
    unsigned int y = luaL_checkunsigned(L, 3);
    unsigned int scale = luaL_optunsigned(L, 4, 0);
    if (scale) {
      luaLcdBuffer->drawBitmap(x, y, b, 0, 0, 0, 0, scale/100.0f);
    }
    else {
      luaLcdBuffer->drawBitmap(x, y, b);
    }
  }

  return 0;
}


/*luadoc
@function lcd.drawBitmapPattern(bitmap, x, y [, flags])

Displays a bitmap pattern at (x,y)

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@param x,y (positive numbers) starting coordinates

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@notice Only available on Horus

@status current Introduced in 2.8.0
*/
static int luaLcdDrawBitmapPattern(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  const char* m = luaL_checkstring(L, 1);

  if (m) {
    auto x = luaL_checkunsigned(L, 2);
    auto y = luaL_checkunsigned(L, 3);
    auto flags = luaL_optunsigned(L, 4, 0);
    flags = colorToRGB(flags);
    luaLcdBuffer->drawBitmapPattern(x, y, reinterpret_cast<const MaskBitmap*>(m), flags);
  }

  return 0;
}

/*luadoc
@function lcd.drawBitmapPatternPie(bitmap, x, y, startAngle, endAngle [, flags])

Displays a bitmap pattern pie at (x,y)

@param bitmap (pointer) point to a bitmap previously opened with Bitmap.open()

@param x,y (positive numbers) starting coordinates

@param startAngle Start angle

@param endAngle End angle

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@notice Only available on Horus

@status current Introduced in 2.8.0
*/
static int luaLcdDrawBitmapPatternPie(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  const char* m = luaL_checkstring(L, 1);

  if (m) {
    auto x = luaL_checkunsigned(L, 2);
    auto y = luaL_checkunsigned(L, 3);
    auto startAngle = luaL_checkinteger(L, 4);
    auto endAngle = luaL_checkinteger(L, 5);
    auto flags = luaL_optunsigned(L, 6, 0);
    flags = colorToRGB(flags);
    luaLcdBuffer->drawBitmapPatternPie(x, y, reinterpret_cast<const MaskBitmap*>(m), flags, startAngle, endAngle);
  }

  return 0;
}

/*luadoc
@function lcd.drawRectangle(x, y, w, h [, flags [, t [, opacity]]])

Draw a rectangle from top left corner (x,y) of specified width and height

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@param t (number) thickness in pixels, defaults to 1 (only on Horus)

@param opacity (number) opacity defaults to 0 (only on Horus)

@status current Introduced in 2.0.0, changed in 2.2.0
*/
static int luaLcdDrawRectangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer) return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  LcdFlags flags = luaL_optunsigned(L, 5, 0);
  flags = colorToRGB(flags);
  unsigned int t = luaL_optunsigned(L, 6, 1);
  uint8_t opacity = luaL_optunsigned(L, 7, 0) & 0x0F;

  luaLcdBuffer->drawRect(x, y, w, h, t, SOLID, flags, opacity);

  return 0;
}

/*luadoc
@function lcd.drawFilledRectangle(x, y, w, h [, flags [, opacity]])

Draw a solid rectangle from top left corner (x,y) of specified width and height

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@param opacity (number) opacity defaults to 0 (only on Horus)

@status current Introduced in 2.0.0
*/
static int luaLcdDrawFilledRectangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  LcdFlags flags = luaL_optunsigned(L, 5, 0);
  flags = colorToRGB(flags);
  uint8_t opacity = luaL_optunsigned(L, 6, 0) & 0x0F;
  
  luaLcdBuffer->drawFilledRect(x, y, w, h, SOLID, flags, opacity);

  return 0;
}


/*luadoc
@function lcd.invertRect(x, y, w, h [, flags])

Invert a rectangle zone from top left corner (x,y) of specified width and height

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.8.0
*/
static int luaLcdInvertRect(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  LcdFlags flags = luaL_optunsigned(L, 5, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->invertRect(x, y, w, h, flags);

  return 0;
}

/*luadoc
@function lcd.drawGauge(x, y, w, h, fill, maxfill [, flags])

Draw a simple gauge that is filled based upon fill value

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param fill (number) amount of fill to apply

@param maxfill (number) total value of fill

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.0.0, changed in 2.2.0
*/
static int luaLcdDrawGauge(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  int num = luaL_checkinteger(L, 5);
  int den = luaL_checkinteger(L, 6);
  LcdFlags flags = luaL_optunsigned(L, 7, 0);
  flags = colorToRGB(flags);
  
  luaLcdBuffer->drawRect(x, y, w, h, 1, 0xff, flags);
  uint16_t len = limit((uint16_t)1, uint16_t(w*num/den), uint16_t(w));
  luaLcdBuffer->drawSolidFilledRect(x+1, y+1, len, h-2, flags);

  return 0;
}

/*luadoc
@function lcd.setColor(colorIndex, color)

Change an indexed color (theme colors and CUSTOM_COLOR). Please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html#color-constants)

Please notice that changing theme colors affects not only other Lua widgets, but the entire radio interface.

@notice Only available on radios with color display

@status current Introduced in 2.2.0
*/
static int luaLcdSetColor(lua_State *L)
{
  unsigned int index = COLOR_VAL(luaL_checkunsigned(L, 1));
  uint16_t color = COLOR_VAL(colorToRGB(luaL_checkunsigned(L, 2)));

  if (index < LCD_COLOR_COUNT && lcdColorTable[index] != color) {
    lcdColorTable[index] = color;
    styles->applyColors();
  }
  return 0;
}

/*luadoc
@function lcd.getColor(flags)

Get the color value from flags

@param flags (flags) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@retval color (flag) only the RGB565 color value of the input

@notice Only available on radios with color display

@status current Introduced in 2.3.11
*/

static int luaLcdGetColor(lua_State *L)
{
  LcdFlags flags = luaL_checkunsigned(L, 1);
  if ((flags & RGB_FLAG) || (COLOR_VAL(flags) & 0xFF) < LCD_COLOR_COUNT)
    lua_pushunsigned(L, colorToRGB(flags) & (COLOR_MASK(~0u) | RGB_FLAG));
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function lcd.RGB(r, g, b | rgb)

Returns a drawing flag with RGB color code

@param r (integer) a number between 0 and 255 that expresses the amount of red in the color

@param g (integer) a number between 0 and 255 that expresses the amount of green in the color

@param b (integer) a number between 0 and 255 that expresses the amount of blue in the color

@param rgb (integer) a number between 0 and 0xFFFFFF that expresses the RGB value (0xFF000=RED, 0x00FF00=GREEN, 0x0000FF=BLUE)

@retval flag with RGB565 color

@notice Only available on radios with color display. Use *either* lcd.RGB(r,g,b) *or* lcd.RGB(rgb)

@status current Introduced in 2.2.0
*/
static int luaRGB(lua_State *L)
{
  int r, g, b;
  if (lua_gettop(L) == 1) {
    int color = luaL_checkinteger(L, 1);
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
  } else {
    r = luaL_checkinteger(L, 1);
    g = luaL_checkinteger(L, 2);
    b = luaL_checkinteger(L, 3);
  }
  lua_pushinteger(L, COLOR2FLAGS(RGB(r, g, b)) | RGB_FLAG);
  return 1;
}

/*luadoc
@function lcd.drawCircle(x, y, r [, flags])

Draw a circle at (x, y) of specified radius

@param x,y (positive numbers) center position

@param r (number) radius in pixels

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawCircle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x = luaL_checkunsigned(L, 1);
  coord_t y = luaL_checkunsigned(L, 2);
  coord_t r = luaL_checkunsigned(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->drawCircle(x, y, r, flags);

  return 0;
}

/*luadoc
@function lcd.drawFilledCircle(x, y, r [, flags])

Draw a filled circle at (x, y) of specified radius

@param x,y (positive numbers) center position

@param r (number) radius in pixels

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawFilledCircle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x = luaL_checkunsigned(L, 1);
  coord_t y = luaL_checkunsigned(L, 2);
  coord_t r = luaL_checkunsigned(L, 3);
  LcdFlags flags = luaL_optunsigned(L, 4, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->drawFilledCircle(x, y, r, flags);

  return 0;
}

/*luadoc
@function lcd.drawTriangle(x1, y1, x2, y2, x3, y3 [, flags])

Draw a triangle

@param x1,y1,x2,y2,x3,y3 (positive numbers) coordinates of the three vertices

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawTriangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x1 = luaL_checkunsigned(L, 1);
  coord_t y1 = luaL_checkunsigned(L, 2);
  coord_t x2 = luaL_checkunsigned(L, 3);
  coord_t y2 = luaL_checkunsigned(L, 4);
  coord_t x3 = luaL_checkunsigned(L, 5);
  coord_t y3 = luaL_checkunsigned(L, 6);
  LcdFlags flags = luaL_optunsigned(L, 7, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->drawLine(x1, y1, x2, y2, SOLID, flags);
  luaLcdBuffer->drawLine(x2, y2, x3, y3, SOLID, flags);
  luaLcdBuffer->drawLine(x3, y3, x1, y1, SOLID, flags);

  return 0;
}

/*luadoc
@function lcd.drawFilledTriangle(x1, y1, x2, y2, x3, y3 [, flags])

Draw a filled triangle

@param x1,y1,x2,y2,x3,y3 (positive numbers) coordinates of the three vertices

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawFilledTriangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x1 = luaL_checkunsigned(L, 1);
  coord_t y1 = luaL_checkunsigned(L, 2);
  coord_t x2 = luaL_checkunsigned(L, 3);
  coord_t y2 = luaL_checkunsigned(L, 4);
  coord_t x3 = luaL_checkunsigned(L, 5);
  coord_t y3 = luaL_checkunsigned(L, 6);
  LcdFlags flags = luaL_optunsigned(L, 7, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->drawFilledTriangle(x1, y1, x2, y2, x3, y3, flags);

  return 0;
}

/*luadoc
@function lcd.drawArc(x, y, r, start, end [, flags])

Draw an arc

@param x,y (positive numbers) coordinates of the center

@param r (positive number) radius

@param start,end (positive numbers) start and end of the arc

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawArc(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x = luaL_checkunsigned(L, 1);
  coord_t y = luaL_checkunsigned(L, 2);
  coord_t r = luaL_checkunsigned(L, 3);
  int start = luaL_checkunsigned(L, 4);
  int end = luaL_checkunsigned(L, 5);
  LcdFlags flags = luaL_optunsigned(L, 6, 0);
  flags = colorToRGB(flags);

  if (r > 0)
    luaLcdBuffer->drawAnnulusSector(x, y, r - 1, r, start, end, flags);

  return 0;
}

/*luadoc
@function lcd.drawPie(x, y, r, start, end [, flags])

Draw a pie slice

@param x,y (positive numbers) coordinates of the center

@param r (positive number) radius

@param start,end (positive numbers) start and end of the pie slice

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawPie(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x = luaL_checkunsigned(L, 1);
  coord_t y = luaL_checkunsigned(L, 2);
  coord_t r = luaL_checkunsigned(L, 3);
  int start = luaL_checkunsigned(L, 4);
  int end = luaL_checkunsigned(L, 5);
  LcdFlags flags = luaL_optunsigned(L, 6, 0);
  flags = colorToRGB(flags);

  if (r > 0)
    luaLcdBuffer->drawAnnulusSector(x, y, 0, r, start, end, flags);

  return 0;
}

/*luadoc
@function lcd.drawAnnulus(x, y, r1, r2, start, end [, flags])

Draw an arc

@param x,y (positive numbers) coordinates of the center

@param r1,r2 (positive numbers) radii of the inside and outside of the annulus

@param start,end (positive numbers) start and end of the annulus

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawAnnulus(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x = luaL_checkunsigned(L, 1);
  coord_t y = luaL_checkunsigned(L, 2);
  coord_t r1 = luaL_checkunsigned(L, 3);
  coord_t r2 = luaL_checkunsigned(L, 4);
  int start = luaL_checkunsigned(L, 5);
  int end = luaL_checkunsigned(L, 6);
  LcdFlags flags = luaL_optunsigned(L, 7, 0);
  flags = colorToRGB(flags);

  luaLcdBuffer->drawAnnulusSector(x, y, r1, r2, start, end, flags);

  return 0;
}

/*luadoc
@function lcd.drawLineWithClipping(x1, y1, x2, y2, xmin, xmax, ymin, ymax, pattern [, flags])

Draw a line only inside a rectangle

@param x1,y1,x2,y1 (positive numbers) coordinates of the start and end of the unclipped line

@param xmin,xmax,ymin,ymax (positive numbers) the limits of the rectangle inside which the line is drawn

@param pattern (FORCE, ERASE, DOTTED) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawLineWithClipping(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  coord_t x1 = luaL_checkunsigned(L, 1);
  coord_t y1 = luaL_checkunsigned(L, 2);
  coord_t x2 = luaL_checkunsigned(L, 3);
  coord_t y2 = luaL_checkunsigned(L, 4);
  coord_t xmin = luaL_checkunsigned(L, 5);
  coord_t xmax = luaL_checkunsigned(L, 6);
  coord_t ymin = luaL_checkunsigned(L, 7);
  coord_t ymax = luaL_checkunsigned(L, 8);
  uint8_t pat = luaL_checkunsigned(L, 9);
  LcdFlags flags = luaL_optunsigned(L, 10, 0);
  flags = colorToRGB(flags);

  // backup clipping rect
  coord_t dc_xmin; coord_t dc_xmax; coord_t dc_ymin; coord_t dc_ymax;
  luaLcdBuffer->getClippingRect(dc_xmin, dc_xmax, dc_ymin, dc_ymax);

  // restricts given clipping rect by drawing clipping rect
  if (dc_xmin > xmin) xmin = dc_xmin;
  if (dc_xmax < xmax) xmax = dc_xmax;
  if (dc_ymin > ymin) ymin = dc_ymin;
  if (dc_ymax < ymax) ymax = dc_ymax;
  
  luaLcdBuffer->setClippingRect(xmin, xmax, ymin, ymax);
  luaLcdBuffer->drawLine(x1, y1, x2, y2, pat, flags);

  // restore original clipping rect
  luaLcdBuffer->setClippingRect(dc_xmin, dc_xmax, dc_ymin, dc_ymax);
  
  return 0;
}

static void drawHudRectangle(BitmapBuffer * dc, float pitch, float roll, coord_t xmin, coord_t xmax, coord_t ymin, coord_t ymax, LcdFlags flags)
{
  constexpr float GRADTORAD = 0.017453293f;

  float dx = sinf(GRADTORAD*roll) * pitch;
  float dy = cosf(GRADTORAD*roll) * pitch * 1.85f;
  float angle = tanf(-GRADTORAD*roll);
  float ox = 0.5f * (xmin + xmax) + dx;
  float oy = 0.5f * (ymin + ymax) + dy;
  coord_t ywidth = (ymax - ymin);

  if (roll == 0.0f) { // prevent divide by zero
    dc->drawSolidFilledRect(
        xmin, max(ymin, ymin + (ywidth/2 + (coord_t)dy)),
        xmax - xmin, max(0,min(ywidth, ywidth/2 - (coord_t)dy)), flags);
  }
  else if (fabs(roll) >= 180.0f) {
    dc->drawSolidFilledRect(xmin, ymin, xmax - xmin, min(ywidth, ywidth/2 + (coord_t)fabsf(dy)), flags);
  }
  else {
    bool inverted = (fabsf(roll) > 90.0f);
    bool fillNeeded = false;
    coord_t ybot = (inverted) ? 0 : LCD_H;

    if (roll > 0.0f) {
      for (coord_t s = 0; s < ywidth; s++) {
        coord_t yy = ymin + s;
        coord_t xx = ox + ((float)yy - oy) / angle; // + 0.5f; rounding not needed
        if (xx >= xmin && xx <= xmax) {
          dc->drawHorizontalLine(xx, yy, xmax - xx + 1, SOLID, flags);
        }
        else if (xx < xmin) {
          ybot = (inverted) ? max(yy, ybot) + 1 : min(yy, ybot);
          fillNeeded = true;
        }
      }
    }
    else {
      for (coord_t s = 0; s < ywidth; s++) {
        coord_t yy = ymin + s;
        coord_t xx = ox + ((float)yy - oy) / angle; // + 0.5f; rounding not needed
        if (xx >= xmin && xx <= xmax) {
          dc->drawHorizontalLine(xmin, yy, xx - xmin, SOLID, flags);
        }
        else if (xx > xmax) {
          ybot = (inverted) ? max(yy, ybot) + 1 : min(yy, ybot);
          fillNeeded = true;
        }
      }
    }

    if (fillNeeded) {
      coord_t ytop = (inverted) ? ymin : ybot;
      coord_t height = (inverted) ? ybot - ymin : ymax - ybot;
      dc->drawSolidFilledRect(xmin, ytop, xmax - xmin, height, flags);
    }
  }
}

/*luadoc
@function lcd.drawHudRectangle(pitch, roll, xmin, xmax, ymin, ymax [, flags])

Draw a rectangle in perspective

@param pitch,roll (positive numbers) pitch and roll to define the orientation of the rectangle

@param xmin,xmax,ymin,ymax (positive numbers) the limits of the rectangle

@param flags (optional) please see [Lcd functions overview](../lcd-functions-less-than-greater-than-luadoc-begin-lcd/lcd_functions-overview.html)

@status current Introduced in 2.4.0
*/
static int luaLcdDrawHudRectangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  float pitch = luaL_checknumber(L, 1);
  float roll = luaL_checknumber(L, 2);
  coord_t xmin = luaL_checkunsigned(L, 3);
  coord_t xmax = luaL_checkunsigned(L, 4);
  coord_t ymin = luaL_checkunsigned(L, 5);
  coord_t ymax = luaL_checkunsigned(L, 6);
  LcdFlags flags = luaL_optunsigned(L, 7, 0);
  flags = colorToRGB(flags);

  drawHudRectangle(luaLcdBuffer, pitch, roll, xmin, xmax, ymin, ymax, flags);

  return 0;
}

/*luadoc
@function lcd.exitFullScreen()

Exit full screen widget mode.

@notice Only available on radios with color display

@status current Introduced in 2.5.0
*/
static int luaLcdExitFullScreen(lua_State *L)
{
  if (runningFS)
    runningFS->closeFullscreen();
  return 0;
}

LROT_BEGIN(lcdlib, NULL, 0)
  LROT_FUNCENTRY( refresh, luaLcdRefresh )
  LROT_FUNCENTRY( clear, luaLcdClear )
  LROT_FUNCENTRY( resetBacklightTimeout, luaLcdResetBacklightTimeout )
  LROT_FUNCENTRY( drawPoint, luaLcdDrawPoint )
  LROT_FUNCENTRY( drawLine, luaLcdDrawLine )
  LROT_FUNCENTRY( drawRectangle, luaLcdDrawRectangle )
  LROT_FUNCENTRY( drawFilledRectangle, luaLcdDrawFilledRectangle )
  LROT_FUNCENTRY( invertRect, luaLcdInvertRect )
  LROT_FUNCENTRY( drawText, luaLcdDrawText )
  LROT_FUNCENTRY( drawTextLines, luaLcdDrawTextLines )
  LROT_FUNCENTRY( sizeText, luaLcdSizeText )
  LROT_FUNCENTRY( drawTimer, luaLcdDrawTimer )
  LROT_FUNCENTRY( drawNumber, luaLcdDrawNumber )
  LROT_FUNCENTRY( drawChannel, luaLcdDrawChannel )
  LROT_FUNCENTRY( drawSwitch, luaLcdDrawSwitch )
  LROT_FUNCENTRY( drawSource, luaLcdDrawSource )
  LROT_FUNCENTRY( drawGauge, luaLcdDrawGauge )
  LROT_FUNCENTRY( drawBitmap, luaLcdDrawBitmap )
  LROT_FUNCENTRY( drawBitmapPattern, luaLcdDrawBitmapPattern )
  LROT_FUNCENTRY( drawBitmapPatternPie, luaLcdDrawBitmapPatternPie )
  LROT_FUNCENTRY( setColor, luaLcdSetColor )
  LROT_FUNCENTRY( getColor, luaLcdGetColor )
  LROT_FUNCENTRY( RGB, luaRGB )
  LROT_FUNCENTRY( drawCircle, luaLcdDrawCircle )
  LROT_FUNCENTRY( drawFilledCircle, luaLcdDrawFilledCircle )
  LROT_FUNCENTRY( drawTriangle, luaLcdDrawTriangle )
  LROT_FUNCENTRY( drawFilledTriangle, luaLcdDrawFilledTriangle )
  LROT_FUNCENTRY( drawArc, luaLcdDrawArc )
  LROT_FUNCENTRY( drawPie, luaLcdDrawPie )
  LROT_FUNCENTRY( drawAnnulus, luaLcdDrawAnnulus )
  LROT_FUNCENTRY( drawLineWithClipping, luaLcdDrawLineWithClipping )
  LROT_FUNCENTRY( drawHudRectangle, luaLcdDrawHudRectangle )
  LROT_FUNCENTRY( exitFullScreen, luaLcdExitFullScreen )
LROT_END(lcdlib, NULL, 0)

LROT_BEGIN(bitmap_mt, NULL, LROT_MASK_GC)
  LROT_FUNCENTRY( __gc, luaDestroyBitmap )
  LROT_FUNCENTRY( getSize, luaGetBitmapSize )
  LROT_FUNCENTRY( resize, luaBitmapResize )
  LROT_FUNCENTRY( toMask, luaBitmapTo8bitMask )
LROT_END(bitmap_mt, NULL, LROT_MASK_GC)

LROT_BEGIN(bitmaplib, NULL, 0)
  LROT_FUNCENTRY( open, luaOpenBitmap )
  LROT_FUNCENTRY( getSize, luaGetBitmapSize )
  LROT_FUNCENTRY( resize, luaBitmapResize )
  LROT_FUNCENTRY( toMask, luaBitmapTo8bitMask )
LROT_END(bitmaplib, NULL, 0)

extern "C" {
  LUALIB_API int luaopen_bitmap(lua_State * L) {
    luaL_rometatable( L, BITMAP_METATABLE,  LROT_TABLEREF(bitmap_mt));
    return 0;
  }
}
