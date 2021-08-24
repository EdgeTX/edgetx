/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"


/* COMMON VALUES FOR ACCEL-GYRO SENSORS */
#define LSM6DS_WHO_AM_I                         0x0f
#define LSM6DS_WHO_AM_I_DEF                     0x69
#define LSM6DS_AXIS_EN_MASK                     0x38
#define LSM6DS_INT1_CTRL_ADDR                   0x0d
#define LSM6DS_INT2_CTRL_ADDR                   0x0e
#define LSM6DS_INT1_FULL                        0x20
#define LSM6DS_INT1_FTH                         0x08
#define LSM6DS_MD1_ADDR                         0x5e
#define LSM6DS_ODR_LIST_NUM                     5
#define LSM6DS_ODR_POWER_OFF_VAL                0x00
#define LSM6DS_ODR_26HZ_VAL                     0x02
#define LSM6DS_ODR_52HZ_VAL                     0x03
#define LSM6DS_ODR_104HZ_VAL                    0x04
#define LSM6DS_ODR_208HZ_VAL                    0x05
#define LSM6DS_ODR_416HZ_VAL                    0x06
#define LSM6DS_FS_LIST_NUM                      4
#define LSM6DS_BDU_ADDR                         0x12
#define LSM6DS_BDU_MASK                         0x40
#define LSM6DS_EN_BIT                           0x01
#define LSM6DS_DIS_BIT                          0x00
#define LSM6DS_FUNC_EN_ADDR                     0x19
#define LSM6DS_FUNC_EN_MASK                     0x04
#define LSM6DS_FUNC_CFG_ACCESS_ADDR             0x01
#define LSM6DS_FUNC_CFG_ACCESS_MASK             0x01
#define LSM6DS_FUNC_CFG_ACCESS_MASK2            0x04
#define LSM6DS_FUNC_CFG_REG2_MASK               0x80
#define LSM6DS_FUNC_CFG_START1_ADDR             0x62
#define LSM6DS_FUNC_CFG_START2_ADDR             0x63
#define LSM6DS_SELFTEST_ADDR                    0x14
#define LSM6DS_SELFTEST_ACCEL_MASK              0x03
#define LSM6DS_SELFTEST_GYRO_MASK               0x0c
#define LSM6DS_SELF_TEST_DISABLED_VAL           0x00
#define LSM6DS_SELF_TEST_POS_SIGN_VAL           0x01
#define LSM6DS_SELF_TEST_NEG_ACCEL_SIGN_VAL     0x02
#define LSM6DS_SELF_TEST_NEG_GYRO_SIGN_VAL      0x03
#define LSM6DS_LIR_ADDR                         0x58
#define LSM6DS_LIR_MASK                         0x01
#define LSM6DS_TIMER_EN_ADDR                    0x58
#define LSM6DS_TIMER_EN_MASK                    0x80
#define LSM6DS_PEDOMETER_EN_ADDR                0x58
#define LSM6DS_PEDOMETER_EN_MASK                0x40
#define LSM6DS_INT2_ON_INT1_ADDR                0x13
#define LSM6DS_INT2_ON_INT1_MASK                0x20
#define LSM6DS_MIN_DURATION_MS                  1638
#define LSM6DS_ROUNDING_ADDR                    0x16
#define LSM6DS_ROUNDING_MASK                    0x04
#define LSM6DS_FIFO_MODE_ADDR                   0x0a
#define LSM6DS_FIFO_MODE_MASK                   0x07
#define LSM6DS_FIFO_MODE_BYPASS                 0x00
#define LSM6DS_FIFO_MODE_CONTINUOS              0x06
#define LSM6DS_FIFO_THRESHOLD_IRQ_MASK          0x08
#define LSM6DS_FIFO_ODR_ADDR                    0x0a
#define LSM6DS_FIFO_ODR_MASK                    0x78
#define LSM6DS_FIFO_ODR_MAX                     0x07
#define LSM6DS_FIFO_ODR_MAX_HZ                  800
#define LSM6DS_FIFO_ODR_OFF                     0x00
#define LSM6DS_FIFO_CTRL3_ADDR                  0x08
#define LSM6DS_FIFO_ACCEL_DECIMATOR_MASK        0x07
#define LSM6DS_FIFO_GYRO_DECIMATOR_MASK         0x38
#define LSM6DS_FIFO_CTRL4_ADDR                  0x09
#define LSM6DS_FIFO_STEP_C_DECIMATOR_MASK       0x38
#define LSM6DS_FIFO_THR_L_ADDR                  0x06
#define LSM6DS_FIFO_THR_H_ADDR                  0x07
#define LSM6DS_FIFO_THR_H_MASK                  0x0f
#define LSM6DS_FIFO_THR_IRQ_MASK                0x08
#define LSM6DS_FIFO_PEDO_E_ADDR                 0x07
#define LSM6DS_FIFO_PEDO_E_MASK                 0x80
#define LSM6DS_FIFO_STEP_C_FREQ                 25

