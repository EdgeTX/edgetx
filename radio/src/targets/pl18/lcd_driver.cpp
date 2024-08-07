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
#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "delays_driver.h"
#include "debug.h"
#include "lcd.h"
#include "lcd_driver.h"

uint8_t TouchControllerType = 0;  //0:cst340; 1 ft6236
static volatile uint16_t lcd_phys_w = LCD_PHYS_W;
static volatile uint16_t lcd_phys_h = LCD_PHYS_H;

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

#define GPIO_AF_LTDC GPIO_AF14

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
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

enum ENUM_IO_SPEED
{
    IO_SPEED_LOW,
    IO_SPEED_MID,
    IO_SPEED_QUICK,
    IO_SPEED_HIGH
};

enum ENUM_IO_MODE
{
    IO_MODE_INPUT,
    IO_MODE_OUTPUT,
    IO_MODE_ALTERNATE,
    IO_MODE_ANALOG
};


static void LCD_AF_GPIOConfig(void)
{
  /* GPIOs Configuration */
  /*
    +---------------------+---------------------+------------------+------------------+
    +                       LCD pins assignment                                       +
    +---------------------+---------------------+------------------+------------------+
    | LCD_CLK <-> PG.07   | LCD_HSYNC <-> PI.12 | LCD_R3 <-> PJ.02 | LCD_G5 <-> PK.00 |
    | LCD VSYNC <-> PI.13 | LCD_R4 <-> PJ.03    | LCD_G6 <-> PK.01 |                  |
    |                     | LCD_R5 <-> PJ.04    | LCD_G7 <-> PK.02 |                  |
    |                     | LCD_R6 <-> PJ.05    | LCD_B4 <-> PK.03 |                  |
    |                     | LCD_R7 <-> PJ.06    | LCD_B5 <-> PK.04 |                  |
    |                     | LCD_G2 <-> PJ.09    | LCD_B6 <-> PK.05 |                  |
    |                     | LCD_G3 <-> PJ.10    | LCD_B7 <-> PK.06 |                  |
    |                     | LCD_G4 <-> PJ.11    | LCD_DE <-> PK.07 |                  |
    |                     | LCD_B3 <-> PJ.15    |                  |                  |
    +---------------------+---------------------+------------------+------------------+
   */

  // GPIOG configuration
  gpio_init_af(GPIO_PIN(GPIOG, 7), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);

  // GPIOI configuration
  gpio_init_af(GPIO_PIN(GPIOI, 12), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 13), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);

  // GPIOJ configuration
  gpio_init_af(GPIO_PIN(GPIOJ, 2), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 3), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 4), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 5), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 6), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 9), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 10), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 11), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 15), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);

  // GPIOK configuration
  gpio_init_af(GPIO_PIN(GPIOK, 0), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 1), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 2), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 3), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 4), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 5), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 6), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 7), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
}

