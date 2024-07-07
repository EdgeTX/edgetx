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

#include "cst8xx_driver.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"

#include "stm32_exti_driver.h"
#include "stm32_gpio.h"
#include "stm32_gpio_driver.h"
#include "stm32_i2c_driver.h"
#include "timers_driver.h"

#include "debug.h"
#include "delays_driver.h"
#include "hal.h"
#include "hal/gpio.h"
#include "hal/i2c_driver.h"
#include "rtos.h"

volatile static bool touchEventOccured;

#define TOUCH_CST836U_I2C_ADDRESS (0x15)

static tc_handle_TypeDef tc_handle = {0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 250  // ms

static TouchState internalTouchState = {};

static void _cst836u_exti_isr(void) { touchEventOccured = true; }

static void TouchAFExtiConfig(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  LL_SYSCFG_SetEXTISource(TOUCH_INT_EXTI_Port, TOUCH_INT_EXTI_SysCfgLine);

  stm32_exti_enable(TOUCH_INT_EXTI_Line, LL_EXTI_TRIGGER_FALLING,
                    _cst836u_exti_isr);
}

void initTouchI2C(void)
{
  TRACE("CST836U I2C Init");

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("CST836U ERROR: i2c_init failed");
    return;
  }
}

#define I2C_TIMEOUT_MAX 5  // 5 ms

bool touch_i2c_read(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len)
{
  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, addr, &reg, 1, 3) < 0) return false;
  delay_us(5);
  if (stm32_i2c_master_rx(TOUCH_I2C_BUS, addr, data, len, I2C_TIMEOUT_MAX) < 0)
    return false;

  return true;
}

static uint8_t TS_IO_Read(uint8_t addr, uint8_t reg)
{
  uint8_t result;
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, &result, 1)) {
    if (--tryCount == 0) break;
    initTouchI2C();
  }
  return result;
}

static uint16_t TS_IO_ReadMultiple(uint8_t addr, uint8_t reg, uint8_t* buffer,
                                   uint16_t length)
{
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, buffer, length)) {
    if (--tryCount == 0) break;
    initTouchI2C();
  }
  return 1;
}

static void touch_cst836u_debug_info(void)
{
#if defined(DEBUG)
  TRACE("cst836u: fw ver 0x%02X %02X",
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_FW_VERSION_H_REG),
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_FW_VERSION_L_REG));
  TRACE("cst836u: module version 0x%02X",
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_MODULE_VERSION_REG));
  TRACE("cst836u: project name 0x%02X",
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_PROJECT_NAME_REG));
  TRACE("cst836u: chip type 0x%02X 0x%02X",
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_CHIP_TYPE_H_REG),
        TS_IO_Read(TOUCH_CST836U_I2C_ADDRESS, CST836U_CHIP_TYPE_L_REG));
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
static void cst836u_TS_GetXY(uint16_t* X, uint16_t* Y, uint32_t* event)
{
  uint8_t regAddress = 0;
  uint8_t dataxy[4];

  if (tc_handle.currActiveTouchIdx < tc_handle.currActiveTouchNb) {
    switch (tc_handle.currActiveTouchIdx) {
      case 0:
        regAddress = CST836U_TOUCH1_XH_REG;
        break;
      case 1:
        regAddress = CST836U_TOUCH2_XH_REG;
        break;
      default:
        break;
    }

    /* Read X and Y positions */
    TS_IO_ReadMultiple(TOUCH_CST836U_I2C_ADDRESS, regAddress, dataxy,
                       sizeof(dataxy));
    /* Send back ready X position to caller */
    *X = ((dataxy[0] & CST836U_MSB_MASK) << 8) | dataxy[1];
    /* Send back ready Y position to caller */
    *Y = ((dataxy[2] & CST836U_MSB_MASK) << 8) | dataxy[3];

    *event = (dataxy[0] & CST836U_TOUCH_EVT_FLAG_MASK) >>
             CST836U_TOUCH_EVT_FLAG_SHIFT;
    /*
    uint32_t weight;
    uint32_t area;
    */
    tc_handle.currActiveTouchIdx++;
  }
}

/**
 * @brief  Return if there is touches detected or not.
 *         Try to detect new touches and forget the old ones (reset internal
 * global variables).
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
  gpio_clear(TOUCH_RST_GPIO);
  delay_ms(10);
  gpio_set(TOUCH_RST_GPIO);
  delay_ms(300);
}

void touchPanelInit(void)
{
  gpio_init(TOUCH_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  initTouchI2C();
  TouchReset();
  TouchAFExtiConfig();
  touch_cst836u_debug_info();
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  cst836u_TS_GetXY(&touchX, &touchY, &tEvent);

  if (tEvent == CST836U_TOUCH_EVT_FLAG_CONTACT) {
    int dx = touchX - internalTouchState.x;
    int dy = touchY - internalTouchState.y;

    internalTouchState.x = touchX;
    internalTouchState.y = touchY;

    if (internalTouchState.event == TE_NONE ||
        internalTouchState.event == TE_UP ||
        internalTouchState.event == TE_SLIDE_END) {
      internalTouchState.startX = internalTouchState.x;
      internalTouchState.startY = internalTouchState.y;
      internalTouchState.event = TE_DOWN;
    } else if (internalTouchState.event == TE_DOWN) {
      if (dx >= SLIDE_RANGE || dx <= -SLIDE_RANGE || dy >= SLIDE_RANGE ||
          dy <= -SLIDE_RANGE) {
        internalTouchState.event = TE_SLIDE;
        internalTouchState.deltaX = (short)dx;
        internalTouchState.deltaY = (short)dy;
      } else {
        internalTouchState.event = TE_DOWN;
        internalTouchState.deltaX = 0;
        internalTouchState.deltaY = 0;
      }
    } else if (internalTouchState.event == TE_SLIDE) {
      internalTouchState.event = TE_SLIDE;  // no change
      internalTouchState.deltaX = (short)dx;
      internalTouchState.deltaY = (short)dy;
    }
  }
}

bool touchPanelEventOccured() { return touchEventOccured; }

TouchState touchPanelRead()
{
  if (!touchEventOccured) return internalTouchState;

  touchEventOccured = false;

  tmr10ms_t now = RTOS_GET_MS();
  internalTouchState.tapCount = 0;

  if (cst836u_TS_DetectTouch()) {
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
  if (internalTouchState.event == TE_UP ||
      internalTouchState.event == TE_SLIDE_END)
    internalTouchState.event = TE_NONE;
  return ret;
}

TouchState getInternalTouchState() { return internalTouchState; }
