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
#include "stm32_gpio_driver.h"
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

uint8_t TouchControllerType = 0;  // 0: other; 1: CST836U
static volatile uint16_t lcd_phys_w = LCD_H;
static volatile uint16_t lcd_phys_h = LCD_W;

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  // given the data cache size, this is probably
  // faster than cleaning by address
  SCB_CleanDCache();

//  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while(_frame_addr_reloaded == 0);
}

lcdSpiInitFucPtr lcdInitFunction;
lcdSpiInitFucPtr lcdOffFunction;
lcdSpiInitFucPtr lcdOnFunction;
uint32_t lcdPixelClock;

volatile uint8_t LCD_ReadBuffer[24] = { 0, 0 };

static void LCD_Delay(void) {
  volatile unsigned int i;

  for (i = 0; i < 20; i++) {
    ;
  }
}

#define LTDC_HSYNC        GPIO_PIN(GPIOI, 12)
#define LTDC_VSYNC        GPIO_PIN(GPIOI, 13)
#define LTDC_CLK          GPIO_PIN(GPIOI, 14)
#define LTDC_DE           GPIO_PIN(GPIOK, 7)

#define LTDC_R3           GPIO_PIN(GPIOJ, 2)
#define LTDC_R4           GPIO_PIN(GPIOJ, 3)
#define LTDC_R5           GPIO_PIN(GPIOJ, 4)
#define LTDC_R6           GPIO_PIN(GPIOJ, 5)
#define LTDC_R7           GPIO_PIN(GPIOJ, 6)

#define LTDC_G2           GPIO_PIN(GPIOJ, 9)
#define LTDC_G3           GPIO_PIN(GPIOJ, 10)
#define LTDC_G4           GPIO_PIN(GPIOJ, 11)
#define LTDC_G5           GPIO_PIN(GPIOK, 0)
#define LTDC_G6           GPIO_PIN(GPIOK, 1)
#define LTDC_G7           GPIO_PIN(GPIOK, 2)

#define LTDC_B3           GPIO_PIN(GPIOJ, 15)
#define LTDC_B4           GPIO_PIN(GPIOK, 3)
#define LTDC_B5           GPIO_PIN(GPIOK, 4)
#define LTDC_B6           GPIO_PIN(GPIOK, 5)
#define LTDC_B7           GPIO_PIN(GPIOK, 6)

