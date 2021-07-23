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
#include "hal/adc_driver.h"

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

static void adc_init_pins()
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
}

static void adc_setup_scan_mode(ADC_TypeDef* ADCx, uint8_t nconv)
{
  ADC_InitTypeDef ADC_InitStructure;
  ADC_StructInit(&ADC_InitStructure);

  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = nconv;
  
  ADC_Init(ADCx, &ADC_InitStructure);

  // enable ADC_MAIN
  ADC_Cmd(ADCx, ENABLE);                            // ADC_CR2_ADON
  // enable DMA for ADC
  ADC_DMACmd(ADCx, ENABLE);                         // ADC_CR2_DMA
  ADC_DMARequestAfterLastTransferCmd(ADCx, ENABLE); // ADC_CR2_DDS
}

etx_hal_adc_channel ADC_MAIN_channels[] = {
    {ADC_CHANNEL_STICK_LH, ADC_SAMPTIME},
    {ADC_CHANNEL_STICK_LV, ADC_SAMPTIME},
    {ADC_CHANNEL_STICK_RV, ADC_SAMPTIME},
    {ADC_CHANNEL_STICK_RH, ADC_SAMPTIME},
#if defined(PCBX10)
    { ADC_CHANNEL_POT1,    ADC_SAMPTIME },
    { ADC_CHANNEL_POT2,    ADC_SAMPTIME },
    { ADC_CHANNEL_POT3,    ADC_SAMPTIME },
    { ADC_CHANNEL_EXT1,    ADC_SAMPTIME },
    { ADC_CHANNEL_EXT2,    ADC_SAMPTIME },
    { ADC_CHANNEL_SLIDER1, ADC_SAMPTIME },
    { ADC_CHANNEL_SLIDER2, ADC_SAMPTIME },
    { ADC_CHANNEL_BATT,    ADC_SAMPTIME }
#else
#if defined(RADIO_T8) || defined(RADIO_TLITE)
    // fake channels to fill unsused POT1/POT2
    {0, 0},
    {0, 0},
#elif defined(PCBX7) || defined(PCBXLITE)
    {ADC_CHANNEL_POT1, ADC_SAMPTIME},
    {ADC_CHANNEL_POT2, ADC_SAMPTIME},
    {ADC_CHANNEL_BATT, ADC_SAMPTIME},
    {ADC_Channel_Vbat, ADC_SAMPTIME}
#elif defined(PCBX9LITE)
    {ADC_CHANNEL_POT1, ADC_SAMPTIME},
    {ADC_CHANNEL_BATT, ADC_SAMPTIME},
    {ADC_Channel_Vbat, ADC_SAMPTIME},
#elif defined(PCBX9E)
    {ADC_CHANNEL_POT2, ADC_SAMPTIME},
    {ADC_CHANNEL_POT3, ADC_SAMPTIME},
    {ADC_CHANNEL_POT4, ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER3, ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER4, ADC_SAMPTIME},
#elif defined(PCBX9D) || defined(PCBX9DP)
    {ADC_CHANNEL_POT1,    ADC_SAMPTIME},
    {ADC_CHANNEL_POT2,    ADC_SAMPTIME},
    {ADC_CHANNEL_POT3,    ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER1, ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER2, ADC_SAMPTIME},
#endif
    {ADC_CHANNEL_BATT, ADC_SAMPTIME},
    {ADC_Channel_Vbat, ADC_SAMPTIME}
#endif
};

static const etx_hal_adc_channel* ADC_MAIN_get_channels()
{
#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED())
    return ADC_MAIN_channels + 4;
#endif
  return ADC_MAIN_channels;
}

static uint8_t ADC_MAIN_get_nconv()
{
  return NUM_ANALOGS_ADC; // based on STICKS_PWM_ENABLED()
}

#if defined(ADC_EXT)
#if defined(PCBX9E)

static const etx_hal_adc_channel ADC_EXT_channels[] = {
    {ADC_CHANNEL_POT1, ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER1, ADC_SAMPTIME},
    {ADC_CHANNEL_SLIDER2, ADC_SAMPTIME}
};

static uint8_t ADC_EXT_get_nconv() { return NUM_ANALOGS_ADC_EXT; }

#elif defined(PCBX10)

static const etx_hal_adc_channel ADC_EXT_channels[] = {
  { ADC_Channel_Vbat, 1, ADC_SAMPTIME }
};

static uint8_t ADC_EXT_get_nconv() { return 1; }

#endif
#endif

etx_hal_adc ADC_hal_def[] = {
  { ADC_MAIN, ADC_MAIN_get_nconv, ADC_MAIN_get_channels },
#if defined(ADC_EXT)
  { ADC_EXT,  ADC_EXT_get_nconv, ADC_EXT_get_channels },
#endif
  { nullptr,  nullptr, nullptr },
};


