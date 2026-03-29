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

#include "delays_driver.h"

#include "icm42627.h"

#include "debug.h"

static etx_i2c_bus_t s_i2c_bus;
static uint16_t s_i2c_addr;

static constexpr uint32_t RESET_DELAY_MS = 150;
static constexpr uint32_t READY_TIMEOUT_MS = 200;
static constexpr uint32_t POWER_DELAY_MS = 20;
static constexpr uint32_t VERIFY_DELAY_MS = 10;
static constexpr uint32_t CONFIG_DELAY_US = 100;
static constexpr uint8_t BURST_READ_LEN = 14;

static int write_cmd(uint8_t reg, uint8_t value)
{
  return i2c_write(s_i2c_bus, s_i2c_addr, reg, 1, &value, 1);
}

static int read_cmd(uint8_t reg, uint8_t *value)
{
  return i2c_read(s_i2c_bus, s_i2c_addr, reg, 1, value, 1);
}

static int read_who_am_i(uint8_t *value)
{
  return read_cmd(ICM42627_WHO_AM_I_REG, value);
}

static int write_checked(uint8_t reg, uint8_t value, const char *error)
{
  if (write_cmd(reg, value) < 0) {
    TRACE("%s", error);
    return -1;
  }

  return 0;
}

static int write_verified(uint8_t reg, uint8_t value, const char *write_error,
                          const char *verify_error)
{
  uint8_t readback = 0;

  if (write_checked(reg, value, write_error) < 0) {
    return -1;
  }

  delay_us(CONFIG_DELAY_US);

  if (read_cmd(reg, &readback) < 0 || readback != value) {
    TRACE("%s", verify_error);
    return -1;
  }

  return 0;
}

static int wait_for_device_ready(uint32_t timeout_ms)
{
  while (timeout_ms > 0) {
    if (i2c_dev_ready(s_i2c_bus, s_i2c_addr) >= 0) {
      return 0;
    }

    delay_ms(10);
    if (timeout_ms < 10) {
      break;
    }
    timeout_ms -= 10;
  }

  return -1;
}

