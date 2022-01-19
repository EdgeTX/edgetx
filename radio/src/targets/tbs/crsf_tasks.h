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

#define CROSSFIRE_STACK_SIZE   64
#define SYSTEM_STACK_SIZE      504

#if defined(FREE_RTOS)
#define CROSSFIRE_TASK_PRIO    (tskIDLE_PRIORITY + 4)
#define RTOS_SYS_TASK_PRIO     (tskIDLE_PRIORITY + 1)
#else
#define CROSSFIRE_TASK_PRIO    (4)
#define RTOS_SYS_TASK_PRIO     (1)
#endif

extern RTOS_TASK_HANDLE crossfireTaskId;
extern RTOS_DEFINE_STACK(crossfireStack, CROSSFIRE_STACK_SIZE);

extern RTOS_TASK_HANDLE systemTaskId;
extern RTOS_DEFINE_STACK(systemStack, SYSTEM_STACK_SIZE);

void crossfireTasksCreate();
void crossfireTasksStart();
void crossfireTasksStop();