static void adc_init_channels(const etx_hal_adc* adc_def)
{
  while (adc_def->adc) {
    uint8_t nconv = 0;
    if (adc_def->get_nconv) nconv = adc_def->get_nconv();

    const etx_hal_adc_channel* chan = nullptr;
    if ((nconv > 0) && adc_def->get_channels)
      chan = adc_def->get_channels();

    if (chan) {
      uint8_t rank = 1;
      while (nconv > 0) {
        ADC_RegularChannelConfig(adc_def->adc, chan->adc_channel, rank,
                                 chan->sample_time);
        nconv--; rank++;
        chan++;
      }
    }
    adc_def++;
  }
}

static void adc_init_dma_channel(ADC_TypeDef* ADCx, DMA_Stream_TypeDef * dma_stream,
                                 uint16_t* dest, uint8_t nconv)
{
  // setup DMA request
  dma_stream->CR = DMA_SxCR_PL | ADC_DMA_SxCR_CHSEL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  dma_stream->PAR = CONVERT_PTR_UINT(&ADCx->DR);
  dma_stream->M0AR = CONVERT_PTR_UINT(dest);
  dma_stream->NDTR = nconv;
  dma_stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;
}


void adcInit()
{
  adc_init_pins();

  // Init common to all ADCs
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  ADC_CommonStructInit(&ADC_CommonInitStruct);
  ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInit(&ADC_CommonInitStruct);

  
  adc_setup_scan_mode(ADC_MAIN, NUM_ANALOGS_ADC);
#if defined(PCBX10) // X10 uses ADC3 for "normal" things
  adc_setup_scan_mode(ADC_EXT, 1);
#endif
#if defined(PCBX9E)
  adc_setup_scan_mode(ADC_EXT, NUM_ANALOGS_ADC_EXT);
#endif

  // configure each channel
  adc_init_channels(ADC_hal_def);

  // Enable vbat sensor
  ADC_VBATCmd(ENABLE);

  adc_init_dma_channel(ADC_MAIN, ADC_DMA_Stream, &adcValues[FIRST_ANALOG_ADC], NUM_ANALOGS_ADC);
#if defined(PCBX9E)
  adc_init_dma_channel(ADC_EXT, ADC_EXT_DMA_Stream, adcValues + NUM_ANALOGS_ADC, NUM_ANALOGS_ADC_EXT);
#endif

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmInit();
  }
#endif
}

#define DMA_Stream0_IT_MASK     (uint32_t)(DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 | \
                                           DMA_LISR_TEIF0 | DMA_LISR_HTIF0 | \
                                           DMA_LISR_TCIF0)
  
#define DMA_Stream4_IT_MASK     (uint32_t)(DMA_HISR_FEIF4 | DMA_HISR_DMEIF4 | \
                                           DMA_HISR_TEIF4 | DMA_HISR_HTIF4 | \
                                           DMA_HISR_TCIF4)

static void adc_dma_clear_flags(DMA_Stream_TypeDef * dma_stream)
{
  // no other choice, sorry for that...
  if (dma_stream == DMA2_Stream4) {
    /* Reset interrupt pending bits for DMA2 Stream4 */
    DMA2->HIFCR = DMA_Stream4_IT_MASK;

  } else if (dma_stream == DMA2_Stream0) {
    /* Reset interrupt pending bits for DMA2 Stream0 */
    DMA2->LIFCR = DMA_Stream0_IT_MASK;
  }
}

static void adc_start_dma_conversion(ADC_TypeDef* ADCx,
                                     DMA_Stream_TypeDef * dma_stream)
{
  dma_stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADCx->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  adc_dma_clear_flags(dma_stream);

  dma_stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;  
}

static void adc_disable_dma(DMA_Stream_TypeDef * dma_stream)
{
  dma_stream->CR &= ~DMA_SxCR_EN; // Disable DMA stream
}

static void adc_start_single_conversion(ADC_TypeDef* ADCx)
{
  ADCx->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;  
}

void adcSingleRead()
{
  adc_start_dma_conversion(ADC_MAIN, ADC_DMA_Stream);

#if defined(ADC_EXT)
#if defined(ADC_EXT_DMA_Stream)
  adc_start_dma_conversion(ADC_EXT, ADC_EXT_DMA_Stream);
#else
  // start a single conversion on ADC1 (VBat only)
  adc_start_single_conversion(ADC_EXT);
#endif
#endif

  //TODO: replace with IRQ trigger (both)
#if defined(ADC_EXT) && defined(ADC_EXT_DMA_Stream)
  // Wait for all ADCs to complete
  for (unsigned int i=0; i<10000; i++) {
    if (ADC_TRANSFER_COMPLETE() && ADC_EXT_TRANSFER_COMPLETE()) {
      break;
    }
  }
  adc_disable_dma(ADC_DMA_Stream);
  adc_disable_dma(ADC_EXT_DMA_Stream);
#else
  // Wait only for the main ADC, and hope others are done as well
  for (unsigned int i = 0; i < 10000; i++) {
    if (ADC_TRANSFER_COMPLETE()) {
      break;
    }
  }
  adc_disable_dma(ADC_DMA_Stream);

#if defined(ADC_EXT)
  if (isVBatBridgeEnabled()) {
    rtcBatteryVoltage = ADC_EXT->DR;
  }
#endif
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