static int gyro42627Init(etx_i2c_bus_t bus, uint16_t addr)
{
  s_i2c_bus = bus;
  s_i2c_addr = addr;

  TRACE("ICM42627 I2C Init at address 0x%x", addr);

  if (i2c_init(s_i2c_bus) < 0) {
    TRACE("ICM42627 ERROR: i2c_init bus error");
    return -1;
  }

  if (i2c_dev_ready(s_i2c_bus, s_i2c_addr) < 0) {
    TRACE("ICM42627 device init error");
    return -1;
  }

  uint8_t who_am_i = 0;
  if (read_who_am_i(&who_am_i) < 0) {
    TRACE("ICM42627 ERROR: WHO_AM_I read error");
    return -1;
  }

  if (who_am_i != ICM42627_WHO_AM_I) {
    TRACE("ICM42627 ERROR: unexpected WHO_AM_I 0x%02X", who_am_i);
    return -1;
  }

  if (write_cmd(ICM42627_DEVICE_CONFIG_REG, ICM42627_RESET) < 0) {
    TRACE("ICM42627 WARN: reset write failed");
  }

  delay_ms(RESET_DELAY_MS);

  if (wait_for_device_ready(READY_TIMEOUT_MS) < 0) {
    TRACE("ICM42627 ERROR: device not ready after reset");
    return -1;
  }

  if (write_checked(ICM42627_PWR_MGMT0_REG, ICM42627_PWR_STAGE1,
                    "ICM42627 ERROR: PWR_MGMT0 stage1 failed") < 0) {
    return -1;
  }
  delay_ms(POWER_DELAY_MS);

  if (write_checked(ICM42627_PWR_MGMT0_REG, ICM42627_PWR_STAGE2,
                    "ICM42627 ERROR: PWR_MGMT0 stage2 failed") < 0) {
    return -1;
  }
  delay_ms(POWER_DELAY_MS);

  if (write_verified(ICM42627_GYRO_CONFIG0_REG, ICM42627_GYRO_CONFIG,
                     "ICM42627 ERROR: GYRO_CONFIG0_REG write failed",
                     "ICM42627 ERROR: GYRO_CONFIG0_REG verify failed") < 0) {
    return -1;
  }

  if (write_verified(ICM42627_ACCEL_CONFIG0_REG, ICM42627_ACCEL_CONFIG,
                     "ICM42627 ERROR: ACCEL_CONFIG0_REG write failed",
                     "ICM42627 ERROR: ACCEL_CONFIG0_REG verify failed") < 0) {
    return -1;
  }

  if (write_checked(ICM42627_GYRO_CONFIG1_REG, ICM42627_GYRO_FILTER,
                    "ICM42627 ERROR: GYRO_CONFIG1_REG error") < 0) {
    return -1;
  }
  delay_us(CONFIG_DELAY_US);

  if (write_checked(ICM42627_ACCEL_GYRO_BW_REG, ICM42627_ACCEL_GYRO_BW,
                    "ICM42627 ERROR: ACCEL_GYRO_BW_REG error") < 0) {
    return -1;
  }
  delay_us(CONFIG_DELAY_US);

  if (write_checked(ICM42627_ACCEL_CONFIG1_REG, ICM42627_ACCEL_FILTER,
                    "ICM42627 ERROR: ACCEL_CONFIG1_REG error") < 0) {
    return -1;
  }
  delay_us(CONFIG_DELAY_US);

  if (write_cmd(ICM42627_BANK_SEL_REG, ICM42627_BANK1) < 0) {
    TRACE("ICM42627 ERROR: bank1 select failed");
    return -1;
  }
  delay_us(CONFIG_DELAY_US);

  if (write_checked(ICM42627_GYRO_STATIC2_REG, ICM42627_GYRO_STATIC2,
                    "ICM42627 ERROR: GYRO_STATIC2_REG error") < 0) {
    return -1;
  }
  delay_us(CONFIG_DELAY_US);

  if (write_cmd(ICM42627_BANK_SEL_REG, ICM42627_BANK0) < 0) {
    TRACE("ICM42627 ERROR: bank0 select failed");
    return -1;
  }
  delay_ms(VERIFY_DELAY_MS);

  if (read_who_am_i(&who_am_i) < 0 ||
      who_am_i != ICM42627_WHO_AM_I) {
    TRACE("ICM42627 ERROR: WHO_AM_I verify failed (0x%02X)", who_am_i);
    return -1;
  }

  TRACE("ICM42627 succeeded");
  return 0;
}

static int gyro42627Read(etx_imu_data_t *data)
{
  uint8_t buf[BURST_READ_LEN] = {0};

  if (i2c_read(s_i2c_bus, s_i2c_addr, ICM42627_DATA_REG, 1, buf, sizeof(buf)) < 0) {
    TRACE("ICM42627 ERROR: burst read failed");
    return -1;
  }

  const int16_t accel_x = (int16_t)((buf[2] << 8) | buf[3]);
  const int16_t accel_y = (int16_t)((buf[4] << 8) | buf[5]);
  const int16_t accel_z = (int16_t)((buf[6] << 8) | buf[7]);
  const int16_t gyro_x = (int16_t)((buf[8] << 8) | buf[9]);
  const int16_t gyro_y = (int16_t)((buf[10] << 8) | buf[11]);
  const int16_t gyro_z = (int16_t)((buf[12] << 8) | buf[13]);

  data->accel_x = accel_y;
  data->accel_y = accel_x;
  data->accel_z = -accel_z;
  data->gyro_x = gyro_y;
  data->gyro_y = gyro_x;
  data->gyro_z = -gyro_z;

  return 0;
}

const etx_imu_driver_t imu_icm42627_driver = {
  gyro42627Init,
  gyro42627Read,
  "ICM42627",
};
