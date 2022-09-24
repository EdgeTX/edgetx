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

#include "opentx_types.h"
#include "libopenui_config.h"
#include "lcd.h"

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  LTDC_Layer1->CFBAR = (uint32_t)buffer;

  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;

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


void GPIO_SetDirection( GPIO_TypeDef *GPIOx, unsigned char Pin, unsigned char IsInput )
{
  unsigned int Mask;
  unsigned int Position;
  unsigned int Register;


  Position = Pin << 1;
  Mask = ~( 0x03UL << Position );

  //EnterCritical();
  Register = GPIOx->OSPEEDR & Mask;
  Register |= IO_SPEED_HIGH << Position;
  GPIOx->OSPEEDR = Register;
  //ExitCritical();

  //EnterCritical();
  Register = GPIOx->MODER & Mask;
  if( !IsInput )
  {
      Register |= IO_MODE_OUTPUT << Position;
  }

  GPIOx->MODER = Register;
  //ExitCritical();
}
static void LCD_AF_GPIOConfig(void) {
  /*
   -----------------------------------------------------------------------------
   LCD_CLK <-> PG.07 | LCD_HSYNC <-> PI.12 | LCD_R3 <-> PJ.02 | LCD_G5 <-> PK.00
   | LCD VSYNC <-> PI.13 | LCD_R4 <-> PJ.03 | LCD_G6 <-> PK.01
   |                     | LCD_R5 <-> PJ.04 | LCD_G7 <-> PK.02
   |                     | LCD_R6 <-> PJ.05 | LCD_B4 <-> PK.03
   |                     | LCD_R7 <-> PJ.06 | LCD_B5 <-> PK.04
   |                     | LCD_G2 <-> PJ.09 | LCD_B6 <-> PK.05
   |                     | LCD_G3 <-> PJ.10 | LCD_B7 <-> PK.06
   |                     | LCD_G4 <-> PJ.11 | LCD_DE <-> PK.07
   |                     | LCD_B3 <-> PJ.15 |
   */

  // GPIOG configuration
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  // GPIOI configuration
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource12, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource13, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_Init(GPIOI, &GPIO_InitStructure);

  // GPIOJ configuration
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource15, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  // GPIOK configuration
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource0, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource1, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOK, &GPIO_InitStructure);
}

static void lcdSpiConfig(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_GPIO_PIN | LCD_SPI_MOSI_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_SPI_CS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NRST_GPIO_PIN;
  GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);

  /* Set the chip select pin aways low */
  CLR_LCD_CS();
}

void lcdDelay() {
  delay_01us(1);
}

