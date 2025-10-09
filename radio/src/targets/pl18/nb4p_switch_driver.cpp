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

/*
 * FlySky NB4+ Switch Driver
 *
 * Hardware mapping:
 * - SW1: ADC_RAW1 (PC.01) - ADC1_IN11 - getAnalogValue(4)
 * - SW2: ADC_RAW2 (PC.00) - ADC1_IN10 - getAnalogValue(5) (multiplexed)
 * - SW3: ADC_RAW2 (PC.00) - ADC1_IN10 - getAnalogValue(5) (multiplexed)
 * - SW4: Not available in current hardware revision
 *
 * ADC Values (12-bit: 0-4095):
 * - Switch UP (released): ~4095 (pulled high)
 * - Switch DOWN (pressed): ~0 (pulled low)
 * - Multiplexed switches use voltage divider for detection
 */

#include "hal/switch_driver.h"
#include "stm32_switch_driver.h"
//#include "stm32_gpio_driver.h"

#include "definitions.h"
#include "edgetx_constants.h"
#include "myeeprom.h"
#include "hal/adc_driver.h"

#include <stdlib.h>

// NB4P switch mapping based on hardware ADC channels:
// - SW1: ADC_RAW1 (channel 4) - PC.01
// - SW2, SW3: ADC_RAW2 (channel 5) - PC.00 (multiplexed)
static const char _switch_names[][4] = {"SW1", "SW2", "SW3", "SW4"};

// ADC thresholds for reliable switch detection (12-bit ADC: 0-4095 range)
// Improved thresholds with hysteresis to prevent false triggers
#define SW_ADC_LOW_THRESHOLD     400   // Below this = switch active (was 512)
#define SW_ADC_MID_LOW           1400  // Mid-low range for multiplexed switches
#define SW_ADC_MID_HIGH          2650  // Mid-high range for multiplexed switches
#define SW_ADC_HIGH_THRESHOLD    3500  // Above this = switch active (was 3584)

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  // SW1 is on ADC channel 4 (RAW1)
  if (idx == 0) {
    uint16_t swVal = getAnalogValue(4);
    return (swVal < SW_ADC_LOW_THRESHOLD) ? SWITCH_HW_DOWN : SWITCH_HW_UP;
  }

  // SW2, SW3 are multiplexed on ADC channel 5 (RAW2)
  else if (idx <= 2) {
    uint16_t swVal = getAnalogValue(5);

    if (idx == 1) { // SW2
      // SW2 active when ADC in low range or mid-low range
      return (swVal < SW_ADC_LOW_THRESHOLD ||
              (swVal >= SW_ADC_MID_LOW && swVal < SW_ADC_MID_HIGH)) ?
              SWITCH_HW_DOWN : SWITCH_HW_UP;
    }
    else { // SW3 (idx == 2)
      // SW3 active when ADC in high range or mid-high range
      return (swVal > SW_ADC_HIGH_THRESHOLD ||
              (swVal >= SW_ADC_MID_LOW && swVal < SW_ADC_MID_HIGH)) ?
              SWITCH_HW_DOWN : SWITCH_HW_UP;
    }
  }

  // SW4 - Future expansion (currently not implemented in hardware)
  else if (idx == 3) {
    // For future hardware revision that might add SW4
    return SWITCH_HW_UP; // Default to UP position
  }

  return SWITCH_HW_UP;
}

const char* boardSwitchGetName(uint8_t idx)
{
  return _switch_names[idx];
}
