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

#ifndef _COLORS_H_
#define _COLORS_H_

#include "libopenui_defines.h"

// TODO common code, not in target
enum LcdColorIndex {
  // this one first for when colour is not set
  DEFAULT_COLOR_INDEX,

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

#define ARGB_SPLIT(color, a, r, g, b) \
  uint16_t a = ((color) & 0xF000) >> 12; \
  uint16_t r = ((color) & 0x0F00) >> 8; \
  uint16_t g = ((color) & 0x00F0) >> 4; \
  uint16_t b = ((color) & 0x000F)

#define RGB_SPLIT(color, r, g, b) \
  uint16_t r = ((color) & 0xF800) >> 11; \
  uint16_t g = ((color) & 0x07E0) >> 5; \
  uint16_t b = ((color) & 0x001F)

#define ARGB_JOIN(a, r, g, b) \
  (((a&0xF) << 12) + ((r&0xF) << 8) + ((g&0xF) << 4) + (b&0xF))

#define RGB_JOIN(r, g, b) \
  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED(color) \
  (((color) & 0xF800) >> 8)

#define GET_GREEN(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) \
  (((color) & 0x001F) << 3)

#define OPACITY_MAX                    0x0Fu
#define OPACITY(value)                 ((value) & OPACITY_MAX)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR2FLAGS(color)             LcdFlags(unsigned(color) << 16u)
#define COLOR_VAL(flags)               ((flags) >> 16u)
#define COLOR_MASK(flags)              ((flags) & 0xFFFF0000u)

//
// Basic color definitions
//

#define WHITE RGB(0xFF, 0xFF, 0xFF)
#define BLACK RGB(0, 0, 0)

#define RGB_FLAG 0x8000u
#define RGB2FLAGS(r, g, b) (COLOR2FLAGS(RGB(r, g, b)) | RGB_FLAG)

//
// Indexed colors
//
extern uint16_t lcdColorTable[LCD_COLOR_COUNT];

inline void lcdSetColor(uint16_t color)
{
  lcdColorTable[CUSTOM_COLOR_INDEX] = color;
}

#define COLOR(index)                                                         \
  ((uint32_t)                                                                \
       lcdColorTable[unsigned(index & 0xFF) >= LCD_COLOR_COUNT               \
                         ? DEFAULT_COLOR_INDEX                               \
                         : ((index & 0xFF) == 0 ? COLOR_THEME_PRIMARY1_INDEX \
                                                : unsigned(index & 0xFF))]   \
   << 16u)

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
#define CUSTOM_COLOR                      COLOR(CUSTOM_COLOR_INDEX)

#if defined(WINDOWS_INSPECT_BORDERS)
#define WINDOWS_INSPECT_BORDER_COLOR COLOR_THEME_PRIMARY3
#endif

constexpr int MAX_SATURATION = 100;
constexpr int MAX_HUE = 360;
constexpr int MAX_BRIGHTNESS = 100;

extern uint32_t HSVtoRGB(float H, float S, float V);
extern void RGBtoHSV(uint8_t R, uint8_t G, uint8_t B, float& fH, float& fS,
                     float& fV);

#endif  // _COLORS_H_
