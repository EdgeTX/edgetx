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
#include "touch_driver.h"

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
#define TOUCH_FT6236_REG_FIRMID           0xa6
#define TOUCH_FT6236_REG_FOCALTECH_ID     0xa8
#define TOUCH_FT6236_REG_RELEASE_CODE_ID  0xaf
#define TOUCH_FT6206_REG_TD_STAT          0x02
#define TOUCH_FT6206_REG_P1_XH            0x03
#define TOUCH_FT6206_EVT_SHIFT            6
#define TOUCH_FT6206_EVT_MASK             (3 << TOUCH_FT6206_EVT_SHIFT)
#define TOUCH_FT6206_EVT_CONTACT          0x02
#define TOUCH_FT6206_MASK_TD_STAT         0x0f

// CST340 definitions
#define TOUCH_CST340_I2C_ADDRESS          0x1a
#define TOUCH_CST340_REG_FINGER1          0x00
#define TOUCH_CST340_EVT_CONTACT          0x06

// CHSC5448 definitions
#define TOUCH_CHSC5448_I2C_ADDRESS        0x2e
#define TOUCH_CHSC5448_REG_ADDR           0x2c000020
#define TOUCH_CHSC5448_EVT_CONTACT        0x08
#define TOUCH_CHSC5448_MAX_POINTS         5

// CST836U definitions
#define TOUCH_CST836U_I2C_ADDRESS         (0x15)
#define TOUCH_CST836U_TOUCH_NUM_REG        0x02
#define TOUCH_CST836U_TOUCH1_XH_REG        0x03
#define TOUCH_CST836U_MSB_MASK             0x0F

typedef enum {TC_NONE, TC_FT6236, TC_CST340, TC_CHSC5448, TC_CST836U} TouchController;

#if defined(DEBUG)
const char TOUCH_CONTROLLER_STR[][10] = {"", "FT6236", "CST340", "CHSC5448", "CST836U"};
#endif

TouchController touchController = TC_NONE;

struct TouchControllerDescriptor
{
  bool (*hasTouchEvent)();
  bool (*touchRead)(uint16_t * X, uint16_t * Y);
  void (*printDebugInfo)();
  bool needTranspose;
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

extern uint8_t TouchControllerType;

static const TouchControllerDescriptor *tcd = nullptr;
static TouchState internalTouchState = {};
volatile static bool touchEventOccured;
static tmr10ms_t downTime = 0;
static tmr10ms_t tapTime = 0;
static short tapCount = 0;

static void _touch_exti_isr(void)
{
  touchEventOccured = true;
}

static void _touch_exti_stop(void) {}
static void _touch_exti_config(void) {}
static void _touch_gpio_config(void) {}
static void _touch_reset() {}

static int _i2c_read(uint8_t addr, uint32_t reg, uint8_t regSize, uint8_t* data, uint16_t len, uint32_t timeout, bool forceLong = false)
{
  if (regSize > 2 || forceLong) {
    if(stm32_i2c_master_tx(TOUCH_I2C_BUS, addr, (uint8_t*) &reg, regSize, 3) < 0)
      return false;
    delay_us(5);
    if(stm32_i2c_master_rx(TOUCH_I2C_BUS, addr, data, len, I2C_TIMEOUT_MAX) < 0)
      return false;
    return true;
  } else {
    return stm32_i2c_read(TOUCH_I2C_BUS, addr, reg, regSize, data, len, timeout);
  }
}

static uint8_t _i2c_readRetry(uint8_t addr, uint32_t reg, uint8_t regSize)
{
  uint8_t result;
  uint8_t tryCount = 3;
  while (_i2c_read(addr, reg, regSize, &result, 1, I2C_TIMEOUT_MAX) < 0) {
    if (--tryCount == 0) break;
  }
  return result;
}

static uint16_t _i2c_readMultipleRetry(uint8_t addr, uint32_t reg, uint8_t regSize, uint8_t * buffer, uint16_t length, bool forceLong = false)
{
  uint8_t tryCount = 3;
  while (_i2c_read(addr, reg, regSize, buffer, length, I2C_TIMEOUT_MAX, forceLong) < 0) {
    if (--tryCount == 0) break;
  }
  return length;
}

static bool ft6236TouchRead(uint16_t * X, uint16_t * Y)
{
  // Read register FT6206_TD_STAT_REG to check number of touches detection
  uint8_t nbTouch = _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6206_REG_TD_STAT, 1);
  nbTouch &= TOUCH_FT6206_MASK_TD_STAT;
  bool hasTouch = nbTouch > 0;

