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

#include "lcd_driver.h"

#include "stm32_gpio.h"
#include "stm32_spi.h"

#include "delays_driver.h"
#include "hal/gpio.h"
#include "lcd.h"

#include "hal.h"
#include "board.h"

#define LCD_NRST_HIGH()               gpio_set(LCD_NRST)
#define LCD_NRST_LOW()                gpio_clear(LCD_NRST)
#define LCD_COMMAND_MODE()            gpio_clear(LCD_SPI_RS)
#define LCD_DATA_MODE()               gpio_set(LCD_SPI_RS)

#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C

static const stm32_spi_t lcdSpi = {
  .SPIx = LCD_SPI,
  .SCK = LCD_SPI_CLK,
  .MISO = LCD_SPI_MISO,
  .MOSI = LCD_SPI_MOSI,
  .CS = LCD_SPI_CS,
  .DMA = LCD_SPI_DMA,
  .txDMA_PeriphRequest = LCD_SPI_TX_DMA,
  .rxDMA_PeriphRequest = LCD_SPI_RX_DMA,
  .txDMA_Stream = LCD_SPI_TX_DMA_STREAM,
  .rxDMA_Stream = LCD_SPI_RX_DMA_STREAM,
};

static void* initialFrameBuffer = nullptr;

static void write_start_end(uint8_t cmd, uint16_t start, uint16_t end)
{
  // big endian format
  uint8_t buf[4] = {
      (uint8_t)(start >> 8),
      (uint8_t)(start & 0xFF),
      (uint8_t)(end >> 8),
      (uint8_t)(end & 0xFF),
  };

  stm32_spi_select(&lcdSpi);

  LCD_COMMAND_MODE();
  stm32_spi_transfer_byte(&lcdSpi, cmd);

  LCD_DATA_MODE();
  stm32_spi_transfer_bytes(&lcdSpi, (uint8_t *)buf, nullptr, sizeof(buf));

  stm32_spi_unselect(&lcdSpi);
}

static inline void set_column_addr(uint16_t xs, uint16_t xe)
{
  write_start_end(CASET, xs, xe);
}

static inline void set_row_addr(uint16_t ys, uint16_t ye)
{
  write_start_end(RASET, ys, ye);
}

static void memory_write(const uint16_t* data, uint32_t length)
{
  stm32_spi_select(&lcdSpi);

  LCD_COMMAND_MODE();
  stm32_spi_transfer_byte(&lcdSpi, RAMWR);

  LCD_DATA_MODE();
  stm32_spi_set_data_width(&lcdSpi, LL_SPI_DATAWIDTH_16BIT);

  SCB_CleanDCache_by_Addr((void *)data, length * sizeof(uint16_t));
  stm32_spi_dma_transmit_words(&lcdSpi, data, length);

  stm32_spi_unselect(&lcdSpi);
  stm32_spi_set_data_width(&lcdSpi, LL_SPI_DATAWIDTH_8BIT);
}

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{  
  (void)disp_drv;

  // TODO: replace through some smarter mechanism without busy wait
  while (!gpio_read(LCD_FMARK));

  coord_t x1 = copy_area.x;
  coord_t x2 = x1 + copy_area.w - 1;
  set_column_addr(x1, x2);

  coord_t y1 = copy_area.y;
  coord_t y2 = y1 + copy_area.h - 1;
  set_row_addr(y1, y2);

  memory_write(buffer, copy_area.w * copy_area.h);
}

lcdSpiInitFucPtr lcdInitFunction;
lcdSpiInitFucPtr lcdOffFunction;
lcdSpiInitFucPtr lcdOnFunction;
uint32_t lcdPixelClock;

