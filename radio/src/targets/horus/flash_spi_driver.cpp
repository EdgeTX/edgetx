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

#include "opentx.h"

#if !defined(SIMU)

#define SPI_MODE        	           0x00
#define SPI_STATUS      	           0x01
#define SPI_BASS        	           0x02
#define SPI_CLOCKF      	           0x03
#define SPI_DECODE_TIME 	           0x04
#define SPI_AUDATA      	           0x05
#define SPI_WRAM        	           0x06
#define SPI_WRAMADDR    	           0x07
#define SPI_HDAT0       	           0x08
#define SPI_HDAT1       	           0x09
#define SPI_AIADDR      	           0x0a
#define SPI_VOL         	           0x0b
#define SPI_AICTRL0     	           0x0c
#define SPI_AICTRL1     	           0x0d
#define SPI_AICTRL2     	           0x0e
#define SPI_AICTRL3     	           0x0f

#define SPI_SPEED_2                    0
#define SPI_SPEED_4                    1
#define SPI_SPEED_8                    2
#define SPI_SPEED_16                   3
#define SPI_SPEED_32                   4
#define SPI_SPEED_64                   5
#define SPI_SPEED_128                  6
#define SPI_SPEED_256                  7


// * RadioMaster/Eachine TX16S, RadioKing TX18S and Jumper T18 use GD25Q127C (16 MByte)
// * FlySky PL18, Paladin EV and NV14 use WinBond W25Q64JV (8 MByte)

struct SerialFlashDescriptor
{
	uint16_t id;

};

void flashSpiInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef SPI_InitStructure;

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MISO_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(FLASH_SPI_MISO_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_GPIO_PIN;
  GPIO_Init(FLASH_SPI_SCK_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MOSI_GPIO_PIN;
  GPIO_Init(FLASH_SPI_MOSI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(FLASH_SPI_CS_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(FLASH_SPI_SCK_GPIO, FLASH_SPI_SCK_GPIO_PinSource, FLASH_SPI_GPIO_AF);
  GPIO_PinAFConfig(FLASH_SPI_MISO_GPIO, FLASH_SPI_MISO_GPIO_PinSource, FLASH_SPI_GPIO_AF);
  GPIO_PinAFConfig(FLASH_SPI_MOSI_GPIO, FLASH_SPI_MOSI_GPIO_PinSource, FLASH_SPI_GPIO_AF);

  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  SPI_I2S_DeInit(FLASH_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(FLASH_SPI, &SPI_InitStructure);
  SPI_Cmd(FLASH_SPI, ENABLE);

  SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_RXNE);
  SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_TXE);

}

void flashSpiSetSpeed(uint8_t speed)
{
  FLASH_SPI->CR1 &= 0xFFC7; // Fsck=Fcpu/256
  switch (speed) {
    case SPI_SPEED_2:
      FLASH_SPI->CR1 |= 0x00 << 3; // Fsck=Fpclk/2=36Mhz
      break;
    case SPI_SPEED_4:
      FLASH_SPI->CR1 |= 0x01 << 3; // Fsck=Fpclk/4=18Mhz
      break;
    case SPI_SPEED_8:
      FLASH_SPI->CR1 |= 0x02 << 3; // Fsck=Fpclk/8=9Mhz
      break;
    case SPI_SPEED_16:
      FLASH_SPI->CR1 |= 0x03 << 3; // Fsck=Fpclk/16=4.5Mhz
      break;
    case SPI_SPEED_32:
      FLASH_SPI->CR1 |= 0x04 << 3; // Fsck=Fpclk/32=2.25Mhz
      break;
    case SPI_SPEED_64:
      FLASH_SPI->CR1 |= 0x05 << 3; // Fsck=Fpclk/16=1.125Mhz
      break;
    case SPI_SPEED_128:
      FLASH_SPI->CR1 |= 0x06 << 3; // Fsck=Fpclk/16=562.5Khz
      break;
    case SPI_SPEED_256:
      FLASH_SPI->CR1 |= 0x07 << 3; // Fsck=Fpclk/16=281.25Khz
      break;
  }
  FLASH_SPI->CR1 |= 0x01 << 6;
}

uint8_t flashSpiReadWriteByte(uint8_t value)
{
   uint16_t time_out = 0x0FFF;
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET) {
    if (--time_out == 0) {
      // reset SPI
      SPI_Cmd(FLASH_SPI, DISABLE);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_OVR);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_BSY);
      SPI_I2S_ClearFlag(FLASH_SPI, I2S_FLAG_UDR);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_TIFRFE);
      SPI_Cmd(FLASH_SPI, ENABLE);
      break;
    }
  }
  SPI_I2S_SendData(FLASH_SPI, value);

  time_out = 0x0FFF;
  while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET) {
    if (--time_out == 0) {
      // reset SPI
      SPI_Cmd(FLASH_SPI, DISABLE);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_OVR);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_BSY);
      SPI_I2S_ClearFlag(FLASH_SPI, I2S_FLAG_UDR);
      SPI_I2S_ClearFlag(FLASH_SPI, SPI_I2S_FLAG_TIFRFE);
      SPI_Cmd(FLASH_SPI, ENABLE);
      break;
    }
  }
  return SPI_I2S_ReceiveData(FLASH_SPI);
}

