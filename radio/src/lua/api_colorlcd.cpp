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

#include <cctype>
#include <cstdio>
#include "opentx.h"
#include "lua_api.h"
#include "libopenui.h"

#include "api_colorlcd.h"

BitmapBuffer* luaLcdBuffer = nullptr;


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

@notice This function only works in stand-alone and telemetry scripts.
*/
static int luaLcdClear(lua_State * L)
{
  if (luaLcdAllowed && luaLcdBuffer) {
    LcdFlags color = luaL_optunsigned(L, 1, DEFAULT_BGCOLOR);
    luaLcdBuffer->clear(color);
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
@function lcd.drawPoint(x, y)

Draw a single pixel at (x,y) position

@param x (positive number) x position

@param y (positive number) y position

@param flags (optional) lcdflags

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
  LcdFlags att = luaL_optunsigned(L, 3, 0);

  // here another definition should be used
  luaLcdBuffer->drawPixel(x, y, COLOR_VAL(att));

  return 0;
}

/*luadoc
@function lcd.drawLine(x1, y1, x2, y2, pattern, flags)

Draw a straight line on LCD

@param x1,y1 (positive numbers) starting coordinate

@param x2,y2 (positive numbers) end coordinate

@param pattern SOLID or DOTTED

@param flags lcdflags

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
  LcdFlags flags = luaL_checkunsigned(L, 6);

  if (x1 > LCD_W || y1 > LCD_H || x2 > LCD_W || y2 > LCD_H)
    return 0;

  uint16_t color_idx = COLOR_VAL(flags) & 0xFF;
  flags = COLOR(color_idx);

  if (pat == SOLID) {
    if (x1 == x2) {
      luaLcdBuffer->drawSolidVerticalLine(
          x1, y1 < y2 ? y1 : y2, y1 < y2 ? (y2 - y1) + 1 : (y1 - y2) + 1,
          flags);
      return 0;
    } else if (y1 == y2) {
      luaLcdBuffer->drawSolidHorizontalLine(
          x1 < x2 ? x1 : x2, y1, x1 < x2 ? (x2 - x1) + 1 : (x1 - x2) + 1,
          flags);
      return 0;
    }
  }

  luaLcdBuffer->drawLine(x1, y1, x2, y2, pat, flags);

  return 0;
}

/*luadoc
@function lcd.drawText(x, y, text [, flags])

Draw a text beginning at (x,y)

@param x,y (positive numbers) starting coordinate

@param text (string) text to display

@param flags (unsigned number) drawing flags. All values can be
combined together using the + character. ie BLINK + DBLSIZE.
See the [Appendix](../appendix/fonts.md) for available characters in each font set.
 * `0 or not specified` normal font
 * `XXLSIZE` jumbo sized font
 * `DBLSIZE` double size font
 * `MIDSIZE` mid sized font
 * `SMLSIZE` small font
 * `INVERS` inverted display
 * `BLINK` blinking text
 * `SHADOWED` Horus only, apply a shadow effect

@status current Introduced in 2.0.0, `SHADOWED` introduced in 2.2.1
*/
static int luaLcdDrawText(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  const char * s = luaL_checkstring(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);

  bool invers = att & INVERS;
  bool blink = att & INVERS;
  bool shadowed = att & SHADOWED;

  // from here on, we need only the color
  att = (att & 0xFFFF) | COLOR(COLOR_VAL(att));
  
  if (shadowed && !invers) {
    // force black
    luaLcdBuffer->drawText(x+1, y+1, s, att & 0xFFFF);
  }

  if (invers) {
    int height = getFontHeight(att & 0xFFFF);
    luaLcdBuffer->drawSolidFilledRect(x - INVERT_BOX_MARGIN,
                                      y - INVERT_BOX_MARGIN,
                                      end_pos - x + 2 * INVERT_BOX_MARGIN,
                                      height + 2 * INVERT_BOX_MARGIN,
                                      );
  }

  coord_t end_pos = luaLcdBuffer->drawText(x, y, s, att);

  return 0;
}

/*luadoc
@function lcd.drawTimer(x, y, value [, flags])

Display a value formatted as time at (x,y)

@param x,y (positive numbers) starting coordinate

@param value (number) time in seconds

@param flags (unsigned number) drawing flags:
 * `0 or not specified` normal representation (minutes and seconds)
 * `TIMEHOUR` display hours
 * other general LCD flag also apply
 * `SHADOWED` Horus only, apply a shadow effect

@status current Introduced in 2.0.0,  `SHADOWED` introduced in 2.2.1
*/
static int luaLcdDrawTimer(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

#if 0 // TODO
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int seconds = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);

  if (att & SHADOWED) {
    luaLcdBuffer->drawTimer(x+1, y+1, seconds, (att & 0xFFFF) | LEFT);
  }
  luaLcdBuffer->drawTimer(x, y, seconds, att | LEFT);
#endif

  return 0;
}

