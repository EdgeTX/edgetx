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
#include "hal/gpio.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio.h"
#include "stm32_gpio_driver.h"
#include "delays_driver.h"
#include "keys.h"

// G11P key/trim decoding.
//
// The keys K1/K2 and K3/K4 share the analog lines K12_ADC (RAW1, index 3) and
// K34_ADC (RAW2, index 4) through resistor ladders, and the trims are on the
// TRx_ADC lines (see hal_g11p.h for the input index map).
//
// TODO(hardware): the logical key mapping and the ladder thresholds must be
// validated against the real hardware.

// Analog input indices (see hal_g11p.h ADC input order)
#define G11P_ADC_K12   3
#define G11P_ADC_K34   4
#define G11P_ADC_TR    8
#define G11P_ADC_TR1   9
#define G11P_ADC_TR2   10
#define G11P_ADC_TR3   11
#define G11P_ADC_TR4   12

// TODO(hardware): ladder thresholds (12-bit)
#define G11P_KEY_LEVEL_LOW   1365
#define G11P_KEY_LEVEL_HIGH  2730

#define G11P_TRIM_LOW        1500
#define G11P_TRIM_HIGH       3500

void keysInit()
{
  gpio_init(GPIO_PIN(GPIOC, 7), GPIO_IN_PU, GPIO_PIN_SPEED_LOW); // K6A
  gpio_init(GPIO_PIN(GPIOA, 8), GPIO_IN_PU, GPIO_PIN_SPEED_LOW); // K7A
  gpio_init(GPIO_PIN(GPIOG, 14), GPIO_IN_PU, GPIO_PIN_SPEED_LOW); // W-KEY
}

uint32_t readKeys()
{
  uint32_t result = 0;

  // Digital keys (active low)
  if (!gpio_read(GPIO_PIN(GPIOC, 7))) result |= 1 << KEY_MENU;   // K6A  (TODO)
  if (!gpio_read(GPIO_PIN(GPIOA, 8))) result |= 1 << KEY_EXIT;   // K7A  (TODO)
  if (!gpio_read(GPIO_PIN(GPIOG, 14))) result |= 1 << KEY_ENTER; // W-KEY

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
  // Physical order: T1(ST) dec/inc, T2(TH) dec/inc, T3, T4
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
