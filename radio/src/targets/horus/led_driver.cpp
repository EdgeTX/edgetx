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

void ledInit()
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin = LED_GPIO_PIN;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(LED_GPIO, &GPIO_InitStructure);
}

#if defined(PCBX12S)
void ledOff()
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin = LED_GPIO_PIN;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(LED_GPIO, &GPIO_InitStructure);
}

void ledRed()
{
  ledInit();
  LL_GPIO_SetOutputPin(LED_GPIO, LED_GPIO_PIN);
}

void ledBlue()
{
  ledInit();
  LL_GPIO_ResetOutputPin(LED_GPIO, LED_GPIO_PIN);
}
#elif defined(PCBX10)
void ledOff()
{
  LL_GPIO_ResetOutputPin(LED_GPIO, LED_GPIO_PIN);
}

void ledRed()
{
  ledOff();
  LL_GPIO_SetOutputPin(LED_GPIO, LED_RED_GPIO_PIN);
}

void ledGreen()
{
  ledOff();
  LL_GPIO_SetOutputPin(LED_GPIO, LED_GREEN_GPIO_PIN);
}

void ledBlue()
{
  ledOff();
  LL_GPIO_SetOutputPin(LED_GPIO, LED_BLUE_GPIO_PIN);
}
#endif