/*luadoc
@function lcd.drawNumber(x, y, value [, flags])

Display a number at (x,y)

@param x,y (positive numbers) starting coordinate

@param value (number) value to display

@param flags (unsigned number) drawing flags:
 * `0 or not specified` display with no decimal (like abs())
 * `PREC1` display with one decimal place (number 386 is displayed as 38.6)
 * `PREC2` display with tow decimal places (number 386 is displayed as 3.86)
 * other general LCD flag also apply
 * `SHADOWED` Horus only, apply a shadow effect

@status current Introduced in 2.0.0,  `SHADOWED` introduced in 2.2.1
*/
static int luaLcdDrawNumber(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer)
    return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int val = luaL_checkinteger(L, 3);

  unsigned int att = luaL_optunsigned(L, 4, 0);
  att = (att & 0xFFFF) | COLOR(COLOR_VAL(att));

  if (att & INVERS)
  
  if ((att & SHADOWED) && !(att & INVERS)) {
    luaLcdBuffer->drawNumber(x, y, val, att & 0xFFFF);
  }
  luaLcdBuffer->drawNumber(x, y, val, att);

  return 0;
}

/*luadoc
@function lcd.drawChannel(x, y, source, flags)

Display a telemetry value at (x,y)

@param x,y (positive numbers) starting coordinate

@param source can be a source identifier (number) or a source name (string).
See getValue()

@param flags (unsigned number) drawing flags

@status current Introduced in 2.0.6, changed in 2.1.0 (only telemetry sources are valid)
*/
static int luaLcdDrawChannel(lua_State *L)
{
#if 0
  if (!luaLcdAllowed)
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
  unsigned int att = luaL_optunsigned(L, 4, 0);
  getvalue_t value = getValue(channel);
  drawSensorCustomValue(x, y, (channel-MIXSRC_FIRST_TELEM)/3, value, att);
#endif
  return 0;
}

/*luadoc
@function lcd.drawSwitch(x, y, switch, flags)

Draw a text representation of switch at (x,y)

@param x,y (positive numbers) starting coordinate

@param switch (number) number of switch to display, negative number
displays negated switch

@param flags (unsigned number) drawing flags, only SMLSIZE, BLINK and INVERS.

@status current Introduced in 2.0.0
*/
static int luaLcdDrawSwitch(lua_State *L)
{
#if 0
  if (!luaLcdAllowed)
    return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  drawSwitch(x, y, s, att);
#endif
  return 0;
}

/*luadoc
@function lcd.drawSource(x, y, source [, flags])

Displays the name of the corresponding input as defined by the source at (x,y)

@param x,y (positive numbers) starting coordinate

@param source (number) source index

@param flags (unsigned number) drawing flags

@status current Introduced in 2.0.0
*/
static int luaLcdDrawSource(lua_State *L)
{
#if 0
  if (!luaLcdAllowed)
    return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  drawSource(x, y, s, att);
#endif
  return 0;
}

#define LUA_BITMAPHANDLE          "BITMAP*"

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
static int luaOpenBitmap(lua_State * L)
{
  const char * filename = luaL_checkstring(L, 1);

  BitmapBuffer ** b = (BitmapBuffer **)lua_newuserdata(L, sizeof(BitmapBuffer *));

  if (luaExtraMemoryUsage > LUA_MEM_EXTRA_MAX) {
    // already allocated more than max allowed, fail
    TRACE("luaOpenBitmap: Error, using too much memory %u/%u", luaExtraMemoryUsage, LUA_MEM_EXTRA_MAX);
    *b = 0;
  }
  else {
    *b = BitmapBuffer::loadBitmap(filename);
    if (*b == NULL && G(L)->gcrunning) {
      luaC_fullgc(L, 1);  /* try to free some memory... */
      *b = BitmapBuffer::loadBitmap(filename);  /* try again */
    }
  }

  if (*b) {
    uint32_t size = (*b)->getDataSize();
    luaExtraMemoryUsage += size;
    TRACE("luaOpenBitmap: %p (%u)", *b, size);
  }

  luaL_getmetatable(L, LUA_BITMAPHANDLE);
  lua_setmetatable(L, -2);

  return 1;
}