static void LCD_AF_GPIOConfig(void) {
  gpio_init_af(LTDC_HSYNC, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_VSYNC, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_CLK, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_DE, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_R3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_G2, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_B3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
}

static void lcdSpiConfig(void) {
  stm32_gpio_enable_clock(LCD_SPI_GPIO);
  stm32_gpio_enable_clock(LCD_NRST_GPIO);
  stm32_gpio_enable_clock(LCD_SPI_CS_GPIO);

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_SCK_GPIO_PIN | LCD_SPI_MOSI_GPIO_PIN | LCD_SPI_MISO_GPIO_PIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull       = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin        = LCD_NRST_GPIO_PIN;
  LL_GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin        = LCD_SPI_CS_GPIO_PIN;
  LL_GPIO_Init(LCD_SPI_CS_GPIO, &GPIO_InitStructure);

  /* Set the chip select pin always low */
  LCD_CS_LOW();
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

unsigned char LCD_ReadByteOnFallingEdge(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();

  for (i = 0; i < 8; i++) {
    LCD_DELAY();
    LCD_SCK_HIGH();
    LCD_DELAY();
    LCD_DELAY();
    ReceiveData <<= 1;

    LCD_SCK_LOW();
    LCD_DELAY();
    LCD_DELAY();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }

  LCD_MOSI_AS_OUTPUT();

  return (ReceiveData);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte) {

  LCD_SCK_LOW();
  lcdDelay();

  if (data_enable) {
    LCD_MOSI_HIGH();
  } else {
    LCD_MOSI_LOW();
  }

  LCD_SCK_HIGH();
  lcdDelay();

  for (int i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();

    if (byte & 0x80) {
      LCD_MOSI_HIGH();
    } else {
      LCD_MOSI_LOW();
    }

    LCD_SCK_HIGH();
    byte <<= 1;

    lcdDelay();
  }

  LCD_SCK_LOW();
}

unsigned char LCD_ReadByte(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();
  for (i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();
    ReceiveData <<= 1;
    LCD_SCK_HIGH();
    lcdDelay();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }
  LCD_SCK_LOW();
  LCD_MOSI_AS_OUTPUT();
  return (ReceiveData);
}

unsigned char LCD_ReadRegister(unsigned char Register) {
  unsigned char ReadData = 0;

  lcdWriteByte(0, Register);
  lcdDelay();
  lcdDelay();
  ReadData = LCD_ReadByte();
  return (ReadData);
}

void lcdWriteCommand(uint8_t command) {
  lcdWriteByte(0, command);
}

void lcdWriteData(uint8_t data) {
  lcdWriteByte(1, data);
}

void LCD_ST7796S_On(void) {
  lcdWriteCommand(0x29);
}

void LCD_ST7796S_Init(void) {
   delay_ms(120);

  lcdWriteCommand(0x11);
  delay_ms(120);

  lcdWriteCommand(0xF0);
  lcdWriteData(0xC3);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x96);

  lcdWriteCommand(0x36);
  lcdWriteData(0x28);

  lcdWriteCommand(0x2A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0xDF);
  lcdWriteCommand(0x2B);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0x3F);

  lcdWriteCommand(0x3A);
  lcdWriteData(0x55);
  lcdWriteCommand(0x34);

  // SET RGB STRAT
  lcdWriteCommand(0xB0);  // SET HS VS DE CLK 上升还是下降有效
  lcdWriteData(0x80);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x01);

  lcdWriteCommand(0xB6);
  //  lcdWriteData( 0x20 );
  //  lcdWriteData( 0x02 );
  //  lcdWriteData( 0x3B );
  lcdWriteData(0x20);
  lcdWriteData(0x02);
  lcdWriteData(0x3B);
  // SET RGB END

  lcdWriteCommand(0xB7);
  lcdWriteData(0xC6);

  lcdWriteCommand(0xB9);
  lcdWriteData(0x02);
  lcdWriteData(0xE0);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x80);
  lcdWriteData(0x65);

  lcdWriteCommand(0xC1);
  lcdWriteData(0x0D);

  lcdWriteCommand(0xC2);
  lcdWriteData(0xA7);

  lcdWriteCommand(0xC5);
  lcdWriteData(0x14);

  lcdWriteCommand(0xE8);
  lcdWriteData(0x40);
  lcdWriteData(0x8A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x29);
  lcdWriteData(0x19);
  lcdWriteData(0xA5);
  lcdWriteData(0x33);

  lcdWriteCommand(0xE0);
  lcdWriteData(0xD0);
  lcdWriteData(0x00);
  lcdWriteData(0x04);
  lcdWriteData(0x05);
  lcdWriteData(0x04);
  lcdWriteData(0x21);
  lcdWriteData(0x25);
  lcdWriteData(0x43);
  lcdWriteData(0x3F);
  lcdWriteData(0x37);
  lcdWriteData(0x13);
  lcdWriteData(0x13);
  lcdWriteData(0x29);
  lcdWriteData(0x32);

  lcdWriteCommand(0xE1);
  lcdWriteData(0xD0);
  lcdWriteData(0x04);
  lcdWriteData(0x06);
  lcdWriteData(0x09);
  lcdWriteData(0x06);
  lcdWriteData(0x03);
  lcdWriteData(0x25);
  lcdWriteData(0x32);
  lcdWriteData(0x3E);
  lcdWriteData(0x18);
  lcdWriteData(0x15);
  lcdWriteData(0x15);
  lcdWriteData(0x2B);
  lcdWriteData(0x30);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x3C);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x69);

  delay_ms(120);

  lcdWriteCommand(0x21);

  LCD_ST7796S_On();
}

void LCD_ST7796S_Off(void) {
  lcdWriteCommand(0x28);
}

unsigned int LCD_ST7796S_ReadID(void) {
  lcdReset();
  unsigned int ID = 0;

  lcdWriteCommand( 0XF0 );
  lcdWriteData( 0XC3 );
  lcdWriteCommand( 0XF0 );
  lcdWriteData( 0X96 );

  lcdWriteCommand( 0XB0 );
  lcdWriteData( 0X80 );

  lcdWriteCommand( 0XD3 );

  LCD_MOSI_AS_INPUT();
  LCD_SCK_LOW();
  lcdDelay();
  lcdDelay();
  LCD_SCK_HIGH();
  lcdDelay();
  lcdDelay();

  LCD_ReadByte();
  ID += (uint16_t)(LCD_ReadByte())<<8;
  ID += LCD_ReadByte();

   return (ID);
 }


