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
#include "hal/rgbleds.h"
#include "stm32_gpio.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "board.h"
#if defined(LED_STRIP_GPIO)
#include "boards/generic_stm32/rgb_leds.h"
#endif

#define GET_RED(color) (((color) & 0xF80000) >>16)
#define GET_GREEN(color) (((color) & 0x000F800) >> 8)
#define GET_BLUE(color) (((color) & 0xF8))

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
static const uint32_t fsLeds[] = {FSLED_GPIO_PIN_1, FSLED_GPIO_PIN_2,
				  FSLED_GPIO_PIN_3, FSLED_GPIO_PIN_4,
				  FSLED_GPIO_PIN_5, FSLED_GPIO_PIN_6};
#endif

void ledInit()
{
#if defined(LED_GREEN_GPIO)
  gpio_init(LED_GREEN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_RED_GPIO)
  gpio_init(LED_RED_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_BLUE_GPIO)
  gpio_init(LED_BLUE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
  for (size_t i = 0; i < DIM(fsLeds); i++) {
    gpio_init(fsLeds[i], GPIO_OUT, GPIO_PIN_SPEED_LOW);
  }
#endif
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
// used to map switch number to led number in the rgbled chain
uint8_t ledMapping[] = {0, 1, 2, 3, 4, 5, 6};

void fsLedRGB(uint8_t index, uint32_t color)
{
   rgbSetLedColor(ledMapping[index], GET_RED(color), \
   GET_GREEN(color),GET_BLUE(color));
}

uint8_t getRGBColorIndex(uint32_t color)
{
  for (uint8_t i = 0; i < (sizeof(colorTable) / sizeof(colorTable[0])); i++) {
    if (color == colorTable[i])
      return(i);
  }
  return 5; // Custom value set with Companion
}
#elif defined(FUNCTION_SWITCHES)
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
  return gpio_read(fsLeds[index]) ? true : false;
}
#endif

void ledOff()
{
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_OFF(LED_RED_GPIO);
#endif
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_OFF(LED_BLUE_GPIO);
#endif
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_OFF(LED_GREEN_GPIO);
#endif
}

void ledRed()
{
  ledOff();
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_ON(LED_RED_GPIO);
#endif
}

void ledGreen()
{
  ledOff();
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_ON(LED_GREEN_GPIO);
#endif
}

void ledBlue()
{
  ledOff();
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_ON(LED_BLUE_GPIO);
#endif
}
