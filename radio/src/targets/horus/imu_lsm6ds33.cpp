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
 
#include "edgetx.h"
#include "i2c_driver.h"
#include "imu_lsm6ds33.h"

I2C_HandleTypeDef hi2c2;

sIMUsettings IMUsettings = {-1, -1.0, -1, -1, -1.0};
sIMUoutput IMUoutput = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

float fIMUtemp = 0.0;

bool I2C_LSM6DS33_ReadRegister(uint8_t reg, uint8_t * buf, uint8_t len)
{
    if (HAL_I2C_Master_Transmit(&hi2c2, LSM6DS33_I2C_ADDR << 1, &reg, 1, 10000) != HAL_OK)
    {
        TRACE("I2C B2 ERROR: ReadRegister write reg. address failed");
        return false;
    }

    if (HAL_I2C_Master_Receive(&hi2c2, LSM6DS33_I2C_ADDR << 1, buf, len, 10000) != HAL_OK)
    {
        TRACE("I2C B2 ERROR: ReadRegister read register failed");
        return false;
    }
    return true;
}

bool I2C_LSM6DS33_WriteRegister(uint8_t reg, uint8_t * buf, uint8_t len)
{
    uint8_t uAddrAndBuf[15];
    uAddrAndBuf[0] = reg;

    if (len > 0)
    {
        for (int i = 0;i < len;i++)
        {
            uAddrAndBuf[i + 1] = buf[i];
        }
    }

    if (HAL_I2C_Master_Transmit(&hi2c2, LSM6DS33_I2C_ADDR << 1, uAddrAndBuf, len + 1, 10000) != HAL_OK)
    {
        TRACE("I2C B2 ERROR: WriteRegister failed");
        return false;
    }
    return true;
}

