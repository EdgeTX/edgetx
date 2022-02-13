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
#include "touch_driver.h"

volatile static bool touchEventOccured;

#define FT6x06_MAX_INSTANCE  1

#define I2C_TOUCH_RCC_AHB1Periph              RCC_AHB1Periph_GPIOB
#define I2C_TOUCH_RCC_APB1Periph              RCC_APB1Periph_I2C1
#define I2C_TOUCH                             I2C1
#define I2C_TOUCH_GPIO                        GPIOB
#define I2C_TOUCH_SCL_GPIO_PIN                GPIO_Pin_8  // PB.08
#define I2C_TOUCH_SDA_GPIO_PIN                GPIO_Pin_7  // PB.09

#define I2C_TOUCH_RESET_GPIO            GPIOB
#define I2C_TOUCH_RESET_GPIO_PIN        GPIO_Pin_12  // PB.12
#define I2C_TOUCH_INT_GPIO              GPIOB
#define I2C_TOUCH_INT_GPIO_PIN          GPIO_Pin_9  // PB.09

#define TOUCH_FT6236_I2C_ADDRESS      (0x70>>1)

uint8_t ft6x06[FT6x06_MAX_INSTANCE] = {0};
static ft6x06_handle_TypeDef ft6x06_handle = {FT6206_I2C_NOT_INITIALIZED, 0, 0};

tmr10ms_t downTime = 0;
tmr10ms_t tapTime = 0;
short tapCount = 0;
#define TAP_TIME 25

static TouchState internalTouchState = {};

void I2C_FreeBus()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  // reset i2c bus by setting clk as output and sending manual clock pulses
  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_SCL_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_TOUCH_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_TOUCH_GPIO, &GPIO_InitStructure);

  //send 100khz clock train for some 100ms
  tmr10ms_t until = get_tmr10ms() + 11;
  while (get_tmr10ms() < until) {
    if (GPIO_ReadInputDataBit(I2C_TOUCH_GPIO, I2C_TOUCH_SDA_GPIO_PIN) == 1) {
      TRACE("touch: i2c free again\n");
      break;
    }
    TRACE("FREEEEE");
    I2C_TOUCH_GPIO->BSRRH = I2C_TOUCH_SCL_GPIO_PIN; //BSRRL
    delay_us(10);
    I2C_TOUCH_GPIO->BSRRL = I2C_TOUCH_SCL_GPIO_PIN;
    delay_us(10);
  }

  //send stop condition:
  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_TOUCH_GPIO, &GPIO_InitStructure);

  //clock is low
  I2C_TOUCH_GPIO->BSRRH = I2C_TOUCH_SCL_GPIO_PIN;
  delay_us(10);
  //sda = lo
  I2C_TOUCH_GPIO->BSRRL = I2C_TOUCH_SDA_GPIO_PIN;
  delay_us(10);
  //clock goes high
  I2C_TOUCH_GPIO->BSRRH = I2C_TOUCH_SCL_GPIO_PIN;
  delay_us(10);
  //sda = hi
  I2C_TOUCH_GPIO->BSRRL = I2C_TOUCH_SDA_GPIO_PIN;
  delay_us(10);
  TRACE("FREE BUS");
}

void Touch_DeInit()
{
  I2C_DeInit(I2C_TOUCH);
  (RCC->APB1RSTR |= (RCC_APB1RSTR_I2C1RST));
  delay_ms(150);
  (RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C1RST));
}

void I2C_Init()
{
  Touch_DeInit();

  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  I2C_InitTypeDef I2C_InitStructure;


  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

  I2C_FreeBus();

  GPIO_PinAFConfig(I2C_TOUCH_GPIO, GPIO_PinSource7, GPIO_AF_I2C1);
  GPIO_PinAFConfig(I2C_TOUCH_GPIO, GPIO_PinSource8, GPIO_AF_I2C1);

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_SCL_GPIO_PIN | I2C_TOUCH_SDA_GPIO_PIN;
  GPIO_Init(I2C_TOUCH_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_RESET_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL; //
  GPIO_Init(I2C_TOUCH_RESET_GPIO, &GPIO_InitStructure);


  //https://community.st.com/s/question/0D50X00009XkZ9FSAV/stm32f4-i2c-issues-solved
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  I2C_StructInit(&I2C_InitStructure);
  I2C_InitStructure.I2C_ClockSpeed = 400000;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C_TOUCH, &I2C_InitStructure);

  I2C_Init(I2C_TOUCH, &I2C_InitStructure);
  //I2C_StretchClockCmd(I2C_TOUCH, ENABLE);
  I2C_Cmd(I2C_TOUCH, ENABLE);

  //ext interupt
  GPIO_InitStructure.GPIO_Pin = I2C_TOUCH_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(I2C_TOUCH_INT_GPIO, &GPIO_InitStructure);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource9);
  EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#define I2C_TIMEOUT_MAX 1000