static void lcdSpiConfig(void)
{
  gpio_init(LCD_SPI_SCK_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_SPI_MOSI_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_SPI_CS_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_NRST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  /* Set the chip select pin aways low */
  CLR_LCD_CS();
}

void lcdDelay() {
  delay_01us(1);
}

static void lcdReset() {
  SET_LCD_NRST();
  delay_ms(1);

  CLR_LCD_NRST(); // RESET();
  delay_ms(100);

  SET_LCD_NRST();
  delay_ms(100);
}

unsigned char LCD_ReadByteOnFallingEdge(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  SET_LCD_DATA();
  SET_LCD_DATA_INPUT();

  for (i = 0; i < 8; i++) {
    lcdDelay();
    SET_LCD_CLK();
    lcdDelay();
    lcdDelay();
    ReceiveData <<= 1;

    CLR_LCD_CLK();
    lcdDelay();
    lcdDelay();
    if (READ_LCD_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }

  SET_LCD_DATA_OUTPUT();

  return (ReceiveData);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte) {

  CLR_LCD_CLK();

  if (data_enable) {
    SET_LCD_DATA();
  } else {
    CLR_LCD_DATA();
  }

  lcdDelay();
  SET_LCD_CLK();

  for (int i = 0; i < 8; i++) {
    lcdDelay();
    CLR_LCD_CLK();

    if (byte & 0x80) {
      SET_LCD_DATA();
    } else {
      CLR_LCD_DATA();
    }

    lcdDelay();
    SET_LCD_CLK();
    byte <<= 1;
  }

  lcdDelay();
  CLR_LCD_CLK();
}

unsigned char LCD_ReadByte(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  SET_LCD_DATA();
  SET_LCD_DATA_INPUT();
  for (i = 0; i < 8; i++) {
    CLR_LCD_CLK();
    lcdDelay();
    ReceiveData <<= 1;
    SET_LCD_CLK();
    lcdDelay();
    if (READ_LCD_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }
  CLR_LCD_CLK();
  SET_LCD_DATA_OUTPUT();
  return (ReceiveData);
}

unsigned char LCD_ReadRegister(unsigned char Register) {
  unsigned char ReadData = 0;

  CLR_LCD_CS();
  lcdWriteByte(0, Register);
  lcdDelay();
  lcdDelay();
  ReadData = LCD_ReadByte();
  SET_LCD_CS();
  lcdDelay();
  return (ReadData);
}

void lcdWriteCommand(uint8_t command) {
  CLR_LCD_CS();
  lcdWriteByte(0, command);
  SET_LCD_CS();
  lcdDelay();
}

void lcdWriteData(uint8_t data) {
  CLR_LCD_CS();
  lcdWriteByte(1, data);
  SET_LCD_CS();
  lcdDelay();
}

void LCD_HX8357D_Init(void) {
#if 0
  lcdWriteCommand(0x11);
  delay_ms(200);

  lcdWriteCommand(0xB9);
  lcdWriteData(0xFF);
  lcdWriteData(0x83);
  lcdWriteData(0x57);

  lcdWriteCommand(0xB1);
  lcdWriteData(0x00);
  lcdWriteData(0x14);
  lcdWriteData(0x1C);
  lcdWriteData(0x1C);
  lcdWriteData(0xC7);
  lcdWriteData(0x21);

  lcdWriteCommand(0xB3);
  lcdWriteData(0x83);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x06);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x11);
  lcdWriteData(0x40);
  lcdWriteData(0x00);
  lcdWriteData(0x2A);
  lcdWriteData(0x2A);
  lcdWriteData(0x20);
  lcdWriteData(0x4E);

  lcdWriteCommand(0xB5);
  lcdWriteData(0x03);
  lcdWriteData(0x03);

  lcdWriteCommand(0xB6);
  lcdWriteData(0x38);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x24);
  lcdWriteData(0x24);
  lcdWriteData(0x00);
  lcdWriteData(0x10);
  lcdWriteData(0xc8);
  lcdWriteData(0x08);

  lcdWriteCommand(0xC2);
  lcdWriteData(0x00);
  lcdWriteData(0x08);
  lcdWriteData(0x04);

  lcdWriteCommand(0xCC);
  lcdWriteData(0x00);

//GAMMA 2.5"
  lcdWriteCommand(0xE0);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x01);

  lcdWriteCommand(0x3A);
  lcdWriteData(0x66);

  lcdWriteCommand(0x36);
  lcdWriteData(0x28);
  lcdWriteCommand( 0x2A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0xDF );
  lcdWriteCommand( 0x2B );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0x3F );

  lcdWriteCommand(0x29);
  delay_ms(10);
