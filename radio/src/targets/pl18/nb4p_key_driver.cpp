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
 * FlySky NB4+ Key and Trim Driver
 *
 * Hardware Configuration:
 * - Navigation Keys: SW1 on ADC_RAW1 (PC.01/ADC1_IN11) - channel 4
 * - Trim 1 (ST): ADC_RAW3 (PA.06/ADC1_IN6) - channel 6
 * - Trim 2 (TH): ADC_RAW4 (PC.04/ADC1_IN14) - channel 7
 *
 * Key Mapping:
 * - KEY_ENTER: Low ADC value (~0V when pressed)
 * - KEY_EXIT:  High ADC value (~3.3V when pressed)
 *
 * Trim Mapping (navigation via trim switches):
 * - STD/STU: Up/Down navigation (trim 1)
 * - THD/THU: Left/Right navigation (trim 2)
 *
 * This driver provides enhanced precision and reliability improvements
 * over the original implementation.
 */

#include "hal/key_driver.h"

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "keys.h"

#if !defined(BOOT)
  #include "hal/adc_driver.h"
#endif

#define BOOTLOADER_KEYS                 0x42

// Enhanced ADC thresholds for NB4+ (12-bit ADC: 0-4095 range)
// Optimized for better precision and noise immunity
#define KEY_ADC_LOW_THRESHOLD           350   // ENTER key threshold (improved from 512)
#define KEY_ADC_HIGH_THRESHOLD         3700   // EXIT key threshold (improved from 3584)
#define KEY_ADC_HYSTERESIS              50    // Hysteresis to prevent bouncing

#define TRIM_ADC_LOW_THRESHOLD          350   // Trim active low threshold
#define TRIM_ADC_MID_LOW               1300   // Trim mid-low boundary
#define TRIM_ADC_MID_HIGH              2700   // Trim mid-high boundary
#define TRIM_ADC_HIGH_THRESHOLD        3700   // Trim active high threshold
#define TRIM_ADC_HYSTERESIS             100   // Larger hysteresis for trims

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
/*    TR2L = 4,
    TR2R,
    TR2D = 8,
    TR2U,*/
};

void keysInit()
{
#if defined(BOOT)
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = ADC_GPIO_PIN_RAW1;
  pinInit.Mode = LL_GPIO_MODE_ANALOG;
  pinInit.Pull = LL_GPIO_PULL_NO;
  stm32_gpio_enable_clock(ADC_GPIO_RAW1);
  LL_GPIO_Init(ADC_GPIO_RAW1, &pinInit);

  // Init ADC clock
  uint32_t adc_idx = (((uint32_t) ADC_MAIN) - ADC1_BASE) / 0x100UL;
  uint32_t adc_msk = RCC_APB2ENR_ADC1EN << adc_idx;
  LL_APB2_GRP1_EnableClock(adc_msk);

  // Init common to all ADCs
  LL_ADC_CommonInitTypeDef commonInit;
  LL_ADC_CommonStructInit(&commonInit);

  commonInit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC_MAIN), &commonInit);

  // ADC must be disabled for the functions used here
  LL_ADC_Disable(ADC_MAIN);

  LL_ADC_InitTypeDef adcInit;
  LL_ADC_StructInit(&adcInit);
  adcInit.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  adcInit.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  adcInit.Resolution = LL_ADC_RESOLUTION_12B;
  LL_ADC_Init(ADC_MAIN, &adcInit);

  LL_ADC_REG_InitTypeDef adcRegInit;
  LL_ADC_REG_StructInit(&adcRegInit);
  adcRegInit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  adcRegInit.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  LL_ADC_REG_Init(ADC_MAIN, &adcRegInit);

  // Enable ADC
  LL_ADC_Enable(ADC_MAIN);
#endif
}

#if defined(BOOT)
uint16_t _adcRead()
{
  // Configure ADC channel
  LL_ADC_REG_SetSequencerRanks(ADC_MAIN, LL_ADC_REG_RANK_1, ADC_CHANNEL_RAW1);
  LL_ADC_SetChannelSamplingTime(ADC_MAIN, ADC_CHANNEL_RAW1, LL_ADC_SAMPLINGTIME_3CYCLES);

  // Start ADC conversion
  LL_ADC_REG_StartConversionSWStart(ADC_MAIN);

  // Wait until ADC conversion is complete
  while (!LL_ADC_IsActiveFlag_EOCS(ADC_MAIN));

  // Read ADC converted value
  return LL_ADC_REG_ReadConversionData12(ADC_MAIN);
}
#endif