bool I2C_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C_TOUCH, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C_WaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C_TOUCH, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C_Send7BitAddress(uint8_t address, uint16_t direction)
{
  I2C_SendData(I2C_TOUCH, (address << 1) | ((direction == I2C_Direction_Receiver) ? 1 : 0));
  // check if slave acknowledged his address within timeout
  if (!I2C_WaitEvent(direction == I2C_Direction_Transmitter ? I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED : I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    return false;
  return true;
}

bool touch_i2c_read(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t len)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY)) return false;
  I2C_GenerateSTART(I2C_TOUCH, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;
  if (!I2C_Send7BitAddress(addr, I2C_Direction_Transmitter)) return false;
  I2C_SendData(I2C_TOUCH, reg);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return false;
  I2C_GenerateSTART(I2C_TOUCH, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;
  if (!I2C_Send7BitAddress(addr, I2C_Direction_Receiver)) return false;

  if (len > 1) I2C_AcknowledgeConfig(I2C_TOUCH, ENABLE);

  while (len) {
    if (len == 1) I2C_AcknowledgeConfig(I2C_TOUCH, DISABLE);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))return false;
    *data++ = I2C_ReceiveData(I2C_TOUCH);;
    len--;
  }
  I2C_GenerateSTOP(I2C_TOUCH, ENABLE);
  return true;
}

static bool touch_i2c_write(uint8_t addr, uint8_t reg, uint8_t * data, uint8_t len)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY)) return false;
  I2C_GenerateSTART(I2C_TOUCH, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;

  if (!I2C_Send7BitAddress(addr, I2C_Direction_Transmitter)) return false;
  I2C_SendData(I2C_TOUCH, (uint8_t) ((reg & 0xFF00) >> 8));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) return false;
  I2C_SendData(I2C_TOUCH, (uint8_t) (reg & 0x00FF));
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) return false;
  while (len--) {
    I2C_SendData(I2C_TOUCH, *data);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) return false;
    data++;
  }
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return false;
  I2C_GenerateSTOP(I2C_TOUCH, ENABLE);
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

/*
static uint8_t TS_IO_Read(uint8_t reg)
{
  return TS_IO_Read(TOUCH_FT6236_I2C_ADDRESS, reg);
}
*/

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
  ft6x06_handle.currActiveTouchNb = nbTouch;

  /* Reset current active touch index on which to work on */
  ft6x06_handle.currActiveTouchIdx = 0;
  return (nbTouch);
}

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
/*static void ft6x06_TS_GetTouchInfo(uint16_t DeviceAddr,
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

    } // end switch(touchIdx)

    // Read weight, area and Event Id of touch index
    TS_IO_ReadMultiple(DeviceAddr, regAddress, dataxy, sizeof(dataxy));

    // Return weight of touch index
    *pWeight = (dataxy[0] & FT6206_TOUCH_WEIGHT_MASK) >> FT6206_TOUCH_WEIGHT_SHIFT;
    // Return area of touch index
    *pArea = (dataxy[1] & FT6206_TOUCH_AREA_MASK) >> FT6206_TOUCH_AREA_SHIFT;
    // Return Event Id  of touch index
    *pEvent = (dataxy[2] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;

  } // of if(touchIdx < ft6x06_handle.currActiveTouchNb)
}
*/

/**
 * @brief  Get the touch screen X and Y positions values
 *         Manage multi touch thanks to touch Index global
 *         variable 'ft6x06_handle.currActiveTouchIdx'.
 * @param  DeviceAddr: Device address on communication Bus.
 * @param  X: Pointer to X position value
 * @param  Y: Pointer to Y position value
 * @retval None.
 */
static void ft6x06_TS_GetXY(uint16_t DeviceAddr, uint16_t * X, uint16_t * Y, uint32_t * event)
{
  uint8_t regAddress = 0;
  uint8_t dataxy[4];

  if (ft6x06_handle.currActiveTouchIdx < ft6x06_handle.currActiveTouchNb) {
    switch (ft6x06_handle.currActiveTouchIdx) {
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
    ft6x06_handle.currActiveTouchIdx++;
  }
}

void TouchReset()
{
  GPIO_ResetBits(I2C_TOUCH_RESET_GPIO, I2C_TOUCH_RESET_GPIO_PIN);
  delay_ms(20);
  GPIO_SetBits(I2C_TOUCH_RESET_GPIO, I2C_TOUCH_RESET_GPIO_PIN);
  delay_ms(300);
}

void TouchInit(void)
{
  I2C_Init();
  TouchReset();
  touch_ft6236_debug_info();
  /* INT generation for new touch available */
  /* Note TS_INT is active low */
  uint8_t regValue = 0;
  regValue = (FT6206_G_MODE_INTERRUPT_TRIGGER & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT)) << FT6206_G_MODE_INTERRUPT_SHIFT;
  /* Set interrupt TOUCH_FT6236_I2C_ADDRESS mode in FT6206_GMODE_REG */
  TS_IO_Write(TOUCH_FT6236_I2C_ADDRESS, FT6206_GMODE_REG, regValue);
  /*trigger reset */
  TouchReset();
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  ft6x06_TS_GetXY(TOUCH_FT6236_I2C_ADDRESS, &touchX, &touchY, &tEvent);
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

extern "C" void EXTI9_5_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line9) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line9);
    touchEventOccured = true;
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

  if (ft6x06_TS_DetectTouch(TOUCH_FT6236_I2C_ADDRESS)) {
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
