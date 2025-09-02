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

#include "hal/gpio.h"
#include "hal/i2c_driver.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "stm32_i2c_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_exti_driver.h"

#include "hal.h"
#include "timers_driver.h"
#include "tp_gt911.h"
#include "delays_driver.h"

#include "os/sleep.h"
#include "edgetx_types.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

#define TP_GT911_ID "911"


bool touchGT911Flag = false;
volatile static bool touchEventOccured = false;
struct TouchData touchData;
uint16_t touchGT911fwver = 0;
uint32_t touchGT911hiccups = 0;

static tmr10ms_t downTime = 0;
static tmr10ms_t tapTime = 0;
static short tapCount = 0;

static TouchState internalTouchState = {};

static void _gt911_exti_isr(void)
{
  touchEventOccured = true;
}

void I2C_Init_Radio(void)
{
  TRACE("GT911 I2C Init");

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("GT911 ERROR: i2c_init failed");
    return;
  }
}

bool I2C_GT911_WriteRegister(uint16_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uAddrAndBuf[258];
  uAddrAndBuf[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uAddrAndBuf[1] = (uint8_t)(reg & 0x00FF);

  if (len > 0) {
    for (int i = 0; i < len; i++) {
      uAddrAndBuf[i + 2] = buf[i];
    }
  }

  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, GT911_I2C_ADDR, uAddrAndBuf, len + 2,
                          100) < 0) {
    TRACE("I2C B1 ERROR: WriteRegister failed");
    return false;
  }
  return true;
}

bool I2C_GT911_ReadRegister(uint16_t reg, uint8_t *buf, uint8_t len)
{
  uint8_t uRegAddr[2];
  uRegAddr[0] = (uint8_t)((reg & 0xFF00) >> 8);
  uRegAddr[1] = (uint8_t)(reg & 0x00FF);

  if (stm32_i2c_master_tx(TOUCH_I2C_BUS, GT911_I2C_ADDR, uRegAddr, 2, 10) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister write reg address failed");
    return false;
  }

  if (stm32_i2c_master_rx(TOUCH_I2C_BUS, GT911_I2C_ADDR, buf, len, 100) < 0) {
    TRACE("I2C B1 ERROR: ReadRegister read reg address failed");
    return false;
  }
  return true;
}

void touchPanelDeInit(void)
{
  gpio_int_disable(TOUCH_INT_GPIO);
  touchGT911Flag = false;
}

uint8_t tp_gt911_cfgVer = GT911_CFG_NUMBER;

