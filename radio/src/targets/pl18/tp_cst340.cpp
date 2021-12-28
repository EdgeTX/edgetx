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
#include "../horus/i2c_driver.h"
#include "tp_cst340.h"

bool touchCST340Flag = false;
volatile static bool touchEventOccured = false;
struct TouchData touchData;
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
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // Not used as 4 bits are used for the pre-emption priority
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
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // Not used as 4 bits are used for the pre-emption priority
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* Currently not used, as no CST340 register needs to be written
bool I2C_CST340_WriteRegister(uint16_t reg, uint8_t * buf, uint8_t len)
{
    uint8_t uAddrAndBuf[6];
    uAddrAndBuf[0] = (uint8_t)((reg & 0xFF00) >> 8);
    uAddrAndBuf[1] = (uint8_t)(reg & 0x00FF);

    if (len > 0)
    {
        for (int i = 0;i < len;i++)
        {
            uAddrAndBuf[i + 2] = buf[i];
        }
    }

    if (HAL_I2C_Master_Transmit(&hi2c1, CST340_I2C_ADDR << 1, uAddrAndBuf, len + 2, 100) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: WriteRegister failed");
        return false;
    }
    return true;
} */

bool I2C_CST340_ReadRegister(uint16_t reg, uint8_t * buf, uint8_t len)
{
    uint8_t uRegAddr[2];
    uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
    uRegAddr[1] = (uint8_t)(reg & 0x00FF);

    if (HAL_I2C_Master_Transmit(&hi2c1, CST340_I2C_ADDR << 1, uRegAddr, 2, 10) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: ReadRegister write reg address failed");
        return false;
    }

    if (HAL_I2C_Master_Receive(&hi2c1, CST340_I2C_ADDR << 1, buf, len, 100) != HAL_OK)
    {
        TRACE("I2C B1 ERROR: ReadRegister read reg address failed");
        return false;
    }
    return true;
}

void touchPanelDeInit(void)
{
  TOUCH_AF_ExtiStop();
  touchCST340Flag = false;
}

bool touchPanelInit(void)
{
  uint8_t tmp[4] = {0};

  if (touchCST340Flag) {
    TOUCH_AF_ExtiConfig();
    return true;
  }
  else {
    TRACE("Touchpanel init start ...");
	
    delay_ms(1);
    // TOUCH RST
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = TOUCH_RST_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(TOUCH_RST_GPIO, &GPIO_InitStructure);

    // TOUCH INT
    GPIO_InitStructure.GPIO_Pin = TOUCH_INT_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(TOUCH_INT_GPIO, &GPIO_InitStructure);

    // I2C configuration
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
	  return false;
    }
    // Configure analog filter
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
      TRACE("I2C B1 ERROR: HAL_I2CEx_ConfigAnalogFilter() failed");
	  return false;
    }
    // Configure digital filter
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
      TRACE("I2C B1 ERROR: HAL_I2CEx_ConfigDigitalFilter() failed");
	  return false;
    }

    TPRST_LOW();
    delay_ms(1);	// FlySky example uses 10ms, datasheet says 0.1us minimally
    TPRST_HIGH();
    delay_ms(400);	// FlySky example uses 20ms only, datasheet says 300ms

	TOUCH_AF_ExtiConfig();
	
    if (!I2C_CST340_ReadRegister(CST340_READ_XY_REG, tmp, 4)) {
      TRACE("CST340 chip NOT FOUND");
      return false;
	}
    touchCST340Flag = true;
  }
  return true;
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
    static uint8_t touchData[4];
    static uint16_t x = 0;
    static uint16_t y = 0; 

    if (!touchEventOccured)
      return internalTouchState;

    touchEventOccured = false;

    internalTouchState.deltaX = 0;
    internalTouchState.deltaY = 0;
    tmr10ms_t now = get_tmr10ms();
    internalTouchState.tapCount = 0;

    if (!I2C_CST340_ReadRegister(CST340_READ_XY_REG, touchData, 4)) {
      touchI2Chiccups++;
      TRACE("CST340 I2C read XY error");
      if (!I2C_ReInit())
          TRACE("I2C B1 ReInit failed");
      return internalTouchState;
    }
	
    if( touchData[0] == 0x06 )
    {
        x = (touchData[1]<<4) + ((touchData[3]>>4)&0x0f);
        y = (touchData[2]<<4) + ((touchData[3])&0x0f);
        if (internalTouchState.event == TE_NONE || internalTouchState.event == TE_UP || internalTouchState.event == TE_SLIDE_END) {
          internalTouchState.event = TE_DOWN;
          internalTouchState.startX = internalTouchState.x = x;
          internalTouchState.startY = internalTouchState.y = y;
          downTime = now;
      } else {
        internalTouchState.deltaX = x - internalTouchState.x;
        internalTouchState.deltaY = y - internalTouchState.y;
        if (internalTouchState.event == TE_SLIDE ||
            abs(internalTouchState.deltaX) >= SLIDE_RANGE ||
            abs(internalTouchState.deltaY) >= SLIDE_RANGE)
        {
          internalTouchState.event = TE_SLIDE;
          internalTouchState.x = x;
          internalTouchState.y = y;
        }
      }
	}
	else
	{
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
      } else if (internalTouchState.event != TE_SLIDE_END) {
        internalTouchState.event = TE_NONE;
      }
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