unsigned int LCD_NT35310_ReadID( void )
{
    unsigned int ID = 0x3531;

    return( ID );

}

void LCD_NT35310_Init( void )
{
#if 1
    lcdWriteCommand(0xED);
    lcdWriteData(0x01);
    lcdWriteData(0xFE);

    lcdWriteCommand(0xEE);
    lcdWriteData(0xDE);
    lcdWriteData(0x21);

    lcdWriteCommand(0x11);
    delay_ms(120);
    lcdWriteCommand(0xB3);
    lcdWriteData(0x21);



    lcdWriteCommand(0xC0);
    lcdWriteData(0x33);
    lcdWriteData(0x33);
    lcdWriteData(0x10);
    lcdWriteData(0x10);


    lcdWriteCommand(0xC4);
    lcdWriteData(0x56);  //3a

    lcdWriteCommand(0xBF);
    lcdWriteData(0xAA);

    lcdWriteCommand(0xB0);
    lcdWriteData(0x0D);
    lcdWriteData(0x00);
    lcdWriteData(0x0D);
    lcdWriteData(0x00);
    lcdWriteData(0x11);
    lcdWriteData(0x00);
    lcdWriteData(0x19);
    lcdWriteData(0x00);
    lcdWriteData(0x21);
    lcdWriteData(0x00);
    lcdWriteData(0x2D);
    lcdWriteData(0x00);
    lcdWriteData(0x3D);
    lcdWriteData(0x00);
    lcdWriteData(0x5D);
    lcdWriteData(0x00);
    lcdWriteData(0x5D);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB1);
    lcdWriteData(0x80);
    lcdWriteData(0x00);
    lcdWriteData(0x8B);
    lcdWriteData(0x00);
    lcdWriteData(0x96);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB2);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x03);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB3);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB4);
    lcdWriteData(0x8B);
    lcdWriteData(0x00);
    lcdWriteData(0x96);
    lcdWriteData(0x00);
    lcdWriteData(0xA1);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB5);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x03);
    lcdWriteData(0x00);
    lcdWriteData(0x04);
    lcdWriteData(0x00);
    lcdWriteCommand(0xB6);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB7);
    lcdWriteData(0x3E);
    lcdWriteData(0x00);
    lcdWriteData(0x5E);
    lcdWriteData(0x00);
    lcdWriteData(0x9E);
    lcdWriteData(0x00);
    lcdWriteData(0x74);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0xAC);
    lcdWriteData(0x00);
    lcdWriteData(0xDC);
    lcdWriteData(0x00);
    lcdWriteData(0x70);
    lcdWriteData(0x00);
    lcdWriteData(0xB9);
    lcdWriteData(0x00);
    lcdWriteData(0xEC);
    lcdWriteData(0x00);
    lcdWriteData(0xDC);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB8);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xBA);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC1);
    lcdWriteData(0x20);
    lcdWriteData(0x00);
    lcdWriteData(0x54);
    lcdWriteData(0x00);
    lcdWriteData(0xFF);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC2);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x04);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC3);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x3A);
    lcdWriteData(0x00);
    lcdWriteData(0x39);
    lcdWriteData(0x00);
    lcdWriteData(0x37);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x2F);
    lcdWriteData(0x00);
    lcdWriteData(0x2C);
    lcdWriteData(0x00);
    lcdWriteData(0x29);
    lcdWriteData(0x00);
    lcdWriteData(0x26);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x2F);
    lcdWriteData(0x00);
    lcdWriteData(0x2C);
    lcdWriteData(0x00);
    lcdWriteData(0x29);
    lcdWriteData(0x00);
    lcdWriteData(0x26);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC4);
    lcdWriteData(0x62);
    lcdWriteData(0x00);
    lcdWriteData(0x05);
    lcdWriteData(0x00);
    lcdWriteData(0x84);
    lcdWriteData(0x00);
    lcdWriteData(0xF0);
    lcdWriteData(0x00);
    lcdWriteData(0x18);
    lcdWriteData(0x00);
    lcdWriteData(0xA4);
    lcdWriteData(0x00);
    lcdWriteData(0x18);
    lcdWriteData(0x00);
    lcdWriteData(0x50);
    lcdWriteData(0x00);
    lcdWriteData(0x0C);
    lcdWriteData(0x00);
    lcdWriteData(0x17);
    lcdWriteData(0x00);
    lcdWriteData(0x95);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);
    lcdWriteData(0xE6);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC5);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);
    lcdWriteData(0x76);
    lcdWriteData(0x00);
    lcdWriteData(0x88);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC6);
    lcdWriteData(0x20);
    lcdWriteData(0x00);
    lcdWriteData(0x17);
    lcdWriteData(0x00);
    lcdWriteData(0x01);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC7);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC8);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE0);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x06);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE1);
    lcdWriteData(0x01);
    lcdWriteData(0x00);
    lcdWriteData(0x05);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE2);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x06);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE3);
    lcdWriteData(0x01);
    lcdWriteData(0x00);
    lcdWriteData(0x05);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE4);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x06);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE5);
    lcdWriteData(0x01);
    lcdWriteData(0x00);
    lcdWriteData(0x05);
    lcdWriteData(0x00);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x25);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x6F);
    lcdWriteData(0x00);
    lcdWriteData(0x7F);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0x98);
    lcdWriteData(0x00);
    lcdWriteData(0xA6);
    lcdWriteData(0x00);
    lcdWriteData(0xAE);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBB);
    lcdWriteData(0x00);
    lcdWriteData(0xC0);
    lcdWriteData(0x00);
    lcdWriteData(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE6);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE7);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE8);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE9);
    lcdWriteData(0xAA);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0x00);
    lcdWriteData(0xAA);

    lcdWriteCommand(0xCF);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xF0);
    lcdWriteData(0x00);
    lcdWriteData(0x50);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xF1);
    lcdWriteData(0x01);

    lcdWriteCommand(0xF9);
    lcdWriteData(0x06);
    lcdWriteData(0x10);
    lcdWriteData(0x29);
    lcdWriteData(0x00);

    lcdWriteCommand(0xDF);
    lcdWriteData(0x10);
    delay_ms(20);
    lcdWriteCommand(0x36);
