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

#include "hal.h"
#include "hal/adc_driver.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "keys.h"

#include "stm32_keys.inc"

// G11P key/trim decoding.
//
// The plain GPIO keys (K6A, K7A, W-KEY) are mapped directly in hal.h and read by
// the generated _read_keys() below. Only the multi-voltage lines need custom
// decoding:
//
//   - K1/K2 share K12_ADC (RAW1) through a resistor ladder
//   - K3/K4 share K34_ADC (RAW2) through a resistor ladder
//   - the trims are on the TRx_ADC lines (RAW3..RAW6), see hal.h for the indices
//
// TODO(hardware): the logical key mapping and the ladder thresholds must be
// validated against the real hardware.

// Analog input indices (see the input order in hal.h)
#define G11P_ADC_K12   3
#define G11P_ADC_K34   4
#define G11P_ADC_TR1   5
#define G11P_ADC_TR2   6
#define G11P_ADC_TR3   7
#define G11P_ADC_TR4   8

// TODO(hardware): ladder thresholds (12-bit)
#define G11P_KEY_LEVEL_LOW   1365
#define G11P_KEY_LEVEL_HIGH  2730

#define G11P_TRIM_LOW        1500
#define G11P_TRIM_HIGH       3500

void keysInit()
{
  _init_keys();
}

uint32_t readKeys()
{
  uint32_t result = _read_keys();

#if !defined(BOOT)
  // K1/K2 on K12_ADC
  uint16_t k12 = getAnalogValue(G11P_ADC_K12);
  if (k12 >= G11P_KEY_LEVEL_HIGH)       result |= 1 << KEY_PAGEUP; // (TODO)
  else if (k12 >= G11P_KEY_LEVEL_LOW)   result |= 1 << KEY_PAGEDN; // (TODO)

  // K3/K4 on K34_ADC
  uint16_t k34 = getAnalogValue(G11P_ADC_K34);
  if (k34 >= G11P_KEY_LEVEL_HIGH)       result |= 1 << KEY_UP;     // (TODO)
  else if (k34 >= G11P_KEY_LEVEL_LOW)   result |= 1 << KEY_DOWN;   // (TODO)
#endif

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

#if !defined(BOOT)
  // TODO(hardware): confirm the trim axes and voltages.
  uint16_t t1 = getAnalogValue(G11P_ADC_TR1);
  uint16_t t2 = getAnalogValue(G11P_ADC_TR2);
  uint16_t t3 = getAnalogValue(G11P_ADC_TR3);
  uint16_t t4 = getAnalogValue(G11P_ADC_TR4);

  if (t1 < G11P_TRIM_LOW)       result |= 1 << 0; // T1 dec
  else if (t1 < G11P_TRIM_HIGH) result |= 1 << 1; // T1 inc
  if (t2 < G11P_TRIM_LOW)       result |= 1 << 2; // T2 dec
  else if (t2 < G11P_TRIM_HIGH) result |= 1 << 3; // T2 inc
  if (t3 < G11P_TRIM_LOW)       result |= 1 << 4; // T3 dec
  else if (t3 < G11P_TRIM_HIGH) result |= 1 << 5; // T3 inc
  if (t4 < G11P_TRIM_LOW)       result |= 1 << 6; // T4 dec
  else if (t4 < G11P_TRIM_HIGH) result |= 1 << 7; // T4 inc
#endif

  return result;
}
