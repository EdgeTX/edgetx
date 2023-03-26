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

#include "rtos.h"

// needed by the mixer scheduler
extern RTOS_TASK_HANDLE mixerTaskId;

// function running the mixer task
TASK_FUNCTION(mixerTask);

// init, create and start the OS task itself
void mixerTaskInit();

// return true if the mixer has
// already started at least once since boot
bool mixerTaskStarted();

// start the main computations
// and pushing channels out
//
// Please note: there is no guarantee as to when the next computation
//              will be running. This is merely settings a flag allowing
//              the main loop to run.
//
void mixerTaskStart();

// stop the mixer (without exiting). It can be restarted at
// any moment with `mixerTaskStart()`.
//
// Please note: the mixer computation and channel pushing is guaranteed
//              to be stoppped as we lock the main mutex when setting
//              the running flag. This means that mixer data and modules
//              can be handled safely without interferences.
//
void mixerTaskStop();


// exit the mixer forever.
//
// This is probably only useful for simulations as it is not needed to really
// exit the task under any normal conditions. There are no guarantees as to when
// the exit will happen, only that it will eventually happen.
//
void mixerTaskExit();

// hint at whether or not the mixer task should be running
//
// Please note: there is no guarantee on anything, this is just returning
//              a combination of running and exit flags.
//
bool mixerTaskRunning();

//
// Lock / unlock functions: use with care!
//
void mixerTaskLock();
void mixerTaskUnlock();

// returns true if the lock could be acquired
bool mixerTaskTryLock();