/* CUSTOM VALUES FOR ACCEL SENSOR */
#define LSM6DS_ACCEL_ODR_ADDR                   0x10
#define LSM6DS_ACCEL_ODR_MASK                   0xf0
#define LSM6DS_ACCEL_FS_ADDR                    0x10
#define LSM6DS_ACCEL_FS_MASK                    0x0c
#define LSM6DS_ACCEL_FS_2G_VAL                  0x00
#define LSM6DS_ACCEL_FS_4G_VAL                  0x02
#define LSM6DS_ACCEL_FS_8G_VAL                  0x03
#define LSM6DS_ACCEL_FS_16G_VAL                 0x01
#define LSM6DS_ACCEL_FS_2G_GAIN                 61
#define LSM6DS_ACCEL_FS_4G_GAIN                 122
#define LSM6DS_ACCEL_FS_8G_GAIN                 244
#define LSM6DS_ACCEL_FS_16G_GAIN                488
#define LSM6DS_ACCEL_OUT_X_L_ADDR               0x28
#define LSM6DS_ACCEL_OUT_Y_L_ADDR               0x2a
#define LSM6DS_ACCEL_OUT_Z_L_ADDR               0x2c
#define LSM6DS_ACCEL_AXIS_EN_ADDR               0x18
#define LSM6DS_ACCEL_DRDY_IRQ_MASK              0x01
#define LSM6DS_ACCEL_STD                        1
#define LSM6DS_ACCEL_STD_FROM_PD                2

/* CUSTOM VALUES FOR GYRO SENSOR */
#define LSM6DS_GYRO_ODR_ADDR                    0x11
#define LSM6DS_GYRO_ODR_MASK                    0xf0
#define LSM6DS_GYRO_FS_ADDR                     0x11
#define LSM6DS_GYRO_FS_MASK                     0x0c
#define LSM6DS_GYRO_FS_245_VAL                  0x00
#define LSM6DS_GYRO_FS_500_VAL                  0x01
#define LSM6DS_GYRO_FS_1000_VAL                 0x02
#define LSM6DS_GYRO_FS_2000_VAL                 0x03
#define LSM6DS_GYRO_FS_245_GAIN                 8750
#define LSM6DS_GYRO_FS_500_GAIN                 17500
#define LSM6DS_GYRO_FS_1000_GAIN                35000
#define LSM6DS_GYRO_FS_2000_GAIN                70000
#define LSM6DS_GYRO_OUT_X_L_ADDR                0x22
#define LSM6DS_GYRO_OUT_Y_L_ADDR                0x24
#define LSM6DS_GYRO_OUT_Z_L_ADDR                0x26
#define LSM6DS_GYRO_AXIS_EN_ADDR                0x19
#define LSM6DS_GYRO_DRDY_IRQ_MASK               0x02
#define LSM6DS_GYRO_STD                         6
#define LSM6DS_GYRO_STD_FROM_PD                 2

