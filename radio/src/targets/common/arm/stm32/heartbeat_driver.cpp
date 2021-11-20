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

#include "mixer_scheduler.h"
#include "board.h"

#if defined(INTMODULE_HEARTBEAT_GPIO)

#include "FreeRTOSConfig.h"

volatile HeartbeatCapture heartbeatCapture;

void init_intmodule_heartbeat()
{
  TRACE("init_intmodule_heartbeat");
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_HEARTBEAT_GPIO_PIN;
  GPIO_Init(INTMODULE_HEARTBEAT_GPIO, &GPIO_InitStructure);

  SYSCFG_EXTILineConfig(INTMODULE_HEARTBEAT_EXTI_PortSource,
                        INTMODULE_HEARTBEAT_EXTI_PinSource);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = INTMODULE_HEARTBEAT_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = INTMODULE_HEARTBEAT_TRIGGER;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_SetPriority(INTMODULE_HEARTBEAT_EXTI_IRQn,
                   // Highest priority interrupt
                   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

  NVIC_EnableIRQ(INTMODULE_HEARTBEAT_EXTI_IRQn);
  heartbeatCapture.valid = true;
}

void stop_intmodule_heartbeat()
{
  TRACE("stop_intmodule_heartbeat");
  heartbeatCapture.valid = false;

#if !defined(INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER)
  NVIC_DisableIRQ(INTMODULE_HEARTBEAT_EXTI_IRQn);
#endif

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = INTMODULE_HEARTBEAT_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = INTMODULE_HEARTBEAT_TRIGGER;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
}

void check_intmodule_heartbeat()
{
  if (EXTI_GetITStatus(INTMODULE_HEARTBEAT_EXTI_LINE) != RESET) {
#if !defined(INTMODULE_USART)
    heartbeatCapture.timestamp = getTmr2MHz();
#endif
#if defined(DEBUG_LATENCY)
    heartbeatCapture.count++;
#endif
    EXTI_ClearITPendingBit(INTMODULE_HEARTBEAT_EXTI_LINE);

    mixerSchedulerResetTimer();
    mixerSchedulerISRTrigger();
  }
}
#endif

#if defined(INTMODULE_HEARTBEAT) && !defined(INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER)
extern "C" void INTMODULE_HEARTBEAT_EXTI_IRQHandler()
{
  check_intmodule_heartbeat();
  #if defined(TELEMETRY_EXTI_REUSE_INTERRUPT_INTMODULE_HEARTBEAT)
    check_telemetry_exti();
  #endif
}
#endif
