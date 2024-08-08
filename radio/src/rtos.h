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

  #define SIMU_SLEEP_OR_EXIT_MS(x)       simuSleep(x)
  #define RTOS_MS_PER_TICK  1

  typedef pthread_t RTOS_TASK_HANDLE;
  typedef pthread_mutex_t RTOS_MUTEX_HANDLE;

  typedef uint32_t RTOS_FLAG_HANDLE;

  typedef sem_t * RTOS_EVENT_HANDLE;

  extern uint64_t simuTimerMicros(void);
  extern uint8_t simuSleep(uint32_t ms);

  static inline void RTOS_START()
  {
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    simuSleep(x);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    RTOS_WAIT_MS(x * RTOS_MS_PER_TICK);
  }

#ifdef __cplusplus
  static inline void RTOS_CREATE_MUTEX(pthread_mutex_t &mutex)
  {
    mutex = PTHREAD_MUTEX_INITIALIZER;
  }

  static inline bool RTOS_LOCK_MUTEX(pthread_mutex_t &mutex)
  {
      return pthread_mutex_lock(&mutex) == 0;
  }

  static inline bool RTOS_TRYLOCK_MUTEX(pthread_mutex_t &mutex)
  {
      return pthread_mutex_trylock(&mutex) == 0;
  }

  static inline void RTOS_UNLOCK_MUTEX(pthread_mutex_t &mutex)
  {
      pthread_mutex_unlock(&mutex);
  }

  template<int SIZE>
  class TaskStack
  {
    public:
      TaskStack()
      {
      }

      void paint()
      {
      }

      uint32_t size()
      {
        return SIZE;
      }

      uint32_t available()
      {
        return SIZE / 2;
      }
  };
  #define RTOS_DEFINE_STACK(taskHandle, name, size) TaskStack<size> name

  #define TASK_FUNCTION(task)           void* task(void *)

  inline void RTOS_CREATE_TASK(pthread_t &taskId, void * (*task)(void *), const char * name)
  {
    pthread_create(&taskId, nullptr, task, nullptr);
#ifdef __linux__
    pthread_setname_np(taskId, name);
#endif
  }

  template <int SIZE>
  inline void RTOS_CREATE_TASK(pthread_t &taskId, void *(*task)(void *),
                               const char *name, TaskStack<SIZE> &,
                               unsigned size = 0, unsigned priority = 0)
  {
    (void)size;
    (void)priority;
    RTOS_CREATE_TASK(taskId, task, name);
  }

#define TASK_RETURN()                 return nullptr

  constexpr uint32_t mainStackAvailable()
  {
    return 500;
  }
#endif  // __cplusplus

  // return 2ms resolution to match CoOS settings
  static inline uint32_t RTOS_GET_TIME(void)
  {
    return (uint32_t)(simuTimerMicros() / 2000);
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (uint32_t)(simuTimerMicros() / 1000);
  }

#elif defined(FREE_RTOS)
#ifdef __cplusplus
  extern "C" {
#endif
    #include <FreeRTOS/include/FreeRTOS.h>
    #include <FreeRTOS/include/task.h>
    #include <FreeRTOS/include/semphr.h>
#ifdef __cplusplus
  }
