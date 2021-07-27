/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"

#if defined(SIMU)
  // not needed
#elif defined(RADIO_T16)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,  -1,1,  -1,1};
#elif defined(RADIO_T18)
  const int8_t adcDirection[NUM_ANALOGS] = { 1,-1,1,-1, -1,1,-1,  -1,1,  -1,1 };
#elif defined(RADIO_TX16S)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,  -1,1,  -1,1};
#elif defined(PCBX10)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,1,-1,   1,1,    1, -1};
#elif defined(PCBX9E)
#if defined(HORUS_STICKS)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,-1,-1,1, -1,1,1,  -1,-1,-1};
#else
  const int8_t adcDirection[NUM_ANALOGS] = {1,1,-1,-1,  -1,-1,-1,1, -1,1,1,  -1,-1,-1};
#endif
  const uint8_t adcMapping[NUM_ANALOGS] = { 0 /*STICK1*/, 1 /*STICK2*/, 2 /*STICK3*/, 3 /*STICK4*/,
                                            11 /*POT1*/, 4 /*POT2*/, 5 /*POT3*/, 6 /*POT4*/,
                                            12 /*SLIDER1*/, 13 /*SLIDER2*/, 7 /*SLIDER3*/, 8 /*SLIDER4*/,
                                            9 /*TX_VOLTAGE*/, 10 /*TX_VBAT*/ };
#elif defined(PCBX9DP)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,-1,  1,1,  1,  1};
#elif defined(PCBX9D)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,0,   1,1,  1,  1};
#elif defined(RADIO_TX12)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  -1,-1,  1,  1};
#elif defined(PCBX7)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1,  1};
#elif defined(PCBX9LITE)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1};
#elif defined(PCBXLITE)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,-1,1,  -1,1,  1,  1};
#endif

#if NUM_PWMSTICKS > 0
  #define FIRST_ANALOG_ADC             (STICKS_PWM_ENABLED() ? NUM_PWMSTICKS : 0)
  #define NUM_ANALOGS_ADC              (STICKS_PWM_ENABLED() ? (NUM_ANALOGS - NUM_PWMSTICKS) : NUM_ANALOGS)
#elif defined(PCBX9E)
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              11
  #define NUM_ANALOGS_ADC_EXT          (NUM_ANALOGS - NUM_ANALOGS_ADC)
#else
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              NUM_ANALOGS
#endif

uint16_t adcValues[NUM_ANALOGS] __DMA;

#if defined(PCBX10)
uint16_t rtcBatteryVoltage;
#endif

void adcInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

#if defined(ADC_GPIOA_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOA_PINS;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOB_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOB_PINS;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOC_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOC_PINS;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOF_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOF_PINS;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
#endif

  ADC_MAIN->CR1 = ADC_CR1_SCAN;
  ADC_MAIN->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC_MAIN->SQR1 = (NUM_ANALOGS_ADC - 1) << 20; // bits 23:20 = number of conversions

#if defined(PCBX10)
  if (STICKS_PWM_ENABLED()) {
    ADC_MAIN->SQR2 = (ADC_CHANNEL_SLIDER2 << 0) + (ADC_CHANNEL_BATT << 5);
    ADC_MAIN->SQR3 = (ADC_CHANNEL_POT1 << 0) + (ADC_CHANNEL_POT2 << 5) + (ADC_CHANNEL_POT3 << 10) + (ADC_CHANNEL_EXT1 << 15) + (ADC_CHANNEL_EXT2 << 20) + (ADC_CHANNEL_SLIDER1 << 25);
  }
  else {
    ADC_MAIN->SQR2 = (ADC_CHANNEL_POT3 << 0) + (ADC_CHANNEL_EXT1 << 5) + (ADC_CHANNEL_EXT2 << 10) + (ADC_CHANNEL_SLIDER1 << 15) + (ADC_CHANNEL_SLIDER2 << 20) + (ADC_CHANNEL_BATT << 25);
    ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT1 << 20) + (ADC_CHANNEL_POT2 << 25);
  }
#elif defined(PCBX9E)
  ADC_MAIN->SQR2 = (ADC_CHANNEL_POT4 << 0) + (ADC_CHANNEL_SLIDER3 << 5) + (ADC_CHANNEL_SLIDER4 << 10) + (ADC_CHANNEL_BATT << 15) + (ADC_Channel_Vbat << 20);
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT2 << 20) + (ADC_CHANNEL_POT3 << 25);
#elif defined(PCBXLITE)
  if (STICKS_PWM_ENABLED()) {
    ADC_MAIN->SQR2 = 0;
    ADC_MAIN->SQR3 = (ADC_CHANNEL_POT1 << 0) + (ADC_CHANNEL_POT2 << 5) + (ADC_CHANNEL_BATT << 10) + (ADC_Channel_Vbat << 15);
  }
  else {
    ADC_MAIN->SQR2 = (ADC_CHANNEL_BATT << 0) + (ADC_Channel_Vbat << 5);
    ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT1 << 20) + (ADC_CHANNEL_POT2 << 25);
  }