#elif 0
  delay_ms(50);
  lcdWriteCommand(0xB9); //EXTC
  lcdWriteData(0xFF); //EXTC
  lcdWriteData(0x83); //EXTC
  lcdWriteData(0x57); //EXTC
  delay_ms(5);

  lcdWriteCommand(0x3A);
  lcdWriteData(0x65); //262k

  lcdWriteCommand(0xB3); //COLOR FORMAT
  lcdWriteData(0x83); //SDO_EN,BYPASS,EPF[1:0],0,0,RM,DM  //43

  lcdWriteCommand(0xB6); //
  lcdWriteData(0x5a); //VCOMDC

  lcdWriteCommand(0x35); // TE ON
  lcdWriteData(0x01);

  lcdWriteCommand(0xB0);
  lcdWriteData(0x68); //70Hz

  lcdWriteCommand(0xCC); // Set Panel
  lcdWriteData(0x00); //

  lcdWriteCommand(0xB1); //
  lcdWriteData(0x00); //
  lcdWriteData(0x11); //BT
  lcdWriteData(0x1C); //VSPR
  lcdWriteData(0x1C); //VSNR
  lcdWriteData(0x83); //AP
  lcdWriteData(0x48); //FS  0xAA

  lcdWriteCommand(0xB4); //
  lcdWriteData(0x02); //NW
  lcdWriteData(0x40); //RTN
  lcdWriteData(0x00); //DIV
  lcdWriteData(0x2A); //DUM
  lcdWriteData(0x2A); //DUM
  lcdWriteData(0x0D); //GDON
  lcdWriteData(0x78); //GDOFF  0x4F
  lcdWriteCommand(0xC0); //STBA
  lcdWriteData(0x50); //OPON
  lcdWriteData(0x50); //OPON
  lcdWriteData(0x01); //
  lcdWriteData(0x3C); //
  lcdWriteData(0x1E); //
  lcdWriteData(0x08); //GEN

  /*
   lcdWriteCommand(0xE0); //
   lcdWriteData(0x02); //1
   lcdWriteData(0x06); //2
   lcdWriteData(0x09); //3
   lcdWriteData(0x1C); //4
   lcdWriteData(0x27); //5
   lcdWriteData(0x3C); //6
   lcdWriteData(0x48); //7
   lcdWriteData(0x50); //8
   lcdWriteData(0x49); //9
   lcdWriteData(0x42); //10
   lcdWriteData(0x3E); //11
   lcdWriteData(0x35); //12
   lcdWriteData(0x31); //13
   lcdWriteData(0x2A); //14
   lcdWriteData(0x28); //15
   lcdWriteData(0x03); //16
   lcdWriteData(0x02); //17 v1
   lcdWriteData(0x06); //18
   lcdWriteData(0x09); //19
   lcdWriteData(0x1C); //20
   lcdWriteData(0x27); //21
   lcdWriteData(0x3C); //22
   lcdWriteData(0x48); //23
   lcdWriteData(0x50); //24
   lcdWriteData(0x49); //25
   lcdWriteData(0x42); //26
   lcdWriteData(0x3E); //27
   lcdWriteData(0x35); //28
   lcdWriteData(0x31); //29
   lcdWriteData(0x2A); //30
   lcdWriteData(0x28); //31
   lcdWriteData(0x03); //32
   lcdWriteData(0x44); //33
   lcdWriteData(0x01); //34
   */
  lcdWriteCommand(0xE0);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteCommand(0x36);
  lcdWriteData(0x38);

  lcdWriteCommand(0x11); // SLPOUT
  delay_ms(200);

  lcdWriteCommand(0x29); // Display On
  delay_ms(25);
  lcdWriteCommand(0x2C);
#else
  lcdWriteCommand(0x11);
  delay_ms(200);

  lcdWriteCommand(0xB9);
  lcdWriteData(0xFF);
  lcdWriteData(0x83);
  lcdWriteData(0x57);
  delay_ms(5);

