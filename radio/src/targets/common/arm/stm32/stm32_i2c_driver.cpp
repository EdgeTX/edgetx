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
#include "hal.h"

#include "debug.h"

struct _i2c_defs {
  GPIO_TypeDef*      SCL_GPIOx;
  uint32_t           SCL_Pin;
  GPIO_TypeDef*      SDA_GPIOx;
  uint32_t           SDA_Pin;
  uint32_t           Alternate;
};

#if defined(I2C_B1)
static I2C_HandleTypeDef hi2c1 = {
  .Instance = I2C_B1,
  .Init = { 0, 0, 0, 0, 0, 0, 0, 0 },
};
static const _i2c_defs pins_hi2c1 = {
  .SCL_GPIOx = I2C_B1_GPIO,
  .SCL_Pin = I2C_B1_SCL_GPIO_PIN,
  .SDA_GPIOx = I2C_B1_GPIO,
  .SDA_Pin = I2C_B1_SDA_GPIO_PIN,
  .Alternate = I2C_B1_GPIO_AF,
};
#endif

#if defined(I2C_B2)
static I2C_HandleTypeDef hi2c2 = {
  .Instance = I2C_B2,
  .Init = { 0, 0, 0, 0, 0, 0, 0, 0 },
};
static const _i2c_defs pins_hi2c2 = {
#if defined(I2C_B2_GPIO)
  .SCL_GPIOx = I2C_B2_GPIO,
#else
  .SCL_GPIOx = I2C_B2_SCL_GPIO,
#endif
  .SCL_Pin = I2C_B2_SCL_GPIO_PIN,
#if defined(I2C_B2_GPIO)
  .SDA_GPIOx = I2C_B2_GPIO,
#else
  .SDA_GPIOx = I2C_B2_SDA_GPIO,
#endif
  .SDA_Pin = I2C_B2_SDA_GPIO_PIN,
  .Alternate = I2C_B2_GPIO_AF,
};
#endif

static I2C_HandleTypeDef* get_i2c_handle(uint8_t bus)
{
#if defined(I2C_B1)
  if (bus == I2C_Bus_1) { return &hi2c1; }
#endif
#if defined(I2C_B2)
  if (bus == I2C_Bus_2) { return &hi2c2; }
#endif
  return nullptr;
}

int stm32_i2c_init(uint8_t bus, uint32_t clock_rate)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;

  I2C_InitTypeDef& init = h->Init;
  if (init.ClockSpeed > 0) {
    if (init.ClockSpeed != clock_rate) return -1;
    return 0;
  }
  
  init.ClockSpeed = clock_rate;
  init.DutyCycle = I2C_DUTYCYCLE_16_9;
  init.OwnAddress1 = 0;
  init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  init.OwnAddress2 = 0;
  init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(h) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2C_Init() failed");
    return -1;
  }

#if  defined(I2C_FLTR_ANOFF) && defined(I2C_FLTR_DNF)
  // Configure Analogue filter
  if (HAL_I2CEx_ConfigAnalogFilter(h, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2CEx_ConfigAnalogFilter() failed");
    return -1;
  }

  // Configure Digital filter
  if (HAL_I2CEx_ConfigDigitalFilter(h, 0) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2CEx_ConfigDigitalFilter() failed");
    return -1;
  }
#endif

  return 1;
}

int stm32_i2c_deinit(uint8_t bus)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  

  if (HAL_I2C_DeInit(h) != HAL_OK) return -1;
  h->Init.ClockSpeed = 0;
  
  return 0;
}

