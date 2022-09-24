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

void EXTERNAL_MODULE_ON()
{
  GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN);

  if (hardwareOptions.pcbrev == PCBREV_NV14) {
    GPIO_ResetBits(EXTMODULE_PWR_FIX_GPIO, EXTMODULE_PWR_FIX_GPIO_PIN);
  }
}

void EXTERNAL_MODULE_OFF()
{
  GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN);

  if (hardwareOptions.pcbrev == PCBREV_NV14) {
    GPIO_SetBits(EXTMODULE_PWR_FIX_GPIO, EXTMODULE_PWR_FIX_GPIO_PIN);
  }
}

void extModuleInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_PWR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN);
  GPIO_Init(EXTMODULE_PWR_GPIO, &GPIO_InitStructure);

  //for additional transistor to ensuring module is completely disabled
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  //pin must be pulled to V+ (voltage of board - VCC is not enough to fully close transistor)
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  if (hardwareOptions.pcbrev == PCBREV_NV14) {
    GPIO_InitStructure.GPIO_Pin = EXTMODULE_PWR_FIX_GPIO_PIN;
    GPIO_SetBits(EXTMODULE_PWR_FIX_GPIO, EXTMODULE_PWR_FIX_GPIO_PIN);
    GPIO_Init(EXTMODULE_PWR_FIX_GPIO, &GPIO_InitStructure);
  }

  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_INVERT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(EXTMODULE_TX_INVERT_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = EXTMODULE_RX_INVERT_GPIO_PIN;
  GPIO_Init(EXTMODULE_RX_INVERT_GPIO, &GPIO_InitStructure);

  EXTMODULE_TX_INVERTED();
  EXTMODULE_RX_INVERTED();
}
