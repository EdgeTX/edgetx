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
#include "delays_driver.h"
#include "touch_driver.h"
#include "keys.h"

#include "debug.h"

#define TAP_TIME 250 // 250 ms
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
#define TOUCH_CST836U_I2C_ADDRESS         0x15
#define TOUCH_CST836U_REG_NUM             0x02
#define TOUCH_CST836U_REG_P1_XH           0x03
#define TOUCH_CST836U_EVT_SHIFT           6
#define TOUCH_CST836U_EVT_MASK            (3 << TOUCH_FT6206_EVT_SHIFT)
#define TOUCH_CST836U_EVT_CONTACT         0x02
#define TOUCH_CST836U_FW_VERSION_L_REG    0xa6
#define TOUCH_CST836U_FW_VERSION_H_REG    0xa7
#define TOUCH_CST836U_MODULE_VERSION_REG  0xa8
#define TOUCH_CST836U_PROJECT_NAME_REG    0xa9
#define TOUCH_CST836U_CHIP_TYPE_L_REG     0xaa
#define TOUCH_CST836U_CHIP_TYPE_H_REG     0xab


typedef enum {TC_NONE, TC_FT6236, TC_CST836U, TC_CST340, TC_CHSC5448} TouchController;

#if defined(DEBUG)
const char TOUCH_CONTROLLER_STR[][10] = {"", "FT6236", "CST836U", "CST340", "CHSC5448"};
#endif

static uint8_t lastTouchKey = 0;
static bool eventFired = false;

static const event_t TOUCHKEY_EVENTS[8] = {
  KEY_EXIT | _MSK_KEY_BREAK,
  KEY_MODEL | _MSK_KEY_BREAK,
  KEY_MODEL | _MSK_KEY_LONG,
  KEY_PAGEUP | _MSK_KEY_FIRST,
  KEY_TELE | _MSK_KEY_BREAK,
  KEY_PAGEDN | _MSK_KEY_FIRST,
  KEY_SYS | _MSK_KEY_LONG,
  KEY_ENTER | _MSK_KEY_BREAK,
};

TouchController touchController = TC_NONE;

enum TouchRotate
{
  DEG_0,
  DEG_90,
  DEG_180,
  DEG_270
};

struct TouchControllerDescriptor
{
  bool (*hasTouchEvent)();
  int (*touchRead)(uint16_t * X, uint16_t * Y);
  void (*printDebugInfo)();
  TouchRotate rotate;
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

const char* boardTouchType = "";
static const TouchControllerDescriptor *tcd = nullptr;
static TouchState internalTouchState = {};
volatile static bool touchEventOccured;
static tmr10ms_t downTime = 0;
static tmr10ms_t tapTime = 0;
static short tapCount = 0;

static constexpr uint32_t TOUCH_POLL_MS = 30;
static constexpr uint8_t TOUCH_ERROR_LIMIT = 3;

static uint32_t lastTouchReadMs = 0;
static bool touchRetryPending = false;
static uint8_t touchReadErrors = 0;

enum class TouchRecoveryStage : uint8_t {
  Idle,
  ResetLow,
  BootWait
};

static TouchRecoveryStage touchRecoveryStage = TouchRecoveryStage::Idle;
static uint32_t touchRecoveryStartedMs = 0;

static void _touch_exti_isr(void)
{
  touchEventOccured = true;
}

static void _touch_reset()
{
  gpio_clear(TOUCH_RST_GPIO);
  delay_ms(10);
  gpio_set(TOUCH_RST_GPIO);
  delay_ms(300);
}

static void _i2c_init(void)
{
  TRACE("Touch I2C Init");

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("Touch I2C Init ERROR: i2c_init failed");
    return;
  }
}

static void _i2c_reInit(void)
{
  TRACE("I2C reinit: bus reset triggered");

  if (i2c_deinit(TOUCH_I2C_BUS) < 0) {
    TRACE("I2C reinit WARNING: i2c_deinit failed, proceeding with reinit");
  }
  delay_ms(1);

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("I2C reinit ERROR: i2c_init failed");
    return;
  }
  TRACE("I2C reinit: bus reset complete");
}

