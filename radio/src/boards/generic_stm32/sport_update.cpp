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

#include "hal.h"
#include "stm32_hal_ll.h"

#include "hal/module_port.h"
#include "opentx.h" // g_eeGeneral

void sportUpdateInit()
{
#if defined(SPORT_UPDATE_PWR_GPIO)

#if defined(RADIO_X7)
  // QX7 has a external S.PORT connector
  // with switchable power from revision 40 on
  if (hardwareOptions.pcbrev == PCBREV_X7_40) {
    extern etx_module_t _sport_module;
    extern void _sport_set_pwr(uint8_t);
    _sport_module.set_pwr = _sport_set_pwr;
  } else {
    return;
  }
#endif

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = SPORT_UPDATE_PWR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(SPORT_UPDATE_PWR_GPIO, &GPIO_InitStructure);
#endif
}

void sportUpdatePowerInit()
{
  modulePortSetPower(SPORT_MODULE, g_eeGeneral.sportUpdatePower);
}