#define LSM6DS_OUT_XYZ_SIZE                     8

/* CUSTOM VALUES FOR SIGNIFICANT MOTION SENSOR */
#define LSM6DS_SIGN_MOTION_EN_ADDR              0x19
#define LSM6DS_SIGN_MOTION_EN_MASK              0x01
#define LSM6DS_SIGN_MOTION_DRDY_IRQ_MASK        0x40

/* CUSTOM VALUES FOR STEP DETECTOR SENSOR */
#define LSM6DS_STEP_DETECTOR_DRDY_IRQ_MASK      0x80

/* CUSTOM VALUES FOR STEP COUNTER SENSOR */
#define LSM6DS_STEP_COUNTER_DRDY_IRQ_MASK       0x80
#define LSM6DS_STEP_COUNTER_OUT_L_ADDR          0x4b
#define LSM6DS_STEP_COUNTER_OUT_SIZE            2
#define LSM6DS_STEP_COUNTER_RES_ADDR            0x19
#define LSM6DS_STEP_COUNTER_RES_MASK            0x06
#define LSM6DS_STEP_COUNTER_RES_ALL_EN          0x03
#define LSM6DS_STEP_COUNTER_RES_FUNC_EN         0x02
#define LSM6DS_STEP_COUNTER_DURATION_ADDR       0x15

/* CUSTOM VALUES FOR TILT SENSOR */
#define LSM6DS_TILT_EN_ADDR                     0x58
#define LSM6DS_TILT_EN_MASK                     0x20
#define LSM6DS_TILT_DRDY_IRQ_MASK               0x02

#define LSM6DS_ENABLE_AXIS                      0x07
#define LSM6DS_FIFO_DIFF_L                      0x3a
#define LSM6DS_FIFO_DIFF_MASK                   0x0fff
#define LSM6DS_FIFO_DATA_OUT_L                  0x3e
#define LSM6DS_FIFO_ELEMENT_LEN_BYTE            6
#define LSM6DS_FIFO_BYTE_FOR_CHANNEL            2
#define LSM6DS_FIFO_DATA_OVR_2REGS              0x4000
#define LSM6DS_FIFO_DATA_OVR                    0x40

#define LSM6DS_SRC_FUNC_ADDR                    0x53
#define LSM6DS_FIFO_DATA_AVL_ADDR               0x3b

#define LSM6DS_SRC_SIGN_MOTION_DATA_AVL         0x40
#define LSM6DS_SRC_STEP_DETECTOR_DATA_AVL       0x10
#define LSM6DS_SRC_TILT_DATA_AVL                0x20
#define LSM6DS_SRC_STEP_COUNTER_DATA_AVL        0x80
#define LSM6DS_FIFO_DATA_AVL                    0x80
#define LSM6DS_RESET_ADDR                       0x12
#define LSM6DS_RESET_MASK                       0x01

#define LSM6DS_ADDRESS                          0xD6
#define LSM6DSLTR_ID                            0x6A
#define LSM6DS33TR_ID                           0x69

static const char configure[][2] = {
  {LSM6DS_ACCEL_AXIS_EN_ADDR, 0x38},
  {LSM6DS_ACCEL_ODR_ADDR, (0x3 << 4) | (0x1 << 2) | (0x3 << 0)},
  {LSM6DS_GYRO_AXIS_EN_ADDR, 0x38},
  {LSM6DS_GYRO_ODR_ADDR, (3 << 4) | (3 << 2) | (0 << 0)},
  {LSM6DS_INT1_CTRL_ADDR, 0x3},
  {LSM6DS_INT2_CTRL_ADDR, 0x3},
};

