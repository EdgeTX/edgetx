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

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "edgetx_types.h"
#include "dma2d.h"
#include "hal.h"
#include "delays_driver.h"
#include "debug.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "board.h"

#include "stm32_gpio.h"
#include "hal/gpio.h"
#include "stm32_spi.h"

static volatile uint16_t lcd_phys_w = LCD_PHYS_W;
static volatile uint16_t lcd_phys_h = LCD_PHYS_H;

static stm32_spi_t lcdSpi;

//static void* initialFrameBuffer = nullptr;
//
//static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
//  while(_frame_addr_reloaded == 0);
}

lcdSpiInitFucPtr lcdInitFunction;
lcdSpiInitFucPtr lcdOffFunction;
lcdSpiInitFucPtr lcdOnFunction;
uint32_t lcdPixelClock;

static void LCD_AF_GPIOConfig(void) {

}

static void lcdSpiConfig(void) {
  lcdSpi.CS = LCD_SPI_CS;
  lcdSpi.SPIx = LCD_SPI;
  lcdSpi.MISO = LCD_SPI_MISO;
  lcdSpi.MOSI = LCD_SPI_MOSI;
  lcdSpi.SCK = LCD_SPI_CLK;
  lcdSpi.DMA = LCD_SPI_DMA;
#if 0
  lcdSpi.rxDMA_Stream = LCD_SPI_RX_DMA_STREAM;
  lcdSpi.txDMA_Stream = LCD_SPI_TX_DMA_STREAM;
  lcdSpi.rxDMA_PeriphRequest = LCD_SPI_RX_DMA;
  lcdSpi.txDMA_PeriphRequest = LCD_SPI_TX_DMA;
#endif
  stm32_spi_init(&lcdSpi, 8);
  stm32_spi_set_max_baudrate(&lcdSpi, LCD_SPI_BAUD);

//#define LCD_FMARK                       GPIO_PIN(GPIOB,  7)

  gpio_init(LCD_NRST, GPIO_OUT, GPIO_PIN_SPEED_HIGH);

//
//  GPIO_InitStructure.Pin        = LCD_NRST_GPIO_PIN;
//  LL_GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);
}

void lcdDelay() {
  delay_01us(1);
}

static void lcdReset() {
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(100);

  LCD_NRST_HIGH();
  delay_ms(100);
}

unsigned char LCD_ReadRegister(unsigned char Register) {
  unsigned char ReadData = 0;

/*  lcdWriteByte(0, Register);
  lcdDelay();
  lcdDelay();
  ReadData = LCD_ReadByte();*/
  return (ReadData);
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
 // initialFrameBuffer = fbAddress;
}

extern "C"
void lcdInit()
{


  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

//  lcdInitFunction();

  lcdSetFlushCb(startLcdRefresh);
}

