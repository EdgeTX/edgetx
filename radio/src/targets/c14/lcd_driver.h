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

#include "bsp_io.h"

#define HBP  ( 24 )
#define VBP  ( 10 )
#define HSW  ( 4 )
#define VSH  ( 2 )
#define HFP  ( 140 - HBP )
#define VFP  ( 22 - VBP )
#define VSW  10

#define LCD_ST7796S_ID      ( 0x7796 )
#define LCD_ILI9481_ID      ( 0x9481 )
#define LCD_ILI9486_ID      ( 0x9486 )
#define LCD_ILI9488_ID      ( 0x9488 )
#define LCD_HX8357D_ID      ( 0x99 )

#define LCD_DELAY()         LCD_Delay()

typedef void (*lcdSpiInitFucPtr)(void);
typedef unsigned int  LcdReadIDFucPtr( void );

extern lcdSpiInitFucPtr lcdInitFunction;
extern lcdSpiInitFucPtr lcdOffFunction;
extern lcdSpiInitFucPtr lcdOnFunction;

#define SET_IO_INPUT( PORT, PIN )            LL_GPIO_SetPinMode( PORT, PIN, LL_GPIO_MODE_INPUT )
#define SET_IO_OUTPUT( PORT, PIN )           LL_GPIO_SetPinMode( PORT, PIN, LL_GPIO_MODE_OUTPUT )

#define HORIZONTAL_SYNC_WIDTH    ( 4 )
#define HORIZONTAL_BACK_PORCH    ( 20 )
#define HORIZONTAL_FRONT_PORCH   ( 60 - HORIZONTAL_BACK_PORCH )
#define VERTICAL_SYNC_HEIGHT     ( 2 )
#define VERTICAL_BACK_PORCH      ( 6 )
#define VERTICAL_FRONT_PORCH     ( 14 - VERTICAL_BACK_PORCH )