bool IMUgetConf(void)
{
    uint8_t ui8_regs[2] = {0};

    if (!I2C_LSM6DS33_ReadRegister(LSM6DS33_CTRL1_XL_ADDR, ui8_regs, 2))
    {
        TRACE("ERROR: LSM6DS33 did not respond to I2C address");
        return false;
    }

    switch ((ui8_regs[0] >> 4) & 0x0F) // CTRL1_XL
    {
        case LSM6DS33_ODR_0Hz:			IMUsettings.linacc_odr = 0; TRACE("LSM6DS33 lin. acc. output turned off"); break;
        case LSM6DS33_ODR_12_5Hz:		IMUsettings.linacc_odr = 13; break;
        case LSM6DS33_ODR_26Hz:			IMUsettings.linacc_odr = 26; break;
        case LSM6DS33_ODR_52Hz:			IMUsettings.linacc_odr = 52; break;
        case LSM6DS33_ODR_104Hz:		IMUsettings.linacc_odr = 104; break;
        case LSM6DS33_ODR_208Hz:		IMUsettings.linacc_odr = 208; break;
        case LSM6DS33_ODR_416Hz:		IMUsettings.linacc_odr = 417; break;
        case LSM6DS33_ODR_833Hz:		IMUsettings.linacc_odr = 833; break;
        case LSM6DS33_ODR_1_66kHz:		IMUsettings.linacc_odr = 1667; break;
        case LSM6DS33_ODR_3_33kHz:		IMUsettings.linacc_odr = 3333; break;
        case LSM6DS33_ODR_6_66kHz:		IMUsettings.linacc_odr = 6667; break;
        default: 						TRACE("ERROR: LSM6DS33 lin.acc. output data rate at illegal setting"); return false; break;
    }
    if (IMUsettings.linacc_odr > 0) 	TRACE("LSM6DS33 lin. acc. output at %i Hz", IMUsettings.linacc_odr);

    switch ((ui8_regs[0] >> 2) & 0x03)
    {
        case LSM6DS33_LA_FS_2G:			IMUsettings.linacc_mG = 0.061; break;
        case LSM6DS33_LA_FS_4G:			IMUsettings.linacc_mG = 0.122; break;
        case LSM6DS33_LA_FS_8G:			IMUsettings.linacc_mG = 0.244; break;
        case LSM6DS33_LA_FS_16G:		IMUsettings.linacc_mG = 0.488; break;
        default:						TRACE("ERROR: LSM6DS33 lin. acc full scale value at illegal setting"); return false; break;
    }
    TRACE("LSM6DS33 lin. acc. resolution %.2f G", IMUsettings.linacc_mG);

    switch ((ui8_regs[0] >> 2) & 0x03)
    {
        case LSM6DS33_LA_AABW_400Hz:	IMUsettings.linacc_aabw = 400; break;
        case LSM6DS33_LA_AABW_200Hz:	IMUsettings.linacc_aabw = 200; break;
        case LSM6DS33_LA_AABW_100Hz:	IMUsettings.linacc_aabw = 100; break;
        case LSM6DS33_LA_AABW_50Hz:		IMUsettings.linacc_aabw = 50; break;
        default:						TRACE("ERROR: LSM6DS33 lin. acc anti aliasing bandwidth at illegal setting"); return false; break;
    }
    TRACE("LSM6DS33 lin. acc. anti aliasing bandwidth at %i Hz", IMUsettings.linacc_aabw);

    switch ((ui8_regs[1] >> 4) & 0x0F) // CTRL2_G
    {
        case LSM6DS33_ODR_0Hz:			IMUsettings.gyro_odr = 0; TRACE("LSM6DS33 gyro output turned off"); break;
        case LSM6DS33_ODR_12_5Hz:		IMUsettings.gyro_odr = 13; break;
        case LSM6DS33_ODR_26Hz:			IMUsettings.gyro_odr = 26; break;
        case LSM6DS33_ODR_52Hz:			IMUsettings.gyro_odr = 52; break;
        case LSM6DS33_ODR_104Hz:		IMUsettings.gyro_odr = 104; break;
        case LSM6DS33_ODR_208Hz:		IMUsettings.gyro_odr = 208; break;
        case LSM6DS33_ODR_416Hz:		IMUsettings.gyro_odr = 417; break;
        case LSM6DS33_ODR_833Hz:		IMUsettings.gyro_odr = 833; break;
        case LSM6DS33_ODR_1_66kHz:		IMUsettings.gyro_odr = 1667; break;
        default: 						TRACE("ERROR: LSM6DS33 gyro output data rate at illegal setting"); return false; break;
    }
    if (IMUsettings.gyro_odr > 0) 	TRACE("LSM6DS33 gyro output at %i Hz", IMUsettings.gyro_odr);

    switch ((ui8_regs[1] >> 1) & 0x07)
    {
        case LSM6DS33_GY_FS_125dps:		IMUsettings.gyro_mDPS	= 4.375; break;
        case LSM6DS33_GY_FS_250dps:		IMUsettings.gyro_mDPS	= 8.75; break;
        case LSM6DS33_GY_FS_500dps:		IMUsettings.gyro_mDPS	= 17.5; break;
        case LSM6DS33_GY_FS_1000dps:	IMUsettings.gyro_mDPS	= 35.0; break;
        case LSM6DS33_GY_FS_2000dps:	IMUsettings.gyro_mDPS	= 70.0; break;
        default:						TRACE("ERROR: LSM6DS33 gyro full scale value at illegal setting"); return false; break;
    }
    TRACE("LSM6DS33 raw gyro resolution: %.2f milli-degrees/sec.", IMUsettings.gyro_mDPS);
    return true;
}

void init_imu_i2c(void)
{
  TRACE("I2C IMU Init");

  hi2c2.Instance = I2C_B2;
  hi2c2.Init.ClockSpeed = I2C_B2_CLK_RATE;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
      TRACE("I2C B2 ERROR: HAL_I2C_Init() failed");
  }
  // Configure Analogue filter
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
      TRACE("I2C B2 ERROR: HAL_I2CEx_ConfigAnalogFilter() failed");
  }
  // Configure Digital filter
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
      TRACE("I2C B2 ERROR: HAL_I2CEx_ConfigDigitalFilter() failed");
  }
}