#endif

  #define RTOS_MS_PER_TICK portTICK_PERIOD_MS

  typedef struct {
    TaskHandle_t rtos_handle;
    StaticTask_t task_struct;
  } RTOS_TASK_HANDLE;

  typedef struct {
    SemaphoreHandle_t rtos_handle;
    StaticSemaphore_t mutex_struct;
  } RTOS_MUTEX_HANDLE;
  
  typedef RTOS_MUTEX_HANDLE RTOS_FLAG_HANDLE;
  
  static inline void RTOS_START()
  {
    vTaskStartScheduler();
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    if (!x)
      return;
    if ((x = x / RTOS_MS_PER_TICK) < 1)
      x = 1;

    vTaskDelay(x);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    vTaskDelay(x);
  }

  static inline void _RTOS_CREATE_TASK(RTOS_TASK_HANDLE *h,
                                       TaskFunction_t pxTaskCode,
                                       const char *name,
                                       StackType_t *const puxStackBuffer,
                                       const uint32_t ulStackDepth,
                                       UBaseType_t uxPriority)
  {
    h->rtos_handle = xTaskCreateStatic(
        pxTaskCode, name, ulStackDepth, NULL, uxPriority,
        puxStackBuffer, &h->task_struct);
  }

  #define RTOS_CREATE_TASK(h,task,name,stackStruct,stackSize,prio) \
    _RTOS_CREATE_TASK(&h,task,name,stackStruct.stack,stackSize,prio)
  
  static inline void _RTOS_CREATE_MUTEX(RTOS_MUTEX_HANDLE* h)
  {
    h->rtos_handle = xSemaphoreCreateMutexStatic(&h->mutex_struct);
    xSemaphoreGive(h->rtos_handle);
  }

  #define RTOS_CREATE_MUTEX(handle) _RTOS_CREATE_MUTEX(&handle)

  static inline bool _RTOS_LOCK_MUTEX(RTOS_MUTEX_HANDLE* h, TickType_t xTickToWait)
  {
    return xSemaphoreTake(h->rtos_handle, xTickToWait) == pdTRUE;
  }

  #define RTOS_LOCK_MUTEX(handle) _RTOS_LOCK_MUTEX(&handle, portMAX_DELAY)
  #define RTOS_TRYLOCK_MUTEX(handle) _RTOS_LOCK_MUTEX(&handle, (TickType_t)0)

  static inline void _RTOS_UNLOCK_MUTEX(RTOS_MUTEX_HANDLE* h)
  {
    xSemaphoreGive(h->rtos_handle);
  }

  #define RTOS_UNLOCK_MUTEX(handle) _RTOS_UNLOCK_MUTEX(&handle)

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

  //#define RTOS_CREATE_FLAG(flag)        flag = CoCreateFlag(false, false)
  //#define RTOS_SET_FLAG(flag)           (void)CoSetFlag(flag)
  //#define RTOS_CLEAR_FLAG(flag)         (void)CoClearFlag(flag)

  // returns true if timeout
  static inline bool _RTOS_WAIT_FLAG(RTOS_FLAG_HANDLE* flag, uint32_t timeout)
  {
    return xSemaphoreTake(flag->rtos_handle, timeout * RTOS_MS_PER_TICK)
      == pdFALSE;
  }

  #define RTOS_WAIT_FLAG(flag,timeout) _RTOS_WAIT_FLAG(&flag,timeout)

  static inline void _RTOS_ISR_SET_FLAG(RTOS_FLAG_HANDLE* flag)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Give semaphore back from ISR to trigger a task waiting for it
    xSemaphoreGiveFromISR( flag->rtos_handle, &xHigherPriorityTaskWoken );

    // If xHigherPriorityTaskWoken was set to true we should yield
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }

  #define RTOS_ISR_SET_FLAG(flag) _RTOS_ISR_SET_FLAG(&flag)

#ifdef __cplusplus
  template<int SIZE>
  class TaskStack
  {
    public:
      TaskStack(RTOS_TASK_HANDLE *h) {
        this->h = h;
      }

      uint32_t size()
      {
        return SIZE * 4;
      }

      uint32_t available()
      {
        return uxTaskGetStackHighWaterMark(h->rtos_handle);
      }

      StackType_t stack[SIZE];
    protected:
      RTOS_TASK_HANDLE *h;
  };
#endif // __cplusplus

  static inline TickType_t RTOS_GET_TIME(void)
  {
    return xTaskGetTickCount();
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (RTOS_GET_TIME() * RTOS_MS_PER_TICK);
  }

  // stack must be aligned to 8 bytes otherwise printf for %f does not work!
  #define RTOS_DEFINE_STACK(taskHandle, name, size) TaskStack<size> __ALIGNED(8) name __CCMRAM (&taskHandle) 

  #define TASK_FUNCTION(task)           void task(void *)
  #define TASK_RETURN()                 vTaskDelete(nullptr)

#else // no RTOS

  #error "No RTOS implementation defined"

#endif  // RTOS type

#ifdef __cplusplus
}
#endif