unsigned char LCD_ReadByteOnFallingEdge(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  SET_LCD_DATA();
  SET_LCD_DATA_INPUT();

  for (i = 0; i < 8; i++) {
    LCD_DELAY();
    SET_LCD_CLK();
    LCD_DELAY();
    LCD_DELAY();
    ReceiveData <<= 1;

    CLR_LCD_CLK();
    LCD_DELAY();
    LCD_DELAY();
    if (READ_LCD_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }

  SET_LCD_DATA_OUTPUT();

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

  SET_LCD_DATA();
  SET_LCD_DATA_INPUT();
  for (i = 0; i < 8; i++) {
    CLR_LCD_CLK();
    LCD_DELAY();
    LCD_DELAY();
    ReceiveData <<= 1;
    SET_LCD_CLK();
    LCD_DELAY();
    LCD_DELAY();
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

  lcdWriteByte(0, Register);
  LCD_DELAY();
  LCD_DELAY();
  ReadData = LCD_ReadByte();
  return (ReadData);
}

void lcdWriteCommand(uint8_t command) {
  lcdWriteByte(0, command);
}

void lcdWriteData(uint8_t data) {
  lcdWriteByte(1, data);
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
  lcdWriteData(0x08);

  lcdWriteCommand(0x29);
  delay_ms(10);
#else
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
  lcdWriteData(0x18);

  lcdWriteCommand(0x11); // SLPOUT
  delay_ms(200);

  lcdWriteCommand(0x29); // Display On
  delay_ms(25);
  lcdWriteCommand(0x2C);

#endif

}

void LCD_HX8357D_On(void) {
  lcdWriteCommand(0x29);
  lcdWriteCommand(0x22);
}

void LCD_HX8357D_Off(void) {
  lcdWriteCommand(0x22);
  lcdWriteCommand(0x28);
}

unsigned int LCD_HX8357D_ReadID(void) {
  int ID = 0;

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

  LCD_DELAY();
  LCD_DELAY();
  LCD_DELAY();

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
  lcdWriteCommand(0x29);
  lcdWriteCommand(0x23); //all pixels on
}

void LCD_ILI9488_Init(void) {
  lcdWriteCommand(0XFB);
  lcdWriteData(0x00);

  lcdWriteCommand(0XF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x11);
  lcdWriteData(0x09);

  lcdWriteCommand(0xC1);
  lcdWriteData(0x41);

  lcdWriteCommand(0XC5);
  lcdWriteData(0x00);
  lcdWriteData(0x0A);
  lcdWriteData(0x80);

  lcdWriteCommand(0xB1);
  lcdWriteData(0xB0);
  lcdWriteData(0x11);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x02);

  lcdWriteCommand(0xB6);
  lcdWriteData(0x30);
  lcdWriteData(0x02);

  lcdWriteCommand(0xB7);
  lcdWriteData(0xc6);

  lcdWriteCommand(0xBE);
  lcdWriteData(0x00);
  lcdWriteData(0x04);

  lcdWriteCommand(0xE9);
  lcdWriteData(0x00);

  lcdWriteCommand(0x36);
  lcdWriteData(0x08);

  lcdWriteCommand(0x3A);
  lcdWriteData(0x65);

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

  lcdWriteCommand(0XE1);
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

  lcdWriteCommand(0x11);
  delay_ms(120);
  lcdWriteCommand(0x28);

  LCD_ILI9488_On();
}

void LCD_ILI9488_Off(void) {
  lcdWriteCommand(0x22); //all pixels off
  lcdWriteCommand(0x28);
}

void LCD_ILI9488_ReadDevice(void) {
  int Index = 0;
  int Parameter = 0x80;

#if 1

#if 1
  lcdWriteCommand(0XF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);

  lcdWriteCommand(0XB0);
  lcdWriteData(0X80);

#endif
  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x00);
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  //lcdWriteCommand(0X2E);
  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x01);        //Parameter2=0X88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x02);        //Parameter2=0X88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(Parameter | 0x03);        //Parameter2=0X88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister(0xd3);
#endif

#if 0
  lcdWriteCommand( 0XFB );
  lcdWriteData( Parameter|0x00 );        //Parameter3=0X94
  LCD_ReadBuffer[Index++] = LCD_ReadRegister( 0xd3 );
  lcdWriteData( Parameter|0x01 );//Parameter3=0X94
  LCD_ReadBuffer[Index++] = LCD_ReadRegister( 0xd3 );
  lcdWriteCommand( 0XFB );
  lcdWriteData( Parameter|0x02 );//Parameter3=0X94
  LCD_ReadBuffer[Index++] = LCD_ReadRegister( 0xd3 );

  lcdWriteCommand( 0XFB );
  lcdWriteData( Parameter|0x03 );//Parameter4=0X88
  LCD_ReadBuffer[Index++] = LCD_ReadRegister( 0xd3 );
#else
  //lcdWriteCommand( 0xd0 );
  //lcdWriteData( Parameter|0x03 );        //Parameter4=0X88
  //LCD_ReadBuffer[Index++] = LCD_ReadRegister( 0xd0 );
#endif
}

unsigned int LCD_ILI9488_ReadID(void) {
  int ID = 0;

  lcdWriteCommand(0XF7);
  lcdWriteData(0xA9);
  lcdWriteData(0x51);
  lcdWriteData(0x2C);
  lcdWriteData(0x82);
  lcdWriteCommand(0XB0);
  lcdWriteData(0X80);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x80 | 0x00);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x80 | 0x01);
  ID = LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
  lcdWriteData(0x80 | 0x02);
  ID = LCD_ReadRegister(0xd3);
  ID <<= 8;

  lcdWriteCommand(0XFB);
  lcdWriteData(0x80 | 0x03);
  ID |= LCD_ReadRegister(0xd3);

  lcdWriteCommand(0XFB);
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
  lcdWriteData( 0x88 );

  lcdWriteCommand( 0x3A );
  lcdWriteData( 0x66 );

  //SET RGB STRAT
  lcdWriteCommand (0xB0 );   //SET HS VS DE CLK 上升还是下降有效
  lcdWriteData( 0x80 );

  lcdWriteCommand( 0xB4 );
  lcdWriteData( 0x01 );

  lcdWriteCommand( 0xB6 );
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

  lcdWriteCommand( 0x21 );

  LCD_ST7796S_On();
}

