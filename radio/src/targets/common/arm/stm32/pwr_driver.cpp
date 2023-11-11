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

#include "board.h"
#include "hal/abnormal_reboot.h"
#include "stm32_hal_ll.h"

void pwrInit()
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;

#if defined(INTMODULE_BOOTCMD_GPIO)
  INIT_INTMODULE_BOOTCMD_PIN();
  GPIO_InitStructure.Pin = INTMODULE_BOOTCMD_GPIO_PIN;
  LL_GPIO_Init(INTMODULE_BOOTCMD_GPIO, &GPIO_InitStructure);
#endif

  // Internal module power
#if defined(HARDWARE_INTERNAL_MODULE)
  INTERNAL_MODULE_OFF();
  GPIO_InitStructure.Pin = INTMODULE_PWR_GPIO_PIN;
  LL_GPIO_Init(INTMODULE_PWR_GPIO, &GPIO_InitStructure);
#endif

  // External module power
  EXTERNAL_MODULE_PWR_OFF();
  GPIO_InitStructure.Pin = EXTMODULE_PWR_GPIO_PIN;
  LL_GPIO_Init(EXTMODULE_PWR_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;

  // PWR switch
  GPIO_InitStructure.Pin = PWR_SWITCH_GPIO_PIN;
  LL_GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);

#if defined(PWR_EXTRA_SWITCH_GPIO)
  // PWR Extra switch
  GPIO_InitStructure.Pin = PWR_EXTRA_SWITCH_GPIO_PIN;
  LL_GPIO_Init(PWR_EXTRA_SWITCH_GPIO, &GPIO_InitStructure);
#endif

#if defined(PCBREV_HARDCODED)
  hardwareOptions.pcbrev = PCBREV_HARDCODED;
#elif defined(PCBREV_GPIO_PIN)
  #if defined(PCBREV_GPIO_PULL_DOWN)
    LL_GPIO_ResetOutputPin(PCBREV_GPIO, PCBREV_GPIO_PIN);
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
  #endif
  GPIO_InitStructure.Pin = PCBREV_GPIO_PIN;
  LL_GPIO_Init(PCBREV_GPIO, &GPIO_InitStructure);
  hardwareOptions.pcbrev = PCBREV_VALUE();
#endif
}

void pwrOn()
{
  // we keep the init of the PIN to have pwrOn as quick as possible

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  LL_GPIO_SetOutputPin(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

void pwrOff()
{
  LL_GPIO_ResetOutputPin(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

#if defined(PWR_EXTRA_SWITCH_GPIO)
bool pwrForcePressed()
{
  return (LL_GPIO_IsInputPinSet(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == 0 && LL_GPIO_IsInputPinSet(PWR_EXTRA_SWITCH_GPIO, PWR_EXTRA_SWITCH_GPIO_PIN) == 0);
}
#endif

bool pwrPressed()
{
#if defined(PWR_EXTRA_SWITCH_GPIO)
  return (LL_GPIO_IsInputPinSet(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == 0
      ||  LL_GPIO_IsInputPinSet(PWR_EXTRA_SWITCH_GPIO, PWR_EXTRA_SWITCH_GPIO_PIN) == 0);
#else
  return LL_GPIO_IsInputPinSet(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == 0;
#endif
}

bool pwrOffPressed()
{
#if defined(PWR_BUTTON_PRESS)
  return pwrPressed();
#else
  return !pwrPressed();
#endif
}

void pwrResetHandler()
{
  RCC->AHB1ENR |= PWR_RCC_AHB1Periph;

  // these two NOPs are needed (see STM32F errata sheet) before the peripheral
  // register can be written after the peripheral clock was enabled
  __ASM volatile ("nop");
  __ASM volatile ("nop");

  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
}