#define XDCS_HIGH() {}
#define CS_HIGH()                      do { FLASH_SPI_CS_GPIO->BSRRL = FLASH_SPI_CS_GPIO_PIN; } while (0)
#define CS_LOW()                       do { FLASH_SPI_CS_GPIO->BSRRH = FLASH_SPI_CS_GPIO_PIN; } while (0)
#define XDCS_LOW() {}

uint16_t flashSpiReadID()
{
  CS_LOW();
  flashSpiReadWriteByte(0x90);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  uint16_t result = flashSpiReadWriteByte(0xff) << 8;
  result += flashSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  CS_HIGH();
  return result;
}



uint16_t flashSpiReadReg(uint8_t address)
{
  CS_LOW();
  flashSpiReadWriteByte(address);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  uint16_t result = flashSpiReadWriteByte(0xff) << 8;
  result += flashSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  CS_HIGH();
  return result;
}

uint16_t flashSpiReadCmd(uint8_t address)
{
  CS_LOW();
//  flashSpiReadWriteByte(VS_READ_COMMAND);
//  flashSpiReadWriteByte(address);
//  uint16_t result = flashSpiReadWriteByte(0) << 8;
//  result |= flashSpiReadWriteByte(0);
//  delay_01us(50); // 5us
  CS_HIGH();
//  return result;
  return 0;
}

uint8_t flashSpiWriteCmd(uint8_t address, uint16_t data)
{
  CS_LOW();
//  flashSpiReadWriteByte(VS_WRITE_COMMAND);
//  flashSpiReadWriteByte(address);
//  flashSpiReadWriteByte(data >> 8);
//  flashSpiReadWriteByte(data);
//  delay_01us(50); // 5us
  CS_HIGH();
  return 1;
}

uint32_t flashSpiWriteData(const uint8_t * buffer, uint32_t size)
{
  XDCS_LOW();

  uint32_t index = 0;
/*  while (index < size && READ_DREQ() != 0) {
    for (int i=0; i<MP3_BUFFER_SIZE && index<size; i++) {
      flashSpiReadWriteByte(buffer[index++]);
    }
  }*/
  return index;
}

void flashSpiWriteBuffer(const uint8_t * buffer, uint32_t size)
{
  const uint8_t * p = buffer;
  while (size > 0) {
    uint32_t written = flashSpiWriteData(p, size);
    p += written;
    size -= written;
  }
}
size_t flashSpiGetSize()
{
	return 16 * 1024 * 1024;
}

size_t flashSpiRead(size_t address, uint8_t* data, size_t size)
{
  size = std::min(size, (size_t)(flashSpiGetSize() - address));
  CS_LOW();
  flashSpiReadWriteByte(0x03);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);
  for(int i = 0; i<size; i++)
    *data++ = flashSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  CS_HIGH();
  return size;
}

size_t flashSpiWrite(size_t address, const uint8_t* data, size_t size)
{
  size = std::min(size, (size_t)(flashSpiGetSize() - address));
  if(size != 256)
	  return -1;
  CS_LOW();
  flashSpiReadWriteByte(0x06);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();

  flashSpiReadWriteByte(0x02);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);
  for(int i = 0; i<size; i++)
    flashSpiReadWriteByte(*data++);
  delay_01us(100); // 10us
  CS_HIGH();

  delay_01us(100); // 10us
  CS_LOW();
  while(true)
  {
    uint8_t status = flashSpiReadWriteByte(0x05);
    if((status & 0x01) == 0)
      break;
  }
  CS_HIGH();
  return size;
}

int flashSpiErase(size_t address)
{
  if(address%4096 != 0)
    return -1;

  CS_LOW();
  flashSpiReadWriteByte(0x06);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();
  flashSpiReadWriteByte(0x20);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us

  CS_LOW();
  while(true)
  {
    uint8_t status = flashSpiReadWriteByte(0x05);
    if((status & 0x01) == 0)
      break;
  }
  CS_HIGH();
  return 0;
}

void flashSpiEraseAll()
{
  CS_LOW();
  flashSpiReadWriteByte(0x06);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us

  CS_LOW();
  flashSpiReadWriteByte(0x60);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us

  CS_LOW();
  while(true)
  {
    uint8_t status = flashSpiReadWriteByte(0x05);
    if((status & 0x01) == 0)
      break;
  }
  CS_HIGH();
}

void flashInit()
{
  flashSpiInit();
  flashSpiSetSpeed(SPI_SPEED_2);
  delay_ms(1); // 1ms
}

#endif