void LCD_ST7796S_Off(void) {
  lcdWriteCommand(0x28);
}

unsigned int LCD_ST7796S_ReadID(void) {
  unsigned int ID = 0;

  lcdWriteCommand( 0XF0 );
  lcdWriteData( 0XC3 );
  lcdWriteCommand( 0XF0 );
  lcdWriteData( 0X96 );

  lcdWriteCommand( 0XB0 );
  lcdWriteData( 0X80 );

  lcdWriteCommand( 0XD3 );

  SET_LCD_CLK_OUTPUT();
  SET_LCD_DATA_INPUT();
  CLR_LCD_CLK();
  LCD_DELAY();
  LCD_DELAY();
  SET_LCD_CLK();
  LCD_DELAY();
  LCD_DELAY();

  LCD_ReadByte();
  ID += (uint16_t)(LCD_ReadByte())<<8;
  ID += LCD_ReadByte();

   return (ID);
 }

static void lcdReset() {
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(100);

  LCD_NRST_HIGH();
  delay_ms(100);
}

void LCD_Init_LTDC() {
  LTDC_InitTypeDef LTDC_InitStruct;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock Mhz */
  uint32_t clock = (lcdPixelClock*16) / 1000000; // clock*16 in MHz
  RCC_PLLSAIConfig(clock, 6, 4);
  RCC_LTDCCLKDivConfig (RCC_PLLSAIDivR_Div4);

  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);

  /* Wait for PLLSAI activation */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET);

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

  /* Configure R,G,B component values for LCD background color */
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;

  /* Configure horizontal synchronization width */
  LTDC_InitStruct.LTDC_HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = VSH;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  LTDC_InitStruct.LTDC_AccumulatedActiveW = LCD_PHYS_W + HBP;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = LCD_PHYS_H + VBP;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = LCD_PHYS_W + HBP + HFP;
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = LCD_PHYS_H + VBP + VFP;

  LTDC_Init(&LTDC_InitStruct);

  // Configure IRQ (line)
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = LTDC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LTDC_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Trigger on last line
  LTDC_LIPConfig(LCD_PHYS_H);
  LTDC_ITConfig(LTDC_IER_LIE, ENABLE);

#if 0
  DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2D_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_SCREEN_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
  DMA2D->IFCR = (unsigned long)DMA2D_IFSR_CTCIF;
#endif
}

void LCD_LayerInit() {
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
   Horizontal start = horizontal synchronization + Horizontal back porch = 30
   Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
   Vertical start   = vertical synchronization + vertical back porch     = 4
   Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = HBP + 1;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_PHYS_W + HBP);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = VBP + 1;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_PHYS_H + VBP);

  /* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
  /* Default Color configuration (configure A,R,G,B component values) */
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;

  /* Configure blending factors */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  /* the length of one line of pixels in bytes + 3 then :
   Line Lenth = Active high width x number of bytes per pixel + 3
   Active high width         = LCD_W
   number of bytes per pixel = 2    (pixel_format : RGB565)
   */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PHYS_W * 2) + 3);
  /* the pitch is the increment from the start of one line of pixels to the
   start of the next line in bytes, then :
   Pitch = Active high width x number of bytes per pixel */
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_PHYS_W * 2);

  /* Configure the number of lines */
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_PHYS_H;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  uint32_t layer_address = (uint32_t)lcdFront->getData();
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = layer_address;

  /* Initialize LTDC layer 1 */
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  /* LTDC configuration reload */
  LTDC_ReloadConfig(LTDC_IMReload);

  LTDC_LayerCmd(LTDC_Layer1, ENABLE);
  LTDC_LayerAlpha(LTDC_Layer1, 255);

  LTDC_ReloadConfig(LTDC_IMReload);

  /* dithering activation */
  LTDC_DitherCmd(ENABLE);
}

