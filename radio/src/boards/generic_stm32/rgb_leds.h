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

#pragma once

#include <stdint.h>

void rgbLedInit();
// Initialise the WS2812 hardware without starting the periodic refresh
// timer. Useful for pre-OS contexts (e.g. charging UI) that need the LED
// strip up before the FreeRTOS scheduler is running.
void rgbLedHwInit();
void rgbLedStop();

void rgbSetLedColor(uint8_t led, uint8_t r, uint8_t g, uint8_t b);
uint32_t rgbGetLedColor(uint8_t led);

void rgbLedClearAll();

bool rgbGetState(uint8_t led);
void rgbLedColorApply();

// Weak hook invoked by the LED refresh timer task before publishing the
// next frame. Boards that drive system LEDs (e.g. the 6POS position
// indicator) implement this to update their LEDs in the timer task
// context, keeping rgb_leds.cpp's back buffer single-writer.
void rgbLedOnUpdate();

// Invoked by the board ADC wait_completion callback once per conversion
// on radios with a 6POS switch. Reads the raw sample from adcValues[],
// runs the sticky-position state machine, and writes the scaled sticky
// value back in place. Linked only when SIXPOS_SWITCH_INDEX is defined.
void sixPosUpdateFromAdc();
