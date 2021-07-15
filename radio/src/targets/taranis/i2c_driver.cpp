/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "board.h"

void eepromPageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite);
void eepromWaitEepromStandbyState(void);

void i2cInit()
{
  I2C_DeInit(I2C);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = I2C_WP_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_WP_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(I2C_WP_GPIO, I2C_WP_GPIO_PIN);

  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_ClockSpeed = I2C_CLK_RATE;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C, &I2C_InitStructure);
  I2C_Cmd(I2C, ENABLE);

  GPIO_PinAFConfig(I2C_SPI_GPIO, I2C_SCL_GPIO_PinSource, I2C_GPIO_AF);
  GPIO_PinAFConfig(I2C_SPI_GPIO, I2C_SDA_GPIO_PinSource, I2C_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_SPI_GPIO, &GPIO_InitStructure);
}

#define I2C_TIMEOUT_MAX 1000
bool I2C_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C_WaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

/**
  * @brief  Reads a block of data from the EEPROM.
  * @param  pBuffer : pointer to the buffer that receives the data read
  *   from the EEPROM.
  * @param  ReadAddr : EEPROM's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the EEPROM.
  * @retval None
  */
bool I2C_EE_ReadBlock(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, I2C_ADDRESS_EEPROM, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return false;

  I2C_SendData(I2C, (uint8_t)((ReadAddr & 0xFF00) >> 8));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;
  I2C_SendData(I2C, (uint8_t)(ReadAddr & 0x00FF));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, I2C_ADDRESS_EEPROM, I2C_Direction_Receiver);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    return false;

  if (NumByteToRead > 1) {
    I2C_AcknowledgeConfig(I2C, ENABLE);
  }

  while (NumByteToRead) {
    if (NumByteToRead == 1) {
      I2C_AcknowledgeConfig(I2C, DISABLE);
    }
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
      return false;
    *pBuffer++ = I2C_ReceiveData(I2C);
    NumByteToRead--;
  }

  I2C_GenerateSTOP(I2C, ENABLE);
  return true;
}

void eepromReadBlock(uint8_t * buffer, size_t address, size_t size)
{
  while (!I2C_EE_ReadBlock(buffer, address, size)) {
    i2cInit();
  }
}

/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  buffer : pointer to the buffer containing the data to be
  *   written to the EEPROM.
  * @param  address : EEPROM's internal address to write to.
  * @param  size : number of bytes to write to the EEPROM.
  * @retval None
  */
void eepromWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  uint8_t offset = address % I2C_FLASH_PAGESIZE;
  uint8_t count = I2C_FLASH_PAGESIZE - offset;
  if (size < count) {
    count = size;
  }
  while (count > 0) {
    eepromPageWrite(buffer, address, count);
    eepromWaitEepromStandbyState();
    address += count;
    buffer += count;
    size -= count;
    count = I2C_FLASH_PAGESIZE;
    if (size < I2C_FLASH_PAGESIZE) {
      count = size;
    }
  }
}

uint8_t eepromIsTransferComplete()
{
  return 1;
}

/**
  * @brief  Writes more than one byte to the EEPROM with a single WRITE cycle.
  * @note   The number of byte can't exceed the EEPROM page size.
  * @param  pBuffer : pointer to the buffer containing the data to be
  *   written to the EEPROM.
  * @param  WriteAddr : EEPROM's internal address to write to.
  * @param  NumByteToWrite : number of bytes to write to the EEPROM.
  * @retval None
  */
bool I2C_EE_PageWrite(uint8_t * pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, I2C_ADDRESS_EEPROM, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return false;

  I2C_SendData(I2C, (uint8_t)((WriteAddr & 0xFF00) >> 8));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;
  I2C_SendData(I2C, (uint8_t)(WriteAddr & 0x00FF));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;

  /* While there is data to be written */
  while (NumByteToWrite--) {
    I2C_SendData(I2C, *pBuffer);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
      return false;
    pBuffer++;
  }

  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return false;

  I2C_GenerateSTOP(I2C, ENABLE);
  return true;
}

void eepromPageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
  while (!I2C_EE_PageWrite(pBuffer, WriteAddr, NumByteToWrite)) {
    i2cInit();
  }
}

/**
  * @brief  Wait for EEPROM Standby state
  * @param  None
  * @retval None
  */
bool I2C_EE_WaitEepromStandbyState(void)
{
  do {
    I2C_GenerateSTART(I2C, ENABLE);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
      return false;

    I2C_Send7bitAddress(I2C, I2C_ADDRESS_EEPROM, I2C_Direction_Transmitter);
  } while (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_GenerateSTOP(I2C, ENABLE);
  return true;
}

void eepromWaitEepromStandbyState(void)
{
  while (!I2C_EE_WaitEepromStandbyState()) {
    i2cInit();
  }
}

#if !defined(BOOT) && !defined(SOFTWARE_VOLUME)
const uint8_t volumeScale[VOLUME_LEVEL_MAX+1] = {
  0,  1,  2,  3,  5,  9,  13,  17,  22,  27,  33,  40,
  64, 82, 96, 105, 112, 117, 120, 122, 124, 125, 126, 127
};

void setScaledVolume(uint8_t volume)
{
  if (volume > VOLUME_LEVEL_MAX) {
    volume = VOLUME_LEVEL_MAX;
  }

  setVolume(volumeScale[volume]);
}

void setVolume(uint8_t volume)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return;

  I2C_Send7bitAddress(I2C, I2C_ADDRESS_VOLUME, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return;

  I2C_SendData(I2C, 0);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return;
  I2C_SendData(I2C, volume);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return;

  I2C_GenerateSTOP(I2C, ENABLE);
}

int32_t getVolume()
{
  return -1; // TODO
}
#endif
