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

#include "rtos.h"
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

#define CS_HIGH()                      do { FLASH_SPI_CS_GPIO->BSRRL = FLASH_SPI_CS_GPIO_PIN; } while (0)
#define CS_LOW()                       do { FLASH_SPI_CS_GPIO->BSRRH = FLASH_SPI_CS_GPIO_PIN; } while (0)


struct SpiFlashDescriptor
{
  uint16_t id;
  uint32_t pageSize;
  uint32_t sectorSize;
  uint32_t blockSize;
  uint32_t blockCount;

  uint8_t readStatusCmd;
  uint8_t readCmd;
  uint8_t writeCmd;
  uint8_t writeEnableCmd;
  uint8_t eraseSectorCmd;
  uint8_t eraseBlockCmd;
  uint8_t eraseChipCmd;
};

// * RadioMaster/Eachine TX16S, RadioKing TX18S and Jumper T18 use GD25Q127C (16 MByte)
// * FlySky PL18, Paladin EV and NV14 use WinBond W25Q64JV (8 MByte)

static const SpiFlashDescriptor spiFlashDescriptors[] =
{
    { // GD25Q127C
        .id = 0xC817,
        .pageSize = 256,
        .sectorSize = 4096,
        .blockSize = 65536,
        .blockCount = 256,

        .readStatusCmd = 0x05,
        .readCmd = 0x03,
        .writeCmd = 0x02,
        .writeEnableCmd = 0x06,
        .eraseSectorCmd = 0x20,
        .eraseBlockCmd = 0x52,
        .eraseChipCmd = 0x60
    },
    { // W25Q64JV
        .id = 0xEF16,
        .pageSize = 256,
        .sectorSize = 4096,
        .blockSize = 65536,
        .blockCount = 128,

        .readStatusCmd = 0x05,
        .readCmd = 0x03,
        .writeCmd = 0x02,
        .writeEnableCmd = 0x06,
        .eraseSectorCmd = 0x20,
        .eraseBlockCmd = 0x52,
        .eraseChipCmd = 0xC7
    }
};

static const SpiFlashDescriptor* flashDescriptor = nullptr;
static DMA_InitTypeDef dmaTxInfo = {0};
static DMA_InitTypeDef dmaRxInfo =  {0};

static RTOS_SEMAPHORE_HANDLE irqSem;
static uint8_t *dmaReadBuf = nullptr;
static uint8_t *dmaWriteBuf = nullptr;
static volatile bool reading = false;

void flashSpiInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef SPI_InitStructure;

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MISO_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(FLASH_SPI_MISO_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_GPIO_PIN;
  GPIO_Init(FLASH_SPI_SCK_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MOSI_GPIO_PIN;
  GPIO_Init(FLASH_SPI_MOSI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(FLASH_SPI_CS_GPIO, &GPIO_InitStructure);
  CS_HIGH();

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

void flashSpiSync()
{
  delay_01us(100);
  CS_LOW();
  while(true)
  {
    uint8_t status = flashSpiReadWriteByte(flashDescriptor->readStatusCmd);
    if((status & 0x01) == 0)
      break;
  }
  CS_HIGH();
  delay_01us(100);
}

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

size_t flashSpiGetSize()
{
	return flashDescriptor->blockSize * flashDescriptor->blockCount;
}

size_t flashSpiRead(size_t address, uint8_t* data, size_t size)
{
  static char buf __DMA = 0;

  flashSpiSync();

  size = std::min(size, (size_t)(flashSpiGetSize() - address));


  CS_LOW();

  flashSpiReadWriteByte(flashDescriptor->readCmd);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);

//  reading = true;
//
//  dmaRxInfo.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(data);
//  dmaRxInfo.DMA_BufferSize = size;
//  DMA_Init(FLASH_SPI_RX_DMA_STREAM, &dmaRxInfo);
//  DMA_Cmd(FLASH_SPI_RX_DMA_STREAM, ENABLE);
//
//  dmaTxInfo.DMA_MemoryInc = DMA_MemoryInc_Disable;
//  dmaTxInfo.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(buf);
//  dmaTxInfo.DMA_BufferSize = size;
//  DMA_Init(FLASH_SPI_TX_DMA_STREAM, &dmaTxInfo);
//  DMA_Cmd(FLASH_SPI_TX_DMA_STREAM, ENABLE);
//  DMA_ITConfig(FLASH_SPI_RX_DMA_STREAM, DMA_IT_TC, ENABLE);
//  DMA_ITConfig(FLASH_SPI_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
//  SPI_I2S_DMACmd(FLASH_SPI, SPI_I2S_DMAReq_Rx|SPI_I2S_DMAReq_Tx, ENABLE);
//
//  RTOS_TAKE_SEMAPHORE(irqSem);

  for(size_t i=0; i < size; i++)
    *data++ = flashSpiReadWriteByte(0xFF);

  delay_01us(100); // 10us
  CS_HIGH();

//  DMA_DeInit(FLASH_SPI_RX_DMA_STREAM);
//  DMA_DeInit(FLASH_SPI_TX_DMA_STREAM);

//  reading = false;

  return size;
}

size_t flashSpiWrite(size_t address, const uint8_t* data, size_t size)
{
  size = std::min(size, (size_t)(flashSpiGetSize() - address));
  if(size != flashDescriptor->pageSize || address%flashDescriptor->pageSize != 0)
	  return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();

  flashSpiReadWriteByte(flashDescriptor->writeCmd);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);

//  dmaTxInfo.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  dmaTxInfo.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(data);
//  dmaTxInfo.DMA_BufferSize = flashDescriptor->pageSize;
//  DMA_Init(FLASH_SPI_TX_DMA_STREAM, &dmaTxInfo);
//  DMA_Cmd(FLASH_SPI_TX_DMA_STREAM, ENABLE);
//  DMA_ITConfig(FLASH_SPI_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
//  SPI_I2S_DMACmd(FLASH_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
//
//  RTOS_TAKE_SEMAPHORE(irqSem);

  for(size_t i=0; i < size; i++)
    flashSpiReadWriteByte(*data++);

  delay_01us(100); // 10us
  CS_HIGH();

  DMA_DeInit(FLASH_SPI_RX_DMA_STREAM);
//  DMA_DeInit(FLASH_SPI_TX_DMA_STREAM);

  flashSpiSync();

  return size;
}

int flashSpiErase(size_t address)
{
  if(address%4096 != 0)
    return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseSectorCmd);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);
  delay_01us(100); // 10us
  CS_HIGH();

  flashSpiSync();

  return 0;
}

