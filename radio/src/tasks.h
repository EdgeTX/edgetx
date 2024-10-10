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

#include "rtos.h"

// stack sizes should be in multiples of 8 for better alignment
#if defined (COLORLCD)
  #define MENUS_STACK_SIZE     (8 * 1024)
#else
  #define MENUS_STACK_SIZE     2000
#endif

#if !defined(DEBUG)
#define MIXER_STACK_SIZE       400
#define AUDIO_STACK_SIZE       400
#else
#define MIXER_STACK_SIZE       512
#define AUDIO_STACK_SIZE       512
#endif

#define CLI_STACK_SIZE         1024  // only consumed with CLI build option

#if defined(FREE_RTOS)
#define MIXER_TASK_PRIO        (tskIDLE_PRIORITY + 4)
#define AUDIO_TASK_PRIO        (tskIDLE_PRIORITY + 3) // Note: FreeRTOSConfig.h defines software timers as priority 2
#define MENUS_TASK_PRIO        (tskIDLE_PRIORITY + 1)
#define CLI_TASK_PRIO          (tskIDLE_PRIORITY + 1)
#else
#define MIXER_TASK_PRIO        (4)
#define AUDIO_TASK_PRIO        (2)
#define MENUS_TASK_PRIO        (1)
#define CLI_TASK_PRIO          (1)
#endif


extern TaskStack<MENUS_STACK_SIZE> menusStack;
extern TaskStack<MIXER_STACK_SIZE> mixerStack;

#if defined(AUDIO)
extern TaskStack<AUDIO_STACK_SIZE> audioStack;
#endif

#if defined(CLI)
extern TaskStack<CLI_STACK_SIZE> cliStack;
#endif

void tasksStart();

extern volatile uint16_t timeForcePowerOffPressed;
inline void resetForcePowerOffRequest()
{
  timeForcePowerOffPressed = 0;
}
