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

#pragma once

#define ICM426xx_I2C_ADDR           0x68

// Register addresses
#define WHO_AM_I_REG          0x75
#define PWR_MGMT0_REG         0x1F
#define SIGNAL_PATH_RESET     0x02
#define ACCEL_CONFIG0_REG     0x21
#define GYRO_CONFIG0_REG      0x20
#define INT_CONFIG_REG        0x06
#define TEMP_DATA_X0_REG      0x09

// Config values
#define SOFT_RESET_CMD        0x10

#define PWR_MGMT0_ENABLE      0x0F  // ACCEL_LNM + GYRO_LNM
#define ACCEL_ODR_1KHZ        0x45  // Accel 1 kHz ±4g
#define GYRO_ODR_1KHZ         0x05  // Gyro 1 kHz  ±2000 dps

#define GYRO_DATA_X0_REG      0x11
#define ACCEL_DATA_X0_REG     0x0B

#define BLK_SEL_W_REG         0x79
#define MADDR_W_REG           0x7A
#define M_W_REG               0x7B
#define ACCEL_WOM_X_THR_REG   0x4B
#define ACCEL_WOM_Y_THR_REG   0x4C
#define ACCEL_WOM_Z_THR_REG   0x4D
#define WOM_CONFIG_REG        0x27
#define INT_SOURCE1_REG       0x2C
#define INT_CONFIG_REG        0x06
#define INT_STATUS2_REG       0x3B

typedef struct {
  float fTemperatureDegC; //°C
  float fAccX, fAccY, fAccZ; // m/s^2
  float fGyroXradps, fGyroYradps, fGyroZradps; // rad/s
} sIMUoutput;

extern sIMUoutput IMUoutput;

int gyroInit(void);
int gyroRead(unsigned char*);
