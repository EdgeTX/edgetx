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

#pragma once

#include "edgetx_types.h"

typedef int8_t horzpos_t;
typedef uint16_t vertpos_t;

typedef void (* MenuHandlerFunc)(event_t event);
typedef bool (* MenuEnabledFunc)();

typedef struct _MenuHandler {
  MenuHandlerFunc menuFunc;
  MenuEnabledFunc enabledFunc;

  bool isEnabled() const
  {
    if (enabledFunc)
      return enabledFunc();
    return true;
  }
} MenuHandler;

extern tmr10ms_t menuEntryTime;
extern vertpos_t menuVerticalPosition;
extern horzpos_t menuHorizontalPosition;
extern vertpos_t menuVerticalOffset;
extern uint8_t menuCalibrationState;
extern MenuHandlerFunc menuHandlers[5];
extern uint8_t menuVerticalPositions[4];
extern uint8_t menuVerticalOffsets[4];
extern uint8_t menuLevel;
extern event_t menuEvent;

void chainMenu(MenuHandlerFunc newMenu);
void pushMenu(MenuHandlerFunc newMenu);
void popMenu();
void abortPopMenu();

inline MenuHandlerFunc lastPopMenu()
{
  return menuHandlers[menuLevel + 1];
}
