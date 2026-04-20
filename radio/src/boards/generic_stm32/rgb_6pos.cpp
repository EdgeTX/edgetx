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

#include "hal.h"

#if defined(SIXPOS_SWITCH_INDEX) && !defined(SIMU)

#include "hal/adc_driver.h"
#include "rgb_leds.h"

#include <stdint.h>

// 6POS switch position tracking.
//
// sixPosUpdateFromAdc() runs once per ADC conversion from the board
// adc_wait_completion callback. It reads the raw sample out of
// adcValues[SIXPOS_SWITCH_INDEX], maintains a sticky "last pressed"
// position (the switch reads ~0 when no button is held, so a plain
// threshold would bounce back to position 0), and writes the scaled
// 0..4096 sticky value back in place. Downstream consumers
// (getAnalogValue, diaganas, the mixer multipos source, …) then see a
// stable value without any of them driving the state machine.
//
// The LED update is NOT done here: _six_pos_state is consumed by
// rgbLedOnUpdate() below, which runs in the LED refresh timer task.
// That keeps the rgb_leds.cpp back buffer single-writer (the LED
// timer task) for the 6POS range, so Lua's applyRGBLedColors pattern
// cannot race with an ADC-context write.

static uint8_t _last_adc_state = 0;
static volatile uint8_t _six_pos_state = 0;

void sixPosUpdateFromAdc()
{
  uint16_t* values = getAnalogValues();
  uint16_t adcValue = values[SIXPOS_SWITCH_INDEX];

  uint8_t current = 0;
  if (adcValue > 3800) current = 6;
  else if (adcValue > 3100) current = 5;
  else if (adcValue > 2300) current = 4;
  else if (adcValue > 1500) current = 3;
  else if (adcValue > 1000) current = 2;
  else if (adcValue > 400)  current = 1;

  if (_last_adc_state != current) {
    _last_adc_state = current;
  } else if (_last_adc_state != 0) {
    _six_pos_state = _last_adc_state - 1;
  }

  values[SIXPOS_SWITCH_INDEX] = (4096 / 5) * _six_pos_state;
}

void rgbLedOnUpdate()
{
  static int8_t last_rendered = -1;

  uint8_t pos = _six_pos_state;
  if (pos == last_rendered) return;
  last_rendered = pos;

  for (uint8_t i = 0; i < 6; i++) {
    if (i == pos) {
      rgbSetLedColor(i, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
    } else {
      rgbSetLedColor(i, 0, 0, 0);
    }
  }
  rgbLedColorApply();
}

#endif  // SIXPOS_SWITCH_INDEX && !SIMU