//    if( IsHorizontal )
//        lcdWriteData(0x00);//需修改
//    else
        lcdWriteData(0x14);

    lcdWriteCommand(0x3A);
    lcdWriteData(0x66);

     lcdWriteCommand(0x21);

    lcdWriteCommand(0x35);
    lcdWriteData(0x00);

    lcdWriteCommand(0x29);
#else
    lcdWriteCommand(0xED);
    lcdWriteData(0x01);
    lcdWriteData(0xFE);

    lcdWriteCommand(0xEE);
    lcdWriteData(0xDE);
    lcdWriteData(0x21);

    lcdWriteCommand(0x11);
    SYSTEM_DelayMS(120);
    lcdWriteCommand(0xB3);
    lcdWriteData(0x21);


    lcdWriteCommand(0xc0);
    lcdWriteData(0x56);
    lcdWriteData(0x56);
    lcdWriteData(0x24);
    lcdWriteData(0x24);

    lcdWriteCommand(0xC4);
    lcdWriteData(0x30);  //3a

    lcdWriteCommand(0xBF);
    lcdWriteData(0xAA);

    lcdWriteCommand(0xB0);
    lcdWriteData(0x0D);
    lcdWriteData(0x00);
    lcdWriteData(0x0D);
    lcdWriteData(0x00);
    lcdWriteData(0x11);
    lcdWriteData(0x00);
    lcdWriteData(0x19);
    lcdWriteData(0x00);
    lcdWriteData(0x21);
    lcdWriteData(0x00);
    lcdWriteData(0x2D);
    lcdWriteData(0x00);
    lcdWriteData(0x3D);
    lcdWriteData(0x00);
    lcdWriteData(0x5D);
    lcdWriteData(0x00);
    lcdWriteData(0x5D);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB1);
    lcdWriteData(0x80);
    lcdWriteData(0x00);
    lcdWriteData(0x8B);
    lcdWriteData(0x00);
    lcdWriteData(0x96);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB2);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x03);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB3);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB4);
    lcdWriteData(0x8B);
    lcdWriteData(0x00);
    lcdWriteData(0x96);
    lcdWriteData(0x00);
    lcdWriteData(0xA1);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB5);
    lcdWriteData(0x02);
    lcdWriteData(0x00);
    lcdWriteData(0x03);
    lcdWriteData(0x00);
    lcdWriteData(0x04);
    lcdWriteData(0x00);
    lcdWriteCommand(0xB6);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB7);
    lcdWriteData(0x3E);
    lcdWriteData(0x00);
    lcdWriteData(0x5E);
    lcdWriteData(0x00);
    lcdWriteData(0x9E);
    lcdWriteData(0x00);
    lcdWriteData(0x74);
    lcdWriteData(0x00);
    lcdWriteData(0x8C);
    lcdWriteData(0x00);
    lcdWriteData(0xAC);
    lcdWriteData(0x00);
    lcdWriteData(0xDC);
    lcdWriteData(0x00);
    lcdWriteData(0x70);
    lcdWriteData(0x00);
    lcdWriteData(0xB9);
    lcdWriteData(0x00);
    lcdWriteData(0xEC);
    lcdWriteData(0x00);
    lcdWriteData(0xDC);
    lcdWriteData(0x00);

    lcdWriteCommand(0xB8);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xBA);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC1);
    lcdWriteData(0x20);
    lcdWriteData(0x00);
    lcdWriteData(0x54);
    lcdWriteData(0x00);
    lcdWriteData(0xFF);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC2);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x04);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC3);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x3A);
    lcdWriteData(0x00);
    lcdWriteData(0x39);
    lcdWriteData(0x00);
    lcdWriteData(0x37);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x2F);
    lcdWriteData(0x00);
    lcdWriteData(0x2C);
    lcdWriteData(0x00);
    lcdWriteData(0x29);
    lcdWriteData(0x00);
    lcdWriteData(0x26);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x36);
    lcdWriteData(0x00);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x2F);
    lcdWriteData(0x00);
    lcdWriteData(0x2C);
    lcdWriteData(0x00);
    lcdWriteData(0x29);
    lcdWriteData(0x00);
    lcdWriteData(0x26);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x24);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC4);
    lcdWriteData(0x62);
    lcdWriteData(0x00);
    lcdWriteData(0x05);
    lcdWriteData(0x00);
    lcdWriteData(0x84);
    lcdWriteData(0x00);
    lcdWriteData(0xF0);
    lcdWriteData(0x00);
    lcdWriteData(0x18);
    lcdWriteData(0x00);
    lcdWriteData(0xA4);
    lcdWriteData(0x00);
    lcdWriteData(0x18);
    lcdWriteData(0x00);
    lcdWriteData(0x50);
    lcdWriteData(0x00);
    lcdWriteData(0x0C);
    lcdWriteData(0x00);
    lcdWriteData(0x17);
    lcdWriteData(0x00);
    lcdWriteData(0x95);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);
    lcdWriteData(0xE6);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC5);
    lcdWriteData(0x32);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);
    lcdWriteData(0x76);
    lcdWriteData(0x00);
    lcdWriteData(0x88);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC6);
    lcdWriteData(0x20);
    lcdWriteData(0x00);
    lcdWriteData(0x17);
    lcdWriteData(0x00);
    lcdWriteData(0x01);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC7);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC8);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xC9);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE0);
    lcdWriteData(0x01);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xCF);
    lcdWriteData(0x00);
    lcdWriteData(0xD3);
    lcdWriteData(0x00);
    lcdWriteData(0xDA);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE1);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xD0);
    lcdWriteData(0x00);
    lcdWriteData(0xD4);
    lcdWriteData(0x00);
    lcdWriteData(0xD9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE2);
    lcdWriteData(0x10);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xCF);
    lcdWriteData(0x00);
    lcdWriteData(0xD3);
    lcdWriteData(0x00);
    lcdWriteData(0xDA);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE3);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xD0);
    lcdWriteData(0x00);
    lcdWriteData(0xD4);
    lcdWriteData(0x00);
    lcdWriteData(0xD9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE4);
    lcdWriteData(0x01);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xCF);
    lcdWriteData(0x00);
    lcdWriteData(0xD3);
    lcdWriteData(0x00);
    lcdWriteData(0xDA);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE5);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x1F);
    lcdWriteData(0x00);
    lcdWriteData(0x3C);
    lcdWriteData(0x00);
    lcdWriteData(0x59);
    lcdWriteData(0x00);
    lcdWriteData(0x67);
    lcdWriteData(0x00);
    lcdWriteData(0x72);
    lcdWriteData(0x00);
    lcdWriteData(0x82);
    lcdWriteData(0x00);
    lcdWriteData(0x93);
    lcdWriteData(0x00);
    lcdWriteData(0xA0);
    lcdWriteData(0x00);
    lcdWriteData(0xAB);
    lcdWriteData(0x00);
    lcdWriteData(0xB4);
    lcdWriteData(0x00);
    lcdWriteData(0xBF);
    lcdWriteData(0x00);
    lcdWriteData(0xC6);
    lcdWriteData(0x00);
    lcdWriteData(0xCA);
    lcdWriteData(0x00);
    lcdWriteData(0xD0);
    lcdWriteData(0x00);
    lcdWriteData(0xD4);
    lcdWriteData(0x00);
    lcdWriteData(0xD9);
    lcdWriteData(0x00);
    lcdWriteData(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE6);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE7);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE8);
    lcdWriteData(0x55);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x56);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x57);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x77);
    lcdWriteData(0x00);
    lcdWriteData(0x66);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x44);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x33);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x23);
    lcdWriteData(0x00);
    lcdWriteData(0x65);
    lcdWriteData(0x00);

    lcdWriteCommand(0xE9);
    lcdWriteData(0xAA);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0x00);
    lcdWriteData(0xAA);

    lcdWriteCommand(0xCF);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xF0);
    lcdWriteData(0x00);
    lcdWriteData(0x50);
    lcdWriteData(0x00);
    lcdWriteData(0x00);
    lcdWriteData(0x00);

    lcdWriteCommand(0xF1);
    lcdWriteData(0x01);

    lcdWriteCommand(0xee);
    lcdWriteData(0xde);
    lcdWriteData(0x21);

    lcdWriteCommand(0xF3);
    lcdWriteData(0x00);

    lcdWriteCommand(0xF9);
    lcdWriteData(0x06);
    lcdWriteData(0x10);
    lcdWriteData(0x29);
    lcdWriteData(0x00);

    lcdWriteCommand(0xDF);
    lcdWriteData(0x10);
    SYSTEM_DelayMS(20);
    lcdWriteCommand(0x36);
    if( IsHorizontal )
        lcdWriteData(0x14);//需修改
    else
        lcdWriteData(0x14);

    lcdWriteCommand(0x3A);
    lcdWriteData(0x66);

    lcdWriteCommand(0x21);

    lcdWriteCommand(0x35);
    lcdWriteData(0x00);

    lcdWriteCommand(0x28);
