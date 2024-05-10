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

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "stm32_i2c_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_exti_driver.h"

#include "rtos.h"

#include "hal.h"
#include "timers_driver.h"
#include "delays_driver.h"
#include "cst8xx_driver.h"

#include "debug.h"

volatile static bool touchEventOccured;

#define TOUCH_CST836U_I2C_ADDRESS         (0x15)

enum TouchControllers {TC_NONE, TC_CST836U};
TouchControllers touchController = TC_NONE;

static tc_handle_TypeDef tc_handle = {0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 250 //ms

struct TouchControllerDescriptor
{
  void (*read)(uint16_t * X, uint16_t * Y, uint32_t * event);
  uint8_t (*detectTouch)();
  void (*printDebugInfo)();
  uint32_t contactEvent;
};

static const TouchControllerDescriptor *tc = nullptr;

static TouchState internalTouchState = {};

static void _cst836u_exti_isr(void)
{
  touchEventOccured = true;
}

static void TOUCH_AF_ExtiStop(void)
{
  stm32_exti_disable(TOUCH_INT_EXTI_Line);
}

static void TOUCH_AF_ExtiConfig(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  LL_SYSCFG_SetEXTISource(TOUCH_INT_EXTI_Port, TOUCH_INT_EXTI_SysCfgLine);

  stm32_exti_enable(TOUCH_INT_EXTI_Line,
		    LL_EXTI_TRIGGER_FALLING,
		    _cst836u_exti_isr);
}

static void TOUCH_AF_GPIOConfig(void)
{
  LL_GPIO_InitTypeDef gpioInit;
  LL_GPIO_StructInit(&gpioInit);

  stm32_gpio_enable_clock(TOUCH_RST_GPIO);
  stm32_gpio_enable_clock(TOUCH_INT_GPIO);
  
  gpioInit.Mode = LL_GPIO_MODE_OUTPUT;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  gpioInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpioInit.Pull = LL_GPIO_PULL_NO;

  gpioInit.Pin = TOUCH_RST_GPIO_PIN;
  LL_GPIO_Init(TOUCH_RST_GPIO, &gpioInit);
  LL_GPIO_SetOutputPin(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);

  gpioInit.Pin = TOUCH_INT_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_INPUT;
  gpioInit.Pull = LL_GPIO_PULL_UP;
  gpioInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  LL_GPIO_Init(TOUCH_INT_GPIO, &gpioInit);
  LL_GPIO_SetOutputPin(TOUCH_INT_GPIO, TOUCH_INT_GPIO_PIN);
}

void I2C_Init_Radio(void)
{
  TRACE("CST836U I2C Init");

  if (stm32_i2c_init(TOUCH_I2C_BUS, TOUCH_I2C_CLK_RATE) < 0) {
    TRACE("CST836U ERROR: stm32_i2c_init failed");
    return;
  }
}

// void Touch_DeInit()
// {
//   I2C_DeInit(I2C_B1);
//   __HAL_RCC_I2C1_FORCE_RESET();
//   delay_ms(150);
//   __HAL_RCC_I2C1_RELEASE_RESET();
// }

#define I2C_TIMEOUT_MAX 5 // 5 ms

bool touch_i2c_read(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t len)
{
  if(touchController == TC_CST836U)
  {
    if(stm32_i2c_master_tx(TOUCH_I2C_BUS, addr, &reg, 1, 3) < 0)
      return false;
    delay_us(5);
    if(stm32_i2c_master_rx(TOUCH_I2C_BUS, addr, data, len, I2C_TIMEOUT_MAX) < 0)
      return false;
  } else {
    if (stm32_i2c_read(TOUCH_I2C_BUS, addr, reg, 1, data, len, I2C_TIMEOUT_MAX) < 0)
      return false;
  }

  return true;
}

static uint8_t TS_IO_Read(uint8_t addr, uint8_t reg)
{
  uint8_t retult;
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, &retult, 1)) {
    if (--tryCount == 0) break;
    I2C_Init_Radio();
  }
  return retult;
}

static uint16_t TS_IO_ReadMultiple(uint8_t addr, uint8_t reg, uint8_t * buffer, uint16_t length)
{
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, buffer, length)) {
    if (--tryCount == 0) break;
    I2C_Init_Radio();
  }
  return 1;
}


static void touch_cst836u_debug_info(void)
{
#if defined(DEBUG)
  TRACE("cst836u: fw ver 0x%02X %02X", TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_FW_VERSION_H_REG), TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_FW_VERSION_L_REG));
  TRACE("cst836u: module version 0x%02X", TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_MODULE_VERSION_REG));
  TRACE("cst836u: project name 0x%02X", TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_PROJECT_NAME_REG));
  TRACE("cst836u: chip type 0x%02X 0x%02X", TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_CHIP_TYPE_H_REG), TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_CHIP_TYPE_L_REG));
#endif
}

/**
 * @brief  Get the touch screen X and Y positions values
 *         Manage multi touch thanks to touch Index global
 *         variable 'tc_handle.currActiveTouchIdx'.
 * @param  DeviceAddr: Device address on communication Bus.
 * @param  X: Pointer to X position value
 * @param  Y: Pointer to Y position value
 * @retval None.
 */
