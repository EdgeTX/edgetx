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

#include "hal/gpio.h"
#include "stm32_gpio.h"

#include "board.h"

void ledInit()
{
#if defined(LED_GPIO)
  gpio_init(LED_GPIO, GPIO_OUT);
#endif
#if defined(LED_RED_GPIO)
  gpio_init(LED_RED_GPIO, GPIO_OUT);
#endif
#if defined(LED_GREEN_GPIO)
  gpio_init(LED_GREEN_GPIO, GPIO_OUT);
#endif
#if defined(LED_BLUE_GPIO)
  gpio_init(LED_BLUE_GPIO, GPIO_OUT);
#endif
}

#if defined(LED_GPIO)

// Single GPIO for dual color LED
void ledOff()
{
  gpio_init(LED_GPIO, GPIO_IN_PU);
}

void ledRed()
{
  ledInit();
  gpio_set(LED_GPIO);
}

void ledBlue()
{
  ledInit();
  gpio_clear(LED_GPIO);
}

#elif defined(LED_RED_GPIO) && defined(LED_GREEN_GPIO) && defined(LED_BLUE_GPIO)

void ledOff()
{
  gpio_clear(LED_RED_GPIO);
  gpio_clear(LED_GREEN_GPIO);
  gpio_clear(LED_BLUE_GPIO);
}

void ledRed()
{
  ledOff();
  gpio_set(LED_RED_GPIO);
}

void ledGreen()
{
  ledOff();
  gpio_set(LED_GREEN_GPIO);
}

void ledBlue()
{
  ledOff();
  gpio_set(LED_BLUE_GPIO);
}

#endif