static BitmapBuffer * checkBitmap(lua_State * L, int index)
{
  BitmapBuffer ** b = (BitmapBuffer **)luaL_checkudata(L, index, LUA_BITMAPHANDLE);
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

const luaL_Reg bitmapFuncs[] = {
  { "open", luaOpenBitmap },
  { "getSize", luaGetBitmapSize },
  { "__gc", luaDestroyBitmap },
  { NULL, NULL }
};

void registerBitmapClass(lua_State * L)
{
  luaL_newmetatable(L, LUA_BITMAPHANDLE);
  luaL_setfuncs(L, bitmapFuncs, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_setglobal(L, "Bitmap");
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
@function lcd.drawRectangle(x, y, w, h [, flags [, t]])

Draw a rectangle from top left corner (x,y) of specified width and height

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param flags (unsigned number) drawing flags

@param t (number) thickness in pixels, defaults to 1 (only on Horus)

@status current Introduced in 2.0.0, changed in 2.2.0
*/
static int luaLcdDrawRectangle(lua_State *L)
{
  if (!luaLcdAllowed || !luaLcdBuffer) return 0;

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  unsigned int flags = luaL_optunsigned(L, 5, 0);

  uint8_t opacity = (flags >> 24) & 0x0F;
  flags = (flags & 0xFFFF) | COLOR(COLOR_VAL(flags));

  unsigned int t = luaL_optunsigned(L, 6, 1);
  luaLcdBuffer->drawRect(x, y, w, h, t, SOLID, flags, opacity);

  return 0;
}

/*luadoc
@function lcd.drawFilledRectangle(x, y, w, h [, flags])

Draw a solid rectangle from top left corner (x,y) of specified width and height

@param x,y (positive numbers) top left corner position

@param w (number) width in pixels

@param h (number) height in pixels

@param flags (unsigned number) drawing flags

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

  unsigned int flags = luaL_optunsigned(L, 5, 0);

  uint8_t opacity = (flags >> 24) & 0x0F;
  flags = (flags & 0xFFFF) | COLOR(COLOR_VAL(flags));
  
  luaLcdBuffer->drawFilledRect(x, y, w, h, SOLID, flags, opacity);

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

@param flags (unsigned number) drawing flags

@status current Introduced in 2.0.0, changed in 2.2.0
*/
static int luaLcdDrawGauge(lua_State *L)
{
#if 0
  if (!luaLcdAllowed)
    return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  int num = luaL_checkinteger(L, 5);
  int den = luaL_checkinteger(L, 6);
  unsigned int flags = luaL_optunsigned(L, 7, 0);
#if defined(PCBHORUS)
  lcdDrawRect(x, y, w, h, 1, 0xff, flags);
#else
  lcdDrawRect(x, y, w, h, 0xff, flags);
#endif
  uint8_t len = limit((uint8_t)1, uint8_t(w*num/den), uint8_t(w));
  lcdDrawSolidFilledRect(x+1, y+1, len, h-2, flags);
#endif
  return 0;
}

/*luadoc
@function lcd.setColor(area, color)

Set a color for specific area

@param area (unsigned number) specific screen area in the list bellow
 * `TEXT_COLOR`
 * `TEXT_BGCOLOR`
 * `TEXT_INVERTED_COLOR`
 * `TEXT_INVERTED_BGCOLOR`
 * `LINE_COLOR`
 * `SCROLLBOX_COLOR`
 * `MENU_TITLE_BGCOLOR`
 * `MENU_TITLE_COLOR`
 * `MENU_TITLE_DISABLE_COLOR`
 * `HEADER_COLOR`
 * `ALARM_COLOR`
 * `WARNING_COLOR`
 * `TEXT_DISABLE_COLOR`
 * `CURVE_AXIS_COLOR`
 * `CURVE_COLOR`
 * `CURVE_CURSOR_COLOR`
 * `TITLE_BGCOLOR`
 * `TRIM_BGCOLOR`
 * `TRIM_SHADOW_COLOR`
 * `HEADER_BGCOLOR`
 * `HEADER_ICON_BGCOLOR`
 * `HEADER_CURRENT_BGCOLOR`
 * `MAINVIEW_PANES_COLOR`
 * `MAINVIEW_GRAPHICS_COLOR`
 * `OVERLAY_COLOR`
 * `BARGRAPH1_COLOR`
 * `BARGRAPH2_COLOR`
 * `BARGRAPH_BGCOLOR`
 * `CUSTOM_COLOR`

@param color (number) color in 5/6/5 rgb format. The following prefined colors are available
 * `WHITE`
 * `GREY`
 * `LIGHTGREY`
 * `DARKGREY`
 * `BLACK`
 * `YELLOW`
 * `BLUE`
 * `RED`
 * `DARKRED`

@notice Only available on Colorlcd radios

@status current Introduced in 2.2.0
*/
static int luaLcdSetColor(lua_State *L)
{
  if (!luaLcdAllowed)
    return 0;
  unsigned int index = luaL_checkunsigned(L, 1) >> 16;
  unsigned int color = luaL_checkunsigned(L, 2);
  lcdColorTable[index] = color;

  return 0;
}

/*luadoc
@function lcd.getColor(area)

Get the color for specific area : see lcd.setColor for area list

@notice Only available on Colorlcd radios

@status current Introduced in 2.3.11
*/

static int luaLcdGetColor(lua_State *L)
{
#if 0
  if (!luaLcdAllowed)
    return 0;

  unsigned int index = luaL_checkunsigned(L, 1) >> 16;
  lua_pushunsigned(L, lcdColorTable[index]);
#endif
  return 1;
}

/*luadoc
@function lcd.RGB(r, g, b)

Returns a 5/6/5 rgb color code, that can be used with lcd.setColor

@param r (integer) a number between 0x00 and 0xff that expresses te amount of red in the color

@param g (integer) a number between 0x00 and 0xff that expresses te amount of green in the color

@param b (integer) a number between 0x00 and 0xff that expresses te amount of blue in the color

@retval number (integer) rgb color expressed in 5/6/5 format

@notice Only available on Colorlcd radios

@status current Introduced in 2.2.0
*/
static int luaRGB(lua_State *L)
{
  if (!luaLcdAllowed)
    return 0;

  int r = luaL_checkinteger(L, 1);
  int g = luaL_checkinteger(L, 2);
  int b = luaL_checkinteger(L, 3);
  lua_pushinteger(L, RGB(r, g, b));

  return 1;
}

const luaL_Reg lcdLib[] = {
  { "refresh", luaLcdRefresh },
  { "clear", luaLcdClear },
  { "resetBacklightTimeout", luaLcdResetBacklightTimeout },
  { "drawPoint", luaLcdDrawPoint },
  { "drawLine", luaLcdDrawLine },
  { "drawRectangle", luaLcdDrawRectangle },
  { "drawFilledRectangle", luaLcdDrawFilledRectangle },
  { "drawText", luaLcdDrawText },
  { "drawTimer", luaLcdDrawTimer },
  { "drawNumber", luaLcdDrawNumber },
  { "drawChannel", luaLcdDrawChannel },
  { "drawSwitch", luaLcdDrawSwitch },
  { "drawSource", luaLcdDrawSource },
  { "drawGauge", luaLcdDrawGauge },
#if defined(COLORLCD)
  { "drawBitmap", luaLcdDrawBitmap },
  { "setColor", luaLcdSetColor },
  { "getColor", luaLcdGetColor },
  { "RGB", luaRGB },
#else
  { "getLastPos", luaLcdGetLastPos },
  { "getLastRightPos", luaLcdGetLastPos },
  { "getLastLeftPos", luaLcdGetLeftPos },
  { "drawPixmap", luaLcdDrawPixmap },
  { "drawScreenTitle", luaLcdDrawScreenTitle },
  { "drawCombobox", luaLcdDrawCombobox },
#endif
  { NULL, NULL }  /* sentinel */
};