static int _i2c_read(uint8_t addr, uint32_t reg, uint8_t regSize, uint8_t* data, uint16_t len, uint32_t timeout)
{
  if (touchController == TC_CST836U || regSize > 2) {
    if (stm32_i2c_master_tx(TOUCH_I2C_BUS, addr,
                            reinterpret_cast<uint8_t*>(&reg), regSize,
                            timeout) < 0) {
      return -1;
    }
    delay_us(5);
    return stm32_i2c_master_rx(TOUCH_I2C_BUS, addr, data, len, timeout);
  }

  return stm32_i2c_read(TOUCH_I2C_BUS, addr, reg, regSize, data, len, timeout);
}

static int _i2c_readMultipleRetry(uint8_t addr, uint32_t reg, uint8_t regSize, uint8_t * buffer, uint16_t length)
{
  for (uint8_t attempt = 0; attempt < 3; ++attempt) {
    if (_i2c_read(addr, reg, regSize, buffer, length, I2C_TIMEOUT_MAX) == 0) {
      return length;
    }

    if (attempt < 2) {
      _i2c_reInit();
    }
  }

  return -1;
}

static int _i2c_readRetry(uint8_t addr, uint32_t reg, uint8_t regSize)
{
  uint8_t result = 0;
  if (_i2c_readMultipleRetry(addr, reg, regSize, &result, 1) < 0) {
    return -1;
  }

  return result;
}

static bool defaultHasTouchEvent()
{
  return touchEventOccured;
}

static int ft6236TouchRead(uint16_t * X, uint16_t * Y)
{
  // Read register FT6206_TD_STAT_REG to check number of touches detection
  int nbTouch = _i2c_readRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6206_REG_TD_STAT, 1);
  if (nbTouch < 0) {
    return -1;
  }
  nbTouch &= TOUCH_FT6206_MASK_TD_STAT;
  bool hasTouch = nbTouch > 0;

  if (hasTouch) {
    uint8_t dataxy[4];
    // Read X and Y positions and event
    if (_i2c_readMultipleRetry(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6206_REG_P1_XH,
                               1, dataxy, sizeof(dataxy)) < 0) {
      return -1;
    }
    
    // Send back ready X position to caller
    *X = ((dataxy[0] & 0x0f) << 8) | dataxy[1];
    // Send back ready Y position to caller
    *Y = ((dataxy[2] & 0x0f) << 8) | dataxy[3];

    uint8_t event = (dataxy[0] & TOUCH_FT6206_EVT_MASK) >> TOUCH_FT6206_EVT_SHIFT;
    return event == TOUCH_FT6206_EVT_CONTACT;
  }
  return false;
}

static void ft6236PrintDebugInfo()
{
#if defined(DEBUG)
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

static int cst836uTouchRead(uint16_t * X, uint16_t * Y)
{
  // Read register TOUCH_CST836U_REG_NUM to check number of touches detection
  int nbTouch = _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_REG_NUM, 1);
  if (nbTouch < 0) {
    return -1;
  }
  bool hasTouch = nbTouch > 0;

  if (hasTouch) {
    uint8_t dataxy[4];
    // Read X and Y positions and event
    if (_i2c_readMultipleRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_REG_P1_XH,
                               1, dataxy, sizeof(dataxy)) < 0) {
      return -1;
    }
    
    // Send back ready X position to caller
    *X = ((dataxy[0] & 0x0f) << 8) | dataxy[1];
    // Send back ready Y position to caller
    *Y = ((dataxy[2] & 0x0f) << 8) | dataxy[3];

    uint8_t event = (dataxy[0] & TOUCH_CST836U_EVT_MASK) >> TOUCH_CST836U_EVT_SHIFT;
    return event == TOUCH_CST836U_EVT_CONTACT;
  }
  return false;
}

static void cst836uPrintDebugInfo(void)
{
#if defined(DEBUG)
  TRACE("cst836u: fw ver 0x%02X %02X", _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_FW_VERSION_H_REG, 1), _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_FW_VERSION_L_REG, 1));
  TRACE("cst836u: module version 0x%02X", _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_MODULE_VERSION_REG, 1));
  TRACE("cst836u: project name 0x%02X", _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_PROJECT_NAME_REG, 1));
  TRACE("cst836u: chip type 0x%02X 0x%02X", _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_CHIP_TYPE_H_REG, 1), _i2c_readRetry(TOUCH_CST836U_I2C_ADDRESS, TOUCH_CST836U_CHIP_TYPE_L_REG, 1));
