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

#ifndef _LCD_H_
#define _LCD_H_

#include "bitmapbuffer.h"
#include "opentx_types.h"
#include "libopenui_globals.h"

#if LCD_W >= 480
  #define LCD_COLS                     40
#else
  #define LCD_COLS                     30
#endif

#define CENTER

#include "colors.h"

#define DISPLAY_PIXELS_COUNT           (LCD_W * LCD_H)
#define DISPLAY_BUFFER_SIZE            (DISPLAY_PIXELS_COUNT)

extern coord_t lcdNextPos;
void lcdNextLayer();

inline void lcdClear()
{
  lcd->clear();
}

void lcdDrawBlackOverlay();

#if defined(BOOT)
  #define BLINK_ON_PHASE               (0)
#else
  #define BLINK_ON_PHASE               (g_blinkTmr10ms & (1<<6))
  #define SLOW_BLINK_ON_PHASE          (g_blinkTmr10ms & (1<<7))
#endif

#endif // _LCD_H_
