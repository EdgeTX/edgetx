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

#include "edgetx_types.h"

#include "colors.h"

#define DISPLAY_PIXELS_COUNT           (LCD_W * LCD_H)
#define DISPLAY_BUFFER_SIZE            (DISPLAY_PIXELS_COUNT)

#if defined(BOOT)
  #define BLINK_ON_PHASE               (0)
#else
  #define BLINK_ON_PHASE               (g_blinkTmr10ms & (1<<6))
  #define SLOW_BLINK_ON_PHASE          (g_blinkTmr10ms & (1<<7))
#endif

struct _lv_disp_drv_t;
typedef _lv_disp_drv_t lv_disp_drv_t;

// Call backs
void lcdSetWaitCb(void (*cb)(lv_disp_drv_t *));
void lcdSetFlushCb(void (*cb)(lv_disp_drv_t *, uint16_t*, const rect_t&));

// Init LVGL and its display driver
void lcdInitDisplayDriver();

void lcdClear();

// Patch the draw context to allow for direct drawing
void lcdInitDirectDrawing();

void lcdRefresh();

void lcdFlushed();