//  lcdWriteCommand(0x36);
//  lcdWriteData(0x10);

  lcdWriteCommand(0xB1);
  lcdWriteData(0x00);
  lcdWriteData(0x14);
  lcdWriteData(0x1C);
  lcdWriteData(0x1C);
  lcdWriteData(0xC7);
  lcdWriteData(0x21);
  lcdWriteCommand(0xB3);
  lcdWriteData(0x83);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x06);
  lcdWriteCommand(0xB4);
  lcdWriteData(0x11);
  lcdWriteData(0x40);
  lcdWriteData(0x00);
  lcdWriteData(0x2A);
  lcdWriteData(0x2A);
  lcdWriteData(0x20);
  lcdWriteData(0x4E);
  lcdWriteCommand(0xB5);
  lcdWriteData(0x03);
  lcdWriteData(0x03);

  lcdWriteCommand(0xB6);
  lcdWriteData(0x38);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x24);
  lcdWriteData(0x24);
  lcdWriteData(0x00);
  lcdWriteData(0x10);
  lcdWriteData(0xc8);
  lcdWriteData(0x08);
  lcdWriteCommand(0xC2);
  lcdWriteData(0x00);
  lcdWriteData(0x08);
  lcdWriteData(0x04);
  //GAMMA 2.5"
  lcdWriteCommand(0xE0);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x06);
  lcdWriteData(0x0D);
  lcdWriteData(0x18);
  lcdWriteData(0x23);
  lcdWriteData(0x3B);
  lcdWriteData(0x45);
  lcdWriteData(0x4D);
  lcdWriteData(0x4D);
  lcdWriteData(0x46);
  lcdWriteData(0x40);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x2F);
  lcdWriteData(0x2B);
  lcdWriteData(0x21);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  // lcdWriteCommand(0x2A);
  // lcdWriteData(0);
  // lcdWriteData(0);
  // lcdWriteData(480 >> 8);
  // lcdWriteData(480);
  // lcdWriteCommand(0x2B);
  // lcdWriteData(0);
  // lcdWriteData(0);
  // lcdWriteData(320 >> 8);
  // lcdWriteData(320);
  lcdWriteCommand(0x3A);
  lcdWriteData(0x66);

  lcdWriteCommand(0xCC);
  lcdWriteData(0x01);

  lcdWriteCommand( 0x2A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0xDF );
  lcdWriteCommand( 0x2B );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0x3F );

  lcdWriteCommand(0x36);
  lcdWriteData(0x20);

  lcdWriteCommand(0xB9);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  delay_ms(5);
  lcdWriteCommand(0x29);
#endif
}

void LCD_HX8357D_On(void) {
//  lcdWriteCommand(0x28);
  lcdWriteCommand(0x29);
}

void LCD_HX8357D_Off(void) {
  lcdWriteCommand(0x28);
}

unsigned int LCD_HX8357D_ReadID(void) {
  lcdReset();
  int ID = 0;

  lcdWriteCommand( 0xB9 );
  lcdWriteData( 0xff );
  lcdWriteData( 0x83 );
  lcdWriteData( 0x57 );

  lcdWriteCommand( 0xFE );
  lcdWriteData( 0xd0 );
  ID = LCD_ReadRegister( 0xff );

  lcdWriteCommand( 0xB9 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );

  return (ID);
}

void LCD_ILI9481_Init(void) {
  lcdWriteCommand(0x11);
  delay_ms(120);

  lcdWriteCommand(0xE4);
  lcdWriteData(0x0A);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x01);

  lcdWriteCommand(0xF3);
  lcdWriteData(0x02);
  lcdWriteData(0x1A);

  lcdWriteCommand(0xD0);
  lcdWriteData(0x07);
  lcdWriteData(0x42);
  lcdWriteData(0x1B);

  lcdWriteCommand(0xD1);
  lcdWriteData(0x00);
  lcdWriteData(0x00); //04
  lcdWriteData(0x1A);

  lcdWriteCommand(0xD2);
  lcdWriteData(0x01);
  lcdWriteData(0x00); //11

  lcdWriteCommand(0xC0);
  lcdWriteData(0x10);
  lcdWriteData(0x3B); //
  lcdWriteData(0x00); //
  lcdWriteData(0x02);
  lcdWriteData(0x11);

  lcdWriteCommand(0xC5);
  lcdWriteData(0x03);

  lcdWriteCommand(0xC8);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0x47);
  lcdWriteData(0x60);
  lcdWriteData(0x04);
  lcdWriteData(0x16);
  lcdWriteData(0x03);
  lcdWriteData(0x67);
  lcdWriteData(0x67);
  lcdWriteData(0x06);
  lcdWriteData(0x0F);
  lcdWriteData(0x00);

  lcdWriteCommand(0x36);
  lcdWriteData(0x08);

  lcdWriteCommand(0x3A);
  lcdWriteData(0x66); //0x55=65k color, 0x66=262k color.

  lcdWriteCommand(0x2A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0x3F);

  lcdWriteCommand(0x2B);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0xE0);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x11);

  lcdWriteCommand(0xc6);
  lcdWriteData(0x82);

  delay_ms(120);

  lcdWriteCommand(0x21);
  lcdWriteCommand(0x29);
  lcdWriteCommand(0x2C);

}

void LCD_ILI9481_On(void) {
  lcdWriteCommand(0x29);
}

