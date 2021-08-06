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

#ifndef _COLORS_H_
#define _COLORS_H_

#include "libopenui_defines.h"


// TODO common code, not in target
enum LcdColorIndex
{
  COLOR_THEME_PRIMARY1_INDEX,
  COLOR_THEME_PRIMARY2_INDEX,
  COLOR_THEME_PRIMARY3_INDEX,
  COLOR_THEME_SECONDARY1_INDEX,
  COLOR_THEME_SECONDARY2_INDEX,
  COLOR_THEME_SECONDARY3_INDEX,
  COLOR_THEME_FOCUS_INDEX,
  COLOR_THEME_EDIT_INDEX,
  COLOR_THEME_ACTIVE_INDEX,
  COLOR_THEME_WARNING_INDEX,
  COLOR_THEME_DISABLED_INDEX,
  CUSTOM_COLOR_INDEX,

  // this one MUST be last
  LCD_COLOR_COUNT,
};


//#define COLOR_THEME_PRIMARY2_INDEX     COLOR_THEME_ACTIVE_INDEX
//#define COLOR_THEME_SECONDARY2_INDEX          COLOR_THEME_PRIMARY3_INDEX


//
// Basic color definitions
// 
#define WHITE                          RGB(0xFF, 0xFF, 0xFF)
#define LIGHTWHITE                     RGB(238, 234, 238)
#define BLACK                          RGB(0, 0, 0)
#define YELLOW                         RGB(0xF0, 0xD0, 0x10)
#define BLUE                           RGB(0x30, 0xA0, 0xE0)
#define DARKBLUE                       RGB(0, 0x35, 0x67)
#define GREY                           RGB(96, 96, 96)
#define DARKGREY                       RGB(64, 64, 64)
#define LIGHTGREY                      RGB(180, 180, 180)
#define RED                            RGB(229, 32, 30)
#define DARKRED                        RGB(160, 0, 6)
#define GREEN                          RGB(25, 150, 50)
#define LIGHTBROWN                     RGB(156, 109, 32)
#define DARKBROWN                      RGB(106, 72, 16)
#define BRIGHTGREEN                    RGB(0, 180, 60)
#define ORANGE                         RGB(229, 100, 30)

//
// Indexed colors
//
extern uint16_t lcdColorTable[LCD_COLOR_COUNT];

inline void lcdSetColor(uint16_t color)
{
  lcdColorTable[CUSTOM_COLOR_INDEX] = color;
}

#define COLOR(index) ((uint32_t)lcdColorTable[ unsigned(index) & 0xFF ] << 16u)

#define COLOR_THEME_PRIMARY1              COLOR(COLOR_THEME_PRIMARY1_INDEX)
#define COLOR_THEME_PRIMARY2              COLOR(COLOR_THEME_PRIMARY2_INDEX)
#define COLOR_THEME_PRIMARY3              COLOR(COLOR_THEME_PRIMARY3_INDEX)
#define COLOR_THEME_SECONDARY1            COLOR(COLOR_THEME_SECONDARY1_INDEX)
#define COLOR_THEME_SECONDARY2            COLOR(COLOR_THEME_SECONDARY2_INDEX)
#define COLOR_THEME_SECONDARY3            COLOR(COLOR_THEME_SECONDARY3_INDEX)
#define COLOR_THEME_FOCUS                 COLOR(COLOR_THEME_FOCUS_INDEX)
#define COLOR_THEME_EDIT                  COLOR(COLOR_THEME_EDIT_INDEX)
#define COLOR_THEME_ACTIVE                COLOR(COLOR_THEME_ACTIVE_INDEX)
#define COLOR_THEME_WARNING               COLOR(COLOR_THEME_WARNING_INDEX)
#define COLOR_THEME_DISABLED              COLOR(COLOR_THEME_DISABLED_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)

#if defined(WINDOWS_INSPECT_BORDERS)
  #define WINDOWS_INSPECT_BORDER_COLOR COLOR_THEME_PRIMARY3
#endif

#endif // _COLORS_H_
