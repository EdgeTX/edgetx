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

#if !defined(BOOT)
  #include "hal/adc_driver.h"
#endif

#define BOOTLOADER_KEYS                 0x42

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
  
  pinInit.Pin = ADC_GPIO_PIN_EXT1;
  pinInit.Mode = LL_GPIO_MODE_ANALOG;
  pinInit.Pull = LL_GPIO_PULL_NO;
  stm32_gpio_enable_clock(ADC_GPIO_EXT1);
  LL_GPIO_Init(ADC_GPIO_EXT1, &pinInit);

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
  LL_ADC_REG_SetSequencerRanks(ADC_MAIN, LL_ADC_REG_RANK_1, ADC_CHANNEL_EXT1);
  LL_ADC_SetChannelSamplingTime(ADC_MAIN, ADC_CHANNEL_EXT1, LL_ADC_SAMPLINGTIME_3CYCLES);

  // Start ADC conversion
  LL_ADC_REG_StartConversionSWStart(ADC_MAIN);

  // Wait until ADC conversion is complete
  uint32_t timeout = 0;
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
  if (value >= 3584)
    result |= 1 << KEY_EXIT;
  else if (value < 512)
    result |= 1 << KEY_ENTER;
#else
  uint16_t value = getAnalogValue(4);
  if (value >= 3584)
    result |= 1 << KEY_EXIT;
  else if (value < 512)
    result |= 1 << KEY_ENTER;
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
  uint16_t tr1Val = getAnalogValue(6);
  uint16_t tr2Val = getAnalogValue(7);
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