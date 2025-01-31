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

  uint8_t buf[4];


  LCD_COMMAND_MODE();

  buf[0]=copy_area.x>>8;
  buf[1]=copy_area.x&0xff;
  coord_t x2 = copy_area.x + copy_area.w-1;
  buf[2]=x2>>8;
  buf[3]=x2&0xff;

  stm32_spi_select(&lcdSpi);
  delay_ms(1);

  stm32_spi_transfer_byte(&lcdSpi, 0x2A);
  LCD_DATA_MODE();
  stm32_spi_transfer_bytes(&lcdSpi, buf, nullptr, 4);

  stm32_spi_unselect(&lcdSpi);
  LCD_COMMAND_MODE();

  buf[0]=copy_area.y>>8;
  buf[1]=copy_area.y&0xff;
  coord_t y2 = copy_area.y + copy_area.y-1;
  buf[2]=y2>>8;
  buf[3]=y2&0xff;
  stm32_spi_select(&lcdSpi);

  stm32_spi_transfer_byte(&lcdSpi, 0x2B);
  LCD_DATA_MODE();
  stm32_spi_transfer_bytes(&lcdSpi, buf, nullptr, 4);

  stm32_spi_unselect(&lcdSpi);

  LCD_COMMAND_MODE();

  stm32_spi_select(&lcdSpi);

  stm32_spi_transfer_byte(&lcdSpi, 0x2C);
  //
  ////  stm32_spi_unselect(&lcdSpi);
  LCD_DATA_MODE();
  SCB_CleanDCache_by_Addr(buffer, copy_area.w * copy_area.h * sizeof(uint16_t));

  stm32_spi_set_data_width(&lcdSpi, LL_SPI_DATAWIDTH_16BIT);

  for(int i=0; i<copy_area.w * copy_area.h; ++i)
  {
    stm32_spi_transfer_word(&lcdSpi, buffer[i]);
  }
//  size_t len = copy_area.w * copy_area.h;
//  while(len)
//  {
//    size_t transferLen  = std::min(len, (size_t)256);
//    stm32_spi_dma_transmit_bytes(&lcdSpi, (uint8_t*)buffer, transferLen*2);
//    len -= transferLen;
//  }
  stm32_spi_unselect(&lcdSpi);
  stm32_spi_set_data_width(&lcdSpi, LL_SPI_DATAWIDTH_8BIT);


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
  lcdSpi.DMA = nullptr;
#if 1
 lcdSpi.DMA = LCD_SPI_DMA;
  lcdSpi.rxDMA_Stream = LCD_SPI_RX_DMA_STREAM;
  lcdSpi.txDMA_Stream = LCD_SPI_TX_DMA_STREAM;
  lcdSpi.rxDMA_PeriphRequest = LCD_SPI_RX_DMA;
  lcdSpi.txDMA_PeriphRequest = LCD_SPI_TX_DMA;
#endif
  stm32_spi_init(&lcdSpi, LL_SPI_DATAWIDTH_8BIT);
  stm32_spi_set_max_baudrate(&lcdSpi, LCD_SPI_BAUD);

//#define LCD_FMARK                       GPIO_PIN(GPIOB,  7)
  gpio_init(LCD_NRST, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
  gpio_init(LCD_SPI_RS, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
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
  delay_ms(120);
}

static void lcdWriteCommand(uint8_t cmd)
{
  LCD_COMMAND_MODE();
  stm32_spi_select(&lcdSpi);
  stm32_spi_transfer_byte(&lcdSpi, cmd);
  stm32_spi_unselect(&lcdSpi);
}

static void lcdWriteReg(uint8_t reg, uint8_t data) {
  LCD_COMMAND_MODE();
  stm32_spi_select(&lcdSpi);
  stm32_spi_transfer_byte(&lcdSpi, reg);
  LCD_DATA_MODE();
  stm32_spi_transfer_byte(&lcdSpi, data);
  stm32_spi_unselect(&lcdSpi);
}

static void lcdWriteData(uint8_t data) {
  LCD_DATA_MODE();
  stm32_spi_select(&lcdSpi);
  stm32_spi_transfer_byte(&lcdSpi, data);
  stm32_spi_unselect(&lcdSpi);
}

static uint8_t lcdReadReg(uint8_t reg) {
  uint8_t data = 0;
  LCD_COMMAND_MODE();
  stm32_spi_select(&lcdSpi);
  stm32_spi_transfer_byte(&lcdSpi, reg);
  LCD_DATA_MODE();
  data = stm32_spi_transfer_byte(&lcdSpi, 0xFF);
  stm32_spi_unselect(&lcdSpi);
  return data;
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
 // initialFrameBuffer = fbAddress;
}

static void lcdSetOn(void)
{
  lcdWriteCommand(0x29);
}

static void lcdSetOff(void)
{
  lcdWriteCommand(0x28);
}

static uint8_t outBuf[48000];
extern pixel_t LCD_FIRST_FRAME_BUFFER[];

extern "C"
void lcdInit()
{


  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();
  stm32_spi_unselect(&lcdSpi);

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  lcdWriteCommand(0x11); // sleep out
  delay_ms(10);
  lcdWriteCommand(0x13); // normal mode

  lcdWriteReg(0x3A, 0x55);

  lcdWriteCommand( 0x36 );
  lcdWriteData( 0xE8 );

  lcdWriteCommand( 0x2A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0x40 );
  lcdWriteCommand( 0x2B );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0xF0 );

  lcdWriteCommand( 0x20 );

  lcdSetOn();

//memset(LCD_FIRST_FRAME_BUFFER, DWT->CYCCNT, DISPLAY_BUFFER_SIZE*2);
//LCD_COMMAND_MODE();
//
//stm32_spi_select(&lcdSpi);
//delay_ms(1);
//
//stm32_spi_transfer_byte(&lcdSpi, 0x2C);
//delay_ms(1);
////
//////  stm32_spi_unselect(&lcdSpi);
//LCD_DATA_MODE();
//stm32_spi_transfer_bytes(&lcdSpi, (uint8_t*)LCD_FIRST_FRAME_BUFFER, nullptr, DISPLAY_BUFFER_SIZE*2);
////stm32_spi_dma_transmit_bytes(&lcdSpi, (uint8_t*)LCD_FIRST_FRAME_BUFFER, DISPLAY_BUFFER_SIZE*2);
//delay_ms(1);
////
//stm32_spi_unselect(&lcdSpi);
LCD_COMMAND_MODE();


//  lcdInitFunction();
  lcdOnFunction = lcdSetOn;
  lcdOffFunction = lcdSetOff;

  lcdSetFlushCb(startLcdRefresh);
}

