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

#pragma once

#include <inttypes.h>
#include "hal.h"

// colours the status LED can show: discrete GPIOs, or the RGB strip
#if defined(STATUS_LED_RGB_STRIP) || defined(LED_RED_GPIO)
  #define STATUS_LED_HAS_RED 1
#else
  #define STATUS_LED_HAS_RED 0
#endif
#if defined(STATUS_LED_RGB_STRIP) || defined(LED_GREEN_GPIO)
  #define STATUS_LED_HAS_GREEN 1
#else
  #define STATUS_LED_HAS_GREEN 0
#endif
#if defined(STATUS_LED_RGB_STRIP) || defined(LED_BLUE_GPIO)
  #define STATUS_LED_HAS_BLUE 1
#else
  #define STATUS_LED_HAS_BLUE 0
#endif

// radios sharing a yaml map must agree on the settings: the H7RS dev kit has
// no status LED but uses the X10 map
#if STATUS_LEDS || defined(RADIO_H7RS)
  #define STATUS_LED_SETTINGS
#endif

// choosing a colour per phase needs at least two to choose from
#if STATUS_LEDS && \
    (STATUS_LED_HAS_RED + STATUS_LED_HAS_GREEN + STATUS_LED_HAS_BLUE) >= 2
  #define STATUS_LED_COLORS
#endif

#if defined(STATUS_LED_PWM)

#define STATUS_LED_BRIGHT_MAX 100
// UI detents: each one is a visibly different PWM level
#define STATUS_LED_BRIGHT_STEPS 10

// Applies to whichever status LED is lit. Never switches it off: 0 is the
// dimmest step, only ledOff() goes dark.
void ledSetBrightness(uint8_t bright);

#endif
