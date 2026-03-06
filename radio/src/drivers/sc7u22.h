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
 * Driver for Silan SC7U22 six-axis IMU (3-axis accelerometer + 3-axis gyroscope).
 *
 * I2C slave address: 0x19 (SDO/SA0 floating or high) / 0x18 (SDO/SA0 grounded)
 * WHO_AM_I register (0x01) expected value: 0x6A
 */

#pragma once

// ---------------------------------------------------------------------------
// I2C address
// ---------------------------------------------------------------------------
// SDO/SA0 tied high or floating → 7-bit address 0x19
// SDO/SA0 grounded              → 7-bit address 0x18
#define SC7U22_I2C_BASE_ADDR             0x18

// ---------------------------------------------------------------------------
// General-purpose registers (directly accessible with 8-bit address)
// ---------------------------------------------------------------------------
#define SC7U22_WHO_AM_I_REG         0x01   // Expected value: 0x6A
#define SC7U22_COM_CONF_REG         0x04   // Communication configuration
#define SC7U22_INT1_OUT_SEL1_REG    0x05
#define SC7U22_INT1_OUT_SEL2_REG    0x06
#define SC7U22_INT2_OUT_SEL1_REG    0x07
#define SC7U22_INT2_OUT_SEL2_REG    0x08
#define SC7U22_INT1_STAT_REG        0x09
#define SC7U22_INT2_STAT_REG        0x0A
#define SC7U22_DATA_STAT_REG        0x0B   // Data ready status

// Accelerometer output registers (16-bit, 2's complement, MSB first)
#define SC7U22_ACC_XH_REG           0x0C
#define SC7U22_ACC_XL_REG           0x0D
#define SC7U22_ACC_YH_REG           0x0E
#define SC7U22_ACC_YL_REG           0x0F
#define SC7U22_ACC_ZH_REG           0x10
#define SC7U22_ACC_ZL_REG           0x11

// Gyroscope output registers (16-bit, 2's complement, MSB first)
#define SC7U22_GYR_XH_REG           0x12
#define SC7U22_GYR_XL_REG           0x13
#define SC7U22_GYR_YH_REG           0x14
#define SC7U22_GYR_YL_REG           0x15
#define SC7U22_GYR_ZH_REG           0x16
#define SC7U22_GYR_ZL_REG           0x17

// Temperature registers: T = (int16_t)(TEMP_H<<8 | TEMP_L) / 512 + 23 °C
#define SC7U22_TEMP_H_REG           0x22
#define SC7U22_TEMP_L_REG           0x23

// FIFO
#define SC7U22_FIFO_CFG0_REG        0x1C
#define SC7U22_FIFO_CFG1_REG        0x1D
#define SC7U22_FIFO_CFG2_REG        0x1E
#define SC7U22_FIFO_STAT0_REG       0x1F
#define SC7U22_FIFO_STAT1_REG       0x20
#define SC7U22_FIFO_DATA_REG        0x21

// Saturation / self-test status
#define SC7U22_DATA_SAT_REG         0x24

// Power control
#define SC7U22_PWR_CTRL_REG         0x7D

// Accelerometer configuration
#define SC7U22_ACC_CONF_REG         0x40   // ACC_FILTER_PERF | ACC_BWP[2:0] | ACC_ODR[3:0]
#define SC7U22_ACC_RANGE_REG        0x41   // FS[1:0]: 00=±2g 01=±4g 10=±8g 11=±16g

// Gyroscope configuration
#define SC7U22_GYR_CONF_REG         0x42   // GYR_FILTER_PERF | GYR_NOISE_PERF | GYR_BWP[1:0] | GYR_ODR[3:0]
#define SC7U22_GYR_RANGE_REG        0x43   // OIS bits | FS[2:0]: 000=±2000dps … 100=±125dps

// Soft reset (write 0xA5 twice after enabling Addr_Auto)
#define SC7U22_SOFT_RST_REG         0x4A

// Segment selector (for special registers)
#define SC7U22_SEG_SEL_REG          0x7F