int stm32_i2c_master_tx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Master_Transmit(h, addr << 1, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_master_rx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Master_Receive(h, addr << 1, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_read(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                   uint8_t* data, uint16_t len, uint32_t timeout)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  

  if (HAL_I2C_Mem_Read(h, addr << 1, reg, reg_size, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_write(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                    uint8_t* data, uint16_t len, uint32_t timeout)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Mem_Write(h, addr << 1, reg, reg_size, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_is_dev_ready(uint8_t bus, uint16_t addr, uint32_t timeout)
{
  I2C_HandleTypeDef* h = get_i2c_handle(bus);
  if (!h) return -1;  

  HAL_StatusTypeDef err = HAL_I2C_IsDeviceReady(h, addr << 1, (uint32_t)-1, timeout);
  if (err != HAL_OK) return -1;

  return 0;
}

static int i2c_enable_gpio_clock(GPIO_TypeDef *GPIOx)
{
  if (GPIOx == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (GPIOx == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (GPIOx == GPIOC)
    __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (GPIOx == GPIOH)
    __HAL_RCC_GPIOH_CLK_ENABLE();
  else
    return -1;

  return 0;
}

static int i2c_enable_clock(I2C_TypeDef* instance)
{
  /* Peripheral clock enable */
  if (instance == I2C1)
    __HAL_RCC_I2C1_CLK_ENABLE();
  else if (instance == I2C2)
    __HAL_RCC_I2C2_CLK_ENABLE();
  else if (instance == I2C3)
    __HAL_RCC_I2C3_CLK_ENABLE();
  else
    return -1;

  return 0;
}

static int i2c_disable_clock(I2C_TypeDef* instance)
{
  /* Peripheral clock disable */
  if (instance == I2C1)
    __HAL_RCC_I2C1_CLK_DISABLE();
  else if (instance == I2C2)
    __HAL_RCC_I2C2_CLK_DISABLE();
  else if (instance == I2C3)
    __HAL_RCC_I2C3_CLK_DISABLE();
  else
    return -1;

  return 0;
}

static int i2c_pins_init(const _i2c_defs* def)
{
  if (i2c_enable_gpio_clock(def->SCL_GPIOx) < 0) return -1;
  if (i2c_enable_gpio_clock(def->SDA_GPIOx) < 0) return -1;

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  
  pinInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  pinInit.Pull = LL_GPIO_PULL_NO;
  pinInit.Alternate = def->Alternate;

  pinInit.Pin = def->SCL_Pin;
  LL_GPIO_Init(def->SCL_GPIOx, &pinInit);

  pinInit.Pin = def->SDA_Pin;
  LL_GPIO_Init(def->SDA_GPIOx, &pinInit);

  return 0;
}

static int i2c_pins_deinit(const _i2c_defs* def)
{
  // reconfigure pins as open-drain input
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  
  pinInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  pinInit.Pull = LL_GPIO_PULL_NO;
  pinInit.Alternate = 0;

  pinInit.Pin = def->SCL_Pin;
  LL_GPIO_Init(def->SCL_GPIOx, &pinInit);

  pinInit.Pin = def->SDA_Pin;
  LL_GPIO_Init(def->SDA_GPIOx, &pinInit);

  return 0;
}

/**
  * @brief  Initialize the I2C MSP.
  * @param  h Pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *h)
{
  const _i2c_defs* defs = nullptr;
#if defined(I2C_B1)
  if (h == &hi2c1) { defs = &pins_hi2c1; }
#endif
#if defined(I2C_B2)
  if (h == &hi2c2) { defs = &pins_hi2c2; }
#endif
  if (!defs) return;
  
  if (i2c_pins_init(defs) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspInit() I2C_GPIO misconfiguration");
    return;
  }

  if (i2c_enable_clock(h->Instance) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspInit() I2C misconfiguration");
    return;
  }
}

/**
  * @brief  DeInitialize the I2C MSP.
  * @param  h Pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *h)
{
  const _i2c_defs* defs = nullptr;
#if defined(I2C_B1)
  if (h == &hi2c1) { defs = &pins_hi2c1; }
#endif
#if defined(I2C_B2)
  if (h == &hi2c2) { defs = &pins_hi2c2; }
#endif
  if (!defs) return;

  if (i2c_disable_clock(h->Instance) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspDeInit() I2C misconfiguration");
    return;
  }

  i2c_pins_deinit(defs);
}
