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

#include "touch_driver.h"

#include "debug.h"

void TouchReset()
{
  LL_GPIO_ResetOutputPin(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(20);
  LL_GPIO_SetOutputPin(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
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