static void lcdSpiConfig(void)
{
  stm32_spi_init(&lcdSpi, LL_SPI_DATAWIDTH_8BIT);
  stm32_spi_set_max_baudrate(&lcdSpi, LCD_SPI_BAUD);

  gpio_init(LCD_FMARK, GPIO_IN, GPIO_PIN_SPEED_HIGH);
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

// static void lcdWriteReg(uint8_t reg, uint8_t data)
// {
//   LCD_COMMAND_MODE();
//   stm32_spi_select(&lcdSpi);
//   stm32_spi_transfer_byte(&lcdSpi, reg);
//   LCD_DATA_MODE();
//   stm32_spi_transfer_byte(&lcdSpi, data);
//   stm32_spi_unselect(&lcdSpi);
// }

static void lcdWriteData(uint8_t data)
{
  LCD_DATA_MODE();
  stm32_spi_select(&lcdSpi);
  stm32_spi_transfer_byte(&lcdSpi, data);
  stm32_spi_unselect(&lcdSpi);
}

// static uint8_t lcdReadReg(uint8_t reg)
// {
//   uint8_t data = 0;
//   LCD_COMMAND_MODE();
//   stm32_spi_select(&lcdSpi);
//   stm32_spi_transfer_byte(&lcdSpi, reg);
//   LCD_DATA_MODE();
//   data = stm32_spi_transfer_byte(&lcdSpi, 0xFF);
//   stm32_spi_unselect(&lcdSpi);
//   return data;
// }

extern "C" void lcdSetInitalFrameBuffer(void *fbAddress)
{
  initialFrameBuffer = fbAddress;
}

static void lcdSetOn(void) { lcdWriteCommand(0x29); }
static void lcdSetOff(void) { lcdWriteCommand(0x28); }

extern "C" void lcdInit()
{
  // Configure the LCD SPI + RESET pins
  lcdSpiConfig();
  stm32_spi_unselect(&lcdSpi);

  // hard reset
  lcdReset();

  // Init command start
  lcdWriteCommand( 0xFE );
  lcdWriteCommand( 0xEF );

  // Display orientation
  lcdWriteCommand( 0x36 );
  lcdWriteData( 0xE8 );

  // Color mode
  lcdWriteCommand( 0x3A );
  lcdWriteData( 0x05 );  // 16-bit color

  // Display control
  lcdWriteCommand( 0x86 );
  lcdWriteData( 0x98 );
  lcdWriteCommand( 0x89 );
  lcdWriteData( 0x13 );
  lcdWriteCommand( 0x8B );
  lcdWriteData( 0x80 );
  lcdWriteCommand( 0x8D );
  lcdWriteData( 0x33 );
  lcdWriteCommand( 0x8E );
  lcdWriteData( 0x0F );

  lcdWriteCommand( 0xEC );
  lcdWriteData( 0x13 );
  lcdWriteData( 0x02 );
  lcdWriteData( 0x88 );

  lcdWriteCommand( 0xED );
  lcdWriteData( 0x18 );
  lcdWriteData( 0x08 );

  lcdWriteCommand( 0xE8 );
  lcdWriteData( 0x12 );
  lcdWriteData( 0x00 );

  // Init command ends
  lcdWriteCommand( 0xFF );
  lcdWriteCommand( 0x62 );

  // Display control
  lcdWriteCommand( 0x99 );
  lcdWriteData( 0x3E );
  lcdWriteCommand( 0x9D );
  lcdWriteData( 0x4B );
  lcdWriteCommand( 0x98 );
  lcdWriteData( 0x3E );
  lcdWriteCommand( 0x9C );
  lcdWriteData( 0x4B );
  lcdWriteCommand( 0xC3 );
  lcdWriteData( 0x27 );
  lcdWriteCommand( 0xC4 );
  lcdWriteData( 0x18 );
  lcdWriteCommand( 0xC9 );
  lcdWriteData( 0x0A );

  // Gamma Control
  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x85 );
  lcdWriteData( 0x0A );
  lcdWriteData( 0x09 );
  lcdWriteData( 0x08 );
  lcdWriteData( 0x04 );
  lcdWriteData( 0x30 );
  lcdWriteCommand( 0xF1);
  lcdWriteData( 0x47 );
  lcdWriteData( 0x5B );
  lcdWriteData( 0xB0 );
  lcdWriteData( 0x3A );
  lcdWriteData( 0x3E );
  lcdWriteData( 0x7F );
  lcdWriteCommand( 0xF2 );
  lcdWriteData( 0x85 );
  lcdWriteData( 0x0A );
  lcdWriteData( 0x09 );
  lcdWriteData( 0x08 );
  lcdWriteData( 0x04 );
  lcdWriteData( 0x30 );
  lcdWriteCommand( 0xF3 );
  lcdWriteData( 0x47 );
  lcdWriteData( 0x5B );
  lcdWriteData( 0xB0 );
  lcdWriteData( 0x3A );
  lcdWriteData( 0x3F );
  lcdWriteData( 0x7F );

  // Display size
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

  // No, invert color
  lcdWriteCommand( 0x20 );

  // Tear on
  lcdWriteCommand( 0x35 );
  lcdWriteData( 0x00 );

  // Tearing scan line
  lcdWriteCommand( 0x44 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x0A );

  // Exit sleep
  lcdWriteCommand( 0x11 );

  // Init LCD RAM
  memory_write((const uint16_t*)initialFrameBuffer, LCD_W * LCD_H);

  lcdSetOn();

  lcdOnFunction = lcdSetOn;
  lcdOffFunction = lcdSetOff;

  lcdSetFlushCb(startLcdRefresh);
}
