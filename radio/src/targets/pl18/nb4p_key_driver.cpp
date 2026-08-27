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

#include "hal/key_driver.h"

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "keys.h"

#include "hal/adc_driver.h"

#if defined(BOOT)
  #include "stm32_adc.h"
  #include "boards/generic_stm32/analog_inputs.h"
#endif

#define BOOTLOADER_KEYS 0x42
#define ADC_KEYS_INDEX 4

/* The output bit-order has to be:
   0  LHL  STD (Left equals down)
   1  LHR  STU
   2  LVD  THD
   3  LVU  THU
*/

enum PhysicalTrims
{
    STD = 0,
    STU,
    THD = 2,
    THU,
/*
    TR2L = 4,
    TR2R,
    TR2D = 8,
    TR2U,
*/
};

void keysInit()
{
#if defined(BOOT)
  _adc_driver.init();
#endif
}

#if defined(BOOT)
static uint16_t _adc_values[MAX_ADC_INPUTS];

uint16_t* getAnalogValues() { return _adc_values; }

static uint16_t _adcRead(uint8_t index)
{
  if (!_adc_driver.start_conversion()) {
    return ADC_MAX_VALUE / 2;
  }
  _adc_driver.wait_completion();
  return _adc_values[4];
}
#endif

uint32_t readKeys()
{
  uint16_t value;
  uint32_t result = 0;

#if defined(BOOT)
  value = _adcRead(ADC_KEYS_INDEX);
#else
  value = getAnalogValue(ADC_KEYS_INDEX);
#endif

  if (value >= 3584)
    result |= 1 << KEY_EXIT;
  else if (value < 512)
    result |= 1 << KEY_ENTER;

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

#if defined(BOOT)
  uint16_t value = _adcRead(ADC_KEYS_INDEX);
  if (value >= 1536 && value < 2560)
    result = BOOTLOADER_KEYS;
#else
  uint16_t tr1Val = getAnalogValue(6); // TODO: define constant
  uint16_t tr2Val = getAnalogValue(7); // TODO: define constant
  if (tr1Val < 500)        // Physical TR1 Left
//    result |= 1 << TR1L;
    ;
  else if (tr1Val < 1500)  // Physical TR1 Up
    result |= 1 << STD;
  else if (tr1Val < 2500)  // Physical TR1 Right
//    result |= 1 << TR1R;
    ;
  else if (tr1Val < 3500)  // Physical TR1 Down
    result |= 1 << STU;
  if (tr2Val < 500)        // Physical TR2 Left
//    result |= 1 << TR2L;
    ;
  else if (tr2Val < 1500)  // Physical TR2 Up
    result |= 1 << THD;
  else if (tr2Val < 2500)  // Physical TR2 Right
//    result |= 1 << TR2R;
    ;
  else if (tr2Val < 3500)  // Physical TR2 Down
    result |= 1 << THU;
#endif

  return result;
}
