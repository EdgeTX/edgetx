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
#include "tp_cst340.h"

#include "debug.h"

#define TOUCH_FT6236_I2C_ADDRESS          (0x70>>1)
#define TOUCH_CST340_I2C_ADDRESS          0x1A


extern uint8_t TouchControllerType;

volatile static bool touchEventOccured;
enum TouchControllers {TC_NONE, TC_FT6236, TC_CST340};
TouchControllers touchController = TC_NONE;

static tc_handle_TypeDef tc_handle = {0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 25

struct TouchControllerDescriptor
{
  void (*read)(uint16_t * X, uint16_t * Y, uint32_t * event);
  uint8_t (*detectTouch)();
  void (*printDebugInfo)();
  uint32_t contactEvent;
};

static const TouchControllerDescriptor *tc = nullptr;

static TouchState internalTouchState = {};

static void _cst340_exti_isr(void)
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
		    _cst340_exti_isr);
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
  TRACE("CST340 I2C Init");

  if (stm32_i2c_init(TOUCH_I2C_BUS, TOUCH_I2C_CLK_RATE) < 0) {
    TRACE("CST340 ERROR: stm32_i2c_init failed");
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
//  if(touchController == TC_CST836U)
//  {
//    if(stm32_i2c_master_tx(TOUCH_I2C_BUS, addr, &reg, 1, 3) < 0)
//      return false;
//    delay_us(5);
//    if(stm32_i2c_master_rx(TOUCH_I2C_BUS, addr, data, len, I2C_TIMEOUT_MAX) < 0)
//      return false;
//  } else {
    if (stm32_i2c_read(TOUCH_I2C_BUS, addr, reg, 1, data, len, I2C_TIMEOUT_MAX) < 0)
      return false;
//  }

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
  uint8_t result;
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, &result, 1)) {
    if (--tryCount == 0) break;
    I2C_Init_Radio();
//    I2C_Init();
  }
  return result;
}

static uint16_t TS_IO_ReadMultiple(uint8_t addr, uint8_t reg, uint8_t * buffer, uint16_t length)
{
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, buffer, length)) {
    if (--tryCount == 0) break;
    I2C_Init_Radio();
//    I2C_Init();
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

static void touch_cst340_debug_info(void)
{
#if 0  // Disabled because cannot compile, will fix when necessary
#if defined(DEBUG)
  uint8_t tmp[4];
  if (!TS_IO_Write(CST340_MODE_DEBUG_INFO, tmp, 0))
      TRACE("CST340 chip NOT FOUND");

  // Check the value, expected ChipID
  uint32_t chipId = tmp[0] << 8) + tmp[1];

  if (!I2C_CST340_ReadRegister(CST340_FWVER_REG, tmp, 4))
    TRACE("Error reading CST340 firmware version!");
  uint32_t fwVersion = tmp[0] << 24 | tmp[1]<<16 | tmp[2]<<8 | tmp[0];

  // Enter normal mode
  if (!I2C_CST340_WriteRegister(CST340_MODE_NORMAL, tmp, 0))
      TRACE("ERROR chaning CST340 mode back to normal!");

  TRACE("cst340: fw ver 0x%08X", fwVersion);
  TRACE("cst836u: chip id 0x%04X", chipId);
#endif
#endif
}

/**
 * @brief  Get the touch screen X and Y positions values
 * @param  DeviceAddr: Device address on communication Bus.
 * @param  X: Pointer to X position value
 * @param  Y: Pointer to Y position value
 * @retval None.
 */
static void cst340_TS_GetXY(uint16_t * X, uint16_t * Y, uint32_t * event)
{
  uint8_t dataxy[4];

  /* Read X and Y positions */
  TS_IO_ReadMultiple(TOUCH_CST340_I2C_ADDRESS, CST340_FINGER1_REG, dataxy, sizeof(dataxy));
  /* Send back ready X position to caller */
  *X = ((dataxy[1]<<4) + ((dataxy[3]>>4)&0x0f));
  *Y = ((dataxy[2]<<4) + ((dataxy[3])&0x0f));
  /* Send back ready Y position to caller */

  *event = dataxy[0];
}

/**
 * @brief  Return if there is touches detected or not.
 *         Try to detect new touches and forget the old ones (reset internal global
 *         variables).
 * @param  DeviceAddr: Device address on communication Bus.
 * @retval : Number of active touches detected
 */
static uint8_t cst340_TS_DetectTouch()
{
  uint8_t nbTouch;
  uint8_t reg = TS_IO_Read(TOUCH_CST340_I2C_ADDRESS, CST340_FINGER1_REG);
  if( reg == 0x06 )
    nbTouch = 1;
  else
    nbTouch = 0;

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

static const TouchControllerDescriptor FT6236 =
{
    .read = ft6x06_TS_GetXY,
    .detectTouch = ft6x06_TS_DetectTouch,
    .printDebugInfo = touch_ft6236_debug_info,
    .contactEvent = FT6206_TOUCH_EVT_FLAG_CONTACT
};
static const TouchControllerDescriptor CST340 =
{
    .read = cst340_TS_GetXY,
    .detectTouch = cst340_TS_DetectTouch,
    .printDebugInfo = touch_cst340_debug_info,
    .contactEvent = CST340_TOUCH_EVT_FLAG_CONTACT
};

void detectTouchController()
{
  if( stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST340_I2C_ADDRESS, 3, 5) == 0)
   {
    TouchControllerType = 0;
     touchController = TC_CST340;
     tc = &CST340;
   } else {
    TouchControllerType = 1;
     touchController = TC_FT6236;
     tc = &FT6236;
   }
}

void TouchInit()
{
  TOUCH_AF_GPIOConfig();  // SET RST=OUT INT=IN INT=HIGH
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

#if defined(DEBUG)
  TRACE("handleTouch: touchX=%d, touchY=%d, tEvent=%d", touchX, touchY, tEvent);
#endif  
  // touch sensor is rotated by 90 deg
  unsigned short tmp = touchY;
  touchY = 319 - touchX;
  touchX = tmp;

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

static bool lastHasTouch = false;
bool touchPanelEventOccured()
{
  bool result = touchEventOccured;
  uint8_t hasTouch = false;
  if (touchEventOccured) {
    hasTouch = tc->detectTouch();
    if (!hasTouch && !lastHasTouch) {
      touchEventOccured = false;
      result = false;
    }
    lastHasTouch = hasTouch;
  }

#if defined(DEBUG)
  TRACE("TouchEvent: %d, %d, %d, %d", touchEventOccured, lastHasTouch, hasTouch, result);
#endif
  return result;      
}

TouchState touchPanelRead()
{
  if (!touchEventOccured) return internalTouchState;

  touchEventOccured = false;

  tmr10ms_t now = get_tmr10ms();
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

#if defined(DEBUG)
  TRACE("%s: Event = %d", touchController == TC_CST340 ? "CST340" : "FT6236", ret.event);
#endif

  return ret;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}
