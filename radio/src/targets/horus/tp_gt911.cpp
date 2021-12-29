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

#include "opentx.h"
#include "i2c_driver.h"
#include "tp_gt911.h"

#if defined (RADIO_T18)
const uint8_t TOUCH_GT911_Cfg[] = {
    GT911_CFG_NUMER,  // 0x8047 Config version
    0xE0,             // 0x8048 X output map : x 480
    0x01,
    0x10,  // 0x804A Y ouptut max : y 272
    0x01,
    GT911_MAX_TP,  // 0x804C Touch number
    0x0C,          // 0x804D Module switch 1 : bit4= xy change Int mode
    0x00,          // 0x804E Module switch 2
    0x01,          // 0x804F Shake_Count
    0x18,          // 0x8050 Filter
    0x28,          // 0x8051 Larger touch
    0x0F,          // 0x8052 Noise reduction
    0x50,          // 0x8053 Screen touch level
    0x3C,          // 0x8054 Screen touch leave
    0x03,          // 0x8055 Low power control
    0x0F,          // 0x8056 Refresh rate
    0x01,          // 0x8057 X threshold
    0x01,          // 0x8058 Y threshold
    0x00,          // 0x8059 Reserved
    0x00,          // 0x805A Reserved
    0x00,          // 0x805B Space (top, bottom)
    0x00,          // 0x805C Space (left, right)
    0x04,          // 0x805D Mini filter
    0x18,          // 0x805E Strech R0
    0x1A,          // 0x805F Strech R1
    0x1E,          // 0x8060 Strech R2
    0x14,          // 0x8061 Strech RM
    0x87,          // 0x8062 Drv groupA num
    0x29,          // 0x8063 Drv groupB num
    0x0A,          // 0x8064 Sensor num
    0x4B,          // 0x8065 FreqA factor
    0x4D,          // 0x8066 FreqB factor
    0xD3,          // 0x8067 Panel bit freq
    0x07,
    0x00,  // 0x8069 Reserved
    0x00,
    0x00,  // 0x806B Panel tx gain
    0x19,  // 0x806C Panel rx gain
    0x02,  // 0x806D Panel dump shift
    0x10,  // 0x806E Drv frame control
    0x32,  // 0x806F Charging level up
    0x00,  // 0x8070 Module switch 3
    0x00,  // 0x8071 Gesture distance
    0x00,  // 0x8072 Gesture long press time
    0x00,  // 0x8073 X/Y slope adjust
    0x03,  // 0x8074 Gesture control
    0x00,  // 0x8075 Gesture switch 1
    0x00,  // 0x8076 Gesture switch 2
    0x00,  // 0x8077 Gesture refresh rate
    0x00,  // 0x8078 Gesture touch level
    0x00,  // 0x8079 New green wake up level
    0x32,  // 0x807A Freq hopping start
    0x73,  // 0x807B Freq hopping end
    0x94,  // 0x807C Noise detect time
    0xD5,  // 0x807D Hopping flag
    0x02,  // 0x807E Hopping flag
    0x07,  // 0x807F Noise threshold
    0x00,  // 0x8080 Nois min threshold old
    0x00,  // 0x8081 Reserved
    0x04,  // 0x8082 Hopping sensor group
    0xAE,  // 0x8083 Hopping Seg1 normalize
    0x36,  // 0x8084 Hopping Seg1 factor
    0x00,  // 0x8085 Main clock adjust
    0x94,  // 0x8086 Hopping Seg2 normalize
    0x40,  // 0x8087 Hopping Seg2 factor
    0x00,  // 0x8088 Reserved
    0x7E,  // 0x8089 Hopping Seg3 normalize
    0x4C,  // 0x808A Hopping Seg3 factor
    0x00,  // 0x808B Reserved
    0x6D,  // 0x808C Hopping Seg4 normalize
    0x59,  // 0x808D Hopping Seg4 factor
    0x00,  // 0x808E Reserved
    0x5D,  // 0x808F Hopping Seg5 normalize
    0x6A,  // 0x8090 Hopping Seg5 factor
    0x00,  // 0x8091 Reserved
    0x5D,  // 0x8092 Hopping Seg6 normalize
    0x00,  // 0x8093 Key1
    0x00,  // 0x8094 Key2
    0x00,  // 0x8095 Key3
    0x00,  // 0x8096 Key4
    0x00,  // 0x8097 Key area
    0x00,  // 0x8098 Key touch level
    0x00,  // 0x8099 Key leave level
    0x00,  // 0x809A Key sens
    0x00,  // 0x809B Key sens
    0x00,  // 0x809C Key restrain
    0x00,  // 0x809D Key restrain time
    0x00,  // 0x809E Large gesture touch
    0x00,  // 0x809F Reserved
    0x00,  // 0x80A0 Reserved
    0x00,  // 0x80A1 Hotknot noise map
    0x00,  // 0x80A2 Link threshold
    0x00,  // 0x80A3 Pxy threshold
    0x00,  // 0x80A4 GHot dump shift
    0x00,  // 0x80A5 GHot rx gain
    0x00,  // 0x80A6 Freg gain
    0x00,  // 0x80A7 Freg gain 1
    0x00,  // 0x80A8 Freg gain 2
    0x00,  // 0x80A9 Freg gain 3
    0x00,  // 0x80AA Reserved
    0x00,  // 0x80AB Reserved
    0x00,  // 0x80AC Reserved
    0x00,  // 0x80AD Reserved
    0x00,  // 0x80AE Reserved
    0x00,  // 0x80AF Reserved
    0x00,  // 0x80B0 Reserved
    0x00,  // 0x80B1 Reserved
    0x00,  // 0x80B2 Reserved
    0x00,  // 0x80B3 Combine dis
    0x00,  // 0x80B4 Split set
    0x00,  // 0x80B5 Reserved
    0x00,  // 0x80B6 Reserved
    0x0A,  // 0x80B7 Sensor CH0
    0x0C,  // 0x80B8 Sensor CH1
    0x0E,  // 0x80B9 Sensor CH2
    0x10,  // 0x80BA Sensor CH3
    0x12,  // 0x80BB Sensor CH4
    0x14,  // 0x80BC Sensor CH5
    0x16,  // 0x80BD Sensor CH6
    0x18,  // 0x80BE Sensor CH7
    0x1A,  // 0x80BF Sensor CH8
    0x1C,  // 0x80C0 Sensor CH9
    0xFF,  // 0x80C1 Sensor CH10
    0xFF,  // 0x80C2 Sensor CH11
    0xFF,  // 0x80C3 Sensor CH12
    0xFF,  // 0x80C4 Sensor CH13
    0xFF,  // 0x80C5 Reserved
    0xFF,  // 0x80C6 Reserved
    0xFF,  // 0x80C7 Reserved
    0xFF,  // 0x80C8 Reserved
    0xFF,  // 0x80C9 Reserved
    0xFF,  // 0x80CA Reserved
    0xFF,  // 0x80CB Reserved
    0xFF,  // 0x80CC Reserved
    0xFF,  // 0x80CD Reserved
    0xFF,  // 0x80CE Reserved
    0xFF,  // 0x80CF Reserved
    0xFF,  // 0x80D0 Reserved
    0xFF,  // 0x80D1 Reserved
    0xFF,  // 0x80D2 Reserved
    0xFF,  // 0x80D3 Reserved
    0xFF,  // 0x80D4 Reserved
    0x00,  // 0x80D5 Driver CH0
    0x02,  // 0x80D6 Driver CH1
    0x04,  // 0x80D7 Driver CH2
    0x06,  // 0x80D8 Driver CH3
    0x08,  // 0x80D9 Driver CH4
    0x0A,  // 0x80DA Driver CH5
    0x0C,  // 0x80DB Driver CH6
    0x1D,  // 0x80DC Driver CH7
    0x1E,  // 0x80DD Driver CH8
    0x1F,  // 0x80DE Driver CH9
    0x20,  // 0x80DF Driver CH10
    0x21,  // 0x80E0 Driver CH11
    0x22,  // 0x80E1 Driver CH12
    0x24,  // 0x80E2 Driver CH13
    0x26,  // 0x80E3 Driver CH14
    0x28,  // 0x80E4 Driver CH15
    0xFF,  // 0x80E5 Driver CH16
    0xFF,  // 0x80E6 Driver CH17
    0xFF,  // 0x80E7 Driver CH18
    0xFF,  // 0x80E8 Driver CH19
    0xFF,  // 0x80E9 Driver CH20
    0xFF,  // 0x80EA Driver CH21
    0xFF,  // 0x80EB Driver CH22
    0xFF,  // 0x80EC Driver CH23
    0xFF,  // 0x80ED Driver CH24
    0xFF,  // 0x80EE Driver CH25
    0xFF,  // 0x80EF Reserved
    0xFF,  // 0x80F0 Reserved
    0xFF,  // 0x80F1 Reserved
    0xFF,  // 0x80F2 Reserved
    0xFF,  // 0x80F3 Reserved
    0xFF,  // 0x80F4 Reserved
    0xFF,  // 0x80F5 Reserved
    0xFF,  // 0x80F6 Reserved
    0xFF,  // 0x80F7 Reserved
    0xFF,  // 0x80F8 Reserved
    0xFF,  // 0x80F9 Reserved
    0xFF,  // 0x80FA Reserved
    0xFF,  // 0x80FB Reserved
    0xFF,  // 0x80FC Reserved
    0xFF,  // 0x80FD Reserved
    0xFF   // 0x80FE Reserved
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
    0x0F,                // 0x8056 Refresh rate
    0x01,                // 0x8057 X threshold
    0x01,                // 0x8058 Y threshold
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

bool touchGT911Flag = false;
volatile static bool touchEventOccured = false;
struct TouchData touchData;
uint16_t touchICfwver = 0;
uint32_t touchI2Chiccups = 0;
tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 25

I2C_HandleTypeDef hi2c1;

static TouchState internalTouchState = {};

static void TOUCH_AF_ExtiStop(void)
{
  SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_PortSource, TOUCH_INT_EXTI_PinSource1);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = TOUCH_INT_EXTI_LINE1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
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
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
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

void I2C_Init_Radio(void)
{
  TRACE("I2C B1 Init");

  hi2c1.Instance = I2C_B1;
  hi2c1.Init.ClockSpeed = I2C_B1_CLK_RATE;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
      TRACE("I2C B1 ERROR: HAL_I2C_Init() failed");
  }
  // Configure Analogue filter
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
      TRACE("I2C B1 ERROR: HAL_I2CEx_ConfigAnalogFilter() failed");
  }
  // Configure Digital filter
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
      TRACE("I2C B1 ERROR: HAL_I2CEx_ConfigDigitalFilter() failed");
  }
}

