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
#include "stm32_i2c_driver.h"
#include "stm32_gpio_driver.h"

#include "hal.h"
#include "hal/i2c_driver.h"
#include "hal/eeprom_driver.h"

static void init_wp_pin()
{
  stm32_gpio_enable_clock(EEPROM_WP_GPIO);
  
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = EEPROM_WP_GPIO_PIN;
  pinInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(EEPROM_WP_GPIO, &pinInit);

  // Enable write operations on EEPROM
  LL_GPIO_ResetOutputPin(EEPROM_WP_GPIO, EEPROM_WP_GPIO_PIN);
}

void eepromInit()
{
  i2c_init(EEPROM_I2C_BUS);
  init_wp_pin();
}

/**
  * @brief  Wait for EEPROM Standby state
  * @param  None
  * @retval None
  */
static bool I2C_EE_WaitEepromStandbyState(void)
{
  if (i2c_dev_ready(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS) < 0)
    return false;

  return true;
}

static void eepromWaitEepromStandbyState(void)
{
  // TODO: seriously? no timeout, no bailout ???
  while (!I2C_EE_WaitEepromStandbyState()) {
    eepromInit();
  }
}

/**
  * @brief  Reads a block of data from the EEPROM.
  * @param  pBuffer : pointer to the buffer that receives the data read
  *   from the EEPROM.
  * @param  ReadAddr : EEPROM's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the EEPROM.
  * @retval None
  */
static bool I2C_EE_ReadBlock(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
  if (stm32_i2c_read(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, ReadAddr, 1,
                     pBuffer, NumByteToRead, 100) < 0) {
    return false;
  }

  return true;
}

void eepromReadBlock(uint8_t * buffer, size_t address, size_t size)
{
  while (!I2C_EE_ReadBlock(buffer, address, size)) {
    eepromInit();
  }
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
static bool I2C_EE_PageWrite(uint8_t * pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
  if (stm32_i2c_write(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, WriteAddr, 1,
                      pBuffer, NumByteToWrite, 100) < 0) {
    return false;
  }

  return true;
}

static void eepromPageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
  while (!I2C_EE_PageWrite(pBuffer, WriteAddr, NumByteToWrite)) {
    eepromInit();
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
  uint8_t offset = address % EEPROM_PAGESIZE;
  uint8_t count = EEPROM_PAGESIZE - offset;
  if (size < count) {
    count = size;
  }
  while (count > 0) {
    eepromPageWrite(buffer, address, count);
    eepromWaitEepromStandbyState();
    address += count;
    buffer += count;
    size -= count;
    count = EEPROM_PAGESIZE;
    if (size < EEPROM_PAGESIZE) {
      count = size;
    }
  }
}

uint8_t eepromIsTransferComplete()
{
  return 1;
}