#endif
}

void LCD_NT35310_On( void )
{
    lcdWriteCommand( 0x29 );
}

void LCD_NT35310_Off( void )
{
    lcdWriteCommand( 0x28 );
}

void LCD_Init_LTDC() {
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock Mhz */
//  uint32_t clock = (lcdPixelClock*16) / 1000000; // clock*16 in MHz
//  RCC_PeriphCLKInitTypeDef clkConfig;
//  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
//  clkConfig.PLLI2S
//  clkConfig.PLLSAI.PLLSAIN = clock;
//  clkConfig.PLLSAI.PLLSAIR = 4;
//  clkConfig.PLLSAIDivQ = 6;
//  clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
//  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IIPC;

  /* Configure R,G,B component values for LCD background color */
  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  /* Configure horizontal synchronization width */
  hltdc.Init.HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  hltdc.Init.VerticalSync = VSH;
  /* Configure accumulated horizontal back porch */
  hltdc.Init.AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  hltdc.Init.AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  hltdc.Init.AccumulatedActiveW = lcd_phys_w + HBP;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = lcd_phys_h + VBP;
  /* Configure total width */
  hltdc.Init.TotalWidth = lcd_phys_w + HBP + HFP;
  /* Configure total height */
  hltdc.Init.TotalHeigh = lcd_phys_h + VBP + VFP;

  HAL_LTDC_Init(&hltdc);

  // Configure IRQ (line)
  NVIC_SetPriority(LTDC_IRQn, LTDC_IRQ_PRIO);
  NVIC_EnableIRQ(LTDC_IRQn);

  // Trigger on last line
  HAL_LTDC_ProgramLineEvent(&hltdc, lcd_phys_h);
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);
}