bool I2C_GT911_WriteRegister(uint16_t reg, uint8_t * buf, uint8_t len)
{
    uint8_t uAddrAndBuf[258];
    uAddrAndBuf[0] = (uint8_t)((reg & 0xFF00) >> 8);
    uAddrAndBuf[1] = (uint8_t)(reg & 0x00FF);

    if (len > 0)
    {
        for (int i = 0;i < len;i++)
        {
            uAddrAndBuf[i + 2] = buf[i];
        }
    }

    if (HAL_I2C_Master_Transmit(&hi2c1, GT911_I2C_ADDR << 1, uAddrAndBuf, len + 2, 100) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: WriteRegister failed");
        return false;
    }
    return true;
}

bool I2C_GT911_ReadRegister(uint16_t reg, uint8_t * buf, uint8_t len)
{
    uint8_t uRegAddr[2];
    uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
    uRegAddr[1] = (uint8_t)(reg & 0x00FF);

    if (HAL_I2C_Master_Transmit(&hi2c1, GT911_I2C_ADDR << 1, uRegAddr, 2, 10) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: ReadRegister write reg address failed");
        return false;
    }

    if (HAL_I2C_Master_Receive(&hi2c1, GT911_I2C_ADDR << 1, buf, len, 100) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: ReadRegister read reg address failed");
        return false;
    }
    return true;
}