#endif
}

static int cst340TouchRead(uint16_t * X, uint16_t * Y)
{
  uint8_t data[4];

  // Read X and Y positions
  if (_i2c_readMultipleRetry(TOUCH_CST340_I2C_ADDRESS, TOUCH_CST340_REG_FINGER1,
                             1, data, sizeof(data)) < 0) {
    return -1;
  }

  // Send back X position to caller
  if (X) *X = ((data[1]<<4) + ((data[3]>>4)&0x0f));
  // Send back Y position to caller
  if (Y) *Y = ((data[2]<<4) + ((data[3])&0x0f));

  return data[0] == TOUCH_CST340_EVT_CONTACT;
}

static bool cst340HasTouchEvent()
{
  return touchEventOccured;
}

static void cst340PrintDebugInfo()
{
  // TODO, when necessary
}

static int chsc5448TouchRead(uint16_t * X, uint16_t * Y)
{
  static uint8_t readbuffer[84];
  const uint8_t reportSize = ((TOUCH_CHSC5448_MAX_POINTS * 5 + 2) + 3) & 0xfc;

  int ptCnt = 0;
  union rpt_point_t* ppt;

  if (_i2c_readMultipleRetry(TOUCH_CHSC5448_I2C_ADDRESS, TOUCH_CHSC5448_REG_ADDR,
                             4, readbuffer, reportSize) < 0) {
    return -1;
  }
  ptCnt = readbuffer[1] & 0x0f;
  ppt = (union rpt_point_t*)&readbuffer[2];
  *X = ((ppt->rp.x_h4 & 0x0f) << 8) | ppt->rp.x_l8;
  *Y = ((ppt->rp.y_h4 & 0x0f) << 8) | ppt->rp.y_l8;
  uint8_t event = ppt->rp.event;
  bool hasEvent = ptCnt > 0 && event == TOUCH_CHSC5448_EVT_CONTACT;

  // Touch keys returns X = 2600, Y = 1 to 8 (total 8 keys)
  uint8_t currTouchKey = 0;  
  if (hasEvent && *X >= 2000) {    
    // Touch key event
    currTouchKey = *Y;
    if (currTouchKey > 8) {
      currTouchKey = 8;
    }
    hasEvent = false;
  }

#if defined(DEBUG)
    TRACE("%s: lastTouchKey=%d, currTouchKey=%d", TOUCH_CONTROLLER_STR[touchController], lastTouchKey, currTouchKey);
#endif

  if (currTouchKey != lastTouchKey) {
    lastTouchKey = currTouchKey;
    eventFired = false;
  } else {
    if (currTouchKey > 0 && !eventFired) {
      pushEvent(TOUCHKEY_EVENTS[currTouchKey - 1]);
      eventFired = true;
    }
  }

  return hasEvent;
}

static void chsc5448PrintDebugInfo()
{
  // TODO, when necessary
}

static const TouchControllerDescriptor FT6236 =
{
  .hasTouchEvent = defaultHasTouchEvent,
  .touchRead = ft6236TouchRead,
  .printDebugInfo = ft6236PrintDebugInfo,
#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  .rotate = DEG_180,
#else
  .rotate = DEG_270,
#endif
};

static const TouchControllerDescriptor CST836U =
{
  .hasTouchEvent = defaultHasTouchEvent,
  .touchRead = cst836uTouchRead,
  .printDebugInfo = cst836uPrintDebugInfo,
#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  .rotate = DEG_180,
#else
  .rotate = DEG_270,
#endif
};

static const TouchControllerDescriptor CST340 =
{
  .hasTouchEvent = cst340HasTouchEvent,
  .touchRead = cst340TouchRead,
  .printDebugInfo = cst340PrintDebugInfo,
#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  .rotate = DEG_180,
#else
  .rotate = DEG_270,
#endif
};

static const TouchControllerDescriptor CHSC5448 =
{
  .hasTouchEvent = defaultHasTouchEvent,
  .touchRead = chsc5448TouchRead,
  .printDebugInfo = chsc5448PrintDebugInfo,
  .rotate = DEG_0,
};

