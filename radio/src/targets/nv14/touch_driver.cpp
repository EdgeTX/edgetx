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

#include "debug.h"

volatile static bool touchEventOccured;

#define FT6x06_MAX_INSTANCE  1

#define TOUCH_FT6236_I2C_ADDRESS          (0x70>>1)
#define TOUCH_CST836U_I2C_ADDRESS         (0x15)

enum TouchControllers {TC_NONE, TC_FT6236, TC_CST836U};
TouchControllers touchController = TC_NONE;

static tc_handle_TypeDef tc_handle = {0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 250  // 250 ms

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

void I2C_Init_Radio(void)
{
  TRACE("CST836U I2C Init");

  if (i2c_init(TOUCH_I2C_BUS) < 0) {
    TRACE("CST836U ERROR: i2c_init failed");
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

#if 0
static bool touch_i2c_write(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t len)
{
  if (stm32_i2c_write(TOUCH_I2C_BUS, addr, reg, 1, data, len, I2C_TIMEOUT_MAX) < 0)
    return false;

  return true;
}

static void TS_IO_Write(uint8_t addr, uint8_t reg, uint8_t data)
{
  uint8_t tryCount = 3;
  while (!touch_i2c_write(addr, reg, &data, 1)) {
    if (--tryCount == 0) break;
    I2C_Init();
  }
}
#endif

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

static void touch_ft6236_debug_info(void)
{
#if defined(DEBUG)
  TRACE("ft6x36: thrhld = %d", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_TH_GROUP) * 4);
  TRACE("ft6x36: rep rate=", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_PERIODACTIVE) * 10);
  TRACE("ft6x36: fw lib 0x%02X %02X", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_LIB_VER_H), TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_LIB_VER_L));
  TRACE("ft6x36: fw v 0x%02X", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_FIRMID));
  TRACE("ft6x36: CHIP ID 0x%02X", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_CIPHER));
  TRACE("ft6x36: CTPM ID 0x%02X", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_FOCALTECH_ID));
  TRACE("ft6x36: rel code 0x%02X", TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, TOUCH_FT6236_REG_RELEASE_CODE_ID));
#endif
}

/**
 * @brief  Return if there is touches detected or not.
 *         Try to detect new touches and forget the old ones (reset internal global
 *         variables).
 * @param  DeviceAddr: Device address on communication Bus.
 * @retval : Number of active touches detected (can be 0, 1 or 2).
 */
static uint8_t ft6x06_TS_DetectTouch()
{
  volatile uint8_t nbTouch = 0;

  /* Read register FT6206_TD_STAT_REG to check number of touches detection */
  nbTouch = TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, FT6206_TD_STAT_REG);
  nbTouch &= FT6206_TD_STAT_MASK;
  if (nbTouch > FT6206_MAX_DETECTABLE_TOUCH) {
    /* If invalid number of touch detected, set it to zero */
    nbTouch = 0;
  }
  /* Update ft6x06 driver internal global : current number of active touches */
  tc_handle.currActiveTouchNb = nbTouch;

  /* Reset current active touch index on which to work on */
  tc_handle.currActiveTouchIdx = 0;
  return (nbTouch);
}

#if 0
/**
 * @brief  Get the touch detailed informations on touch number 'touchIdx' (0..1)
 *         This touch detailed information contains :
 *         - weight that was applied to this touch
 *         - sub-area of the touch in the touch panel
 *         - event of linked to the touch (press down, lift up, ...)
 * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT6x06).
 * @param  touchIdx : Passed index of the touch (0..1) on which we want to get the
 *                    detailed information.
 * @param  pWeight : Pointer to to get the weight information of 'touchIdx'.
 * @param  pArea   : Pointer to to get the sub-area information of 'touchIdx'.
 * @param  pEvent  : Pointer to to get the event information of 'touchIdx'.

 * @retval None.
 */