bool I2C_GT911_SendConfig(void)
{
  uint8_t buf[2];
  uint8_t i = 0;
  buf[0] = 0;
  buf[1] = 1;
  bool bResult = true;

  for (i = 0; i < sizeof(TOUCH_GT911_Cfg); i++)
    buf[0] += TOUCH_GT911_Cfg[i];//check sum

  buf[0] = (~buf[0]) + 1;
  if (!I2C_GT911_WriteRegister(GT_CFGS_REG, (uint8_t *) TOUCH_GT911_Cfg, sizeof(TOUCH_GT911_Cfg)))
  {
    TRACE("GT911 ERROR: write config failed");
    bResult = false;
  }

  if (!I2C_GT911_WriteRegister(GT_CHECK_REG, buf, 2)) //write checksum
  {
    TRACE("GT911 ERROR: write config checksum failed");
    bResult = false;
  }
  return bResult;
}

void touchPanelDeInit(void)
{
  TOUCH_AF_ExtiStop();
  touchGT911Flag = false;
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
    I2C_Init_Radio();

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
    if (!I2C_GT911_ReadRegister(GT_PID_REG, tmp, 4))
    {
      TRACE("GT911 ERROR: Product ID read failed");
    }

    if (strcmp((char *) tmp, "911") == 0) //ID==9147
    {
      TRACE("GT911 chip detected");
      tmp[0] = 0X02;
      if (!I2C_GT911_WriteRegister(GT_CTRL_REG, tmp, 1))
      {
        TRACE("GT911 ERROR: write to control register failed");
      }
      if (!I2C_GT911_ReadRegister(GT_CFGS_REG, tmp, 1))
      {
          TRACE("GT911 ERROR: configuration register read failed");
      }

      TRACE("Chip config Ver:%x", tmp[0]);
      if (tmp[0] < GT911_CFG_NUMER)  //Config ver
      {
        TRACE("Sending new config %d", GT911_CFG_NUMER);
        if (!I2C_GT911_SendConfig())
        {
          TRACE("GT911 ERROR: sending configration failed");
        }
      }

      if (!I2C_GT911_ReadRegister(GT911_FIRMWARE_VERSION_REG, tmp, 2))
      {
        TRACE("GT911 ERROR: reading firmware version failed");
      }
      else
      {
          touchICfwver = (tmp[1] << 8) + tmp[0];
          TRACE("GT911 FW version: %u", touchICfwver);
      }

      delay_ms(10);
      tmp[0] = 0X00;
      if (!I2C_GT911_WriteRegister(GT_CTRL_REG, tmp, 1))  //end reset
      {
        TRACE("GT911 ERROR: write to control register failed");
      }
      touchGT911Flag = true;

      TOUCH_AF_ExtiConfig();


      return true;
    }
    TRACE("GT911 chip NOT FOUND");
    return false;
  }
}

