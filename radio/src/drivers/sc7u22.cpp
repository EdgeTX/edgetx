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

#include "stm32_i2c_driver.h"
#include "stm32_gpio.h"
#include "delays_driver.h"

#include "sc7u22.h"

#include "inactivity_timer.h"
#include "debug.h"

#include "hal.h"

constexpr uint32_t I2C_TIMEOUT = 5; // ms

static etx_i2c_bus_t s_i2c_bus;
static uint16_t s_i2c_addr;

static int write_reg(uint8_t reg, uint8_t val)
{
  return i2c_write(s_i2c_bus, s_i2c_addr, reg, 1, &val, 1);
}

static int read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
  return stm32_i2c_read(s_i2c_bus, s_i2c_addr, reg, 1, buf, len,
                        I2C_TIMEOUT);
}

static int soft_reset(void)
{
  // Ensure Addr_Auto is enabled (default 0x50 has it set, but be explicit)
  if (write_reg(SC7U22_COM_CONF_REG, SC7U22_COM_CONF_DEFAULT) < 0)
    return -1;
  delay_ms(1);

  // Write 0xA5 twice to SOFT_RST
  if (write_reg(SC7U22_SOFT_RST_REG, 0xA5) < 0) return -1;
  if (write_reg(SC7U22_SOFT_RST_REG, 0xA5) < 0) return -1;

  // Datasheet recommends ~100 ms after reset for chip to re-initialise
  delay_ms(100);
  return 0;
}

// ---------------------------------------------------------------------------
// Optional: WoM ISR (mirror of ICM42607C pattern)
// ---------------------------------------------------------------------------
#if defined(IMU_INT_GPIO)
static void imu_exti_isr(void)
{
  inactivityTimerReset(ActivitySource::MainControls);
}
#endif

static int16_t __attribute__((unused)) getSC7U22Temperature(void)
{
  uint8_t buf[2] = {0};

  if (read_regs(SC7U22_TEMP_H_REG, buf, 2) < 0) {
    TRACE("SC7U22 ERROR: TEMP read error");
    return -1;
  }

  int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
  return (int16_t)(raw / 512 + 23);
}


