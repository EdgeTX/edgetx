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

#define HBP  42
  #define VBP  12

  #define HSW  2
  #define VSW  10

  #define HFP  3
  #define VFP  2

#define LCD_DELAY()         LCD_Delay()

typedef void (*lcdSpiInitFucPtr)(void);
typedef unsigned int  LcdReadIDFucPtr( void );

extern lcdSpiInitFucPtr lcdInitFunction;
extern lcdSpiInitFucPtr lcdOffFunction;
extern lcdSpiInitFucPtr lcdOnFunction;

#define LCD_NRST_HIGH()                      LL_GPIO_SetOutputPin(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN)
#define LCD_NRST_LOW()                       LL_GPIO_ResetOutputPin(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN)

#endif