void LCD_ILI9481_Off(void) {
  lcdWriteCommand(0x28);
}

unsigned int LCD_ILI9481_ReadID(void) {
#if 1
  /* Have a issue here */
  return 0;
#else
  int ID = 0;
  int Data;


  lcdWriteByte(0, 0xBF);

  Data = LCD_ReadByteOnFallingEdge();
  Data = LCD_ReadByteOnFallingEdge();
  ID = LCD_ReadByteOnFallingEdge();
  ID <<= 8;
  ID |= LCD_ReadByteOnFallingEdge();
  Data = LCD_ReadByteOnFallingEdge();
  Data = LCD_ReadByteOnFallingEdge();

  lcdDelay();
  lcdDelay();
  lcdDelay();

  lcdWriteCommand(0xC6);
  lcdWriteData(0x82);
  //lcdWriteData( 0x9b );
  return (ID);
#endif
}

void LCD_ILI9486_On(void) {
  lcdWriteCommand(0x29);
}

void LCD_ILI9486_Init(void) {
  lcdWriteCommand(0XFB);
  lcdWriteData(0x00);

  lcdWriteCommand(0xf2);
  lcdWriteData(0x18);
  lcdWriteData(0xa3);
  lcdWriteData(0x12);
  lcdWriteData(0x02);
  lcdWriteData(0xb2);
  lcdWriteData(0x12);
  lcdWriteData(0xff);
  lcdWriteData(0x13);
  lcdWriteData(0x00);
  lcdWriteCommand(0xf1);
  lcdWriteData(0x36);
  lcdWriteData(0x04);
  lcdWriteData(0x00);
  lcdWriteData(0x3c);
  lcdWriteData(0x0f);
  lcdWriteData(0x8f);
  lcdWriteCommand(0xf8);
  lcdWriteData(0x21);
  lcdWriteData(0x04);
  lcdWriteCommand(0xf9);
  lcdWriteData(0x00);
  lcdWriteData(0x08);
  lcdWriteCommand(0x36);
  lcdWriteData(0x18);
  lcdWriteCommand(0x3a);
  lcdWriteData(0x65);
  lcdWriteCommand(0xc0);
  lcdWriteData(0x0f);
  lcdWriteData(0x0f);
  lcdWriteCommand(0xc1);
  lcdWriteData(0x41);

  lcdWriteCommand(0xc5);
  lcdWriteData(0x00);
  lcdWriteData(0x27);
  lcdWriteData(0x80);
  lcdWriteCommand(0xb6);
  lcdWriteData(0xb2);
  lcdWriteData(0x42);
  lcdWriteData(0x3b);
  lcdWriteCommand(0xb1);
  lcdWriteData(0xb0);
  lcdWriteData(0x11);
  lcdWriteCommand(0xb4);
  lcdWriteData(0x02);
  lcdWriteCommand(0xb7);
  lcdWriteData(0xC6);

  lcdWriteCommand(0xe0);
  lcdWriteData(0x0f);
  lcdWriteData(0x1C);
  lcdWriteData(0x18);
  lcdWriteData(0x0B);
  lcdWriteData(0x0D);
  lcdWriteData(0x06);
  lcdWriteData(0x48);
  lcdWriteData(0x87);
  lcdWriteData(0x3A);
  lcdWriteData(0x09);
  lcdWriteData(0x15);
  lcdWriteData(0x08);
  lcdWriteData(0x0D);
  lcdWriteData(0x04);
  lcdWriteData(0x00);

  lcdWriteCommand(0xe1);
  lcdWriteData(0x0f);
  lcdWriteData(0x37);
  lcdWriteData(0x34);
  lcdWriteData(0x0A);
  lcdWriteData(0x0B);
  lcdWriteData(0x03);
  lcdWriteData(0x4B);
  lcdWriteData(0x31);
  lcdWriteData(0x39);
  lcdWriteData(0x03);
  lcdWriteData(0x0F);
  lcdWriteData(0x03);
  lcdWriteData(0x22);
  lcdWriteData(0x1D);
  lcdWriteData(0x00);

  lcdWriteCommand(0x21);
  lcdWriteCommand(0x11);
  delay_ms(120);
  lcdWriteCommand(0x28);

  LCD_ILI9486_On();
}

