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

#include "board.h"

#define HBP  ( 24 ) // TODO use names from FlySky
#define VBP  ( 10 )

#define HSW  ( 4 )
#define VSH  ( 2 )

#define HFP  ( 140 - HBP )
#define VFP  ( 22 - VBP )


#define PORT_LCD_CS         ( GPIOE )
#define LCD_CS_PIN          ( GPIO_Pin_4 )
#define PIN_LCD_CS          ( 4 )

#define PORT_LCD_CLK        ( GPIOE )
#define LCD_CLK_PIN         ( GPIO_Pin_2 )
#define PIN_LCD_CLK         ( 2 )

#define PORT_LCD_MOSI       ( GPIOE )
#define LCD_MOSI_PIN        ( GPIO_Pin_6 )
#define PIN_LCD_MOSI        ( 6 )

#define PORT_LCD_MISO       ( GPIOE )
#define LCD_MISO_PIN        ( GPIO_Pin_5 )
#define PIN_LCD_MISO        ( 5 )

#define PORT_LCD_DE         ( GPIOK )
#define LCD_DE_PIN          ( GPIO_Pin_7 )
#define PIN_LCD_DE          ( 7 )

#define PORT_LCD_RESET      ( GPIOG )
#define LCD_RESET_PIN       ( GPIO_Pin_9 )
#define PIN_LCD_RESET       ( 9 )

#define PORT_LCD_HSYNC      ( GPIOI )
#define LCD_HSYNC_PIN       ( GPIO_Pin_12 )
#define PIN_LCD_HSYNC       ( 12 )

#define PORT_LCD_VSYNC      ( GPIOI )
#define LCD_VSYNC_PIN       ( GPIO_Pin_13 )
#define PIN_LCD_VSYNC       ( 13 )

#define PORT_LCD_DOTCLK     ( GPIOG )
#define LCD_DOTCLK_PIN      ( GPIO_Pin_7 )
#define PIN_LCD_DOTCLK      ( 7 )

#define SUPPORTED_LCD_CNT   ( 5 )

#define LCD_ST7796S_ID      ( 0x7796 )
#define LCD_ILI9481_ID      ( 0x9481 )
#define LCD_ILI9486_ID      ( 0x9486 )
#define LCD_ILI9488_ID      ( 0x9488 )
#define LCD_HX8357D_ID      ( 0x99 )

#define LCD_DELAY()         LCD_Delay()

typedef void (*lcdSpiInitFucPtr)(void);
typedef unsigned int  LcdReadIDFucPtr( void );

extern void GPIO_SetDirection( GPIO_TypeDef *GPIOx, unsigned char Pin, unsigned char IsInput );

extern lcdSpiInitFucPtr lcdInitFunction;
extern lcdSpiInitFucPtr lcdOffFunction;
extern lcdSpiInitFucPtr lcdOnFunction;

#define SET_IO_INPUT( PORT, PIN )            GPIO_SetDirection( PORT, PIN, 1 )
#define SET_IO_OUTPUT( PORT, PIN )           GPIO_SetDirection( PORT, PIN, 0 )

#define LCD_NRST_HIGH()               GPIO_WriteBit(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN, Bit_SET)
#define LCD_NRST_LOW()                GPIO_WriteBit(LCD_NRST_GPIO, LCD_NRST_GPIO_PIN, Bit_RESET)

#define LCD_CS_HIGH()                 GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_CS_GPIO_PIN, Bit_SET)
#define LCD_CS_LOW()                  GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_CS_GPIO_PIN, Bit_RESET)

#define LCD_SCK_HIGH()                GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_SCK_GPIO_PIN, Bit_SET)
#define LCD_SCK_LOW()                 GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_SCK_GPIO_PIN, Bit_RESET)

#define LCD_MOSI_HIGH()               GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_MOSI_GPIO_PIN, Bit_SET)
#define LCD_MOSI_LOW()                GPIO_WriteBit(LCD_SPI_GPIO, LCD_SPI_MOSI_GPIO_PIN, Bit_RESET)

#define SET_LCD_CS()                  GPIO_WriteBit(PORT_LCD_CS, LCD_CS_PIN, Bit_SET)
#define CLR_LCD_CS()                  GPIO_WriteBit(PORT_LCD_CS, LCD_CS_PIN, Bit_RESET)
#define SET_LCD_CS_OUTPUT()           SET_IO_OUTPUT( PORT_LCD_CS, PIN_LCD_CS )

#define SET_LCD_CLK()                 GPIO_WriteBit( PORT_LCD_CLK, LCD_CLK_PIN, Bit_SET )
#define CLR_LCD_CLK()                 GPIO_WriteBit( PORT_LCD_CLK, LCD_CLK_PIN, Bit_RESET )
#define SET_LCD_CLK_OUTPUT()          SET_IO_OUTPUT( PORT_LCD_CLK, PIN_LCD_CLK )

#define SET_LCD_DATA()                GPIO_WriteBit( PORT_LCD_MOSI, LCD_MOSI_PIN, Bit_SET )
#define CLR_LCD_DATA()                GPIO_WriteBit( PORT_LCD_MOSI, LCD_MOSI_PIN, Bit_RESET )
#define SET_LCD_DATA_INPUT()          SET_IO_INPUT( PORT_LCD_MOSI, PIN_LCD_MOSI )
#define SET_LCD_DATA_OUTPUT()         SET_IO_OUTPUT( PORT_LCD_MOSI, PIN_LCD_MOSI )

#define READ_LCD_DATA_PIN()           GPIO_ReadInputDataBit(PORT_LCD_MOSI, LCD_MOSI_PIN)



#if 1
#define HORIZONTAL_SYNC_WIDTH 			       ( 4 )
#define HORIZONTAL_BACK_PORCH		               ( 24 )
#define HORIZONTAL_FRONT_PORCH                         ( 140 - HORIZONTAL_BACK_PORCH )
#define VERTICAL_SYNC_HEIGHT   		               ( 2 )
#define VERTICAL_BACK_PORCH  		               ( 10 )
#define VERTICAL_FRONT_PORCH    	               ( 22 - VERTICAL_BACK_PORCH )
#else
#define HORIZONTAL_SYNC_WIDTH 			       ( 4 )
#define HORIZONTAL_BACK_PORCH		               ( 20 )
#define HORIZONTAL_FRONT_PORCH                         ( 60 - HORIZONTAL_BACK_PORCH )
#define VERTICAL_SYNC_HEIGHT   		               ( 2 )
#define VERTICAL_BACK_PORCH  		               ( 6 )
#define VERTICAL_FRONT_PORCH    	               ( 14 - VERTICAL_BACK_PORCH )
#endif



#endif




