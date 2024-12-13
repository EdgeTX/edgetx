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
#include "boards/generic_stm32/rgb_leds.h"
#include "board.h"
#include "colors.h"

void ledInit()
{
#if defined(LED_GPIO)
  gpio_init(LED_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif
#if defined(LED_RED_GPIO)
  gpio_init(LED_RED_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif
#if defined(LED_GREEN_GPIO)
  gpio_init(LED_GREEN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif
#if defined(LED_BLUE_GPIO)
  gpio_init(LED_BLUE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
  gpio_init(FSLED_GPIO_1, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(FSLED_GPIO_2, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(FSLED_GPIO_3, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(FSLED_GPIO_4, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(FSLED_GPIO_5, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(FSLED_GPIO_6, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
// used to map switch number to led number in the rgbled chain
uint8_t ledMapping[] = {1, 2, 3, 4, 5, 6};

void fsLedOff(uint8_t index, uint32_t color)
{
   rgbSetLedColor(ledMapping[index], GET_RED(color), \
   GET_GREEN(color),GET_BLUE(color));
}

void fsLedOn(uint8_t index, uint32_t color)
{
   rgbSetLedColor(ledMapping[index], GET_RED(color), \
   GET_GREEN(color),GET_BLUE(color));
}

#elif defined(FUNCTION_SWITCHES)
gpio_t fsLeds[] = {FSLED_GPIO_1, FSLED_GPIO_2,
                   FSLED_GPIO_3, FSLED_GPIO_4,
                   FSLED_GPIO_5, FSLED_GPIO_6};

void fsLedOff(uint8_t index)
{
   gpio_clear(fsLeds[index]);
}

void fsLedOn(uint8_t index)
{
  gpio_set(fsLeds[index]);
}

bool fsLedState(uint8_t index)
{
  return (gpio_read(fsLeds[index]));
}
#endif

#if defined(LED_GPIO)

// Single GPIO for dual color LED
void ledOff()
{
  gpio_init(LED_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
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
