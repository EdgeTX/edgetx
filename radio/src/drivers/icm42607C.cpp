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

/*
 WARNING: while this ICM42607C belong to the ICM426xx familly, it is NOT compatible
		  with code for generic ICM4267XX. All register have different addresses
*/

#include "hal/i2c_driver.h"
#include "hal/imu.h"

#include "stm32_i2c_driver.h"
#include "stm32_gpio.h"
#include "delays_driver.h"

#include "icm42607C.h"

#include "inactivity_timer.h"
#include "debug.h"

#include "hal.h"

constexpr uint32_t I2C_TIMEOUT = 5; // ms

static etx_i2c_bus_t s_i2c_bus;
static uint16_t s_i2c_addr;

static int16_t __attribute__((unused)) get42607Temperature()
{
  uint8_t reg = TEMP_DATA_X0_REG;
  uint8_t buf[2] = {0};

  if (stm32_i2c_read(s_i2c_bus, s_i2c_addr, reg, 1, buf, 2, I2C_TIMEOUT) < 0) {
    TRACE("ICM426xx ERROR: TEMP_DATA_X0_REG i2c read error");
    return -1;
  }

  int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
  return (int16_t)(raw / 128 + 25);
}

static int write_cmd(uint8_t reg, uint8_t val)
{
  return i2c_write(s_i2c_bus, s_i2c_addr, reg, 1, &val, 1);
}

#if defined(IMU_INT_GPIO)
static void imu_exti_isr(void)
{
  inactivityTimerReset(ActivitySource::MainControls);
}
#endif

static int write_mreg1(uint8_t reg, uint8_t val) {
    if (write_cmd(BLK_SEL_W_REG, 0x00) < 0) return -1;
    if (write_cmd(MADDR_W_REG, reg) < 0) return -1;
    if (write_cmd(M_W_REG, val) < 0) return -1;
    delay_us(10); // small delay as recommended by datasheet
    return 0;
}

static int gyro42607Init(etx_i2c_bus_t bus, uint16_t addr)
{
  s_i2c_bus  = bus;
  s_i2c_addr = addr;

  TRACE("ICM426xx I2C Init at address 0x%x", addr);

  if (i2c_init(s_i2c_bus) < 0) {
    TRACE("ICM426xx ERROR: i2c_init bus error");
    return -1;
  }

  if (i2c_dev_ready(s_i2c_bus, s_i2c_addr) < 0) {
    TRACE("ICM426xx device init error");
    return -1;
  }

  uint8_t data = 0;
  if (stm32_i2c_read(s_i2c_bus, s_i2c_addr, WHO_AM_I_REG, 1, &data, 1, I2C_TIMEOUT) < 0) {
    TRACE("ICM426xx ERROR: i2c read error");
    return -1;
  }

  if (data != 0x61) {
    TRACE("ICM426xx ERROR: this code only works on ICM42607C");
    return -1;
  }

  if(write_cmd(SIGNAL_PATH_RESET, SOFT_RESET_CMD) < 0) {
    TRACE("ICM426xx ERROR: SOFT_RESET_CMD error");
    return -1;
  }
  delay_ms(100);

  if(write_cmd(PWR_MGMT0_REG, PWR_MGMT0_ENABLE) < 0) {
    TRACE("ICM426xx ERROR: PWR_MGMT0_REG error");
    return -1;
  }
  delay_ms(10);

  // Configure gyro: 2000 dps, 1 kHz ODR
  if(write_cmd(GYRO_CONFIG0_REG, GYRO_ODR_1KHZ) < 0) {
    TRACE("ICM426xx ERROR: GYRO_CONFIG0_REG error");
    return -1;
  }
  delay_ms(1);

  // Configure accelerometer: ±4g, 1 kHz ODR
  if (write_cmd(ACCEL_CONFIG0_REG, ACCEL_ODR_1KHZ) < 0) {
    TRACE("ICM426xx ERROR: ACCEL_CONFIG0_REG error");
    return -1;
  }
  delay_ms(1);


  if (write_cmd(INT_CONFIG_REG, 0x00) < 0) {
    TRACE("ICM426xx ERROR: INT_CONFIG_REG error");
    return -1;
  }
  delay_ms(1);

#if defined(IMU_INT_GPIO)
  // Configure Wake-on-Motion thresholds via MREG1 interface
  // Values chosen to approximate activity detection behaviour used elsewhere
  if (write_mreg1(ACCEL_WOM_X_THR_REG, 0xFE) < 0) return -1;
  if (write_mreg1(ACCEL_WOM_Y_THR_REG, 0xFE) < 0) return -1;
  if (write_mreg1(ACCEL_WOM_Z_THR_REG, 0xFE) < 0) return -1;

  delay_ms(1);
  // Configure WoM mode (OR across axes)
  if (write_cmd(WOM_CONFIG_REG, 0x03) < 0) return -1; // OR mode
  delay_ms(1);
  // Enable INT1 source bits for XYZ
  if (write_cmd(INT_SOURCE1_REG, 0x07) < 0) return -1; // XYZ interrupt
  delay_ms(1);
  // Ensure INT1 is configured to default behaviour
  if (write_cmd(INT_CONFIG_REG, 0x00) < 0) return -1;

  gpio_init_int(IMU_INT_GPIO, GPIO_IN_PU, GPIO_FALLING, imu_exti_isr);
#endif

  TRACE("ICM426xx succeeded");
  return 0;
}

static int gyro42607Read(etx_imu_data_t* data)
{
  uint8_t buf[6];

  uint8_t reg = GYRO_DATA_X0_REG;
  if (stm32_i2c_read(s_i2c_bus, s_i2c_addr, reg, 1, buf, 6, I2C_TIMEOUT) < 0) {
    TRACE("ICM426xx ERROR: gyro read error");
    return -1;
  }
  data->gyro_x = -((int16_t)(buf[0] << 8) | buf[1]);
  data->gyro_y = -((int16_t)(buf[2] << 8) | buf[3]);
  data->gyro_z =  ((int16_t)(buf[4] << 8) | buf[5]);

  reg = ACCEL_DATA_X0_REG;
  if (stm32_i2c_read(s_i2c_bus, s_i2c_addr, reg, 1, buf, 6, I2C_TIMEOUT) < 0) {
    TRACE("ICM426xx ERROR: accel read error");
    return -1;
  }
  data->accel_x = ((int16_t)(buf[0] << 8) | buf[1]);
  data->accel_y = ((int16_t)(buf[2] << 8) | buf[3]);
  data->accel_z = ((int16_t)(buf[4] << 8) | buf[5]);

  return 0;
}

const etx_imu_driver_t imu_icm42607_driver = {
  gyro42607Init,
  gyro42607Read,
};
