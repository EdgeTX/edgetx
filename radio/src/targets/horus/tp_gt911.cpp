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
#include "tp_gt911.h"
#include "touch.h"

#if defined(RADIO_T18)

// TODO:
//  This config has been provided by Jumper, but seems to be wrong:
//  - coordinates are inverted, and only parts of the screen is covered
//
const uint8_t TOUCH_GT911_Cfg[] = {
    0xFF, 0xE0, 0x01, 0x10, 0x01, 0x05, 0x0C, 0x00, 0x01, 0x18, 0x28, 0x0F,
    0x50, 0x3C, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x18,
    0x1A, 0x1E, 0x14, 0x87, 0x29, 0x0A, 0x4B, 0x4D, 0xD3, 0x07, 0x00, 0x00,
    0x00, 0x19, 0x02, 0x10, 0x32, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x73, 0x94, 0xD5, 0x02, 0x07, 0x00, 0x00, 0x04,
    0xAE, 0x36, 0x00, 0x94, 0x40, 0x00, 0x7E, 0x4C, 0x00, 0x6D, 0x59, 0x00,
    0x5D, 0x6A, 0x00, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18,
    0x1A, 0x1C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02,
    0x04, 0x06, 0x08, 0x0A, 0x0C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24,
    0x26, 0x28, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x78, 0x01
};

#else

