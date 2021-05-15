/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"

void ledInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

#if defined(LED_GREEN_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_GREEN_GPIO_PIN;
  GPIO_Init(LED_GREEN_GPIO, &GPIO_InitStructure);
#endif

#if defined(LED_RED_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_RED_GPIO_PIN;
  GPIO_Init(LED_RED_GPIO, &GPIO_InitStructure);
#endif

#if defined(LED_BLUE_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_BLUE_GPIO_PIN;
  GPIO_Init(LED_BLUE_GPIO, &GPIO_InitStructure);
#endif
}

void ledOff()
{
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_OFF(LED_RED_GPIO, LED_RED_GPIO_PIN);
#endif
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_OFF(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
#endif
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_OFF(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
#endif
}

void ledRed()
{
  ledOff();
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_ON(LED_RED_GPIO, LED_RED_GPIO_PIN);
#endif
}

void ledGreen()
{
  ledOff();
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_ON(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
#endif
}

void ledBlue()
{
  ledOff();
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_ON(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
#endif
}
