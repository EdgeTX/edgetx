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

#define ICM42627_WHO_AM_I_REG       0x75
#define ICM42627_WHO_AM_I           0x20

#define ICM42627_DEVICE_CONFIG_REG  0x11
#define ICM42627_PWR_MGMT0_REG      0x4E
#define ICM42627_GYRO_CONFIG0_REG   0x4F
#define ICM42627_ACCEL_CONFIG0_REG  0x50
#define ICM42627_GYRO_CONFIG1_REG   0x51
#define ICM42627_ACCEL_GYRO_BW_REG  0x52
#define ICM42627_ACCEL_CONFIG1_REG  0x53
#define ICM42627_BANK_SEL_REG       0x76

#define ICM42627_GYRO_STATIC2_REG   0x0B
#define ICM42627_DATA_REG           0x1D

#define ICM42627_BANK0              0x00
#define ICM42627_BANK1              0x01

#define ICM42627_RESET              0x01
#define ICM42627_PWR_STAGE1         0x0F
#define ICM42627_PWR_STAGE2         0x2F
#define ICM42627_GYRO_CONFIG        0x07
#define ICM42627_ACCEL_CONFIG       0x47
#define ICM42627_GYRO_FILTER        0x0A
#define ICM42627_ACCEL_GYRO_BW      0x22
#define ICM42627_ACCEL_FILTER       0x14
#define ICM42627_GYRO_STATIC2       0x00

extern const etx_imu_driver_t imu_icm42627_driver;