// ---------------------------------------------------------------------------
// COM_CONF (0x04) bit definitions
// ---------------------------------------------------------------------------
#define SC7U22_COM_CONF_BOOT        (1 << 7)  // Reboot memory content
#define SC7U22_COM_CONF_BDU         (1 << 6)  // Block data update
#define SC7U22_COM_CONF_OIS_EN      (1 << 5)  // OIS enable
#define SC7U22_COM_CONF_ADDR_AUTO   (1 << 4)  // Address auto-increment
#define SC7U22_COM_CONF_PP_OD       (1 << 3)  // INT push-pull / open-drain
#define SC7U22_COM_CONF_INT_H       (1 << 2)  // INT active level
#define SC7U22_COM_CONF_OISM        (1 << 1)  // OIS SPI mode
#define SC7U22_COM_CONF_SIM         (1 << 0)  // SPI 4-wire / 3-wire

// Default after reset: BDU=1, Addr_Auto=1  → 0x50
#define SC7U22_COM_CONF_DEFAULT     0x50

// ---------------------------------------------------------------------------
// DATA_STAT (0x0B) bit definitions
// ---------------------------------------------------------------------------
#define SC7U22_DATA_STAT_DRDY_GYR   (1 << 1)
#define SC7U22_DATA_STAT_DRDY_ACC   (1 << 0)
#define SC7U22_DATA_STAT_DRDY_TMP   (1 << 2)

// ---------------------------------------------------------------------------
// PWR_CTRL (0x7D) bit definitions
// ---------------------------------------------------------------------------
#define SC7U22_PWR_CTRL_TEMP_EN     (1 << 3)
#define SC7U22_PWR_CTRL_ACC_EN      (1 << 2)
#define SC7U22_PWR_CTRL_GYR_EN      (1 << 1)

// Enable accelerometer + gyroscope + temperature sensor
#define SC7U22_PWR_CTRL_ENABLE      (SC7U22_PWR_CTRL_TEMP_EN | \
                                     SC7U22_PWR_CTRL_ACC_EN  | \
                                     SC7U22_PWR_CTRL_GYR_EN)

// ---------------------------------------------------------------------------
// ACC_CONF (0x40) preset values
// ACC_FILTER_PERF=1 (high-perf), ACC_BWP=010 (NORM_AVG4), ACC_ODR=1000 (100 Hz)
//   → bits[7:0] = 1_010_1000 = 0xA8  (matches chip default)
// For 1 kHz high-performance: ACC_ODR=1100 → 0xAC
// ---------------------------------------------------------------------------
#define SC7U22_ACC_CONF_HP_1KHZ     0xAC  // High-perf, NORM_AVG4, 1600 Hz ODR
#define SC7U22_ACC_CONF_HP_100HZ    0xA8  // High-perf, NORM_AVG4, 100 Hz ODR (default)

// ACC_RANGE: ±4g = 0x01
#define SC7U22_ACC_RANGE_4G         0x01

// ---------------------------------------------------------------------------
// GYR_CONF (0x42) preset values
// GYR_FILTER_PERF=1 (high-perf/normal), GYR_NOISE_PERF=0, GYR_BWP=10 (NORM_AVG4),
// GYR_ODR=1000 (100 Hz) → 0xA8 … for 1 kHz: GYR_ODR=1100 → 0xAC
// Chip default is 0xA9 (BWP=10, ODR=1001 = 200 Hz)
// ---------------------------------------------------------------------------
#define SC7U22_GYR_CONF_HP_1KHZ     0xAC  // High-perf, NORM_AVG4, 1600 Hz ODR
#define SC7U22_GYR_CONF_HP_100HZ    0xA8  // High-perf, NORM_AVG4, 100 Hz ODR

// GYR_RANGE: ±2000 dps = FS[2:0]=000
#define SC7U22_GYR_RANGE_2000DPS    0x00

// ---------------------------------------------------------------------------
// WoM / interrupt registers (for optional wake-on-motion, same principle as
// ICM42607C but using SC7U22 AOI mechanism via AOI1_CFG / AOI2_CFG)
// ---------------------------------------------------------------------------
#define SC7U22_AOI1_CFG_REG         0x30
#define SC7U22_AOI1_STAT_REG        0x31
#define SC7U22_AOI1_THS_REG         0x32
#define SC7U22_AOI1_DURATION_REG    0x33
#define SC7U22_AOI2_CFG_REG         0x34
#define SC7U22_AOI2_STAT_REG        0x35
#define SC7U22_AOI2_THS_REG         0x36
#define SC7U22_AOI2_DURATION_REG    0x37
#define SC7U22_AOI1_AOI2_CFG_REG    0x3F

// INT1_OUT_SEL1 bits (0x05)
#define SC7U22_INT1_ACC             (1 << 0)
#define SC7U22_INT1_GYR             (1 << 2)
