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

static TouchControllerDesc touchController;
static bool touchEventOccured = false;
static TouchState internalTouchState = {};

void registerTouchController(TouchControllerDesc tc)
{
  touchController = tc;
}

void handleTouch()
{
  unsigned short touchX;
  unsigned short touchY;
  uint32_t tEvent = 0;
  TouchState evt = touchController.read();

#if defined( LCD_DIRECTION ) && (LCD_DIRECTION == LCD_VERTICAL)
  touchX = LCD_WIDTH - touchX;
  touchY = LCD_HEIGHT - touchY;
#else
  unsigned short tmp = (LCD_WIDTH - 1) - touchY;
  touchY = touchX;
  touchX = tmp;
#endif
  if (evt.event == TE_DOWN) {
    internalTouchState.x = evt.x;
    internalTouchState.y = evt.y;
  }
}


void touchIrqCb()
{
  touchEventOccured = true;
}

bool touchPanelEventOccured()
{
  return touchEventOccured;
}

TouchState touchPanelRead()
{
  if (!touchEventOccured) return internalTouchState;

  touchEventOccured = false;
  touchController.read

  if (ft6x06_TS_DetectTouch(TOUCH_FT6236_I2C_ADDRESS)) {
    handleTouch();
    if (internalTouchState.event == TE_DOWN && downTime == 0) {
      downTime = now;
    }
  } else {
    if (internalTouchState.event == TE_DOWN)
      internalTouchState.event = TE_UP;
  }
  TouchState ret = internalTouchState;
  if(internalTouchState.event == TE_UP)
    internalTouchState.event = TE_NONE;
  return ret;
}

TouchState getInternalTouchState()
{
  return internalTouchState;
}
