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
#if defined(LED_GPIO_PIN)
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(LED_GPIO, &GPIO_InitStructure);
#else
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
#endif

#if defined(FUNCTION_SWITCHES)
  RCC_AHB1PeriphClockCmd(FS_RCC_AHB1Periph, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_1;
  GPIO_Init(FSLED_GPIO_1, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_2;
  GPIO_Init(FSLED_GPIO_2, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_3;
  GPIO_Init(FSLED_GPIO_3, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_4;
  GPIO_Init(FSLED_GPIO_4, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_5;
  GPIO_Init(FSLED_GPIO_5, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_6;
  GPIO_Init(FSLED_GPIO_6, &GPIO_InitStructure);
#endif
}


#if defined(FUNCTION_SWITCHES)
constexpr uint32_t fsLeds[] = {FSLED_GPIO_PIN_1, FSLED_GPIO_PIN_2,
                               FSLED_GPIO_PIN_3, FSLED_GPIO_PIN_4,
                               FSLED_GPIO_PIN_5, FSLED_GPIO_PIN_6};

constexpr GPIO_TypeDef* fsLeds_gpio[] = {FSLED_GPIO_1, FSLED_GPIO_2,
                                         FSLED_GPIO_3, FSLED_GPIO_4,
                                         FSLED_GPIO_5, FSLED_GPIO_6};

void fsLedOff(uint8_t index)
{
  GPIO_FSLED_GPIO_OFF(fsLeds_gpio[index], fsLeds[index]);
}

void fsLedOn(uint8_t index)
{
  GPIO_FSLED_GPIO_ON(fsLeds_gpio[index], fsLeds[index]);
}

bool getFSLedState(uint8_t index)
{
  return (fsLeds_gpio[index]->ODR & fsLeds[index]);
}
#endif

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
#if defined(LED_RED_GPIO)
  GPIO_ResetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
#endif
#if defined(LED_BLUE_GPIO)
  GPIO_ResetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
#endif
#if defined(LED_GREEN_GPIO)
  GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
#endif
}

void ledRed()
{
  ledOff();
#if defined(LED_RED_GPIO)
  GPIO_SetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
#endif
}

void ledGreen()
{
  ledOff();
#if defined(LED_GREEN_GPIO)
  GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
#endif
}

void ledBlue()
{
  ledOff();
#if defined(LED_BLUE_GPIO)
  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
#endif
}
#endif