//GT911 param table
const uint8_t TOUCH_GT911_Cfg[] =
  {
    GT911_CFG_NUMER,     // 0x8047 Config version
    0xE0,                // 0x8048 X output map : x 480
    0x01,
    0x10,                // 0x804A Y ouptut max : y 272
    0x01,
    GT911_MAX_TP,        // 0x804C Touch number
    0x3C,                // 0x804D Module switch 1 : bit4= xy change Int mode
    0x20,                // 0x804E Module switch 2
    0x22,                // 0x804F Shake_Count
    0x0A,                // 0x8050 Filter
    0x28,                // 0x8051 Larger touch
    0x0F,                // 0x8052 Noise reduction
    0x5A,                // 0x8053 Screen touch level
    0x3C,                // 0x8054 Screen touch leave
    0x03,                // 0x8055 Low power control
    0x05,                // 0x8056 Refresh rate
    0x00,                // 0x8057 X threshold
    0x00,                // 0x8058 Y threshold
    0x00,                // 0x8059 Reserved
    0x00,                // 0x805A Reserved
    0x11,                // 0x805B Space (top, bottom)
    0x11,                // 0x805C Space (left, right)
    0x08,                // 0x805D Mini filter
    0x18,                // 0x805E Strech R0
    0x1A,                // 0x805F Strech R1
    0x1E,                // 0x8060 Strech R2
    0x14,                // 0x8061 Strech RM
    0x87,                // 0x8062 Drv groupA num
    0x29,                // 0x8063 Drv groupB num
    0x0A,                // 0x8064 Sensor num
    0xCF,                // 0x8065 FreqA factor
    0xD1,                // 0x8066 FreqB factor
    0xB2,                // 0x8067 Panel bit freq
    0x04,
    0x00,                // 0x8069 Reserved
    0x00,
    0x00,                // 0x806B Panel tx gain
    0xD8,                // 0x806C Panel rx gain
    0x02,                // 0x806D Panel dump shift
    0x1D,                // 0x806E Drv frame control
    0x00,                // 0x806F Charging level up
    0x01,                // 0x8070 Module switch 3
    0x00,                // 0x8071 Gesture distance
    0x00,                // 0x8072 Gesture long press time
    0x00,                // 0x8073 X/Y slope adjust
    0x00,                // 0x8074 Gesture control
    0x00,                // 0x8075 Gesture switch 1
    0x00,                // 0x8076 Gesture switch 2
    0x00,                // 0x8077 Gesture refresh rate
    0x00,                // 0x8078 Gesture touch level
    0x00,                // 0x8079 New green wake up level
    0xB4,                // 0x807A Freq hopping start
    0xEF,                // 0x807B Freq hopping end
    0x94,                // 0x807C Noise detect time
    0xD5,                // 0x807D Hopping flag
    0x02,                // 0x807E Hopping flag
    0x07,                // 0x807F Noise threshold
    0x00,                // 0x8080 Nois min threshold old
    0x00,                // 0x8081 Reserved
    0x04,                // 0x8082 Hopping sensor group
    0x6E,                // 0x8083 Hopping Seg1 normalize
    0xB9,                // 0x8084 Hopping Seg1 factor
    0x00,                // 0x8085 Main clock adjust
    0x6A,                // 0x8086 Hopping Seg2 normalize
    0xC4,                // 0x8087 Hopping Seg2 factor
    0x00,                // 0x8088 Reserved
    0x66,                // 0x8089 Hopping Seg3 normalize
    0xCF,                // 0x808A Hopping Seg3 factor
    0x00,                // 0x808B Reserved
    0x62,                // 0x808C Hopping Seg4 normalize
    0xDB,                // 0x808D Hopping Seg4 factor
    0x00,                // 0x808E Reserved
    0x5E,                // 0x808F Hopping Seg5 normalize
    0xE8,                // 0x8090 Hopping Seg5 factor
    0x00,                // 0x8091 Reserved
    0x5E,                // 0x8092 Hopping Seg6 normalize
    0x00,                // 0x8093 Key1
    0x00,                // 0x8094 Key2
    0x00,                // 0x8095 Key3
    0x00,                // 0x8096 Key4
    0x00,                // 0x8097 Key area
    0x00,                // 0x8098 Key touch level
    0x00,                // 0x8099 Key leave level
    0x00,                // 0x809A Key sens
    0x00,                // 0x809B Key sens
    0x00,                // 0x809C Key restrain
    0x00,                // 0x809D Key restrain time
    0x00,                // 0x809E Large gesture touch
    0x00,                // 0x809F Reserved
    0x00,                // 0x80A0 Reserved
    0x00,                // 0x80A1 Hotknot noise map
    0x00,                // 0x80A2 Link threshold
    0x00,                // 0x80A3 Pxy threshold
    0x00,                // 0x80A4 GHot dump shift
    0x00,                // 0x80A5 GHot rx gain
    0x00,                // 0x80A6 Freg gain
    0x00,                // 0x80A7 Freg gain 1
    0x00,                // 0x80A8 Freg gain 2
    0x00,                // 0x80A9 Freg gain 3
    0x00,                // 0x80AA Reserved
    0x00,                // 0x80AB Reserved
    0x00,                // 0x80AC Reserved
    0x00,                // 0x80AD Reserved
    0x00,                // 0x80AE Reserved
    0x00,                // 0x80AF Reserved
    0x00,                // 0x80B0 Reserved
    0x00,                // 0x80B1 Reserved
    0x00,                // 0x80B2 Reserved
    0x00,                // 0x80B3 Combine dis
    0x00,                // 0x80B4 Split set
    0x00,                // 0x80B5 Reserved
    0x00,                // 0x80B6 Reserved
    0x14,                // 0x80B7 Sensor CH0
    0x12,                // 0x80B8 Sensor CH1
    0x10,                // 0x80B9 Sensor CH2
    0x0E,                // 0x80BA Sensor CH3
    0x0C,                // 0x80BB Sensor CH4
    0x0A,                // 0x80BC Sensor CH5
    0x08,                // 0x80BD Sensor CH6
    0x06,                // 0x80BE Sensor CH7
    0x04,                // 0x80BF Sensor CH8
    0x02,                // 0x80C0 Sensor CH9
    0xFF,                // 0x80C1 Sensor CH10
    0xFF,                // 0x80C2 Sensor CH11
    0xFF,                // 0x80C3 Sensor CH12
    0xFF,                // 0x80C4 Sensor CH13
    0x00,                // 0x80C5 Reserved
    0x00,                // 0x80C6 Reserved
    0x00,                // 0x80C7 Reserved
    0x00,                // 0x80C8 Reserved
    0x00,                // 0x80C9 Reserved
    0x00,                // 0x80CA Reserved
    0x00,                // 0x80CB Reserved
    0x00,                // 0x80CC Reserved
    0x00,                // 0x80CD Reserved
    0x00,                // 0x80CE Reserved
    0x00,                // 0x80CF Reserved
    0x00,                // 0x80D0 Reserved
    0x00,                // 0x80D1 Reserved
    0x00,                // 0x80D2 Reserved
    0x00,                // 0x80D3 Reserved
    0x00,                // 0x80D4 Reserved
    0x28,                // 0x80D5 Driver CH0
    0x26,                // 0x80D6 Driver CH1
    0x24,                // 0x80D7 Driver CH2
    0x22,                // 0x80D8 Driver CH3
    0x21,                // 0x80D9 Driver CH4
    0x20,                // 0x80DA Driver CH5
    0x1F,                // 0x80DB Driver CH6
    0x1E,                // 0x80DC Driver CH7
    0x1D,                // 0x80DD Driver CH8
    0x0C,                // 0x80DE Driver CH9
    0x0A,                // 0x80DF Driver CH10
    0x08,                // 0x80E0 Driver CH11
    0x06,                // 0x80E1 Driver CH12
    0x04,                // 0x80E2 Driver CH13
    0x02,                // 0x80E3 Driver CH14
    0x00,                // 0x80E4 Driver CH15
    0xFF,                // 0x80E5 Driver CH16
    0xFF,                // 0x80E6 Driver CH17
    0xFF,                // 0x80E7 Driver CH18
    0xFF,                // 0x80E8 Driver CH19
    0xFF,                // 0x80E9 Driver CH20
    0xFF,                // 0x80EA Driver CH21
    0xFF,                // 0x80EB Driver CH22
    0xFF,                // 0x80EC Driver CH23
    0xFF,                // 0x80ED Driver CH24
    0xFF,                // 0x80EE Driver CH25
    0x00,                // 0x80EF Reserved
    0x00,                // 0x80F0 Reserved
    0x00,                // 0x80F1 Reserved
    0x00,                // 0x80F2 Reserved
    0x00,                // 0x80F3 Reserved
    0x00,                // 0x80F4 Reserved
    0x00,                // 0x80F5 Reserved
    0x00,                // 0x80F6 Reserved
    0x00,                // 0x80F7 Reserved
    0x00,                // 0x80F8 Reserved
    0x00,                // 0x80F9 Reserved
    0x00,                // 0x80FA Reserved
    0x00,                // 0x80FB Reserved
    0x00,                // 0x80FC Reserved
    0x00,                // 0x80FD Reserved
    0x00                 // 0x80FE Reserved
  };