uint32_t readKeys()
{
  uint32_t result = 0;

#if defined(BOOT)
  uint16_t value = _adcRead();
  // Use enhanced thresholds for more reliable key detection in bootloader
  if (value >= KEY_ADC_HIGH_THRESHOLD)
    result |= 1 << KEY_EXIT;
  else if (value <= KEY_ADC_LOW_THRESHOLD)
    result |= 1 << KEY_ENTER;
#else
  uint16_t value = getAnalogValue(4);

  // Enhanced key detection with hysteresis for debouncing
  static uint8_t lastKeyState = 0; // 0=none, 1=enter, 2=exit

  if (value <= KEY_ADC_LOW_THRESHOLD) {
    result |= 1 << KEY_ENTER;
    lastKeyState = 1;
  }
  else if (value >= KEY_ADC_HIGH_THRESHOLD) {
    result |= 1 << KEY_EXIT;
    lastKeyState = 2;
  }
  else {
    // In neutral zone - apply hysteresis based on last state
    if (lastKeyState == 1 && value <= (KEY_ADC_LOW_THRESHOLD + KEY_ADC_HYSTERESIS)) {
      result |= 1 << KEY_ENTER; // Maintain ENTER if close to threshold
    }
    else if (lastKeyState == 2 && value >= (KEY_ADC_HIGH_THRESHOLD - KEY_ADC_HYSTERESIS)) {
      result |= 1 << KEY_EXIT;  // Maintain EXIT if close to threshold
    }
    else {
      lastKeyState = 0; // Reset state when clearly in neutral zone
    }
  }
#endif

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

#if defined(BOOT)
  uint16_t value = _adcRead();
  if (value >= 1536 && value < 2560)
    result = BOOTLOADER_KEYS;
#else
  uint16_t tr1Val = getAnalogValue(6);  // Trim 1 (ST) - Up/Down navigation
  uint16_t tr2Val = getAnalogValue(7);  // Trim 2 (TH) - Left/Right navigation

  // Enhanced trim detection with hysteresis and dead zones
  static uint8_t lastTrim1State = 0; // 0=neutral, 1=down, 2=up
  static uint8_t lastTrim2State = 0; // 0=neutral, 1=left, 2=right

  // Trim 1 (ST) processing - Up/Down navigation
  if (tr1Val <= TRIM_ADC_LOW_THRESHOLD) {
    // Left physical position -> not used for navigation
    lastTrim1State = 0;
  }
  else if (tr1Val >= TRIM_ADC_LOW_THRESHOLD + TRIM_ADC_HYSTERESIS &&
           tr1Val <= TRIM_ADC_MID_LOW) {
    result |= 1 << STD;  // Navigation DOWN
    lastTrim1State = 1;
  }
  else if (tr1Val >= TRIM_ADC_MID_HIGH &&
           tr1Val <= TRIM_ADC_HIGH_THRESHOLD - TRIM_ADC_HYSTERESIS) {
    result |= 1 << STU;  // Navigation UP
    lastTrim1State = 2;
  }
  else if (tr1Val >= TRIM_ADC_HIGH_THRESHOLD) {
    // Right physical position -> not used for navigation
    lastTrim1State = 0;
  }
  else {
    // In dead zone - apply hysteresis based on last state
    if (lastTrim1State == 1 && tr1Val <= (TRIM_ADC_MID_LOW + TRIM_ADC_HYSTERESIS)) {
      result |= 1 << STD;  // Maintain DOWN if close to threshold
    }
    else if (lastTrim1State == 2 && tr1Val >= (TRIM_ADC_MID_HIGH - TRIM_ADC_HYSTERESIS)) {
      result |= 1 << STU;  // Maintain UP if close to threshold
    }
    else {
      lastTrim1State = 0;  // Reset to neutral
    }
  }

  // Trim 2 (TH) processing - Left/Right navigation
  if (tr2Val <= TRIM_ADC_LOW_THRESHOLD) {
    // Left physical position -> not used for navigation
    lastTrim2State = 0;
  }
  else if (tr2Val >= TRIM_ADC_LOW_THRESHOLD + TRIM_ADC_HYSTERESIS &&
           tr2Val <= TRIM_ADC_MID_LOW) {
    result |= 1 << THD;  // Navigation LEFT
    lastTrim2State = 1;
  }
  else if (tr2Val >= TRIM_ADC_MID_HIGH &&
           tr2Val <= TRIM_ADC_HIGH_THRESHOLD - TRIM_ADC_HYSTERESIS) {
    result |= 1 << THU;  // Navigation RIGHT
    lastTrim2State = 2;
  }
  else if (tr2Val >= TRIM_ADC_HIGH_THRESHOLD) {
    // Right physical position -> not used for navigation
    lastTrim2State = 0;
  }
  else {
    // In dead zone - apply hysteresis based on last state
    if (lastTrim2State == 1 && tr2Val <= (TRIM_ADC_MID_LOW + TRIM_ADC_HYSTERESIS)) {
      result |= 1 << THD;  // Maintain LEFT if close to threshold
    }
    else if (lastTrim2State == 2 && tr2Val >= (TRIM_ADC_MID_HIGH - TRIM_ADC_HYSTERESIS)) {
      result |= 1 << THU;  // Maintain RIGHT if close to threshold
    }
    else {
      lastTrim2State = 0;  // Reset to neutral
    }
  }
#endif

  return result;
}
