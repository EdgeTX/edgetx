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

#include "i2c_driver.h"

#define GRAVITY_EARTH					9.80665F	// m/s^2

#define LSM6DS33_I2C_ADDR				0x6A
#define LSM6DS33_WHOAMI					0x69
#define LSM6DS33_TIMEOUT				3 			// 3ms

// LSM6DS33 register map
#define LSM6DS33_FUNC_CFG_ACCESS_ADDR	0x01
#define LSM6DS33_FIFO_CTRL1_ADDR		0x06
#define LSM6DS33_FIFO_CTRL2_ADDR		0x07
#define LSM6DS33_FIFO_CTRL3_ADDR		0x08
#define LSM6DS33_FIFO_CTRL4_ADDR		0x09
#define LSM6DS33_FIFO_CTRL5_ADDR		0x0A
#define LSM6DS33_ORIENT_CFG_G_ADDR		0x0B
#define LSM6DS33_INT1_CTRL_ADDR			0x0D
#define LSM6DS33_INT2_CTRL_ADDR			0x0E
#define LSM6DS33_WHO_AM_I_ADDR			0x0F
#define LSM6DS33_CTRL1_XL_ADDR			0x10
#define LSM6DS33_CTRL2_G_ADDR			0x11
#define LSM6DS33_CTRL3_C_ADDR			0x12
#define LSM6DS33_CTRL4_C_ADDR			0x13
#define LSM6DS33_CTRL5_C_ADDR			0x14
#define LSM6DS33_CTRL6_C_ADDR			0x15
#define LSM6DS33_CTRL7_G_ADDR			0x16
#define LSM6DS33_CTRL8_XL_ADDR			0x17
#define LSM6DS33_CTRL9_XL_ADDR			0x18
#define LSM6DS33_CTRL10_C_ADDR			0x19
#define LSM6DS33_WAKE_UP_SRC_ADDR		0x1B
#define LSM6DS33_TAP_SRC_ADDR			0x1C
#define LSM6DS33_D6D_SRC_ADDR			0x1D
#define LSM6DS33_STATUS_REG_ADDR		0x1E
#define LSM6DS33_OUT_TEMP_L_ADDR		0x20
#define LSM6DS33_OUT_TEMP_H_ADDR		0x21
#define LSM6DS33_OUTX_L_G_ADDR			0x22
#define LSM6DS33_OUTX_H_G_ADDR			0x23
#define LSM6DS33_OUTY_L_G_ADDR			0x24
#define LSM6DS33_OUTY_H_G_ADDR			0x25
#define LSM6DS33_OUTZ_L_G_ADDR			0x26
#define LSM6DS33_OUTZ_H_G_ADDR			0x27
#define LSM6DS33_OUTX_L_XL_ADDR			0x28
#define LSM6DS33_OUTX_H_XL_ADDR			0x29
#define LSM6DS33_OUTY_L_XL_ADDR			0x2A
#define LSM6DS33_OUTY_H_XL_ADDR			0x2B
#define LSM6DS33_OUTZ_L_XL_ADDR			0x2C
#define LSM6DS33_OUTZ_H_XL_ADDR			0x2D
#define LSM6DS33_FIFO_STATUS1_ADDR		0x3A
#define LSM6DS33_FIFO_STATUS2_ADDR		0x3B
#define LSM6DS33_FIFO_STATUS3_ADDR		0x3C
#define LSM6DS33_FIFO_STATUS4_ADDR		0x3D
#define LSM6DS33_FIFO_DATA_OUT_L_ADDR	0x3E
#define LSM6DS33_FIFO_DATA_OUT_H_ADDR	0x3F
#define LSM6DS33_TIMESTAMP0_REG_ADDR	0x40
#define LSM6DS33_TIMESTAMP1_REG_ADDR	0x41
#define LSM6DS33_TIMESTAMP2_REG_ADDR	0x42
#define LSM6DS33_STEP_TIMESTAMP_L_ADDR	0x49
#define LSM6DS33_STEP_TIMESTAMP_H_ADDR	0x4A
#define LSM6DS33_STEP_COUNTER_L_ADDR	0x4B
#define LSM6DS33_STEP_COUNTER_H_ADDR	0x4C
#define LSM6DS33_FUNC_SRC_ADDR			0x53
#define LSM6DS33_TAP_CFG_ADDR			0x58
#define LSM6DS33_TAP_THS_6D_ADDR		0x59
#define LSM6DS33_INT_DUR2_ADDR			0x5A
#define LSM6DS33_WAKE_UP_THS_ADDR		0x5B
#define LSM6DS33_WAKE_UP_DUR_ADDR		0x5C
#define LSM6DS33_FREE_FALL_ADDR			0x5D
#define LSM6DS33_MD1_CFG_ADDR			0x5E
#define LSM6DS33_MD2_CFG_ADDR			0x5F

#define LSM6DS33_ODR_0Hz				0x00
#define LSM6DS33_ODR_12_5Hz				0x01
#define LSM6DS33_ODR_26Hz				0x02
#define LSM6DS33_ODR_52Hz				0x03
#define LSM6DS33_ODR_104Hz				0x04
#define LSM6DS33_ODR_208Hz				0x05
#define LSM6DS33_ODR_416Hz				0x06
#define LSM6DS33_ODR_833Hz				0x07
#define LSM6DS33_ODR_1_66kHz			0x08
#define LSM6DS33_ODR_3_33kHz			0x09
#define LSM6DS33_ODR_6_66kHz			0x0A

#define LSM6DS33_LA_FS_2G				0x00
#define LSM6DS33_LA_FS_16G				0x01
#define LSM6DS33_LA_FS_4G				0x02
#define LSM6DS33_LA_FS_8G				0x03

#define LSM6DS33_LA_AABW_400Hz			0x00
#define LSM6DS33_LA_AABW_200Hz			0x01
#define LSM6DS33_LA_AABW_100Hz			0x02
#define LSM6DS33_LA_AABW_50Hz			0x03

#define LSM6DS33_GY_FS_125dps			0x01
#define LSM6DS33_GY_FS_250dps			0x00
#define LSM6DS33_GY_FS_500dps			0x02
#define LSM6DS33_GY_FS_1000dps			0x04
#define LSM6DS33_GY_FS_2000dps			0x05

typedef struct {
    int16_t linacc_odr;
    float linacc_mG; // milli G per Bit
    int16_t linacc_aabw;
    int16_t gyro_odr;
    float gyro_mDPS; // milli degrees per second per Bit
} sIMUsettings;

typedef struct {
    float fTemperatureDegC; //°C
    float fAccX, fAccY, fAccZ; // m/s^2
    float fGyroXradps, fGyroYradps, fGyroZradps; // rad/s
} sIMUoutput;

extern sIMUoutput IMUoutput;

bool imu_lsm6ds33_init(void);
bool imu_lsm6ds33_read();