bool touchPanelInit(void)
{
  uint8_t tmp[4] = {0};

  if (touchGT911Flag) {
    gpio_init_int(TOUCH_INT_GPIO, GPIO_IN_PU, GPIO_RISING, _gt911_exti_isr);
    return true;
  } else {
    TRACE("Touchpanel init start ...");

    gpio_init(TOUCH_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
    gpio_init(TOUCH_INT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
    I2C_Init_Radio();

    gpio_clear(TOUCH_RST_GPIO);
    gpio_set(TOUCH_INT_GPIO);
    delay_us(200);

    gpio_set(TOUCH_RST_GPIO);
    delay_ms(6);

    gpio_clear(TOUCH_INT_GPIO);
    delay_ms(55);

    gpio_init(TOUCH_INT_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);

    delay_ms(50);

    TRACE("Reading Touch registry");
    if (!I2C_GT911_ReadRegister(GT911_PRODUCT_ID_REG, tmp, 4)) {
      TRACE("GT911 ERROR: Product ID read failed");
    }

    if (strcmp((char *)tmp, TP_GT911_ID) == 0) {
      TRACE("GT911 chip detected");
      tmp[0] = 0X02;
      if (!I2C_GT911_WriteRegister(GT911_COMMAND_REG, tmp, 1)) {
        TRACE("GT911 ERROR: write to control register failed");
      }
      if (!I2C_GT911_ReadRegister(GT911_CONFIG_REG, tmp, 1)) {
        TRACE("GT911 ERROR: configuration register read failed");
      }

      TRACE("Chip config Ver:%x", tmp[0]);
      if ((tp_gt911_cfgVer == 0) || (tmp[0] < tp_gt911_cfgVer)) { // Config ver
        TRACE("Sending new config %d", GT911_CFG_NUMBER);
        if (!I2C_GT911_ReadRegister(GT911_CONFIG_REG, tmp, 1)) {
          TRACE("GT911 ERROR: configuration register read failed");
        }
        tp_gt911_cfgVer = tmp[0];
      }

      if (!I2C_GT911_ReadRegister(GT911_FIRMWARE_VERSION_REG, tmp, 2)) {
        TRACE("GT911 ERROR: reading firmware version failed");
      } else {
        touchGT911fwver = (tmp[1] << 8) + tmp[0];
        TRACE("GT911 FW version: %u", touchGT911fwver);
      }

      delay_ms(10);
      tmp[0] = 0X00;
      if (!I2C_GT911_WriteRegister(GT911_COMMAND_REG, tmp, 1)) { // end reset
        TRACE("GT911 ERROR: write to command register failed");
      }
      touchGT911Flag = true;

      gpio_init_int(TOUCH_INT_GPIO, GPIO_IN_PU, GPIO_RISING, _gt911_exti_isr);

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
  if (stm32_i2c_deinit(TOUCH_I2C_BUS) < 0)
    TRACE("I2C B1 ReInit - I2C DeInit failed");

  // If DeInit fails, try to re-init anyway
  if (!touchPanelInit()) {
    TRACE("I2C B1 ReInit - touchPanelInit failed");
    return false;
  }
  return true;
}

#if defined(SIMU) || defined(SEMIHOSTING) || defined(DEBUG)
static const char *event2str(uint8_t ev)
{
  switch (ev) {
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

  if (!touchEventOccured) return internalTouchState;

  touchEventOccured = false;

  uint32_t startReadStatus = timersGetMsTick();
  do {
    if (!I2C_GT911_ReadRegister(GT911_READ_XY_REG, &state, 1)) {
      // ledRed();
      touchGT911hiccups++;
      TRACE("GT911 I2C read XY error");
      if (!I2C_ReInit()) TRACE("I2C B1 ReInit failed");
      return internalTouchState;
    }

    if (state & 0x80u) {
      // ready
      break;
    }
    sleep_ms(1);
  } while (timersGetMsTick() - startReadStatus < GT911_TIMEOUT);

  internalTouchState.deltaX = 0;
  internalTouchState.deltaY = 0;
  TRACE("touch state = 0x%x", state);
  if (state & 0x80u) {
    uint8_t pointsCount = (state & 0x0Fu);
    uint32_t now = timersGetMsTick();
    internalTouchState.tapCount = 0;

    if (pointsCount > 0 && pointsCount <= GT911_MAX_TP) {
      if (!I2C_GT911_ReadRegister(GT911_READ_XY_REG + 1, touchData.data,
                                  pointsCount * sizeof(TouchPoint))) {
        // ledRed();
        touchGT911hiccups++;
        TRACE("GT911 I2C data read error");
        if (!I2C_ReInit()) TRACE("I2C B1 ReInit failed");
        return internalTouchState;
      }
        
      if (internalTouchState.event == TE_NONE ||
          internalTouchState.event == TE_UP ||
          internalTouchState.event == TE_SLIDE_END) {
        internalTouchState.event = TE_DOWN;
        internalTouchState.startX = internalTouchState.x =
            touchData.points[0].x;
        internalTouchState.startY = internalTouchState.y =
            touchData.points[0].y;
        downTime = now;
      } else {
        internalTouchState.deltaX =
            touchData.points[0].x - internalTouchState.x;
        internalTouchState.deltaY =
            touchData.points[0].y - internalTouchState.y;
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
        if (now - downTime <= GT911_TAP_TIME) {
          if (now - tapTime > GT911_TAP_TIME)
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
  if (!I2C_GT911_WriteRegister(GT911_READ_XY_REG, &zero, 1)) {
    TRACE("GT911 ERROR: clearing XY register failed");
  }

  TRACE("touch event = %s", event2str(internalTouchState.event));
  return internalTouchState;
}

bool touchPanelEventOccured()
{
  return touchEventOccured;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}