bool I2C_ReInit(void)
{
    TRACE("I2C B1 ReInit");
    touchPanelDeInit();
    if (HAL_I2C_DeInit(&hi2c1) != HAL_OK)
        TRACE("I2C B1 ReInit - I2C DeInit failed");

    // If DeInit fails, try to re-init anyway
    if (!touchPanelInit())
    {
        TRACE("I2C B1 ReInit - touchPanelInit failed");
        return false;
    }
    return true;
}


#if defined(SIMU) || defined(SEMIHOSTING) || defined(DEBUG)
static const char* event2str(uint8_t ev)
{
  switch(ev){
  case TE_NONE:
    return "NONE";
  case TE_UP:
    return "UP";
  case TE_DOWN:
    return "DOWN";
  case TE_SLIDE_END:
    return "SLIDE_END";
  case TE_SLIDE:
    return "SLIDE";
  default:
    return "UNKNOWN";
  }
}
#endif

struct TouchState touchPanelRead()
{
  uint8_t state = 0;

  if (!touchEventOccured)
    return internalTouchState;

  touchEventOccured = false;

  uint32_t startReadStatus = RTOS_GET_MS();
  do {
    if (!I2C_GT911_ReadRegister(GT911_READ_XY_REG, &state, 1)) {
      //ledRed();
      touchI2Chiccups++;
      TRACE("GT911 I2C read XY error");
      if (!I2C_ReInit())
          TRACE("I2C B1 ReInit failed");
      return internalTouchState;
    }

    if (state & 0x80u) {
      // ready
      break;
    }
    RTOS_WAIT_MS(1);
  } while(RTOS_GET_MS() - startReadStatus < GT911_TIMEOUT);

