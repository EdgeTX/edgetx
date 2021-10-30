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
  // this one first for when colour is not set
  DEFAULT_COLOR_INDEX,

  ALARM_COLOR_INDEX,
  BARGRAPH1_COLOR_INDEX,
  BARGRAPH2_COLOR_INDEX,
  BARGRAPH_BGCOLOR_INDEX,
  BATTERY_CHARGE_COLOR_INDEX,
  CHECKBOX_COLOR_INDEX,
  CURVE_AXIS_COLOR_INDEX,
  CURVE_COLOR_INDEX,
  CURVE_CURSOR_COLOR_INDEX,
  CUSTOM_COLOR_INDEX,
  DEFAULT_BGCOLOR_INDEX,
  DISABLE_COLOR_INDEX,
  EDIT_MARKER_COLOR_INDEX,
  FIELD_BGCOLOR_INDEX,
  FIELD_FRAME_COLOR_INDEX,
  FOCUS_BGCOLOR_INDEX,
  FOCUS_COLOR_INDEX,
  HEADER_BGCOLOR_INDEX,
  HEADER_COLOR_INDEX,
  HEADER_CURRENT_BGCOLOR_INDEX,
  HEADER_ICON_BGCOLOR_INDEX,
  HIGHLIGHT_COLOR_INDEX,
  LINE_COLOR_INDEX,
  MENU_HIGHLIGHT_BGCOLOR_INDEX,
  MAINVIEW_GRAPHICS_COLOR_INDEX,
  MAINVIEW_PANES_COLOR_INDEX,
  MENU_BGCOLOR_INDEX,
  MENU_COLOR_INDEX,
  MENU_HIGHLIGHT_COLOR_INDEX,
  MENU_LINE_COLOR_INDEX,
  MENU_TITLE_DISABLE_COLOR_INDEX,
  OVERLAY_COLOR_INDEX,
  SCROLLBAR_COLOR_INDEX,
  SCROLLBOX_COLOR_INDEX,
  TABLE_BGCOLOR_INDEX,
  TABLE_HEADER_BGCOLOR_INDEX,
  TEXT_DISABLE_COLOR_INDEX,
  TEXT_STATUSBAR_COLOR_INDEX,
  TITLE_BGCOLOR_INDEX,
  TRIM_BGCOLOR_INDEX,
  TRIM_SHADOW_COLOR_INDEX,

  // this one MUST be last
  LCD_COLOR_COUNT,
};


//#define MENU_HIGHLIGHT_COLOR_INDEX     HIGHLIGHT_COLOR_INDEX
//#define MENU_LINE_COLOR_INDEX          LINE_COLOR_INDEX


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

#define ALARM_COLOR                    COLOR(ALARM_COLOR_INDEX)
#define BARGRAPH1_COLOR                COLOR(BARGRAPH1_COLOR_INDEX)
#define BARGRAPH2_COLOR                COLOR(BARGRAPH2_COLOR_INDEX)
#define BARGRAPH_BGCOLOR               COLOR(BARGRAPH_BGCOLOR_INDEX)
#define BATTERY_CHARGE_COLOR           COLOR(BATTERY_CHARGE_COLOR_INDEX)
#define CHECKBOX_COLOR                 COLOR(CHECKBOX_COLOR_INDEX)
#define CURVE_AXIS_COLOR               COLOR(CURVE_AXIS_COLOR_INDEX)
#define CURVE_COLOR                    COLOR(CURVE_COLOR_INDEX)
#define CURVE_CURSOR_COLOR             COLOR(CURVE_CURSOR_COLOR_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)
#define DEFAULT_BGCOLOR                COLOR(DEFAULT_BGCOLOR_INDEX)
#define DEFAULT_COLOR                  COLOR(DEFAULT_COLOR_INDEX)
#define DISABLE_COLOR                  COLOR(DISABLE_COLOR_INDEX)
#define EDIT_MARKER_COLOR              COLOR(EDIT_MARKER_COLOR_INDEX)
#define FIELD_BGCOLOR                  COLOR(FIELD_BGCOLOR_INDEX)
#define FIELD_FRAME_COLOR              COLOR(FIELD_FRAME_COLOR_INDEX)
#define FOCUS_BGCOLOR                  COLOR(FOCUS_BGCOLOR_INDEX)
#define FOCUS_COLOR                    COLOR(FOCUS_COLOR_INDEX)
#define HEADER_BGCOLOR                 COLOR(HEADER_BGCOLOR_INDEX)
#define HEADER_COLOR                   COLOR(HEADER_COLOR_INDEX)
#define HEADER_CURRENT_BGCOLOR         COLOR(HEADER_CURRENT_BGCOLOR_INDEX)
#define HEADER_ICON_BGCOLOR            COLOR(HEADER_ICON_BGCOLOR_INDEX)
//#define HEADER_SEPARATOR_COLOR         COLOR(HEADER_SEPARATOR_COLOR_INDEX)
#define HIGHLIGHT_COLOR                COLOR(HIGHLIGHT_COLOR_INDEX)
#define LINE_COLOR                     COLOR(LINE_COLOR_INDEX)
#define MAINVIEW_GRAPHICS_COLOR        COLOR(MAINVIEW_GRAPHICS_COLOR_INDEX)
#define MAINVIEW_PANES_COLOR           COLOR(MAINVIEW_PANES_COLOR_INDEX)
#define MENU_BGCOLOR                   COLOR(MENU_BGCOLOR_INDEX)
#define MENU_COLOR                     COLOR(MENU_COLOR_INDEX)
#define MENU_HIGHLIGHT_BGCOLOR         COLOR(MENU_HIGHLIGHT_BGCOLOR_INDEX)
#define MENU_HIGHLIGHT_COLOR           COLOR(MENU_HIGHLIGHT_COLOR_INDEX)
#define MENU_LINE_COLOR                COLOR(MENU_LINE_COLOR_INDEX)
//#define MENU_TITLE_BGCOLOR             COLOR(MENU_TITLE_BGCOLOR_INDEX)
#define MENU_TITLE_DISABLE_COLOR       COLOR(MENU_TITLE_DISABLE_COLOR_INDEX)
#define OVERLAY_COLOR                  COLOR(OVERLAY_COLOR_INDEX)
#define SCROLLBAR_COLOR                COLOR(SCROLLBAR_COLOR_INDEX)
#define TABLE_BGCOLOR                  COLOR(TABLE_BGCOLOR_INDEX)
#define TABLE_HEADER_BGCOLOR           COLOR(TABLE_HEADER_BGCOLOR_INDEX)
#define TEXT_DISABLE_COLOR             COLOR(TEXT_DISABLE_COLOR_INDEX)
#define TEXT_STATUSBAR_COLOR           COLOR(TEXT_STATUSBAR_COLOR_INDEX)
#define TITLE_BGCOLOR                  COLOR(TITLE_BGCOLOR_INDEX)
#define TRIM_BGCOLOR                   COLOR(TRIM_BGCOLOR_INDEX)
#define TRIM_SHADOW_COLOR              COLOR(TRIM_SHADOW_COLOR_INDEX)

#if defined(WINDOWS_INSPECT_BORDERS)
  #define WINDOWS_INSPECT_BORDER_COLOR LINE_COLOR
#endif

#endif // _COLORS_H_