const char* boardLcdType = "";

void lcdInit(void)
{
  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  // TODO: init lVGL
  
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
  } else if (LCD_HX8357D_ReadID() == LCD_HX8357D_ID) {
    TRACE("LCD INIT: HX8357D");
    boardLcdType = "HX8357D";
    lcdInitFunction = LCD_HX8357D_Init;
    lcdOffFunction = LCD_HX8357D_Off;
    lcdOnFunction = LCD_HX8357D_On;
    lcdPixelClock = 12000000;
  } else { /* if (LCD_ST7796S_ReadID() == LCD_ST7796S_ID) { */ // ST7796S detection is unreliable
    TRACE("LCD INIT (default): ST7796S");
    boardLcdType = "ST7796S";
    lcdInitFunction = LCD_ST7796S_Init;
    lcdOffFunction = LCD_ST7796S_Off;
    lcdOnFunction = LCD_ST7796S_On;
    lcdPixelClock = 14500000;
/*  } else {
    TRACE("LCD INIT: unknown LCD controller");
    boardLcdType = "unknown";*/
  }

  lcdInitFunction();

  LCD_Init_LTDC();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
  LTDC_ReloadConfig(LTDC_IMReload);

  lcdSetFlushCb(startLcdRefresh);
}

void DMAWait()
{
  while(DMA2D->CR & DMA2D_CR_START);
}
  
void DMACopyBitmap(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                   uint16_t y, const uint16_t *src, uint16_t srcw,
                   uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                   uint16_t h)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

void DMACopyAlphaBitmap(uint16_t *dest, uint16_t destw, uint16_t desth,
                        uint16_t x, uint16_t y, const uint16_t *src,
                        uint16_t srcw, uint16_t srch, uint16_t srcx,
                        uint16_t srcy, uint16_t w, uint16_t h)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB4444;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  DMA2D_BG_InitTypeDef DMA2D_BG_InitStruct;
  DMA2D_BG_StructInit(&DMA2D_BG_InitStruct);
  DMA2D_BG_InitStruct.DMA2D_BGMA = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_BG_InitStruct.DMA2D_BGO = destw - w;
  DMA2D_BG_InitStruct.DMA2D_BGCM = CM_RGB565;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_VALUE = 0;
  DMA2D_BGConfig(&DMA2D_BG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

// same as DMACopyAlphaBitmap(), but with an 8 bit mask for each pixel (used by fonts)
void DMACopyAlphaMask(uint16_t *dest, uint16_t destw, uint16_t desth,
                      uint16_t x, uint16_t y, const uint8_t *src, uint16_t srcw,
                      uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                      uint16_t h, uint16_t bg_color)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND;
  DMA2D_InitStruct.DMA2D_CMode = CM_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_A8; // 8 bit inputs every time
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FG_InitStruct.DMA2D_FGC_RED   = GET_RED(bg_color);   // 8 bit red
  DMA2D_FG_InitStruct.DMA2D_FGC_GREEN = GET_GREEN(bg_color); // 8 bit green
  DMA2D_FG_InitStruct.DMA2D_FGC_BLUE  = GET_BLUE(bg_color);  // 8 bit blue

  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  DMA2D_BG_InitTypeDef DMA2D_BG_InitStruct;
  DMA2D_BG_StructInit(&DMA2D_BG_InitStruct);
  DMA2D_BG_InitStruct.DMA2D_BGMA = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_BG_InitStruct.DMA2D_BGO = destw - w;
  DMA2D_BG_InitStruct.DMA2D_BGCM = CM_RGB565;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_VALUE = 0;
  DMA2D_BGConfig(&DMA2D_BG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_PFC;
  DMA2D_InitStruct.DMA2D_CMode = format;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = 0;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src);
  DMA2D_FG_InitStruct.DMA2D_FGO = 0;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB8888;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = REPLACE_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

extern "C" void LTDC_IRQHandler(void)
{
  // clear interrupt flag
  LTDC->ICR = LTDC_ICR_CLIF;
  _frame_addr_reloaded = 1;
}