void _detect_touch_controller()
{
  if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST340_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_CST340;
    tcd = &CST340;
    boardTouchType = "CST340";
    TouchControllerType = 0;
  } else if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CHSC5448_I2C_ADDRESS, 3, I2C_TIMEOUT_MAX) == 0) {
    touchController = TC_CHSC5448;
    tcd = &CHSC5448;
    TouchControllerType = 0;
    boardTouchType = "CHSC5448";
  } else if (stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST836U_I2C_ADDRESS, 3, 5) == 0) {
    touchController = TC_CST836U;
    tcd = &CST836U;
    boardTouchType = "CST836U";
  } else {
    touchController = TC_FT6236;
    tcd = &FT6236;
    boardTouchType = "FT6236";
#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
    TouchControllerType = 0;
#else
    TouchControllerType = 1;
#endif
  }
}

void touchPanelDeInit()
{
  gpio_int_disable(TOUCH_INT_GPIO);
  touchRecoveryStage = TouchRecoveryStage::Idle;
  touchRetryPending = false;
  touchEventOccured = false;
  tcd = nullptr;
}

bool touchPanelInit()
{
  gpio_init(TOUCH_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  _i2c_init();
  _touch_reset();
  _detect_touch_controller();
  internalTouchState = {};
  touchEventOccured = false;
  lastTouchReadMs = timersGetMsTick();
  touchRetryPending = true;
  touchReadErrors = 0;
  touchRecoveryStage = TouchRecoveryStage::Idle;
  touchRecoveryStartedMs = 0;
  lastTouchKey = 0;
  eventFired = false;
  downTime = 0;
  tapTime = 0;
  tapCount = 0;
  gpio_init_int(TOUCH_INT_GPIO, GPIO_IN_PU, GPIO_FALLING, _touch_exti_isr);

  tcd->printDebugInfo();

  return touchController != TC_NONE;
}

bool touchPanelEventOccured()
{
  if (!tcd) {
    return false;
  }

  uint32_t now = timersGetMsTick();

  if (touchRecoveryStage == TouchRecoveryStage::ResetLow) {
    if (now - touchRecoveryStartedMs >= 10) {
      gpio_set(TOUCH_RST_GPIO);
      touchRecoveryStartedMs = now;
      touchRecoveryStage = TouchRecoveryStage::BootWait;
    }
    return false;
  }

  if (touchRecoveryStage == TouchRecoveryStage::BootWait) {
    if (now - touchRecoveryStartedMs < 300) {
      return false;
    }
    _i2c_reInit();
    touchRecoveryStage = TouchRecoveryStage::Idle;
    touchRetryPending = true;
  }

  bool pressed = internalTouchState.event == TE_DOWN ||
                 internalTouchState.event == TE_SLIDE;
  bool needsPolling = pressed || lastTouchKey != 0 || touchRetryPending ||
                      gpio_read(TOUCH_INT_GPIO) == 0;

  return tcd->hasTouchEvent() ||
         (needsPolling && now - lastTouchReadMs >= TOUCH_POLL_MS);
}

struct TouchState touchPanelRead()
{
  if (!touchPanelEventOccured()) {
    return internalTouchState;
  }

  touchEventOccured = false;

  uint32_t now = timersGetMsTick();
  lastTouchReadMs = now;
  uint16_t touchX = 0;
  uint16_t touchY = 0;
  int readResult = tcd->touchRead(&touchX, &touchY);

  if (readResult < 0) {
    touchRetryPending = true;

    if (++touchReadErrors >= TOUCH_ERROR_LIMIT) {
      touchReadErrors = 0;
      lastTouchKey = 0;
      eventFired = false;
      gpio_clear(TOUCH_RST_GPIO);
      touchRecoveryStartedMs = timersGetMsTick();
      touchRecoveryStage = TouchRecoveryStage::ResetLow;
    }

    TouchState ret = internalTouchState;
    ret.event = TE_NONE;
    ret.tapCount = 0;
    ret.deltaX = 0;
    ret.deltaY = 0;
    return ret;
  }

  touchReadErrors = 0;
  touchRetryPending = false;
  internalTouchState.tapCount = 0;
  bool hasTouchContact = readResult > 0;

  if (hasTouchContact) {
    unsigned short tmp;
    switch (tcd->rotate) {
      case DEG_270:
        tmp = touchY;
        touchY = 319 - touchX;
        touchX = tmp;
        break;
      case DEG_180:
        touchY = 479 - touchY;
        touchX = 319 - touchX;
        break;
      default:
        break;
    }

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