void LCD_LayerInit() {
  auto& layer = hltdc.LayerCfg[0];

  /* Windowing configuration */
  layer.WindowX0 = 0;
  layer.WindowX1 = lcd_phys_w;
  layer.WindowY0 = 0;
  layer.WindowY1 = lcd_phys_h;

  /* Pixel Format configuration*/
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;

  /* Alpha constant (255 totally opaque) */
  layer.Alpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  layer.Backcolor.Blue = 0;
  layer.Backcolor.Green = 0;
  layer.Backcolor.Red = 0;
  layer.Alpha0 = 0;

  /* Configure blending factors */
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

  layer.ImageWidth = lcd_phys_w;
  layer.ImageHeight = lcd_phys_h;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  layer.FBStartAdress = (intptr_t)initialFrameBuffer;

  /* Initialize LTDC layer 1 */
  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);

  /* dithering activation */
  HAL_LTDC_EnableDither(&hltdc);
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
  initialFrameBuffer = fbAddress;
}

const char* boardLcdType = "";

extern "C"
void lcdInit(void)
{
  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  /* Send LCD initialization commands */
//  if (LCD_ILI9481_ReadID() == LCD_ILI9481_ID) {
//    TRACE("LCD INIT: ILI9481");
//    boardLcdType = "ILI9481";
//    lcdInitFunction = LCD_ILI9481_Init;
//    lcdOffFunction = LCD_ILI9481_Off;
//    lcdOnFunction = LCD_ILI9481_On;
//    lcdPixelClock = 12000000;
//  } else if (LCD_ILI9486_ReadID() == LCD_ILI9486_ID) {
//    TRACE("LCD INIT: ILI9486");
//    boardLcdType = "ILI9486";
//    lcdInitFunction = LCD_ILI9486_Init;
//    lcdOffFunction = LCD_ILI9486_Off;
//    lcdOnFunction = LCD_ILI9486_On;
//    lcdPixelClock = 12000000;
//  } else if (LCD_ILI9488_ReadID() == LCD_ILI9488_ID) {
//    TRACE("LCD INIT: ILI9488");
//    boardLcdType = "ILI9488";
//    lcdInitFunction = LCD_ILI9488_Init;
//    lcdOffFunction = LCD_ILI9488_Off;
//    lcdOnFunction = LCD_ILI9488_On;
//    lcdPixelClock = 12000000;
//    lcd_phys_w = LCD_PHYS_H;
//    lcd_phys_h = LCD_PHYS_W;
//  } else if (LCD_HX8357D_ReadID() == LCD_HX8357D_ID) {
//    TRACE("LCD INIT: HX8357D");
//    boardLcdType = "HX8357D";
//    lcdInitFunction = LCD_HX8357D_Init;
//    lcdOffFunction = LCD_HX8357D_Off;
//    lcdOnFunction = LCD_HX8357D_On;
//    lcdPixelClock = 12000000;
//  } else if (LCD_ST7796S_ReadID() == LCD_ST7796S_ID ) {
//    TRACE("LCD INIT: ST7796S");
//    boardLcdType = "ST7796S";
//    lcdInitFunction = LCD_ST7796S_Init;
//    lcdOffFunction = LCD_ST7796S_Off;
//    lcdOnFunction = LCD_ST7796S_On;
//    lcdPixelClock = 14500000;
//  }
//  else{
    TRACE("LCD INIT (default): ST7796S");
    boardLcdType = "ST7796S (Default)";
    lcdInitFunction = LCD_ST7796S_Init;
    lcdOffFunction = LCD_ST7796S_Off;
    lcdOnFunction = LCD_ST7796S_On;
    lcdPixelClock = 12000000;
//  }

  __HAL_RCC_LTDC_CLK_ENABLE();
  lcdInitFunction();

  LCD_Init_LTDC();
  LCD_LayerInit();

  // Enable LCD display
  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_LI);
  _frame_addr_reloaded = 1;
}