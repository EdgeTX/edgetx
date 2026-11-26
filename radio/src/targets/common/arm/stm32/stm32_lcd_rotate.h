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

#include "lcd_rotate.h"

// Software rotation for a portrait LTDC panel (see lcd_rotate.h), with the
// DMA2D as copy engine and the LTDC layer 1 showing the frame buffers.

// Clears the frame buffers and returns the one LTDC layer 1 starts with
// (LCD_H pixels wide, LCD_W lines high)
pixel_t* lcdRotateInit(LcdRotation rotation);

// LVGL flush callback, see lcdSetFlushCb()
void lcdRotateFlush(lv_disp_drv_t* disp_drv, uint16_t* buffer,
                    const rect_t& area);
