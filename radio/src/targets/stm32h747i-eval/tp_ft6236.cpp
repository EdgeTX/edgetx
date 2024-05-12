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

#include "hal.h"
#include "timers_driver.h"
#include "delays_driver.h"
#include "tp_ft6236.h"

#include "hal/i2c_driver.h"

#include "debug.h"

#define TAP_TIME 25
#define I2C_TIMEOUT_MAX 5 // 5 ms

// FT6236 definitions
#define TOUCH_FT6236_I2C_ADDRESS          (0x70>>1)
#define TOUCH_FT6236_REG_TH_GROUP         0x80
#define TOUCH_FT6236_REG_PERIODACTIVE     0x88
#define TOUCH_FT6236_REG_LIB_VER_H        0xa1
#define TOUCH_FT6236_REG_LIB_VER_L        0xa2
#define TOUCH_FT6236_REG_CIPHER           0xa3
#define TOUCH_FT6236_REG_GMODE            0xa4
#define TOUCH_FT6236_REG_FIRMID           0xa6
#define TOUCH_FT6236_REG_FOCALTECH_ID     0xa8
#define TOUCH_FT6236_REG_RELEASE_CODE_ID  0xaf
#define TOUCH_FT6206_REG_TD_STAT          0x02
#define TOUCH_FT6206_REG_P1_XH            0x03
#define TOUCH_FT6206_EVT_SHIFT            6
#define TOUCH_FT6206_EVT_MASK             (3 << TOUCH_FT6206_EVT_SHIFT)
#define TOUCH_FT6206_EVT_CONTACT          0x02
#define TOUCH_FT6206_MASK_TD_STAT         0x0f

struct TouchControllerDescriptor
{
  bool (*hasTouchEvent)();
  bool (*touchRead)(uint16_t * X, uint16_t * Y);
  void (*printDebugInfo)();
};

union rpt_point_t
{
  struct 
  {
    unsigned char x_l8;
    unsigned char y_l8;
    unsigned char z;
    unsigned char x_h4:4;
    unsigned char y_h4:4;
    unsigned char id:4;
    unsigned char event:4;
  }rp;
  unsigned char data[5];
};

static TouchState internalTouchState = {};
volatile bool touchEventOccured;

static tmr10ms_t downTime = 0;
static tmr10ms_t tapTime = 0;
static short tapCount = 0;

static void _i2c_init(void)
{
  TRACE("FT6236 I2C Init");

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("FT6236 ERROR: stm32_i2c_init failed");
    return;
  }
}

static int _i2c_read(uint32_t reg, void* data, uint16_t len)
{
  return stm32_i2c_read(TOUCH_I2C_BUS, TOUCH_FT6236_I2C_ADDRESS, reg, 1,
                        (uint8_t*)data, len, I2C_TIMEOUT_MAX);
}

static bool ft6236TouchRead(uint16_t * X, uint16_t * Y)
{
  uint8_t dataxy[4];

  // Read X and Y positions and event
  if (_i2c_read(TOUCH_FT6206_REG_P1_XH, dataxy, sizeof(dataxy)) < 0) {
    TRACE("FT6206: error w/ P1_XH");
    return false;
  }

  // Send back ready X position to caller
  *X = ((dataxy[0] & 0x0f) << 8) | dataxy[1];
  // Send back ready Y position to caller
  *Y = ((dataxy[2] & 0x0f) << 8) | dataxy[3];

  uint8_t event = (dataxy[0] & TOUCH_FT6206_EVT_MASK) >> TOUCH_FT6206_EVT_SHIFT;
  // TRACE("FT6206: event = 0x%02X", event);

  if (event == TOUCH_FT6206_EVT_CONTACT) {
    // TRACE("X=%d;Y=%d", *X, *Y);
    return true;
  }

  return false;
}