static void ft6x06_TS_GetTouchInfo(uint16_t DeviceAddr,
                                   uint32_t touchIdx,
                                   uint32_t * pWeight,
                                   uint32_t * pArea,
                                   uint32_t * pEvent)
{
  uint8_t regAddress = 0;
  uint8_t dataxy[3];

  if (touchIdx < ft6x06_handle.currActiveTouchNb) {
    switch (touchIdx) {
      case 0 :
        regAddress = FT6206_P1_WEIGHT_REG;
        break;

      case 1 :
        regAddress = FT6206_P2_WEIGHT_REG;
        break;

      default :
        break;

    } /* end switch(touchIdx) */

    /* Read weight, area and Event Id of touch index */
    TS_IO_ReadMultiple(DeviceAddr, regAddress, dataxy, sizeof(dataxy));

    /* Return weight of touch index */
    *pWeight = (dataxy[0] & FT6206_TOUCH_WEIGHT_MASK) >> FT6206_TOUCH_WEIGHT_SHIFT;
    /* Return area of touch index */
    *pArea = (dataxy[1] & FT6206_TOUCH_AREA_MASK) >> FT6206_TOUCH_AREA_SHIFT;
    /* Return Event Id  of touch index */
    *pEvent = (dataxy[2] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;

  } /* of if(touchIdx < ft6x06_handle.currActiveTouchNb) */
}
#endif

/**
 * @brief  Get the touch screen X and Y positions values
 *         Manage multi touch thanks to touch Index global
 *         variable 'tc_handle.currActiveTouchIdx'.
 * @param  DeviceAddr: Device address on communication Bus.
 * @param  X: Pointer to X position value
 * @param  Y: Pointer to Y position value
 * @retval None.
 */
static void ft6x06_TS_GetXY(uint16_t * X, uint16_t * Y, uint32_t * event)
{
  uint8_t regAddress = 0;
  uint8_t dataxy[4];

  if (tc_handle.currActiveTouchIdx < tc_handle.currActiveTouchNb) {
    switch (tc_handle.currActiveTouchIdx) {
      case 0 :
        regAddress = FT6206_P1_XH_REG;
        break;
      case 1 :
        regAddress = FT6206_P2_XH_REG;
        break;

      default :
        break;
    }

    /* Read X and Y positions */
    TS_IO_ReadMultiple(TOUCH_FT6236_I2C_ADDRESS, regAddress, dataxy, sizeof(dataxy));
    /* Send back ready X position to caller */
    *X = ((dataxy[0] & FT6206_MSB_MASK) << 8) | (dataxy[1] & FT6206_LSB_MASK);
    /* Send back ready Y position to caller */
    *Y = ((dataxy[2] & FT6206_MSB_MASK) << 8) | (dataxy[3] & FT6206_LSB_MASK);

    *event = (dataxy[0] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;
    /*
    uint32_t weight;
    uint32_t area;
    ft6x06_TS_GetTouchInfo(DeviceAddr, ft6x06_handle.currActiveTouchIdx, &weight, &area, event);
    */
    tc_handle.currActiveTouchIdx++;
  }
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
  gpio_clear(TOUCH_RST_GPIO);
  delay_ms(10);
  gpio_set(TOUCH_RST_GPIO);
  delay_ms(300);
}


static const TouchControllerDescriptor FT6236 =
{
    .read = ft6x06_TS_GetXY,
    .detectTouch = ft6x06_TS_DetectTouch,
    .printDebugInfo = touch_ft6236_debug_info,
    .contactEvent = FT6206_TOUCH_EVT_FLAG_CONTACT
};
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
  } else {
    touchController = TC_FT6236;
    tc = &FT6236;
  }
}

void TouchInit(void)
{
  gpio_init(TOUCH_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  I2C_Init_Radio();
  TouchReset();
  detectTouchController();
  gpio_init_int(TOUCH_INT_GPIO, GPIO_IN_PU, GPIO_FALLING, _cst836u_exti_isr);
  tc->printDebugInfo();
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  tc->read(&touchX, &touchY, &tEvent);

#if defined( LCD_DIRECTION ) && (LCD_DIRECTION == LCD_VERTICAL)
  touchX = LCD_WIDTH - touchX;
  touchY = LCD_HEIGHT - touchY;
#else
  unsigned short tmp = (LCD_WIDTH - 1) - touchY;
  touchY = touchX;
  touchX = tmp;
#endif
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

  uint32_t now = timersGetMsTick();
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
