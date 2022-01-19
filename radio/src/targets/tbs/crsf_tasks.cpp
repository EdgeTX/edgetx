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
#include "mixer_scheduler.h"
#include "crsf_tasks.h"

#if !defined(SIMU)

#include "io/crsf/crsf.h"
#include "io/crsf/crossfire.h"

RTOS_TASK_HANDLE crossfireTaskId;
RTOS_DEFINE_STACK(crossfireStack, CROSSFIRE_STACK_SIZE);

RTOS_TASK_HANDLE systemTaskId;
RTOS_DEFINE_STACK(systemStack, SYSTEM_STACK_SIZE);

TASK_FUNCTION(systemTask)
{
  static uint32_t getModelIdDelay = 0;
  volatile uint32_t delayCount = 0;
  bkregSetStatusFlag(CRSF_SET_MODEL_ID_PENDING);

  while (1) {
    if (getCrsfFlag(CRSF_FLAG_SHOW_BOOTLOADER_ICON)) {
      if (delayCount == 0) {
        delayCount = RTOS_GET_TIME();
        vTaskSuspendAll();
        lcdOn();
        drawDownload();
        storageDirty(EE_GENERAL|EE_MODEL);
        storageCheck(true);
        sdDone();
      }
      if (RTOS_GET_TIME() - delayCount >= 200) {
        NVIC_SystemReset();
      }
    }

    crsfSharedFifoHandler();
    agentHandler();

    if (bkregGetStatusFlag(CRSF_SET_MODEL_ID_PENDING) && get_tmr10ms() - getModelIdDelay > 100) {
      crsfSetModelID();
      crsfGetModelID();
      if (currentCrsfModelId == g_model.header.modelId[INTERNAL_MODULE])
        bkregClrStatusFlag(CRSF_SET_MODEL_ID_PENDING);
      getModelIdDelay = get_tmr10ms();
    }
    if (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_NONE && isMixerTaskScheduled()) {
      clearMixerTaskSchedule();
      mixerSchedulerISRTrigger();
    }
  }
  TASK_RETURN();
}

typedef void      (*FUNCPtr)(void *);
StaticSemaphore_t xSemaphoreBuffer[TASK_SEM_COUNT];

void crossfireTasksCreate()
{
  RTOS_CREATE_TASK(crossfireTaskId, (FUNCPtr)CROSSFIRE_TASK_ADDRESS, "crossfire", crossfireStack, CROSSFIRE_STACK_SIZE, CROSSFIRE_TASK_PRIO);
  RTOS_CREATE_TASK(systemTaskId, systemTask, "system", systemStack, SYSTEM_STACK_SIZE, RTOS_SYS_TASK_PRIO);
}

void crossfireTasksStart()
{
  crossfireStack.paint();
  systemStack.paint();

  SemaphoreHandle_t taskSem[TASK_SEM_COUNT] = {0};
  // Test if crossfire task is available and start it
  if (*(uint32_t *)CROSSFIRE_TASK_ADDRESS != 0xFFFFFFFF) {
    crossfireTasksCreate();
    RTOS_CREATE_SEM( taskSem[XF_TASK_SEM], xSemaphoreBuffer[XF_TASK_SEM]);
    RTOS_CREATE_SEM( taskSem[CRSF_SD_TASK_SEM], xSemaphoreBuffer[CRSF_SD_TASK_SEM]);
    RTOS_CREATE_SEM( taskSem[BOOTLOADER_ICON_WAIT_SEM], xSemaphoreBuffer[BOOTLOADER_ICON_WAIT_SEM]);

    for (uint8_t i = 0; i < TASK_SEM_COUNT; i++) {
      crossfireSharedData.taskSem[i] = (uint32_t *)taskSem[i];
    }
  }
}

void crossfireTasksStop()
{
  NVIC_DisableIRQ(INTERRUPT_EXTI_IRQn);
  NVIC_DisableIRQ(INTERRUPT_NOT_TIMER_IRQn);

  // TODO: is that really necessary?
  // RTOS_DEL_TASK(crossfireTaskId);
  // RTOS_DEL_TASK(systemTaskId);
}
#endif
