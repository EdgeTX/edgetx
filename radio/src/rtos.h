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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C++" {
#endif

#if defined(SIMU)
  #include <pthread.h>
  #include <semaphore.h>

  static inline void RTOS_START()
  {
  }

  static inline uint32_t mainStackAvailable()
  {
    return 500;
  }

#elif defined(FREE_RTOS)

  #include <FreeRTOS/include/FreeRTOS.h>
  #include <FreeRTOS/include/task.h>
  
  static inline void RTOS_START()
  {
    vTaskStartScheduler();
  }

  static inline uint32_t getStackAvailable(void * address, uint32_t size)
  {
    uint32_t * array = (uint32_t *)address;
    uint32_t i = 0;
    while (i < size && array[i] == 0x55555555) {
      i++;
    }
    return i;
  }

  extern int _estack;
  extern int _main_stack_start;
  static inline uint32_t stackSize()
  {
    return ((unsigned char *)&_estack - (unsigned char *)&_main_stack_start) / 4;
  }

  static inline uint32_t mainStackAvailable()
  {
    return getStackAvailable(&_main_stack_start, stackSize());
  }

#endif  // RTOS type

#ifdef __cplusplus
}
#endif
