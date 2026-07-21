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

#include "hal/i2c_driver.h"
#include "hal/imu.h"
#include "hal/gpio.h"

#include "delays_driver.h"

#include "icm42670.h"

#include "inactivity_timer.h"
#include "debug.h"

static etx_i2c_bus_t s_i2c_bus;
static uint16_t s_i2c_addr;

static int write_cmd(uint8_t reg, uint8_t val)
{
  return i2c_write(s_i2c_bus, s_i2c_addr, reg, 1, &val, 1);
}

#if defined(IMU_INT_GPIO)
static void imu_exti_isr(void)
{
  inactivityTimerReset(ActivitySource::MainControls);
}

static int write_mreg1(uint8_t reg, uint8_t val) {
    if (write_cmd(BLK_SEL_W_REG, MREG1_BANK_SEL) < 0) return -1;
    if (write_cmd(MADDR_W_REG, reg) < 0) return -1;
    if (write_cmd(M_W_REG, val) < 0) return -1;
    delay_us(10); // 42670-L pdf section 12 "Accessing MREG1, MREG2 And MREG3 Registers"
    return 0;
}
#endif

static int icm42670Init(etx_i2c_bus_t bus, uint16_t addr)
{
  s_i2c_bus  = bus;
  s_i2c_addr = addr;

  TRACE("ICM42670 I2C Init at address 0x%x", addr);

  if (i2c_init(s_i2c_bus) < 0) {
    TRACE("ICM42670 ERROR: i2c_init bus error");
    return -1;
  }

  if (i2c_dev_ready(s_i2c_bus, s_i2c_addr) < 0) {
    TRACE("ICM42670 device init error");
    return -1;
  }

  uint8_t data = 0;
  if (i2c_read(s_i2c_bus, s_i2c_addr, WHO_AM_I_REG, 1, &data, 1) < 0) {
    TRACE("ICM42670 ERROR: i2c read error");
    return -1;
  }

  if (data != 0x63) {
    TRACE("ICM42670 ERROR: this code only works on ICM42670-L");
    return -1;
  }

  if (write_cmd(SIGNAL_PATH_RESET_REG, SOFT_RESET_CMD) < 0) {
    TRACE("ICM42670 ERROR: SOFT_RESET_CMD error");
    return -1;
  }
  delay_ms(200);

  if (write_cmd(PWR_MGMT0_REG, PWR_MGMT0_ENABLE) < 0) {
    TRACE("ICM42670 ERROR: PWR_MGMT0_REG error");
    return -1;
  }
  delay_ms(10);

  if (write_cmd(INT_CONFIG_REG, 0x00) < 0) {
    TRACE("ICM42670 ERROR: INT_CONFIG_REG error");
    return -1;
  }
  delay_ms(1);

#if defined(IMU_INT_GPIO)
  if (write_mreg1(ACCEL_WOM_X_THR_REG, 0xFE) < 0) return -1;
  if (write_mreg1(ACCEL_WOM_Y_THR_REG, 0xFE) < 0) return -1;
  if (write_mreg1(ACCEL_WOM_Z_THR_REG, 0xFE) < 0) return -1;
  delay_ms(1);
  if (write_cmd(WOM_CONFIG_REG, 0x03) < 0) return -1;
  delay_ms(1);
  if (write_cmd(INT_SOURCE1_REG, 0x07) < 0) return -1;
  delay_ms(1);
  if (write_cmd(INT_CONFIG_REG, 0x00) < 0) return -1;
  gpio_init_int(IMU_INT_GPIO, GPIO_IN_PU, GPIO_FALLING, imu_exti_isr);
#endif

  TRACE("ICM42670 succeeded");
  return 0;
}

static int icm42670Read(etx_imu_data_t* data)
{
  uint8_t buf[6];

  if (i2c_read(s_i2c_bus, s_i2c_addr, GYRO_DATA_X1_REG, 1, buf, 6) < 0) {
    TRACE("ICM42670 ERROR: gyro read error");
    return -1;
  }
  data->gyro_x = -((int16_t)(buf[0] << 8) | buf[1]);
  data->gyro_y = -((int16_t)(buf[2] << 8) | buf[3]);
  data->gyro_z =  ((int16_t)(buf[4] << 8) | buf[5]);

  if (i2c_read(s_i2c_bus, s_i2c_addr, ACCEL_DATA_X1_REG, 1, buf, 6) < 0) {
    TRACE("ICM42670 ERROR: accel read error");
    return -1;
  }
  data->accel_x =  ((int16_t)(buf[0] << 8) | buf[1]);
  data->accel_y =  ((int16_t)(buf[2] << 8) | buf[3]);
  data->accel_z =  ((int16_t)(buf[4] << 8) | buf[5]);

  return 0;
}

const etx_imu_driver_t imu_icm42670_driver = {
  icm42670Init,
  icm42670Read,
  "ICM42670",
};
