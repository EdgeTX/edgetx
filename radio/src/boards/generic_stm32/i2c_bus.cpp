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

#include "stm32_i2c_driver.h"
#include "stm32_hal_ll.h"

#include "hal/i2c_driver.h"
#include "hal.h"

#define I2C_DEFAULT_TIMEOUT 10
#define I2C_DEFAULT_RETRIES 2

#if defined(I2C_B1)
static const stm32_i2c_hw_def_t _i2c1 = {
  .I2Cx = I2C_B1,
  .SCL_GPIO = (gpio_t)I2C_B1_SCL_GPIO,
  .SDA_GPIO = (gpio_t)I2C_B1_SDA_GPIO,
  .GPIO_AF = I2C_B1_GPIO_AF,
  .set_pwr = nullptr,
};
#endif

#if defined(I2C_B2_PWR_GPIO)
void _i2c_b2_pwr(bool enable)
{
  gpio_init(I2C_B2_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_write(I2C_B2_PWR_GPIO, enable);
}
#else
# define _i2c_b2_pwr nullptr
#endif

#if defined(I2C_B2)
static const stm32_i2c_hw_def_t _i2c2 = {
  .I2Cx = I2C_B2,
  .SCL_GPIO = (gpio_t)I2C_B2_SCL_GPIO,
  .SDA_GPIO = (gpio_t)I2C_B2_SDA_GPIO,
  .GPIO_AF = I2C_B2_GPIO_AF,
  .set_pwr = _i2c_b2_pwr,
};
#endif

int i2c_init(etx_i2c_bus_t bus)
{
#if defined(I2C_B1)
  if (bus == I2C_Bus_1) {
    return stm32_i2c_init(I2C_Bus_1, I2C_B1_CLK_RATE, &_i2c1);
  }
#endif

#if defined(I2C_B2)
  if (bus == I2C_Bus_2) {
    return stm32_i2c_init(I2C_Bus_2, I2C_B2_CLK_RATE, &_i2c2);
  }
#endif

  return -1;
}

int i2c_deinit(etx_i2c_bus_t bus)
{
#if defined(I2C_B1)
  if (bus == I2C_Bus_1) {
    return stm32_i2c_deinit(I2C_Bus_1);
  }
#endif

#if defined(I2C_B2)
  if (bus == I2C_Bus_2) {
    return stm32_i2c_deinit(I2C_Bus_2);
  }
#endif

  return -1;
}

int i2c_dev_ready(etx_i2c_bus_t bus, uint16_t addr)
{
  return stm32_i2c_is_dev_ready(bus, addr, I2C_DEFAULT_RETRIES,
                                I2C_DEFAULT_TIMEOUT);
}

int i2c_read(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
             uint8_t* data, uint16_t len)
{
  return stm32_i2c_read(bus, addr, reg, reg_size, data, len,
                        I2C_DEFAULT_TIMEOUT);
}

int i2c_write(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
              uint8_t* data, uint16_t len)
{
  return stm32_i2c_write(bus, addr, reg, reg_size, data, len,
                         I2C_DEFAULT_TIMEOUT);
}