#endif

uint8_t touchGT911Flag = 0;
uint8_t touchEventOccured = 0;
struct TouchData touchData;

static void TOUCH_AF_ExtiStop(void)
{
  SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_PortSource, TOUCH_INT_EXTI_PinSource1);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = TOUCH_INT_EXTI_LINE1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);


  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TOUCH_INT_EXTI_IRQn1;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pre-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void TOUCH_AF_ExtiConfig(void)
{
  SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_PortSource, TOUCH_INT_EXTI_PinSource1);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = TOUCH_INT_EXTI_LINE1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TOUCH_INT_EXTI_IRQn1;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pre-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void TOUCH_AF_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TOUCH_RST_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TOUCH_RST_GPIO, &GPIO_InitStructure);

  GPIO_ResetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);

  GPIO_InitStructure.GPIO_Pin = TOUCH_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TOUCH_INT_GPIO, &GPIO_InitStructure);

  GPIO_ResetBits(TOUCH_INT_GPIO, TOUCH_INT_GPIO_PIN);
}

void TOUCH_AF_INT_Change(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = TOUCH_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(TOUCH_INT_GPIO, &GPIO_InitStructure);
}

void I2C_Init()
{
  I2C_DeInit(I2C);

  GPIO_InitTypeDef GPIO_InitStructure;
  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C, &I2C_InitStructure);
  I2C_Cmd(I2C, ENABLE);

  GPIO_PinAFConfig(I2C_GPIO, I2C_SCL_GPIO_PinSource, I2C_GPIO_AF);
  GPIO_PinAFConfig(I2C_GPIO, I2C_SDA_GPIO_PinSource, I2C_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);
}

bool I2C_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C_WaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

uint8_t I2C_GT911_WriteRegister(uint16_t reg, uint8_t * buf, uint8_t len)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, GT_CMD_WR, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return false;

  I2C_SendData(I2C, (uint8_t)((reg & 0xFF00) >> 8));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;
  I2C_SendData(I2C, (uint8_t)(reg & 0x00FF));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;

  /* While there is data to be written */
  while (len--) {
    I2C_SendData(I2C, *buf);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
      return false;
    buf++;
  }

  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return false;

  I2C_GenerateSTOP(I2C, ENABLE);
  return true;
}

bool I2C_GT911_ReadRegister(u16 reg, uint8_t * buf, uint8_t len)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, GT_CMD_WR, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return false;

  I2C_SendData(I2C, (uint8_t)((reg & 0xFF00) >> 8));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    return false;
  I2C_SendData(I2C, (uint8_t)(reg & 0x00FF));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return false;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return false;

  I2C_Send7bitAddress(I2C, GT_CMD_RD, I2C_Direction_Receiver);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    return false;

  if (len > 1) {
    I2C_AcknowledgeConfig(I2C, ENABLE);
  }

  while (len) {
    if (len == 1) {
      I2C_AcknowledgeConfig(I2C, DISABLE);
    }
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
      return false;
    *buf++ = I2C_ReceiveData(I2C);
    len--;
  }

  I2C_GenerateSTOP(I2C, ENABLE);
  return true;
}