  if (hasTouch) {
    uint8_t dataxy[4];
    // Read X and Y positions and event
    _i2c_readMultipleRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6206_REG_P1_XH, 1, dataxy, sizeof(dataxy));
    
    // Send back ready X position to caller
    *X = ((dataxy[0] & 0x0f) << 8) | dataxy[1];
    // Send back ready Y position to caller
    *Y = ((dataxy[2] & 0x0f) << 8) | dataxy[3];

    uint8_t event = (dataxy[0] & TOUCH_FT6206_EVT_MASK) >> TOUCH_FT6206_EVT_SHIFT;
    return event == TOUCH_FT6206_EVT_CONTACT;
  }
  return false;
}

static bool ft6236HasTouchEvent()
{
  touchEventOccured = true;
  return true;
  return touchEventOccured;
}

static void ft6236PrintDebugInfo()
{
#if defined(DEBUG)||1
  TRACE("ft6x36: thrhld = %d", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_TH_GROUP, 1) * 4);
  TRACE("ft6x36: rep rate=", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_PERIODACTIVE, 1) * 10);
  TRACE("ft6x36: fw lib 0x%02X %02X", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_LIB_VER_H, 1),
        _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_LIB_VER_L, 1));
  TRACE("ft6x36: fw v 0x%02X", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_FIRMID, 1));
  TRACE("ft6x36: CHIP ID 0x%02X", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_CIPHER, 1));
  TRACE("ft6x36: CTPM ID 0x%02X", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_FOCALTECH_ID, 1));
  TRACE("ft6x36: rel code 0x%02X", _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_RELEASE_CODE_ID, 1));
#endif

}

static bool cst340TouchRead(uint16_t * X, uint16_t * Y)
{
  uint8_t data[4];

  // Read X and Y positions
  _i2c_readMultipleRetry(TOUCH_CST340_I2C_ADDRESS, TOUCH_CST340_REG_FINGER1, 1, data, sizeof(data));

  // Send back X position to caller
  if (X) *X = ((data[1]<<4) + ((data[3]>>4)&0x0f));
  // Send back Y position to caller
  if (Y) *Y = ((data[2]<<4) + ((data[3])&0x0f));

  return data[0] == TOUCH_CST340_EVT_CONTACT;
}

static bool cst340HasTouchEvent()
{
  static bool lastHasTouch = false;
  bool ret = touchEventOccured;
  if (ret) {
    uint8_t hasTouch = cst340TouchRead(nullptr, nullptr);
    if (!hasTouch && !lastHasTouch) {
      TRACE("Interrupt occurs without touch event!!");
      touchEventOccured = false;
      ret = false;
    }
    lastHasTouch = hasTouch;
  }

  return ret;
}

static void cst340PrintDebugInfo()
{
  // TODO, when necessary
}

static bool chsc5448TouchRead(uint16_t * X, uint16_t * Y)
{
  static uint8_t readbuffer[84];
  const uint8_t reportSize = ((TOUCH_CHSC5448_MAX_POINTS * 5 + 2) + 3) & 0xfc;

  int ptCnt = 0;
  union rpt_point_t* ppt;

  _i2c_readMultipleRetry(TOUCH_CHSC5448_I2C_ADDRESS, TOUCH_CHSC5448_REG_ADDR, 4, readbuffer, reportSize);
  ptCnt = readbuffer[1] & 0x0f;
  ppt = (union rpt_point_t*)&readbuffer[2];
  *X = ((ppt->rp.x_h4 & 0x0f) << 8) | ppt->rp.x_l8;
  *Y = ((ppt->rp.y_h4 & 0x0f) << 8) | ppt->rp.y_l8;
  uint8_t event = ppt->rp.event;

  return ptCnt > 0 && event == TOUCH_CHSC5448_EVT_CONTACT;
}

static bool chsc5448HasTouchEvent()
{
  return touchEventOccured;
}

static void chsc5448PrintDebugInfo()
{
  // TODO, when necessary
}

