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

// Timings for the 480x272 RGB panel, taken from the T18 (STM32F4) target.
// These are already expressed as LTDC register values (accumulated, 0-based),
// so they are programmed without further adjustment.
#define HBP  ( 43 )
#define VBP  ( 12 )

#define HSW  ( 2 )
#define VSH  ( 4 )

#define HFP  ( 8 )
#define VFP  ( 8 )

typedef void (*lcdSpiInitFucPtr)(void);

extern lcdSpiInitFucPtr lcdInitFunction;
extern lcdSpiInitFucPtr lcdOffFunction;
extern lcdSpiInitFucPtr lcdOnFunction;

#define LCD_NRST_HIGH()               LL_GPIO_SetOutputPin(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN)
#define LCD_NRST_LOW()                LL_GPIO_ResetOutputPin(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN)
