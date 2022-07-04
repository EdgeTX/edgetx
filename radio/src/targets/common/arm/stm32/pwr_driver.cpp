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

void pwrInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

#if defined(INTMODULE_BOOTCMD_GPIO)
  INIT_INTMODULE_BOOTCMD_PIN();
  GPIO_InitStructure.GPIO_Pin = INTMODULE_BOOTCMD_GPIO_PIN;
  GPIO_Init(INTMODULE_BOOTCMD_GPIO, &GPIO_InitStructure);
#endif

  // Internal module power
  INTERNAL_MODULE_OFF();
  GPIO_InitStructure.GPIO_Pin = INTMODULE_PWR_GPIO_PIN;
  GPIO_Init(INTMODULE_PWR_GPIO, &GPIO_InitStructure);

  // External module power
  EXTERNAL_MODULE_PWR_OFF();
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_PWR_GPIO_PIN;
  GPIO_Init(EXTMODULE_PWR_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;

  // PWR switch
  GPIO_InitStructure.GPIO_Pin = PWR_SWITCH_GPIO_PIN;
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);

#if defined(PWR_EXTRA_SWITCH_GPIO)
  // PWR Extra switch
  GPIO_InitStructure.GPIO_Pin = PWR_EXTRA_SWITCH_GPIO_PIN;
  GPIO_Init(PWR_EXTRA_SWITCH_GPIO, &GPIO_InitStructure);
#endif

#if defined(PCBREV_HARDCODED)
  hardwareOptions.pcbrev = PCBREV_HARDCODED;
#elif defined(PCBREV_GPIO_PIN)
  #if defined(PCBREV_GPIO_PULL_DOWN)
    GPIO_ResetBits(PCBREV_GPIO, PCBREV_GPIO_PIN);
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  #endif
  GPIO_InitStructure.GPIO_Pin = PCBREV_GPIO_PIN;
  GPIO_Init(PCBREV_GPIO, &GPIO_InitStructure);
  hardwareOptions.pcbrev = PCBREV_VALUE();
#endif

#if defined(TRAINER_DETECT_GPIO_PIN)
  GPIO_InitStructure.GPIO_Pin = TRAINER_DETECT_GPIO_PIN;
  GPIO_Init(TRAINER_DETECT_GPIO, &GPIO_InitStructure);
#endif

#if defined(SD_PRESENT_GPIO_PIN)
  GPIO_ResetBits(SD_PRESENT_GPIO, SD_PRESENT_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = SD_PRESENT_GPIO_PIN;
  GPIO_Init(SD_PRESENT_GPIO, &GPIO_InitStructure);
#endif

#if defined(INTMODULE_USART) && defined(TRAINER_MODULE_CPPM)
  GPIO_SetBits(TRAINER_MODULE_CPPM_GPIO, TRAINER_MODULE_CPPM_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = TRAINER_MODULE_CPPM_GPIO_PIN;
  GPIO_Init(TRAINER_MODULE_CPPM_GPIO, &GPIO_InitStructure);
#endif
}

void pwrOn()
{
  // we keep the init of the PIN to have pwrOn as quick as possible

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

void pwrOff()
{
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

#if defined(PWR_EXTRA_SWITCH_GPIO)
bool pwrForcePressed()
{
  return (GPIO_ReadInputDataBit(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET && GPIO_ReadInputDataBit(PWR_EXTRA_SWITCH_GPIO, PWR_EXTRA_SWITCH_GPIO_PIN) == Bit_RESET);
}
#endif

bool pwrPressed()
{
#if defined(PWR_EXTRA_SWITCH_GPIO)
  return (GPIO_ReadInputDataBit(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET || GPIO_ReadInputDataBit(PWR_EXTRA_SWITCH_GPIO, PWR_EXTRA_SWITCH_GPIO_PIN) == Bit_RESET);
#else
  return GPIO_ReadInputDataBit(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET;
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
