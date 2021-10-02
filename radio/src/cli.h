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

#ifndef _CLI_H_
#define _CLI_H_

#include "serial.h"

extern uint8_t cliTracesEnabled;

#ifdef __cplusplus
#include "fifo.h"
extern Fifo<uint8_t, 256> cliRxFifo;
#include "tasks.h"
extern RTOS_TASK_HANDLE cliTaskId;
extern RTOS_DEFINE_STACK(cliStack, CLI_STACK_SIZE);
#endif

void cliStart();

#endif // _CLI_H_
