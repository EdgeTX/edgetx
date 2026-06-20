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

#pragma once

#include "hal/imu.h"

#define ICM42670_I2C_BASE_ADDR      0x69

// Bank 0 register addresses (TDK InvenSense DS-000431 rev 1.1)
#define SIGNAL_PATH_RESET_REG  0x02
#define INT_CONFIG_REG         0x06
#define ACCEL_DATA_X1_REG      0x0B  // upper byte; burst-read gives [X1 X0 Y1 Y0 Z1 Z0]
#define GYRO_DATA_X1_REG       0x11  // upper byte; burst-read gives [X1 X0 Y1 Y0 Z1 Z0]
#define PWR_MGMT0_REG          0x1F
#define GYRO_CONFIG0_REG       0x20
#define ACCEL_CONFIG0_REG      0x21
#define WOM_CONFIG_REG         0x27
#define INT_SOURCE1_REG        0x2C
#define WHO_AM_I_REG           0x75
#define BLK_SEL_W_REG          0x79
#define MADDR_W_REG            0x7A
#define M_W_REG                0x7B

// Bank MREG1 register addresses (accessed indirectly via BLK_SEL_W/MADDR_W/M_W)
#define MREG1_BANK_SEL         0x00
#define ACCEL_WOM_X_THR_REG    0x4B
#define ACCEL_WOM_Y_THR_REG    0x4C
#define ACCEL_WOM_Z_THR_REG    0x4D

// Config values
#define SOFT_RESET_CMD         0x10  // SIGNAL_PATH_RESET.SOFT_RESET_DEVICE_CONFIG

#define PWR_MGMT0_ENABLE       0x0F  // ACCEL_LNM + GYRO_LNM

extern const etx_imu_driver_t imu_icm42670_driver;