static bool cst836uTouchRead(uint16_t * X, uint16_t * Y)
{
  uint8_t data[5];

  // Read X and Y positions
  _i2c_readMultipleRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_TOUCH_NUM_REG, 1, data, sizeof(data), true);

  // Send back X position to caller
  if(X) *X = ((data[1] & TOUCH_CST836U_MSB_MASK) << 8) | data[2];
  // Send back Y position to caller
  if(Y) *Y = ((data[3] & TOUCH_CST836U_MSB_MASK) << 8) | data[4];

  return data[0] > 0 && data[0] <= 2;
}

static bool cst836uHasTouchEvent()
{
  static bool lastHasTouch = false;
  touchEventOccured = true;
  bool ret = touchEventOccured;
  if (ret||1) {
    uint8_t data;
    _i2c_readMultipleRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_TOUCH_NUM_REG, 1, &data, sizeof(data), true);
    uint8_t hasTouch = data;
    if(data > 2)
      hasTouch = 0;
    if (!hasTouch && !lastHasTouch) {
      TRACE("Interrupt occurs without touch event!!");
      touchEventOccured = false;
      ret = false;
    }
    lastHasTouch = hasTouch;
  }

  return ret;
}

static void cst836uPrintDebugInfo()
{
  // TODO, when necessary
}

static const TouchControllerDescriptor FT6236 =
{
  .hasTouchEvent = ft6236HasTouchEvent,
  .touchRead = ft6236TouchRead,
  .printDebugInfo = ft6236PrintDebugInfo,
  .needTranspose = true,
};

static const TouchControllerDescriptor CST340 =
{
  .hasTouchEvent = cst340HasTouchEvent,
  .touchRead = cst340TouchRead,
  .printDebugInfo = cst340PrintDebugInfo,
  .needTranspose = true,
};

static const TouchControllerDescriptor CHSC5448 =
{
  .hasTouchEvent = chsc5448HasTouchEvent,
  .touchRead = chsc5448TouchRead,
  .printDebugInfo = chsc5448PrintDebugInfo,
  .needTranspose = false,
};

static const TouchControllerDescriptor CST836U =
{
  .hasTouchEvent = cst836uHasTouchEvent,
  .touchRead = cst836uTouchRead,
  .printDebugInfo = cst836uPrintDebugInfo,
  .needTranspose = true,
};

void _detect_touch_controller()
{
  if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST340_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_CST340;
    tcd = &CST340;
    TouchControllerType = 0;
  } else if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CHSC5448_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_CHSC5448;
    tcd = &CHSC5448;
    TouchControllerType = 0;
  } else if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_FT6236_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_FT6236;
    tcd = &FT6236;
    TouchControllerType = 0;
  } else if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST836U_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_CST836U;
    tcd = &CST836U;
    TouchControllerType = 1;
  } else {
    touchController = TC_NONE;
  }
}

void touchPanelDeInit()
{
  _touch_exti_stop();
}

bool touchPanelInit()
{
  _touch_gpio_config();
  _touch_reset();
  _detect_touch_controller();
  _touch_exti_config();

  if(touchController != TC_NONE)
    tcd->printDebugInfo();

  return touchController != TC_NONE;
}

bool touchPanelEventOccured()
{
  if(touchController == TC_NONE)
    return false;
  return tcd->hasTouchEvent();
}

struct TouchState touchPanelRead()
{
  if (!touchEventOccured || touchController == TC_NONE) return internalTouchState;

  touchEventOccured = false;

  tmr10ms_t now = get_tmr10ms();
  internalTouchState.tapCount = 0;
  unsigned short touchX;
  unsigned short touchY;
  bool hasTouchContact = tcd->touchRead(&touchX, &touchY);

  if (tcd->needTranspose) {
    // Touch sensor is rotated by 90 deg
    unsigned short tmp = touchY;
    touchY = 319 - touchX;
    touchX = tmp;
  }

  if (hasTouchContact) {
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

    if (internalTouchState.event == TE_DOWN && downTime == 0) {
      downTime = now;
    }
  }
  else {
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

#if defined(DEBUG)
  TRACE("%s: event=%d,X=%d,Y=%d", TOUCH_CONTROLLER_STR[touchController], ret.event, ret.x, ret.y);
#endif

  return ret;
}

struct TouchState getInternalTouchState()
{
  return internalTouchState;
}
