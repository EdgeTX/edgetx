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

#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__

// H17 uses a parallel RGB (LTDC) panel with no MCU-side SPI configuration bus.
// Only the LTDC data/sync lines and the panel reset line (LCD_RESET_GPIO) are
// wired, so the SPI bit-bang init/read path used by the RadioMaster boards is
// intentionally absent here.

// Timings taken from the horus/tx16s driver, which drives this same panel.
// Values are in this board's raw-porch convention (lcd_driver.cpp subtracts 1
// and accumulates), and reproduce the exact LTDC register values horus programs:
//   HSync=2 AccHBP=42 AccActiveW=522 TotalWidth=525
//   VSync=10 AccVBP=12 AccActiveH=284 TotalHeigh=286
#define HBP  ( 40 )
#define VBP  ( 2 )

#define HSW  ( 3 )
#define VSH  ( 11 )

#define HFP  ( 3 )
#define VFP  ( 2 )

#define LCD_NRST_HIGH()               LL_GPIO_SetOutputPin(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN)
#define LCD_NRST_LOW()                LL_GPIO_ResetOutputPin(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN)

#endif