static void i2c2Init()
{
  I2C_DeInit(I2C_B2);

  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_ClockSpeed = I2C_B2_SPEED;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C_B2, &I2C_InitStructure);
  I2C_Cmd(I2C_B2, ENABLE);

  GPIO_PinAFConfig(I2C_B2_SCL_GPIO, I2C_B2_SCL_GPIO_PinSource, I2C_B2_GPIO_AF);
  GPIO_PinAFConfig(I2C_B2_SDA_GPIO, I2C_B2_SDA_GPIO_PinSource, I2C_B2_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = I2C_B2_SCL_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_B2_SCL_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = I2C_B2_SDA_GPIO_PIN;
  GPIO_Init(I2C_B2_SDA_GPIO, &GPIO_InitStructure);
}

#define I2C_TIMEOUT_MAX 10000
bool I2C2_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C_B2, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C2_WaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C_B2, event)) {
    if ((timeout--) == 0) {
      TRACE("I2C Timeout!");
      return false;
    }
  }
  return true;
}

int setGyroRegister(uint8_t address, uint8_t value)
{
  if (!I2C2_WaitEventCleared(I2C_FLAG_BUSY))
    return -1;

  I2C_GenerateSTART(I2C_B2, ENABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -1;

  I2C_Send7bitAddress(I2C_B2, LSM6DS_ADDRESS, I2C_Direction_Transmitter);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return -1;

  I2C_SendData(I2C_B2, address);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return -1;

  I2C_SendData(I2C_B2, value);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return -1;

  I2C_GenerateSTOP(I2C_B2, ENABLE);

  return 0;
}

int readGyroRegister(uint8_t address)
{
  if (!I2C2_WaitEventCleared(I2C_FLAG_BUSY))
    return -1;

  I2C_GenerateSTART(I2C_B2, ENABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -1;

  I2C_Send7bitAddress(I2C_B2, LSM6DS_ADDRESS, I2C_Direction_Transmitter);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return -1;

  I2C_SendData(I2C_B2, address);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return -1;

  I2C_GenerateSTART(I2C_B2, ENABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -1;

  I2C_Send7bitAddress(I2C_B2, LSM6DS_ADDRESS, I2C_Direction_Receiver);

  I2C_AcknowledgeConfig(I2C_B2, DISABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
    return -1;

  uint8_t value = I2C_ReceiveData(I2C_B2);

  I2C_GenerateSTOP(I2C_B2, ENABLE);

  return value;
}

int gyroInit()
{
  i2c2Init();

  uint8_t id = readGyroRegister(LSM6DS_WHO_AM_I);
  if (id != LSM6DSLTR_ID && id != LSM6DS33TR_ID) {  // LSM6DS33TR works with LSM6DSLTR code for our use
    return -1;
  }

  setGyroRegister(0x12, 0x05);
  delay_ms(1);
  setGyroRegister(0x12, 0x04);
  delay_ms(1);

  for (uint8_t i = 0; i < DIM(configure); i++) {
    setGyroRegister(configure[i][0], configure[i][1]);
  }

  return 0;
}

int gyroRead(uint8_t buffer[GYRO_BUFFER_LENGTH])
{
  if (!I2C2_WaitEventCleared(I2C_FLAG_BUSY))
    return -1;

  I2C_GenerateSTART(I2C_B2, ENABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -1;

  I2C_Send7bitAddress(I2C_B2, LSM6DS_ADDRESS, I2C_Direction_Transmitter);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return -1;

  I2C_SendData(I2C_B2, LSM6DS_GYRO_OUT_X_L_ADDR);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return -1;

  I2C_GenerateSTART(I2C_B2, ENABLE);
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -1;

  I2C_Send7bitAddress(I2C_B2, LSM6DS_ADDRESS, I2C_Direction_Receiver);

  I2C_AcknowledgeConfig(I2C_B2, ENABLE);
  for (uint8_t i=0; i<GYRO_BUFFER_LENGTH; i++) {
    if (i == GYRO_BUFFER_LENGTH - 1)
      I2C_AcknowledgeConfig(I2C_B2, DISABLE);
    if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
      return -1;
    buffer[i] = I2C_ReceiveData(I2C_B2);
  }

  I2C_GenerateSTOP(I2C_B2, ENABLE);
  return 0;
}
