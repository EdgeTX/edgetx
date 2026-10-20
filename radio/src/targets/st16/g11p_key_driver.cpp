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

#if defined(BOOT)
  #include "board.h"
#endif

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

static uint32_t decodeKeys(uint16_t k12, uint16_t k34)
{
  uint32_t result = 0;

  // K1/K2 on K12_ADC
  if (k12 >= G11P_KEY_LEVEL_HIGH)       result |= 1 << KEY_PAGEUP; // (TODO)
  else if (k12 >= G11P_KEY_LEVEL_LOW)   result |= 1 << KEY_PAGEDN; // (TODO)

  // K3/K4 on K34_ADC
  if (k34 >= G11P_KEY_LEVEL_HIGH)       result |= 1 << KEY_UP;     // (TODO)
  else if (k34 >= G11P_KEY_LEVEL_LOW)   result |= 1 << KEY_DOWN;   // (TODO)

  return result;
}

#if defined(BOOT)
static void _adcInit()
{
  // K12_ADC = PB0, K34_ADC = PB1
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = ADC_GPIO_PIN_RAW1 | ADC_GPIO_PIN_RAW2;
  pinInit.Mode = LL_GPIO_MODE_ANALOG;
  pinInit.Pull = LL_GPIO_PULL_NO;
  stm32_gpio_enable_clock(GPIOB);
  LL_GPIO_Init(GPIOB, &pinInit);

  // ADC1/ADC2 clock is on AHB1 on H7
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_ADC12);

  // Init common to all ADCs
  LL_ADC_CommonInitTypeDef commonInit;
  LL_ADC_CommonStructInit(&commonInit);
  commonInit.CommonClock = LL_ADC_CLOCK_ASYNC_DIV4;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC_MAIN), &commonInit);

  // ADC must be disabled for the functions used here
  LL_ADC_Disable(ADC_MAIN);

  // H7: exit deep power down and enable the internal regulator
  if (LL_ADC_IsDeepPowerDownEnabled(ADC_MAIN) != 0UL) {
    LL_ADC_DisableDeepPowerDown(ADC_MAIN);
  }
  if (LL_ADC_IsInternalRegulatorEnabled(ADC_MAIN) == 0UL) {
    LL_ADC_EnableInternalRegulator(ADC_MAIN);

    uint32_t wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US / 10UL) *
                                ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
    while (wait_loop_index != 0UL) {
      wait_loop_index--;
    }
  }

  // H7: calibration is mandatory before use
  LL_ADC_StartCalibration(ADC_MAIN, LL_ADC_CALIB_OFFSET_LINEARITY,
                          LL_ADC_SINGLE_ENDED);
  while (LL_ADC_IsCalibrationOnGoing(ADC_MAIN) != 0UL);

  LL_ADC_InitTypeDef adcInit;
  LL_ADC_StructInit(&adcInit);
  adcInit.Resolution = LL_ADC_RESOLUTION_12B;
  LL_ADC_Init(ADC_MAIN, &adcInit);

  LL_ADC_REG_InitTypeDef adcRegInit;
  LL_ADC_REG_StructInit(&adcRegInit);
  adcRegInit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  adcRegInit.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  adcRegInit.SequencerLength = 1 << ADC_SQR1_L_Pos;
  LL_ADC_REG_Init(ADC_MAIN, &adcRegInit);

  LL_ADC_REG_SetSequencerRanks(ADC_MAIN, LL_ADC_REG_RANK_1, ADC_CHANNEL_RAW1);
  LL_ADC_SetChannelSamplingTime(ADC_MAIN, ADC_CHANNEL_RAW1, ADC_SAMPTIME);
  LL_ADC_SetChannelPreselection(ADC_MAIN, ADC_CHANNEL_RAW1);

  LL_ADC_REG_SetSequencerRanks(ADC_MAIN, LL_ADC_REG_RANK_2, ADC_CHANNEL_RAW2);
  LL_ADC_SetChannelSamplingTime(ADC_MAIN, ADC_CHANNEL_RAW2, ADC_SAMPTIME);
  LL_ADC_SetChannelPreselection(ADC_MAIN, ADC_CHANNEL_RAW2);

  LL_ADC_ClearFlag_ADRDY(ADC_MAIN);
  LL_ADC_Enable(ADC_MAIN);
  while (!LL_ADC_IsActiveFlag_ADRDY(ADC_MAIN)) {
    if (!LL_ADC_IsEnabled(ADC_MAIN)) LL_ADC_Enable(ADC_MAIN);
  }
}

static void _adcReadKeys(uint16_t &k12, uint16_t &k34)
{
  LL_ADC_ClearFlag_EOC(ADC_MAIN);
  LL_ADC_REG_StartConversion(ADC_MAIN);

  while (!LL_ADC_IsActiveFlag_EOC(ADC_MAIN));
  k12 = LL_ADC_REG_ReadConversionData12(ADC_MAIN);

  while (!LL_ADC_IsActiveFlag_EOC(ADC_MAIN));
  k34 = LL_ADC_REG_ReadConversionData12(ADC_MAIN);
}
#endif

void keysInit()
{
#if defined(BOOT)
  _adcInit();
#endif
  _init_keys();
}

uint32_t readKeys()
{
  uint32_t result = _read_keys();
  uint16_t k12, k34;

#if defined(BOOT)
  _adcReadKeys(k12, k34);
#else
  k12 = getAnalogValue(G11P_ADC_K12);
  k34 = getAnalogValue(G11P_ADC_K34);
#endif

  return result | decodeKeys(k12, k34);
}

uint32_t readTrims()
{
  uint32_t result = 0;

#if defined(BOOT)
  uint16_t k12, k34;
  _adcReadKeys(k12, k34);

  uint32_t keys = decodeKeys(k12, k34);
  if ((keys & (1 << KEY_PAGEUP)) && (keys & (1 << KEY_DOWN))) {
    result = BOOTLOADER_KEYS;
  }
#else
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