void LCD_ILI9486_Off(void) {
  lcdWriteCommand(0x28);
}

unsigned int LCD_ILI9486_ReadID(void) {
  int ID = 0;

  lcdWriteCommand(0XF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);
  lcdWriteCommand(0XB0);
  lcdWriteData(0X80);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x10 | 0x00);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x10 | 0x01);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x10 | 0x02);
  ID = LCD_ReadRegister(0xd3);
  ID <<= 8;
  lcdWriteCommand(0XFB);
  lcdWriteData(0x10 | 0x03);
  ID |= LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x00);

  return (ID);
}

void LCD_ILI9488_On(void) {
  // Display ON
  lcdWriteCommand(0x29);
}

void LCD_ILI9488_Init(void) {

  // lcdWriteCommand(0xFB);
  // lcdWriteData(0x00);

  // Adjust Control 3:
  // -> DSI write DCS command, use stream packet RGB 666
  lcdWriteCommand(0xF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);

  // Power Control 1:
  // -> VREG1OUT = 4.6250
  // -> VREG2OUT = -4.1250
  lcdWriteCommand(0xC0);
  lcdWriteData(0x11);
  lcdWriteData(0x09);

  // Power Control 2:
  // -> VGH = VCI x 6, VGL = VCI x 4
  lcdWriteCommand(0xC1);
  lcdWriteData(0x41);

  // VCOM Control
  lcdWriteCommand(0xC5);
  lcdWriteData(0x00); // NV memory not programmed
  lcdWriteData(0x0A); // VCM_REG
  lcdWriteData(0x80); // VCM_REG_EN

  // Frame Rate Control
  lcdWriteCommand(0xB1);
  lcdWriteData(0xB0);
  lcdWriteData(0x11);

  // Display Inversion Control
  lcdWriteCommand(0xB4);
  lcdWriteData(0x01);

  lcdWriteCommand(0xB5);
  lcdWriteData(VFP);
  lcdWriteData(VBP);
  lcdWriteData(HFP);
  lcdWriteData(HBP);

  // Display Function Control
  lcdWriteCommand(0xB6);
  // !DM | RM | RCM
  lcdWriteData(0x20 | 0x40);
  lcdWriteData(0x02);
  lcdWriteData(0x3B); // (59 + 1) x 8 = 480 lines

  // Entry Set Mode
  lcdWriteCommand(0xB7);
  lcdWriteData(0xc6);

  lcdWriteCommand(0xBE);
  lcdWriteData(0x00);
  // lcdWriteData(0x04); // ????

  lcdWriteCommand(0xE9);
  lcdWriteData(0x00);

  // Column Address Set
  // -> SC=0, EC=479
  lcdWriteCommand( 0x2A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0xDF );

  // Page Address Set
  // -> SP=0, EP=319
  lcdWriteCommand( 0x2B );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0x3F );

  // Memory Access Control
  lcdWriteCommand(0x36);
  lcdWriteData(0x28); // 0x20 -> swap col/rows

  lcdWriteCommand(0x3A);
  lcdWriteData(0x66);

  lcdWriteCommand(0xE0);
  lcdWriteData(0x00);
  lcdWriteData(0x07);
  lcdWriteData(0x10);
  lcdWriteData(0x09);
  lcdWriteData(0x17);
  lcdWriteData(0x0B);
  lcdWriteData(0x41);
  lcdWriteData(0x89);
  lcdWriteData(0x4B);
  lcdWriteData(0x0A);
  lcdWriteData(0x0C);
  lcdWriteData(0x0E);
  lcdWriteData(0x18);
  lcdWriteData(0x1B);
  lcdWriteData(0x0F);

  lcdWriteCommand(0xE1);
  lcdWriteData(0x00);
  lcdWriteData(0x17);
  lcdWriteData(0x1A);
  lcdWriteData(0x04);
  lcdWriteData(0x0E);
  lcdWriteData(0x06);
  lcdWriteData(0x2F);
  lcdWriteData(0x45);
  lcdWriteData(0x43);
  lcdWriteData(0x02);
  lcdWriteData(0x0A);
  lcdWriteData(0x09);
  lcdWriteData(0x32);
  lcdWriteData(0x36);
  lcdWriteData(0x0F);

  // Sleep OUT
  lcdWriteCommand(0x11);
  delay_ms(120);

  // Display OFF
  lcdWriteCommand(0x28);

  LCD_ILI9488_On();
}