bool imu_lsm6ds33_init(void)
{
    uint8_t ui8_reg[2] = {0};

    GPIO_SetBits(AUX_I2C_B2_PWR_GPIO, AUX_I2C_B2_PWR_GPIO_PIN);   // Turn on AUX1 power

    init_imu_i2c();

    if (!I2C_LSM6DS33_ReadRegister(LSM6DS33_WHO_AM_I_ADDR, ui8_reg, 1))
    {
        TRACE("ERROR: LSM6DS33 did not respond to I2C address");
        return false;
    }
    if (ui8_reg[0] != LSM6DS33_WHOAMI)
    {
        TRACE("LSM6DS33 ERROR: Product ID read failes");
        return false;
    }
    TRACE("LSM6DS33 detected");

    // Set IMU configuration
    ui8_reg[0] = (LSM6DS33_ODR_104Hz << 4) | (LSM6DS33_LA_FS_4G << 2) | LSM6DS33_LA_AABW_100Hz; // Linear acceleration configuration
    ui8_reg[1] = (LSM6DS33_ODR_104Hz << 4) | (LSM6DS33_GY_FS_125dps < 1);						// Gyro configuration
    if (!I2C_LSM6DS33_WriteRegister(LSM6DS33_CTRL1_XL_ADDR, ui8_reg, 2))
    {
        TRACE("ERROR: Failed to write LSM6DS33 configuration");
        return false;
    }

    if (!IMUgetConf())
    {
        TRACE("ERROR: Failed ro read LSM6DS33 configuration");
        return false;
    }
    return true;
}

bool imu_lsm6ds33_read()
{
    uint8_t ui8_regs[14] = {0};

    int16_t i16_rawTemperature = 0;
    int16_t i16_rawGyroX = 0;
    int16_t i16_rawGyroY = 0;
    int16_t i16_rawGyroZ = 0;
    int16_t i16_rawAccX = 0;
    int16_t i16_rawAccY = 0;
    int16_t i16_rawAccZ = 0;

    // Check first if new data is available
    if (!I2C_LSM6DS33_ReadRegister(LSM6DS33_STATUS_REG_ADDR, ui8_regs, 1))
    {
        TRACE("ERROR: LSM6DS33 did not respond to I2C address");
        return false;
    }
    if ((ui8_regs[0] & 0x07) != 0x07)
    {
        TRACE("ERROR: LSM6DS33 has no new data available yet");
        return false;
    }

    if (!I2C_LSM6DS33_ReadRegister(LSM6DS33_OUT_TEMP_L_ADDR, ui8_regs, 14))
    {
        TRACE("ERROR: LSM6DS33 did not respond to I2C address");
        return false;
    }

    i16_rawTemperature = (ui8_regs[1] << 8) | ui8_regs[0];
    i16_rawGyroX = (ui8_regs[3] << 8) | ui8_regs[2];
    i16_rawGyroY = (ui8_regs[5] << 8) | ui8_regs[4];
    i16_rawGyroZ = (ui8_regs[7] << 8) | ui8_regs[6];
    i16_rawAccX = (ui8_regs[9] << 8) | ui8_regs[8];
    i16_rawAccY = (ui8_regs[11] << 8) | ui8_regs[10];
    i16_rawAccZ = (ui8_regs[13] << 8) | ui8_regs[12];

    IMUoutput.fGyroXradps = i16_rawGyroX * IMUsettings.gyro_mDPS * M_PI/180.0 / 1000.0;
    IMUoutput.fGyroYradps = i16_rawGyroY * IMUsettings.gyro_mDPS * M_PI/180.0 / 1000.0;
    IMUoutput.fGyroZradps = i16_rawGyroZ * IMUsettings.gyro_mDPS * M_PI/180.0 / 1000.0;

    IMUoutput.fAccX =  i16_rawAccX * IMUsettings.linacc_mG * GRAVITY_EARTH / 1000.0;
    IMUoutput.fAccY =  i16_rawAccY * IMUsettings.linacc_mG * GRAVITY_EARTH / 1000.0;
    IMUoutput.fAccZ =  i16_rawAccZ * IMUsettings.linacc_mG * GRAVITY_EARTH / 1000.0;

    IMUoutput.fTemperatureDegC = (float)i16_rawTemperature / 16.0 + 25.0;
    /*TRACE("LSM6DS33: %.2f deg.C, GyroXYZ [rad/s]: %.2f, %.2f, %.2f, AccXYZ [m/s^2]: %.2f %.2f %.2f",
          IMUoutput.fTemperatureDegC,
          IMUoutput.fGyroXradps, IMUoutput.fGyroYradps, IMUoutput.fGyroZradps,
          IMUoutput.fAccX, IMUoutput.fAccY, IMUoutput.fAccZ); */
    return true;
}