uint8_t I2C_GT911_SendConfig(uint8_t mode)
{
  uint8_t buf[2];
  uint8_t i = 0;
  buf[0] = 0;
  buf[1] = mode;
  for (i = 0; i < sizeof(TOUCH_GT911_Cfg); i++)
    buf[0] += TOUCH_GT911_Cfg[i];//check sum

  buf[0] = (~buf[0]) + 1;
  I2C_GT911_WriteRegister(GT_CFGS_REG, (uint8_t *) TOUCH_GT911_Cfg, sizeof(TOUCH_GT911_Cfg));//
  I2C_GT911_WriteRegister(GT_CHECK_REG, buf, 2);//write checksum
  return 0;
}

void touchPanelDeInit(void)
{
  TOUCH_AF_ExtiStop();
}

bool touchPanelInit(void)
{
  uint8_t tmp[4] = {0};

  if (touchGT911Flag) {
    TOUCH_AF_ExtiConfig();
    return true;
  }
  else {
    TRACE("Touchpanel init start ...");

    TOUCH_AF_GPIOConfig(); //SET RST=OUT INT=OUT INT=LOW
    I2C_Init();

    TPRST_LOW();
    TPINT_HIGH();
    delay_us(200);

    TPRST_HIGH();
    delay_ms(6);

    TPINT_LOW();
    delay_ms(55);

    TOUCH_AF_INT_Change();  //Set INT INPUT INT=LOW

    delay_ms(50);

    TRACE("Reading Touch registry");
    I2C_GT911_ReadRegister(GT_PID_REG, tmp, 4);

    if (strcmp((char *) tmp, "911") == 0) //ID==9147
    {
      TRACE("GT911 chip detected");
      tmp[0] = 0X02;
      I2C_GT911_WriteRegister(GT_CTRL_REG, tmp, 1);
      I2C_GT911_ReadRegister(GT_CFGS_REG, tmp, 1);

      TRACE("Chip config Ver:%x\r\n", tmp[0]);
      if (tmp[0] < GT911_CFG_NUMER)  //Config ver
      {
        TRACE("Sending new config %d", GT911_CFG_NUMER);
        I2C_GT911_SendConfig(1);
      }

      delay_ms(10);
      tmp[0] = 0X00;
      I2C_GT911_WriteRegister(GT_CTRL_REG, tmp, 1);  //end reset
      touchGT911Flag = true;

      TOUCH_AF_ExtiConfig();

      return true;
    }
    TRACE("GT911 chip NOT FOUND");
    return false;
  }
}

void touchPanelRead()
{
  uint8_t state = 0;

  if (!touchEventOccured)
    return;

  touchEventOccured = false;
  I2C_GT911_ReadRegister(GT911_READ_XY_REG, &state, 1);

  if ((state & 0x80u) == 0x00) {
    // not ready
    return;
  }

  uint8_t pointsCount = (state & 0x0Fu);

  if (pointsCount > 0 && pointsCount < GT911_MAX_TP) {
    I2C_GT911_ReadRegister(GT911_READ_XY_REG + 1, touchData.data, pointsCount * sizeof(TouchPoint));
    if (touchData.pointsCount == 0) {
      touchState.event = TE_DOWN;
      touchState.startX = touchState.x = touchData.points[0].x;
      touchState.startY = touchState.y = touchData.points[0].y;
    }
    else {
      touchState.deltaX = touchData.points[0].x - touchState.x;
      touchState.deltaY = touchData.points[0].y - touchState.y;
      if (touchState.event == TE_SLIDE || abs(touchState.deltaX) >= SLIDE_RANGE || abs(touchState.deltaY) >= SLIDE_RANGE) {
        touchState.event = TE_SLIDE;
        touchState.x = touchData.points[0].x;
        touchState.y = touchData.points[0].y;
      }
    }
    touchData.pointsCount = pointsCount;
  }
  else {
    if (touchData.pointsCount > 0) {
      touchData.pointsCount = 0;
      if (touchState.event == TE_SLIDE)
        touchState.event = TE_SLIDE_END;
      else
        touchState.event = TE_UP;
    }
  }

  uint8_t zero = 0;
  I2C_GT911_WriteRegister(GT911_READ_XY_REG, &zero, 1);
}

extern "C" void TOUCH_INT_EXTI_IRQHandler1(void)
{
  if (EXTI_GetITStatus(TOUCH_INT_EXTI_LINE1) != RESET) {
    if (g_eeGeneral.backlightMode & e_backlight_mode_keys) {
      // on touch turn the light on
      resetBacklightTimeout();
    }
    touchEventOccured = 1;
    EXTI_ClearITPendingBit(TOUCH_INT_EXTI_LINE1);
  }
}

bool touchPanelEventOccured()
{
  return touchEventOccured;
}
