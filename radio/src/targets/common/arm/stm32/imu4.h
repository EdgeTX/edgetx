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

typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t s16;
typedef int32_t s32;

// read
extern s16 tNewGyro_Gx;  // gyro x
extern s16 tNewGyro_Gy;
extern s16 tNewGyro_Gz;

extern s16 NewGyro_Gx;  // gyro x
extern s16 NewGyro_Gy;  // gyro y
extern s16 NewGyro_Gz;  // gyro z

extern s16 NewAccel_Ax;  // acc x
extern s16 NewAccel_Ay;  // acc y
extern s16 NewAccel_Az;  // acc z

extern s16 AccelSum_Ax;
extern s16 AccelSum_Ay;
extern s16 AccelSum_Az;

extern u16 AccelGyroJust;

extern s32 AccelAdd_X;
extern s32 AccelAdd_Y;
extern s32 AccelAdd_Z;

extern s16 AccelGyroParHX;
extern s16 AccelGyroParHY;

extern s16 AccelGyroParVX;
extern s16 AccelGyroParVY;

extern s16 BalanceAccelGyroX;
extern s16 BalanceAccelGyroY;

extern s16 tBalanceAccelGyroX;  // x,y
extern s16 tBalanceAccelGyroY;

extern u8 HVJustFlag;
extern u8 ROLLJustMode;
extern u8 ROLLJustFlag;

u8 MPU6050_getMotionImu(u8 data);
void IMU4_getValues(u8 *m);