#elif defined(RADIO_T8) || defined(RADIO_TLITE)
  ADC_MAIN->SQR2 = (ADC_CHANNEL_BATT << 0) + (ADC_Channel_Vbat << 5);
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15);
#elif defined(PCBX7)
  ADC_MAIN->SQR2 = (ADC_CHANNEL_BATT << 0) + (ADC_Channel_Vbat << 5);
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT1 << 20) + (ADC_CHANNEL_POT2 << 25);
#elif defined(PCBX9LITE)
  ADC_MAIN->SQR2 = (ADC_Channel_Vbat << 0);
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT1 << 20) + (ADC_CHANNEL_BATT << 25);
#elif defined(PCBX9D) || defined(PCBX9DP)
  ADC_MAIN->SQR2 = (ADC_CHANNEL_POT3 << 0) + (ADC_CHANNEL_SLIDER1 << 5) + (ADC_CHANNEL_SLIDER2 << 10) + (ADC_CHANNEL_BATT << 15) + (ADC_Channel_Vbat << 20);
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH << 0) + (ADC_CHANNEL_STICK_LV << 5) + (ADC_CHANNEL_STICK_RV << 10) + (ADC_CHANNEL_STICK_RH << 15) + (ADC_CHANNEL_POT1 << 20) + (ADC_CHANNEL_POT2 << 25);
#endif

  ADC_MAIN->SMPR1 = ADC_MAIN_SMPR1;
  ADC_MAIN->SMPR2 = ADC_MAIN_SMPR2;

  ADC->CCR = ADC_CCR_VBATE; // Enable vbat sensor

  ADC_DMA_Stream->CR = DMA_SxCR_PL | ADC_DMA_SxCR_CHSEL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC_MAIN->DR);
  ADC_DMA_Stream->M0AR = CONVERT_PTR_UINT(&adcValues[FIRST_ANALOG_ADC]);
  ADC_DMA_Stream->NDTR = NUM_ANALOGS_ADC;
  ADC_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

#if defined(PCBX10)
  ADC1->CR1 = ADC_CR1_SCAN;
  ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC1->SQR1 = (1 - 1) << 20;
  ADC1->SQR2 = 0;
  ADC1->SQR3 = (ADC_Channel_Vbat << 0);
  ADC1->SMPR1 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24);
  ADC1->SMPR2 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24) + (ADC_SAMPTIME << 27);
#endif

#if defined(PCBX9E)
  ADC_EXT->CR1 = ADC_CR1_SCAN;
  ADC_EXT->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC_EXT->SQR1 = (NUM_ANALOGS_ADC_EXT - 1) << 20;
  ADC_EXT->SQR2 = 0;
  ADC_EXT->SQR3 = (ADC_CHANNEL_POT1 << 0) + (ADC_CHANNEL_SLIDER1 << 5) + (ADC_CHANNEL_SLIDER2 << 10); // conversions 1 to 3
  ADC_EXT->SMPR1 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24);
  ADC_EXT->SMPR2 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24) + (ADC_SAMPTIME << 27);

  ADC_EXT_DMA_Stream->CR = DMA_SxCR_PL | DMA_SxCR_CHSEL_1 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC_EXT_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC_EXT->DR);
  ADC_EXT_DMA_Stream->M0AR = CONVERT_PTR_UINT(adcValues + NUM_ANALOGS_ADC);
  ADC_EXT_DMA_Stream->NDTR = NUM_ANALOGS_ADC_EXT;
  ADC_EXT_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;
#endif

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmInit();
  }
#endif
}

void adcSingleRead()
{
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC_MAIN->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC_SET_DMA_FLAGS();
  ADC_DMA_Stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADC_MAIN->CR2 |= (uint32_t)ADC_CR2_SWSTART;

#if defined(PCBX10)
  ADC1->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;
#endif

#if defined(PCBX9E)
  ADC_EXT_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC_EXT->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC_EXT_SET_DMA_FLAGS();
  ADC_EXT_DMA_Stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADC_EXT->CR2 |= (uint32_t)ADC_CR2_SWSTART;
#endif

#if defined(PCBX9E)
  for (unsigned int i=0; i<10000; i++) {
    if (ADC_TRANSFER_COMPLETE() && ADC_EXT_TRANSFER_COMPLETE()) {
      break;
    }
  }
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC_EXT_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
#else
  for (unsigned int i = 0; i < 10000; i++) {
    if (ADC_TRANSFER_COMPLETE()) {
      break;
    }
  }
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
#endif

#if defined(PCBX10)
  if (isVBatBridgeEnabled()) {
    rtcBatteryVoltage = ADC1->DR;
  }
#endif
}

void adcRead()
{
  uint16_t temp[NUM_ANALOGS] = { 0 };

  for (int i=0; i<4; i++) {
    adcSingleRead();
    for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
    adcValues[x] = temp[x] >> 2;
  }

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmRead(adcValues);
  }
#endif
}

#if defined(PCBX10)
uint16_t getRTCBatteryVoltage()
{
  return (rtcBatteryVoltage * 2 * ADC_VREF_PREC2) / 2048;
}
#else
uint16_t getRTCBatteryVoltage()
{
  return (getAnalogValue(TX_RTC_VOLTAGE) * ADC_VREF_PREC2) / 2048;
}
#endif

#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_SLIDER_AVAILABLE(index)) {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs,
    // which produces ghost readings on these inputs.
    return 0;
  }
#if defined(PCBX9E)
  index = adcMapping[index];
#endif
  if (adcDirection[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