static int gyroSC7U22Init(etx_i2c_bus_t bus, uint16_t addr)
{
  s_i2c_bus  = bus;
  s_i2c_addr = addr;

  TRACE("SC7U22 I2C Init at address 0x%x", addr);

  if (i2c_init(s_i2c_bus) < 0) {
    TRACE("SC7U22 ERROR: i2c_init bus error");
    return -1;
  }

  if (i2c_dev_ready(s_i2c_bus, s_i2c_addr) < 0) {
    TRACE("SC7U22 ERROR: device not ready");
    return -1;
  }

  uint8_t who_am_i = 0;
  if (read_regs(SC7U22_WHO_AM_I_REG, &who_am_i, 1) < 0) {
    TRACE("SC7U22 ERROR: WHO_AM_I read error");
    return -1;
  }

  if (who_am_i != 0x6A) {
    TRACE("SC7U22 ERROR: unexpected WHO_AM_I 0x%02X (expected 0x6A)", who_am_i);
    return -1;
  }

  if (soft_reset() < 0) {
    TRACE("SC7U22 ERROR: soft reset failed");
    return -1;
  }

  // -------------------------------------------------------------------------
  // 4. Configure COM_CONF (0x04)
  //    BDU=1 (block data update – output not updated until both bytes read)
  //    Addr_Auto=1 (auto-increment for burst reads)
  //    All other bits at default (push-pull, active-high INT, 4-wire SPI)
  // -------------------------------------------------------------------------
  if (write_reg(SC7U22_COM_CONF_REG, SC7U22_COM_CONF_DEFAULT) < 0) {
    TRACE("SC7U22 ERROR: COM_CONF write error");
    return -1;
  }
  delay_ms(1);

  // -------------------------------------------------------------------------
  // 5. Enable accelerometer, gyroscope and temperature sensor via PWR_CTRL
  //    Datasheet §5.47: ACC_EN | GYR_EN | TEMP_EN
  //    Delay ≥1 ms after configuring before reading.
  // -------------------------------------------------------------------------
  if (write_reg(SC7U22_PWR_CTRL_REG, SC7U22_PWR_CTRL_ENABLE) < 0) {
    TRACE("SC7U22 ERROR: PWR_CTRL write error");
    return -1;
  }
  delay_ms(10); // Allow sensors to start up

  // -------------------------------------------------------------------------
  // 6. Configure accelerometer
  //    ACC_CONF (0x40): high-performance mode, NORM_AVG4 filter, 1600 Hz ODR
  //    ACC_RANGE (0x41): ±4g
  // -------------------------------------------------------------------------
  if (write_reg(SC7U22_ACC_CONF_REG, SC7U22_ACC_CONF_HP_1KHZ) < 0) {
    TRACE("SC7U22 ERROR: ACC_CONF write error");
    return -1;
  }
  delay_ms(1);

  if (write_reg(SC7U22_ACC_RANGE_REG, SC7U22_ACC_RANGE_4G) < 0) {
    TRACE("SC7U22 ERROR: ACC_RANGE write error");
    return -1;
  }
  delay_ms(1);

  // -------------------------------------------------------------------------
  // 7. Configure gyroscope
  //    GYR_CONF (0x42): high-performance mode, NORM_AVG4 filter, 1600 Hz ODR
  //    GYR_RANGE (0x43): ±2000 dps
  // -------------------------------------------------------------------------
  if (write_reg(SC7U22_GYR_CONF_REG, SC7U22_GYR_CONF_HP_1KHZ) < 0) {
    TRACE("SC7U22 ERROR: GYR_CONF write error");
    return -1;
  }
  delay_ms(1);

  if (write_reg(SC7U22_GYR_RANGE_REG, SC7U22_GYR_RANGE_2000DPS) < 0) {
    TRACE("SC7U22 ERROR: GYR_RANGE write error");
    return -1;
  }
  delay_ms(1);

  // -------------------------------------------------------------------------
  // 8. Optional: Wake-on-Motion via AOI1
  //    Uses the AOI1 OR-interrupt with all three axes enabled.
  //    Threshold: 0x7F LSB.  At ±4g, 1 LSB = 32 mg → 0x7F ≈ 4g (max, safe).
  //    For activity detection similar to ICM42607C (0xFE ≈ 1g), set 0x1F ≈ 1g.
  // -------------------------------------------------------------------------
#if defined(IMU_INT_GPIO)
  // AOI1_CFG: AOI=0 (OR mode), 6D=0, enable ZHIE|YHIE|XHIE
  if (write_reg(SC7U22_AOI1_CFG_REG, 0x15) < 0) return -1; // 0b00010101
  delay_ms(1);

  // AOI1_THS: ~0.5g threshold for ±4g range (1 LSB = 32 mg → 16 LSB ≈ 512 mg)
  if (write_reg(SC7U22_AOI1_THS_REG, 0x10) < 0) return -1;
  delay_ms(1);

  // AOI1_DURATION: minimal (1 sample)
  if (write_reg(SC7U22_AOI1_DURATION_REG, 0x00) < 0) return -1;
  delay_ms(1);

  // AOI1&AOI2_CFG (0x3F): enable AOI, high-pass filter on AOI1
  // AOI_EN=1 (bit6), HPIS1=1 (bit4) for motion detection with HP data
  if (write_reg(SC7U22_AOI1_AOI2_CFG_REG, 0x50) < 0) return -1;
  delay_ms(1);

  // Route AOI1 interrupt to INT1 pin (INT1_OUT_SEL2, 0x06, bit0 = INT1_AOI1)
  if (write_reg(SC7U22_INT1_OUT_SEL2_REG, 0x01) < 0) return -1;
  delay_ms(1);

  // Configure INT1 GPIO as falling-edge interrupt input
  gpio_init_int(IMU_INT_GPIO, GPIO_IN_PU, GPIO_FALLING, imu_exti_isr);
#endif

  TRACE("SC7U22 succeeded");
  return 0;
}

static int gyroSC7U22Read(etx_imu_data_t* data)
{
  uint8_t buf[6];

  // Read gyroscope: GYR_XH ... GYR_ZL (0x12 ... 0x17, 6 bytes)
  if (read_regs(SC7U22_GYR_XH_REG, buf, 6) < 0) {
    TRACE("SC7U22 ERROR: gyro read error");
    return -1;
  }
  data->gyro_x = -((int16_t)(buf[0] << 8) | buf[1]);
  data->gyro_y = -((int16_t)(buf[2] << 8) | buf[3]);
  data->gyro_z =  ((int16_t)(buf[4] << 8) | buf[5]);

  // Read accelerometer: ACC_XH ... ACC_ZL (0x0C ... 0x11, 6 bytes)
  if (read_regs(SC7U22_ACC_XH_REG, buf, 6) < 0) {
    TRACE("SC7U22 ERROR: accel read error");
    return -1;
  }
  data->accel_x = ((int16_t)(buf[0] << 8) | buf[1]);
  data->accel_y = ((int16_t)(buf[2] << 8) | buf[3]);
  data->accel_z = ((int16_t)(buf[4] << 8) | buf[5]);

  return 0;
}

const etx_imu_driver_t imu_sc7u22_driver = {
  gyroSC7U22Init,
  gyroSC7U22Read,
};