static void cst836u_TS_GetXY(uint16_t * X, uint16_t * Y, uint32_t * event)
{
  uint8_t regAddress = 0;
  uint8_t dataxy[4];

  if (tc_handle.currActiveTouchIdx < tc_handle.currActiveTouchNb) {
    switch (tc_handle.currActiveTouchIdx) {
      case 0 :
        regAddress = CST836U_TOUCH1_XH_REG;
        break;
      case 1 :
        regAddress = CST836U_TOUCH2_XH_REG;
        break;
      default :
        break;
    }

    /* Read X and Y positions */
    TS_IO_ReadMultiple(TOUCH_CST836U_I2C_ADDRESS, regAddress, dataxy, sizeof(dataxy));
    /* Send back ready X position to caller */
    *X = ((dataxy[0] & CST836U_MSB_MASK) << 8) | dataxy[1];
    /* Send back ready Y position to caller */
    *Y = ((dataxy[2] & CST836U_MSB_MASK) << 8) | dataxy[3];

    *event = (dataxy[0] & CST836U_TOUCH_EVT_FLAG_MASK) >> CST836U_TOUCH_EVT_FLAG_SHIFT;
    /*
    uint32_t weight;
    uint32_t area;
    */
    tc_handle.currActiveTouchIdx++;
  }
}

/**
 * @brief  Return if there is touches detected or not.
 *         Try to detect new touches and forget the old ones (reset internal global
 *         variables).
 * @param  DeviceAddr: Device address on communication Bus.
 * @retval : Number of active touches detected (can be 0, 1 or 2).
 */
static uint8_t cst836u_TS_DetectTouch()
{
  volatile uint8_t nbTouch = 0;

  /* Read register CST836U_TOUCH_NUM_REG to check number of touches detection */
  nbTouch = TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_TOUCH_NUM_REG);
  if (nbTouch > CST836U_MAX_DETECTABLE_TOUCH) {
    /* If invalid number of touch detected, set it to zero */
    nbTouch = 0;
  }
  tc_handle.currActiveTouchNb = nbTouch;

  tc_handle.currActiveTouchIdx = 0;
  return (nbTouch);
}

void TouchReset()
{
  LL_GPIO_ResetOutputPin(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(10);
  LL_GPIO_SetOutputPin(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(300);
}

static const TouchControllerDescriptor CST836U =
{
    .read = cst836u_TS_GetXY,
    .detectTouch = cst836u_TS_DetectTouch,
    .printDebugInfo = touch_cst836u_debug_info,
    .contactEvent = CST836U_TOUCH_EVT_FLAG_CONTACT
};

void detectTouchController()
{
  if( stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST836U_I2C_ADDRESS, 3, 5) == 0)
  {
    touchController = TC_CST836U;
    tc = &CST836U;
  }
}

void touchPanelInit(void)
{
  TOUCH_AF_GPIOConfig();
  I2C_Init_Radio();
  TouchReset();
  detectTouchController();
  TOUCH_AF_ExtiConfig();

  tc->printDebugInfo();
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  tc->read(&touchX, &touchY, &tEvent);

  if (tEvent == tc->contactEvent) {
    int dx = touchX - internalTouchState.x;
    int dy = touchY - internalTouchState.y;

    internalTouchState.x = touchX;
    internalTouchState.y = touchY;

    if (internalTouchState.event == TE_NONE || internalTouchState.event == TE_UP || internalTouchState.event == TE_SLIDE_END) {
      internalTouchState.startX = internalTouchState.x;
      internalTouchState.startY = internalTouchState.y;
      internalTouchState.event = TE_DOWN;
    }
    else if (internalTouchState.event == TE_DOWN) {
      if (dx >= SLIDE_RANGE || dx <= -SLIDE_RANGE || dy >= SLIDE_RANGE || dy <= -SLIDE_RANGE) {
        internalTouchState.event = TE_SLIDE;
        internalTouchState.deltaX = (short) dx;
        internalTouchState.deltaY = (short) dy;
      }
      else {
        internalTouchState.event = TE_DOWN;
        internalTouchState.deltaX = 0;
        internalTouchState.deltaY = 0;
      }
    }
    else if (internalTouchState.event == TE_SLIDE) {
      internalTouchState.event = TE_SLIDE; //no change
      internalTouchState.deltaX = (short) dx;
      internalTouchState.deltaY = (short) dy;
    }
  }
}

bool touchPanelEventOccured()
{
  return touchEventOccured;
}

TouchState touchPanelRead()
{
  if (!touchEventOccured) return internalTouchState;

  touchEventOccured = false;

  tmr10ms_t now = RTOS_GET_MS();
  internalTouchState.tapCount = 0;

  if (tc->detectTouch()) {
    handleTouch();
    if (internalTouchState.event == TE_DOWN && downTime == 0) {
      downTime = now;
    }
  } else {
    if (internalTouchState.event == TE_DOWN) {
      internalTouchState.event = TE_UP;
      if (now - downTime <= TAP_TIME) {
        if (now - tapTime > TAP_TIME) {
          tapCount = 1;
        } else {
          tapCount++;
        }
        internalTouchState.tapCount = tapCount;
        tapTime = now;
      } else {
        internalTouchState.tapCount = 0;  // not a tap
      }
      downTime = 0;
    } else {
      tapCount = 0;
      internalTouchState.tapCount = 0;
      internalTouchState.event = TE_SLIDE_END;
    }
  }
  TouchState ret = internalTouchState;
  internalTouchState.deltaX = 0;
  internalTouchState.deltaY = 0;
  if(internalTouchState.event == TE_UP || internalTouchState.event == TE_SLIDE_END)
    internalTouchState.event = TE_NONE;
  return ret;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}