void LCD_ILI9488_Off(void) {
  lcdWriteCommand(0x28);
}

void LCD_ILI9488_ReadDevice(void) {
  int Index = 0;
  int Parameter = 0x80;

  lcdWriteCommand(0xF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);

  lcdWriteCommand(0xB0);
  lcdWriteData(0x80);

  lcdWriteCommand(0xFB);
  lcdWriteData(Parameter | 0x00);
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  //lcdWriteCommand(0X2E);
  lcdWriteCommand(0xFB);
  lcdWriteData(Parameter | 0x01);        //Parameter2=0x88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x02);        //Parameter2=0x88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x03);        //Parameter2=0x88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);
}

unsigned int LCD_ILI9488_ReadID(void) {
  int ID = 0;

  // Adjust Control 3:
  // -> DSI write DCS command, use stream packet RGB 666
  lcdWriteCommand(0xF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);

  // Interface Mode Control: 
  //   SDA_EN = 1, DIN/SDA pin is used for 3/4 wire serial
  //               interface and SDO pin is not used.
  lcdWriteCommand(0xB0);
  lcdWriteData(0x80);

  // first byte is dummy one
  lcdWriteCommand(0xFB);
  lcdWriteData(0x80 | 0x00);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0xFB);
  lcdWriteData(0x80 | 0x01);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0xFB);
  lcdWriteData(0x80 | 0x02);
  ID = LCD_ReadRegister(0xd3);
  ID <<= 8;

  lcdWriteCommand(0xFB);
  lcdWriteData(0x80 | 0x03);
  ID |= LCD_ReadRegister(0xd3);

  lcdWriteCommand(0xFB);
  lcdWriteData(0x00);

  return (ID);
}

void LCD_ST7796S_On(void) {
  lcdWriteCommand(0x29);
}

