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
#include "stm32_hal_ll.h"

void usbChargerInit()
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin = USB_CHARGER_GPIO_PIN | USB_USBDet_GPIO_PIN;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_CHARGER_GPIO, &GPIO_InitStructure);
}

bool usbChargerLed()
{
  if (LL_GPIO_IsInputPinSet(USB_CHARGER_GPIO, USB_USBDet_GPIO_PIN) == 0)
    return 0;

  return LL_GPIO_IsInputPinSet(USB_CHARGER_GPIO, USB_CHARGER_GPIO_PIN) == 0;
}