  internalTouchState.deltaX = 0;
  internalTouchState.deltaY = 0;
  TRACE("touch state = 0x%x", state);
  if (state & 0x80u) {
    uint8_t pointsCount = (state & 0x0Fu);
    tmr10ms_t now = get_tmr10ms();
    internalTouchState.tapCount = 0;

    if (pointsCount > 0 && pointsCount <= GT911_MAX_TP) {
      if (!I2C_GT911_ReadRegister(GT911_READ_XY_REG + 1, touchData.data,
                                  pointsCount * sizeof(TouchPoint))) {
        //ledRed();
        touchI2Chiccups++;
        TRACE("GT911 I2C data read error");
        if (!I2C_ReInit())
            TRACE("I2C B1 ReInit failed");
        return internalTouchState;
      }
      if (internalTouchState.event == TE_NONE || internalTouchState.event == TE_UP ||
          internalTouchState.event == TE_SLIDE_END) {
        internalTouchState.event = TE_DOWN;
        internalTouchState.startX = internalTouchState.x = touchData.points[0].x;
        internalTouchState.startY = internalTouchState.y = touchData.points[0].y;
        downTime = now;
      } else {
        internalTouchState.deltaX = touchData.points[0].x - internalTouchState.x;
        internalTouchState.deltaY = touchData.points[0].y - internalTouchState.y;
        if (internalTouchState.event == TE_SLIDE ||
            abs(internalTouchState.deltaX) >= SLIDE_RANGE ||
            abs(internalTouchState.deltaY) >= SLIDE_RANGE) {
          internalTouchState.event = TE_SLIDE;
          internalTouchState.x = touchData.points[0].x;
          internalTouchState.y = touchData.points[0].y;
        }
      }
    } else {
      if (internalTouchState.event == TE_SLIDE) {
        internalTouchState.event = TE_SLIDE_END;
      } else if (internalTouchState.event == TE_DOWN) {
        internalTouchState.event = TE_UP;
        if (now - downTime <= TAP_TIME) {
          if (now - tapTime > TAP_TIME)
            tapCount = 1;
          else
            tapCount++;
          internalTouchState.tapCount = tapCount;
          tapTime = now;
        }
      } else {
        internalTouchState.event = TE_NONE;
      }
    }
  }

  uint8_t zero = 0;
  if (!I2C_GT911_WriteRegister(GT911_READ_XY_REG, &zero, 1))
  {
    TRACE("GT911 ERROR: clearing XY register failed");
  }

  TRACE("touch event = %s", event2str(internalTouchState.event));
  return internalTouchState;
}

extern "C" void TOUCH_INT_EXTI_IRQHandler1(void)
{
  if (EXTI_GetITStatus(TOUCH_INT_EXTI_LINE1) != RESET) {
    touchEventOccured = true;
    EXTI_ClearITPendingBit(TOUCH_INT_EXTI_LINE1);
  }
}

bool touchPanelEventOccured()
{
  return touchEventOccured;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}