static bool ft6236HasTouchEvent()
{
  // uint8_t nbTouch = 0;
  // if (_i2c_read(TOUCH_FT6206_REG_TD_STAT, &nbTouch, 1) < 0) {
  //   TRACE("FT6206: error w/ TD_STAT");
  //   return false;
  // }

  // if ((nbTouch & TOUCH_FT6206_MASK_TD_STAT) > 0) {
  //   TRACE("FT6206: %d", nbTouch);
  //   return true;
  // }

  // return false;
  return true;
}

static void ft6236PrintDebugInfo(void)
{
#if defined(DEBUG)
  uint8_t byte = 0;
  if (_i2c_read(TOUCH_FT6236_REG_TH_GROUP, &byte, 1) == 0) {
    TRACE("ft6x36: thrhld = %d", byte * 4);
  }
  if (_i2c_read(TOUCH_FT6236_REG_PERIODACTIVE, &byte, 1) == 0) {
    TRACE("ft6x36: rep rate= %d", byte * 10);
  }
  uint16_t hword;
  if (_i2c_read(TOUCH_FT6236_REG_LIB_VER_H, &hword, 1) == 0) {
    TRACE("ft6x36: fw lib 0x%04X", hword);
  }
  if (_i2c_read(TOUCH_FT6236_REG_FIRMID, &byte, 1) == 0) {
    TRACE("ft6x36: fw v 0x%02X", byte);
  }
  if (_i2c_read(TOUCH_FT6236_REG_CIPHER, &byte, 1) == 0) {
    TRACE("ft6x36: CHIP ID 0x%02X", byte);
  }
  if (_i2c_read(TOUCH_FT6236_REG_FOCALTECH_ID, &byte, 1) == 0) {
    TRACE("ft6x36: CTPM ID 0x%02X", byte);
  }
  if (_i2c_read(TOUCH_FT6236_REG_RELEASE_CODE_ID, &byte, 1) == 0) {
    TRACE("ft6x36: rel code 0x%02X", byte);
  }
#endif
}

void touchPanelInit()
{
  _i2c_init();
  ft6236PrintDebugInfo();
}

bool touchPanelEventOccured() { return ft6236HasTouchEvent(); }

struct TouchState touchPanelRead()
{
  touchEventOccured = false;

  tmr10ms_t now = get_tmr10ms();
  internalTouchState.tapCount = 0;
  unsigned short touchX;
  unsigned short touchY;
  bool hasTouchContact = ft6236TouchRead(&touchX, &touchY);

  // implement "set orientation" instead
  auto tmp = touchX;
  touchX = touchY;
  touchY = 480 - tmp;

  if (hasTouchContact) {
    int dx = touchX - internalTouchState.x;
    int dy = touchY - internalTouchState.y;
    internalTouchState.x = touchX;
    internalTouchState.y = touchY;

    if (internalTouchState.event == TE_NONE || internalTouchState.event == TE_UP || internalTouchState.event == TE_SLIDE_END) {
      internalTouchState.startX = internalTouchState.x;
      internalTouchState.startY = internalTouchState.y;
      internalTouchState.event = TE_DOWN;
    } else if (internalTouchState.event == TE_DOWN) {
      if (dx >= SLIDE_RANGE || dx <= -SLIDE_RANGE || dy >= SLIDE_RANGE || dy <= -SLIDE_RANGE) {
        internalTouchState.event = TE_SLIDE;
        internalTouchState.deltaX = (short) dx;
        internalTouchState.deltaY = (short) dy;
      } else {
        internalTouchState.event = TE_DOWN;
        internalTouchState.deltaX = 0;
        internalTouchState.deltaY = 0;
      }
    } else if (internalTouchState.event == TE_SLIDE) {
      internalTouchState.event = TE_SLIDE; //no change
      internalTouchState.deltaX = (short) dx;
      internalTouchState.deltaY = (short) dy;
    }

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
  if (internalTouchState.event == TE_UP || internalTouchState.event == TE_SLIDE_END)
    internalTouchState.event = TE_NONE;

// #if defined(DEBUG)
//   TRACE("FT6236: event=%d, X=%d, Y=%d", ret.event, ret.x, ret.y);
// #endif

  return ret;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}
