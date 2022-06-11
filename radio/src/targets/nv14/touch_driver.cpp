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
#include "hal.h"

#include "stm32_i2c_driver.h"
#include "timers_driver.h"
#include "delays_driver.h"
#include "touch_driver.h"

#include "debug.h"

volatile static bool touchEventOccured;

#define FT6x06_MAX_INSTANCE  1

#define TOUCH_RCC_AHB1Periph              RCC_AHB1Periph_GPIOB
#define TOUCH_RCC_APB1Periph              RCC_APB1Periph_I2C1
#define TOUCH_GPIO                        I2C_B1_GPIO
#define TOUCH_SCL_GPIO_PIN                I2C_B1_SCL_GPIO_PIN  // PB.08
#define TOUCH_SDA_GPIO_PIN                I2C_B1_SDA_GPIO_PIN  // PB.09

#define TOUCH_FT6236_I2C_ADDRESS          (0x70>>1)
#define TOUCH_CST836U_I2C_ADDRESS         (0x15)

enum TouchControllers {TC_NONE, TC_FT6236, TC_CST836U};
TouchControllers touchController = TC_NONE;

static tc_handle_TypeDef tc_handle = {0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 25

static TouchState internalTouchState = {};

void I2C_FreeBus()
{
  LL_GPIO_InitTypeDef gpioInit;
  LL_GPIO_StructInit(&gpioInit);

  // reset i2c bus by setting clk as output and sending manual clock pulses
  gpioInit.Pin = TOUCH_SCL_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_OUTPUT;
  gpioInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpioInit.Pull = LL_GPIO_PULL_NO;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  LL_GPIO_Init(TOUCH_GPIO, &gpioInit);

  gpioInit.Pin = TOUCH_SDA_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_INPUT;
  gpioInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  gpioInit.Pull = LL_GPIO_PULL_UP;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  LL_GPIO_Init(TOUCH_GPIO, &gpioInit);

  //send 100khz clock train for some 100ms
  tmr10ms_t until = get_tmr10ms() + 11;
  while (get_tmr10ms() < until) {
    if (LL_GPIO_IsInputPinSet(TOUCH_GPIO, TOUCH_SDA_GPIO_PIN)) {
      TRACE("touch: i2c free again\n");
      break;
    }
    TRACE("FREEEEE");
    LL_GPIO_ResetOutputPin(TOUCH_GPIO, TOUCH_SCL_GPIO_PIN);
    delay_us(10);
    LL_GPIO_SetOutputPin(TOUCH_GPIO, TOUCH_SCL_GPIO_PIN);
    delay_us(10);
  }

  //send stop condition:
  gpioInit.Pin = TOUCH_SDA_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_OUTPUT;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  LL_GPIO_Init(TOUCH_GPIO, &gpioInit);

  //clock is low
  LL_GPIO_ResetOutputPin(TOUCH_GPIO, TOUCH_SCL_GPIO_PIN);
  delay_us(10);
  //sda = lo
  LL_GPIO_SetOutputPin(TOUCH_GPIO, TOUCH_SDA_GPIO_PIN);
  delay_us(10);
  //clock goes high
  LL_GPIO_ResetOutputPin(TOUCH_GPIO, TOUCH_SCL_GPIO_PIN);
  delay_us(10);
  //sda = hi
  LL_GPIO_SetOutputPin(TOUCH_GPIO, TOUCH_SDA_GPIO_PIN);
  delay_us(10);
  TRACE("FREE BUS");
}

// void Touch_DeInit()
// {
//   I2C_DeInit(I2C_B1);
//   __HAL_RCC_I2C1_FORCE_RESET();
//   delay_ms(150);
//   __HAL_RCC_I2C1_RELEASE_RESET();
// }

static int _enable_gpio_clock(GPIO_TypeDef *GPIOx)
{
  if (GPIOx == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else
    return -1;

  return 0;
}

void I2C_Init()
{
  stm32_i2c_deinit(TOUCH_I2C_BUS);

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  __HAL_RCC_I2C1_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_DISABLE();

  I2C_FreeBus();

  stm32_i2c_init(TOUCH_I2C_BUS, TOUCH_I2C_CLK_RATE);
  
  LL_GPIO_InitTypeDef gpioInit;
  LL_GPIO_StructInit(&gpioInit);

  _enable_gpio_clock(TOUCH_RST_GPIO);
  _enable_gpio_clock(TOUCH_INT_GPIO);

  gpioInit.Pin = TOUCH_RST_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_OUTPUT;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  gpioInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpioInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(TOUCH_RST_GPIO, &gpioInit);

  //ext interupt
  gpioInit.Pin = TOUCH_INT_GPIO_PIN;
  gpioInit.Mode = LL_GPIO_MODE_INPUT;
  gpioInit.Pull = LL_GPIO_PULL_UP;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpioInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  LL_GPIO_Init(TOUCH_INT_GPIO, &gpioInit);

  LL_SYSCFG_SetEXTISource(TOUCH_INT_EXTI_Port, TOUCH_INT_EXTI_SysCfgLine);

  LL_EXTI_InitTypeDef extiInit;
  LL_EXTI_StructInit(&extiInit);

  extiInit.Line_0_31 = TOUCH_INT_EXTI_Line;
  extiInit.Mode = LL_EXTI_MODE_IT;
  extiInit.Trigger = LL_EXTI_TRIGGER_FALLING;
  extiInit.LineCommand = ENABLE;
  LL_EXTI_Init(&extiInit);

  NVIC_SetPriority(TOUCH_INT_EXTI_IRQn, 8);
  NVIC_EnableIRQ(TOUCH_INT_EXTI_IRQn);
}

#define I2C_TIMEOUT_MAX 5 // 5 ms

bool touch_i2c_read(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t len)
{
  if(touchController == TC_CST836U)
  {
    if(stm32_i2c_master_tx(TOUCH_I2C_BUS, addr, &reg, 1, 3) < 0)
      return false;
    delay_us(50);
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
    I2C_Init();
  }
  return retult;
}

static uint16_t TS_IO_ReadMultiple(uint8_t addr, uint8_t reg, uint8_t * buffer, uint16_t length)
{
  uint8_t tryCount = 3;
  while (!touch_i2c_read(addr, reg, buffer, length)) {
    if (--tryCount == 0) break;
    I2C_Init();
  }
  return 1;
}

static uint8_t TS_IO_Read(uint8_t reg)
{
  return TS_IO_Read((touchController==TC_FT6236)?TOUCH_FT6236_I2C_ADDRESS:TOUCH_CST836U_I2C_ADDRESS, reg);
}

static void touch_ft6236_debug_info(void)
{
#if defined(DEBUG)
  TRACE("ft6x36: thrhld = %d", TS_IO_Read(TOUCH_FT6236_REG_TH_GROUP) * 4);
  TRACE("ft6x36: rep rate=", TS_IO_Read(TOUCH_FT6236_REG_PERIODACTIVE) * 10);
  TRACE("ft6x36: fw lib 0x%02X %02X", TS_IO_Read(TOUCH_FT6236_REG_LIB_VER_H), TS_IO_Read(TOUCH_FT6236_REG_LIB_VER_L));
  TRACE("ft6x36: fw v 0x%02X", TS_IO_Read(TOUCH_FT6236_REG_FIRMID));
  TRACE("ft6x36: CHIP ID 0x%02X", TS_IO_Read(TOUCH_FT6236_REG_CIPHER));
  TRACE("ft6x36: CTPM ID 0x%02X", TS_IO_Read(TOUCH_FT6236_REG_FOCALTECH_ID));
  TRACE("ft6x36: rel code 0x%02X", TS_IO_Read(TOUCH_FT6236_REG_RELEASE_CODE_ID));
#endif
}

/**
 * @brief  Return if there is touches detected or not.
 *         Try to detect new touches and forget the old ones (reset internal global
 *         variables).
 * @param  DeviceAddr: Device address on communication Bus.
 * @retval : Number of active touches detected (can be 0, 1 or 2).
 */
static uint8_t ft6x06_TS_DetectTouch(uint16_t DeviceAddr)
{
  volatile uint8_t nbTouch = 0;

  /* Read register FT6206_TD_STAT_REG to check number of touches detection */
  nbTouch = TS_IO_Read(DeviceAddr, FT6206_TD_STAT_REG);
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
static void ft6x06_TS_GetXY(uint16_t DeviceAddr, uint16_t * X, uint16_t * Y, uint32_t * event)
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
    TS_IO_ReadMultiple(DeviceAddr, regAddress, dataxy, sizeof(dataxy));
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
  TRACE("cst836u: fw ver 0x%02X %02X", TS_IO_Read(CST836U_FW_VERSION_H_REG), TS_IO_Read(CST836U_FW_VERSION_L_REG));
  TRACE("cst836u: module version 0x%02X", TS_IO_Read(CST836U_MODULE_VERSION_REG));
  TRACE("cst836u: project name 0x%02X", TS_IO_Read(CST836U_PROJECT_NAME_REG));
  TRACE("cst836u: chip type 0x%02X 0x%02X", TS_IO_Read(CST836U_CHIP_TYPE_H_REG), TS_IO_Read(CST836U_CHIP_TYPE_L_REG));
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
  nbTouch = TS_IO_Read(CST836U_TOUCH_NUM_REG);
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

void detectTouchController()
{
  touchController = TC_FT6236;
  if( stm32_i2c_is_dev_ready(TOUCH_I2C_BUS, TOUCH_CST836U_I2C_ADDRESS, 3, 5) == 0)
      touchController = TC_CST836U;
}

void TouchInit(void)
{
  I2C_Init();
  TouchReset();
  detectTouchController();
  switch(touchController)
  {
  case TC_FT6236:
    touch_ft6236_debug_info();
    break;
  case TC_CST836U:
    touch_cst836u_debug_info();
    break;
  default:
    break;
  }
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  switch(touchController)
  {
  case TC_FT6236:
    ft6x06_TS_GetXY(TOUCH_FT6236_I2C_ADDRESS, &touchX, &touchY, &tEvent);
    break;
  case TC_CST836U:
    cst836u_TS_GetXY(&touchX, &touchY, &tEvent);
    break;
  default:
    break;
  }
  // uint32_t gesture;
  // ft6x06_TS_GetGestureID(TOUCH_FT6236_I2C_ADDRESS, &gesture);
#if defined( LCD_DIRECTION ) && (LCD_DIRECTION == LCD_VERTICAL)
  touchX = LCD_WIDTH - touchX;
  touchY = LCD_HEIGHT - touchY;
#else
  unsigned short tmp = (LCD_WIDTH - 1) - touchY;
  touchY = touchX;
  touchX = tmp;
#endif
  if (tEvent == FT6206_TOUCH_EVT_FLAG_CONTACT) {
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

#if !defined(TOUCH_INT_EXTI_IRQHandler)
  #error "TOUCH_INT_EXTI_IRQHandler is not defined"
#endif
extern "C" void TOUCH_INT_EXTI_IRQHandler(void)
{
  if (LL_EXTI_IsEnabledIT_0_31(TOUCH_INT_EXTI_Line)  &&
      LL_EXTI_IsActiveFlag_0_31(TOUCH_INT_EXTI_Line)) {
    touchEventOccured = true;
    LL_EXTI_ClearFlag_0_31(TOUCH_INT_EXTI_Line);
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

  tmr10ms_t now = get_tmr10ms();
  internalTouchState.tapCount = 0;

  uint8_t tDetected = 0;
  switch(touchController)
  {
  case TC_FT6236:
    tDetected = ft6x06_TS_DetectTouch(TOUCH_FT6236_I2C_ADDRESS);
    break;
  case TC_CST836U:
    tDetected = cst836u_TS_DetectTouch();
    break;
  default:
    break;
  }

  if (tDetected) {
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