void LCD_ST7796S_Init(void) {
  delay_ms(120);

  lcdWriteCommand( 0x11 );
  delay_ms(120);

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0xC3 );

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x96 );

  lcdWriteCommand( 0x36 );
  lcdWriteData( 0x28 );

  lcdWriteCommand( 0x2A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0xDF );
  lcdWriteCommand( 0x2B );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x01 );
  lcdWriteData( 0x3F );

  lcdWriteCommand( 0x3A );
  lcdWriteData( 0x66 );

  //SET RGB STRAT
  lcdWriteCommand (0xB0 );   //SET HS VS DE CLK 上升还是下降有效
  lcdWriteData( 0x80 );

  lcdWriteCommand( 0xB4 );
  lcdWriteData( 0x01 );

  lcdWriteCommand( 0xB6 );
  //  lcdWriteData( 0x20 );
  //  lcdWriteData( 0x02 );
  //  lcdWriteData( 0x3B );
    lcdWriteData( 0x20 );
    lcdWriteData( 0x02 );
    lcdWriteData( 0x3B );
  //SET RGB END

  lcdWriteCommand( 0xB7);
  lcdWriteData( 0xC6);

  lcdWriteCommand( 0xB9 );
  lcdWriteData( 0x02 );
  lcdWriteData( 0xE0 );

  lcdWriteCommand( 0xC0 );
  lcdWriteData( 0x80 );
  lcdWriteData( 0x65 );

  lcdWriteCommand( 0xC1 );
  lcdWriteData( 0x0D );

  lcdWriteCommand( 0xC2 );
  lcdWriteData( 0xA7 );

  lcdWriteCommand( 0xC5 );
  lcdWriteData( 0x14 );

  lcdWriteCommand( 0xE8 );
  lcdWriteData( 0x40 );
  lcdWriteData( 0x8A );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x29 );
  lcdWriteData( 0x19 );
  lcdWriteData( 0xA5 );
  lcdWriteData( 0x33 );

  lcdWriteCommand( 0xE0 );
  lcdWriteData( 0xD0 );
  lcdWriteData( 0x00 );
  lcdWriteData( 0x04 );
  lcdWriteData( 0x05 );
  lcdWriteData( 0x04 );
  lcdWriteData( 0x21 );
  lcdWriteData( 0x25 );
  lcdWriteData( 0x43 );
  lcdWriteData( 0x3F );
  lcdWriteData( 0x37 );
  lcdWriteData( 0x13 );
  lcdWriteData( 0x13 );
  lcdWriteData( 0x29 );
  lcdWriteData( 0x32 );

  lcdWriteCommand( 0xE1 );
  lcdWriteData( 0xD0 );
  lcdWriteData( 0x04 );
  lcdWriteData( 0x06 );
  lcdWriteData( 0x09 );
  lcdWriteData( 0x06 );
  lcdWriteData( 0x03 );
  lcdWriteData( 0x25 );
  lcdWriteData( 0x32 );
  lcdWriteData( 0x3E );
  lcdWriteData( 0x18 );
  lcdWriteData( 0x15 );
  lcdWriteData( 0x15 );
  lcdWriteData( 0x2B );
  lcdWriteData( 0x30 );

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x3C );

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x69 );

  delay_ms(120);

  if( !TouchControllerType ) {
    lcdWriteCommand( 0x21 );
  }

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

  SET_LCD_DATA_INPUT();
  CLR_LCD_CLK();
  lcdDelay();
  lcdDelay();
  SET_LCD_CLK();
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
  __HAL_RCC_LTDC_CLK_ENABLE();
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock Mhz */
  uint32_t clock = (lcdPixelClock*16) / 1000000; // clock*16 in MHz
  RCC_PeriphCLKInitTypeDef clkConfig;
  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  clkConfig.PLLSAI.PLLSAIN = clock;
  clkConfig.PLLSAI.PLLSAIR = 4;
  clkConfig.PLLSAIDivQ = 6;
  clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

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
  if (LCD_ILI9481_ReadID() == LCD_ILI9481_ID) {
    TRACE("LCD INIT: ILI9481");
    boardLcdType = "ILI9481";
    lcdInitFunction = LCD_ILI9481_Init;
    lcdOffFunction = LCD_ILI9481_Off;
    lcdOnFunction = LCD_ILI9481_On;
    lcdPixelClock = 12000000;
  } else if (LCD_ILI9486_ReadID() == LCD_ILI9486_ID) {
    TRACE("LCD INIT: ILI9486");
    boardLcdType = "ILI9486";
    lcdInitFunction = LCD_ILI9486_Init;
    lcdOffFunction = LCD_ILI9486_Off;
    lcdOnFunction = LCD_ILI9486_On;
    lcdPixelClock = 12000000;
  } else if (LCD_ILI9488_ReadID() == LCD_ILI9488_ID) {
    TRACE("LCD INIT: ILI9488");
    boardLcdType = "ILI9488";
    lcdInitFunction = LCD_ILI9488_Init;
    lcdOffFunction = LCD_ILI9488_Off;
    lcdOnFunction = LCD_ILI9488_On;
    lcdPixelClock = 12000000;
    lcd_phys_w = LCD_PHYS_H;
    lcd_phys_h = LCD_PHYS_W;
  } else if (LCD_HX8357D_ReadID() == LCD_HX8357D_ID) {
    TRACE("LCD INIT: HX8357D");
    boardLcdType = "HX8357D";
    lcdInitFunction = LCD_HX8357D_Init;
    lcdOffFunction = LCD_HX8357D_Off;
    lcdOnFunction = LCD_HX8357D_On;
    lcdPixelClock = 12000000;
  } else if (LCD_ST7796S_ReadID() == LCD_ST7796S_ID ) {
    TRACE("LCD INIT: ST7796S");
    boardLcdType = "ST7796S";
    lcdInitFunction = LCD_ST7796S_Init;
    lcdOffFunction = LCD_ST7796S_Off;
    lcdOnFunction = LCD_ST7796S_On;
    lcdPixelClock = 14500000;
  }
  else{
    TRACE("LCD INIT (default): ST7796S");
    boardLcdType = "ST7796S (Default)";
    lcdInitFunction = LCD_ST7796S_Init;
    lcdOffFunction = LCD_ST7796S_Off;
    lcdOnFunction = LCD_ST7796S_On;
    lcdPixelClock = 12000000;
  }

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