int flashSpiBlockErase(size_t address)
{
  if(address%32768 != 0)
    return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseBlockCmd);
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);
  delay_01us(100); // 10us
  CS_HIGH();

  flashSpiSync();

  return 0;
}

void flashSpiEraseAll()
{
  flashSpiSync();

  delay_01us(100); // 10us//

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseChipCmd);
  delay_01us(100); // 10us
  CS_HIGH();

  flashSpiSync();
}

uint16_t flashSpiGetPageSize()
{
  return flashDescriptor->pageSize;
}
uint16_t flashSpiGetSectorSize()
{
  return flashDescriptor->sectorSize;
}
uint16_t flashSpiGetSectorCount()
{
  return flashDescriptor->blockCount * (flashDescriptor->blockSize / flashDescriptor->sectorSize);
}

//extern "C" void FLASH_SPI_TX_DMA_IRQHandler(void)
//{
//  if (DMA_GetITStatus(FLASH_SPI_TX_DMA_STREAM, FLASH_SPI_TX_DMA_FLAG_TC))
//  {
//    DMA_ClearITPendingBit(FLASH_SPI_TX_DMA_STREAM, FLASH_SPI_TX_DMA_FLAG_TC);
//    if(!reading)
//      RTOS_GIVE_SEMAPHORE_ISR(irqSem);
//  }
//}
extern "C" void FLASH_SPI_RX_DMA_IRQHandler(void)
{
  if (DMA_GetITStatus(FLASH_SPI_RX_DMA_STREAM, FLASH_SPI_RX_DMA_FLAG_TC))
  {
    DMA_ClearITPendingBit(FLASH_SPI_RX_DMA_STREAM, FLASH_SPI_RX_DMA_FLAG_TC);
    RTOS_GIVE_SEMAPHORE_ISR(irqSem);
  }
}

static void flashSpiInitDMA()
{
  RTOS_CREATE_SEAPHORE(irqSem);
//  dmaReadBuf = (uint8_t*)aligned_alloc(4, flashDescriptor->pageSize);
//  dmaWriteBuf = (uint8_t*)aligned_alloc(4, flashDescriptor->pageSize);
//  DMA_DeInit(FLASH_SPI_TX_DMA_STREAM);
//  dmaTxInfo.DMA_Channel = FLASH_SPI_TX_DMA_CHANNEL;
//  dmaTxInfo.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&FLASH_SPI->DR);
//  dmaTxInfo.DMA_DIR = DMA_DIR_MemoryToPeripheral;
//  dmaTxInfo.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(dmaWriteBuf);
//  dmaTxInfo.DMA_BufferSize = flashDescriptor->pageSize;
//  dmaTxInfo.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  dmaTxInfo.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  dmaTxInfo.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//  dmaTxInfo.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//  dmaTxInfo.DMA_Mode = DMA_Mode_Normal;
//  dmaTxInfo.DMA_Priority = DMA_Priority_Low;
//  dmaTxInfo.DMA_FIFOMode = DMA_FIFOMode_Disable;
//  dmaTxInfo.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
//  dmaTxInfo.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//  dmaTxInfo.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
//
//  /* enable interrupt and set it's priority */
//  NVIC_EnableIRQ(FLASH_SPI_TX_DMA_IRQn);
//  NVIC_SetPriority(FLASH_SPI_TX_DMA_IRQn, 5);

  DMA_DeInit(FLASH_SPI_RX_DMA_STREAM);
  dmaRxInfo.DMA_Channel = FLASH_SPI_RX_DMA_CHANNEL;
  dmaRxInfo.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&FLASH_SPI->DR);
  dmaRxInfo.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dmaRxInfo.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(dmaReadBuf);
  dmaRxInfo.DMA_BufferSize = flashDescriptor->pageSize;
  dmaRxInfo.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaRxInfo.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaRxInfo.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaRxInfo.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaRxInfo.DMA_Mode = DMA_Mode_Normal;
  dmaRxInfo.DMA_Priority = DMA_Priority_Low;
  dmaRxInfo.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dmaRxInfo.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  dmaRxInfo.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dmaRxInfo.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

  /* enable interrupt and set it's priority */
  NVIC_EnableIRQ(FLASH_SPI_RX_DMA_IRQn);
  NVIC_SetPriority(FLASH_SPI_RX_DMA_IRQn, 5);

}

void flashInit()
{
  flashSpiInit();
  flashSpiSetSpeed(SPI_SPEED_4);
  delay_ms(1);

  uint16_t id = flashSpiReadID();
  for(size_t i = 0; i < sizeof(spiFlashDescriptors)/sizeof(SpiFlashDescriptor); i++)
  {
    if(spiFlashDescriptors[i].id == id)
    {
      flashDescriptor = &spiFlashDescriptors[i];
      break;
    }
  }
  if(flashDescriptor != nullptr)
    flashSpiInitDMA();
}

#endif
