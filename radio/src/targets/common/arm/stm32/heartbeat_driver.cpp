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

#include "stm32_hal_ll.h"
#include "stm32_exti_driver.h"
#include "mixer_scheduler.h"
#include "board.h"
#include "debug.h"

#include "heartbeat_driver.h"
#include "timers_driver.h"

#if defined(INTMODULE_HEARTBEAT_GPIO)

#include "FreeRTOSConfig.h"

volatile HeartbeatCapture heartbeatCapture;

static void trigger_intmodule_heartbeat()
{
#if defined(DEBUG_LATENCY)
  heartbeatCapture.count++;
#endif

  mixerSchedulerSoftTrigger();
}

void init_intmodule_heartbeat()
{
  TRACE("init_intmodule_heartbeat");

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  
  pinInit.Pin = INTMODULE_HEARTBEAT_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP;

  LL_GPIO_Init(INTMODULE_HEARTBEAT_GPIO, &pinInit);
  
  LL_SYSCFG_SetEXTISource(INTMODULE_HEARTBEAT_EXTI_PORT, INTMODULE_HEARTBEAT_EXTI_SYS_LINE);
  
  stm32_exti_enable(INTMODULE_HEARTBEAT_EXTI_LINE, INTMODULE_HEARTBEAT_TRIGGER,
                    trigger_intmodule_heartbeat);

  heartbeatCapture.valid = true;
}

void stop_intmodule_heartbeat()
{
  TRACE("stop_intmodule_heartbeat");
  heartbeatCapture.valid = false;

  stm32_exti_disable(INTMODULE_HEARTBEAT_EXTI_LINE);
}

#endif
