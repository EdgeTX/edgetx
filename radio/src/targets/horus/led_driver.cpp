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

void ledInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(LED_GPIO, &GPIO_InitStructure);
}

#if defined(PCBX12S)
void ledOff()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(LED_GPIO, &GPIO_InitStructure);
}

void ledRed()
{
  ledInit();
  GPIO_SetBits(LED_GPIO, LED_GPIO_PIN);
}

void ledBlue()
{
  ledInit();
  GPIO_ResetBits(LED_GPIO, LED_GPIO_PIN);
}
#elif defined(PCBX10)
void ledOff()
{
  GPIO_ResetBits(LED_GPIO, LED_GPIO_PIN);
}

void ledRed()
{
  ledOff();
  GPIO_SetBits(LED_GPIO, LED_RED_GPIO_PIN);
}

void ledGreen()
{
  ledOff();
  GPIO_SetBits(LED_GPIO, LED_GREEN_GPIO_PIN);
}

void ledBlue()
{
  ledOff();
  GPIO_SetBits(LED_GPIO, LED_BLUE_GPIO_PIN);
}
#endif
