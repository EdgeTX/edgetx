/*
 * Copyright (C) EdgeTx
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

#include <stdint.h>
#include "hal/rgbleds.h"

bool usbChargerLed() { return true; }
void ledRed() {}
void ledGreen() {}
void ledBlue() {}
void ledOff() {}
void fsLedOn(uint8_t) {}
void fsLedOff(uint8_t) {}
void fsLedRGB(uint8_t, uint32_t) {}
bool fsLedState(uint8_t) { return false;}
void rgbSetLedColor(unsigned char, unsigned char, unsigned char, unsigned char) {}
void rgbLedColorApply() {}

uint8_t getRGBColorIndex(uint32_t color)
{
  for (uint8_t i = 0; i < (sizeof(colorTable) / sizeof(colorTable[0])); i++) {
    if (color == colorTable[i])
      return(i);
  }
  return 5; // Custom value set with Companion
}

// RGB
#define WS2812_BYTES_PER_LED 3

// Maximum number of supported LEDs
#if !defined(WS2812_MAX_LEDS)
#  define WS2812_MAX_LEDS 48
#endif

// Pixel values
static uint8_t _led_colors[WS2812_BYTES_PER_LED * WS2812_MAX_LEDS];

uint32_t rgbGetLedColor(uint8_t led)
{
  uint8_t* pixel = &_led_colors[led * WS2812_BYTES_PER_LED];
  return  (pixel[1] << 16) +  (pixel[0] << 8) + pixel[2];
}