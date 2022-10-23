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

#include "stm32_hal_adc.h"
#include "opentx.h"

#define ADC_COMMON ((ADC_Common_TypeDef *) ADC_BASE)

// STM32 uses a 25K+25K voltage divider bridge to measure the battery voltage
// Measuring VBAT puts considerable drain (22 µA) on the battery instead of
// normal drain (~10 nA)
void enableVBatBridge()
{
  // Set internal measurement path for vbat sensor
  LL_ADC_SetCommonPathInternalCh(ADC_COMMON, LL_ADC_PATH_INTERNAL_VBAT);
}

void disableVBatBridge()
{
  // Set internal measurement path to none
  LL_ADC_SetCommonPathInternalCh(ADC_COMMON, LL_ADC_PATH_INTERNAL_NONE);
}

bool isVBatBridgeEnabled()
{
  return LL_ADC_GetCommonPathInternalCh(ADC_COMMON) == LL_ADC_PATH_INTERNAL_VBAT;
}

static bool adc_disable_dma(DMA_TypeDef* DMAx, uint32_t stream);
static void adc_dma_clear_flags(DMA_TypeDef* DMAx, uint32_t stream);

static void adc_init_pins(const stm32_adc_gpio_t* GPIOs, uint8_t n_GPIO)
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  
  pinInit.Mode = LL_GPIO_MODE_ANALOG;
  pinInit.Pull = LL_GPIO_PULL_NO;

  const stm32_adc_gpio_t* gpio = GPIOs;
  while (n_GPIO > 0) {

    pinInit.Pin = 0;

    for (uint8_t pin_idx = 0; pin_idx < gpio->n_pins; pin_idx++) {

      uint32_t pin = gpio->pins[pin_idx];
      uint32_t mode = LL_GPIO_GetPinMode(gpio->GPIOx, pin);

      // Output or AF: pin is probably used somewhere else
      if (mode != LL_GPIO_MODE_INPUT && mode != LL_GPIO_MODE_ANALOG) continue;
      
      pinInit.Pin |= pin;
    }

    LL_GPIO_Init(gpio->GPIOx, &pinInit);
    gpio++; n_GPIO--;
  }
}

static const uint32_t _seq_length_lookup[] = {
  LL_ADC_REG_SEQ_SCAN_DISABLE,
  LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_4RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_5RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_6RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_7RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_8RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_9RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_10RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_11RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_12RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_13RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_14RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_15RANKS,
  LL_ADC_REG_SEQ_SCAN_ENABLE_16RANKS,
};

static void adc_setup_scan_mode(ADC_TypeDef* ADCx, uint8_t nconv)
{
  // ADC must be disabled for the functions used here
  LL_ADC_Disable(ADCx);

  LL_ADC_InitTypeDef adcInit;
  LL_ADC_StructInit(&adcInit);

  if (nconv > 1) {
    adcInit.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
  }

  adcInit.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;  
  LL_ADC_Init(ADCx, &adcInit);

  LL_ADC_REG_InitTypeDef adcRegInit;
  LL_ADC_REG_StructInit(&adcRegInit);

  adcRegInit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;

  if (nconv > 1) {
    adcRegInit.SequencerLength = _seq_length_lookup[nconv - 1];
    adcRegInit.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    adcRegInit.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
  }

  LL_ADC_REG_Init(ADCx, &adcRegInit);

  // Enable ADC
  // TODO: check ADC_CR2_DDS (normally only for circular DMA
  //       but the old was using it (no clue why)
  LL_ADC_Enable(ADCx);  
}

static const uint32_t _rank_lookup[] = {
  LL_ADC_REG_RANK_1,
  LL_ADC_REG_RANK_2,
  LL_ADC_REG_RANK_3,
  LL_ADC_REG_RANK_4,
  LL_ADC_REG_RANK_5,
  LL_ADC_REG_RANK_6,
  LL_ADC_REG_RANK_7,
  LL_ADC_REG_RANK_8,
  LL_ADC_REG_RANK_9,
  LL_ADC_REG_RANK_10,
  LL_ADC_REG_RANK_11,
  LL_ADC_REG_RANK_12,
  LL_ADC_REG_RANK_13,
  LL_ADC_REG_RANK_14,
  LL_ADC_REG_RANK_15,
  LL_ADC_REG_RANK_16,
};

static uint8_t adc_init_channels(const stm32_adc_t* adc,
                                 const stm32_adc_input_t* inputs,
                                 const uint8_t* chan,
                                 uint8_t nconv)
{
  if (!chan || !nconv) return 0;

  uint8_t rank = 0;
  while (nconv > 0) {

    const stm32_adc_input_t* input = &inputs[*chan];

    // TODO: save some bitmask with used channels
    uint32_t mode = LL_GPIO_GetPinMode(input->GPIOx, input->GPIO_Pin);
    if (mode != LL_GPIO_MODE_ANALOG) {
      // skip channel
      nconv--; chan++;
      continue;
    }

    LL_ADC_REG_SetSequencerRanks(adc->ADCx, _rank_lookup[rank],
                                 input->ADC_Channel);

    LL_ADC_SetChannelSamplingTime(adc->ADCx, input->ADC_Channel,
                                  adc->sample_time);
    nconv--;
    rank++;
    chan++;
  }

  return rank;
}

static bool adc_init_dma_stream(ADC_TypeDef* adc, DMA_TypeDef* DMAx,
                                uint32_t stream, uint32_t channel,
                                uint16_t* dest, uint8_t nconv)
{
  // Disable DMA before continuing (see ref. manual "Stream configuration procedure")
  if (!adc_disable_dma(DMAx, stream))
      return false;

  // Clear Interrupt flags
  adc_dma_clear_flags(DMAx, stream);

  // setup DMA request
  LL_DMA_ConfigAddresses(DMAx, stream, CONVERT_PTR_UINT(&adc->DR),
                         CONVERT_PTR_UINT(dest),
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetDataLength(DMAx, stream, nconv);
  LL_DMA_SetChannelSelection(DMAx, stream, channel);

  // Very high priority, 1 byte transfers, increment memory
  LL_DMA_ConfigTransfer(DMAx, stream,
                        LL_DMA_PRIORITY_VERYHIGH | LL_DMA_MDATAALIGN_HALFWORD |
                        LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MEMORY_INCREMENT |
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  // disable direct mode, half full FIFO
  LL_DMA_EnableFifoMode(DMAx, stream);
  LL_DMA_SetFIFOThreshold(DMAx, stream, LL_DMA_FIFOTHRESHOLD_1_2);

  return true;
}

// Max 16 channels per ADC
static uint16_t _adc_dma_buffer[16] __DMA;

bool stm32_hal_adc_init(const stm32_adc_t* ADCs, uint8_t n_ADC,
                        const stm32_adc_input_t* inputs,
                        const stm32_adc_gpio_t* ADC_GPIOs, uint8_t n_GPIO)
{
  adc_init_pins(ADC_GPIOs, n_GPIO);

  // Init common to all ADCs
  LL_ADC_CommonInitTypeDef commonInit;
  LL_ADC_CommonStructInit(&commonInit);

  commonInit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
  LL_ADC_CommonInit(ADC_COMMON, &commonInit);

  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    // TODO: compute this thing based on initialised ports
    uint8_t nconv = adc->n_channels;
    if (nconv > 0) {

      // configure each channel
      const uint8_t* chan = adc->channels;
      nconv = adc_init_channels(adc, inputs, chan, nconv);
      adc_setup_scan_mode(adc->ADCx, nconv);

      if (adc->DMAx) {
        if (!adc_init_dma_stream(adc->ADCx, adc->DMAx, adc->DMA_Stream,
                                 adc->DMA_Channel, _adc_dma_buffer, nconv))
          return false;
      }
    }

    // move to next ADC definition
    adc++; n_ADC--;
  }

  //TODO: move VBat & PWM sticks somewhere else.

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmInit();
  }
#endif

  return true;
}

#define DMA_Stream0_IT_MASK     (uint32_t)(DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 | \
                                           DMA_LISR_TEIF0 | DMA_LISR_HTIF0 | \
                                           DMA_LISR_TCIF0)
  
#define DMA_Stream4_IT_MASK     (uint32_t)(DMA_HISR_FEIF4 | DMA_HISR_DMEIF4 | \
                                           DMA_HISR_TEIF4 | DMA_HISR_HTIF4 | \
                                           DMA_HISR_TCIF4)

static void adc_dma_clear_flags(DMA_TypeDef* DMAx, uint32_t stream)
{
  // no other choice, sorry for that...
  if (stream == LL_DMA_STREAM_4) {
    /* Reset interrupt pending bits for DMA2 Stream4 */
    DMAx->HIFCR = DMA_Stream4_IT_MASK;

  } else if (stream == LL_DMA_STREAM_0) {
    /* Reset interrupt pending bits for DMA2 Stream0 */
    DMAx->LIFCR = DMA_Stream0_IT_MASK;
  }
}

static bool adc_start_dma_conversion(ADC_TypeDef* ADCx, DMA_TypeDef* DMAx, uint32_t stream)
{
  // Disable DMA before continuing (see ref. manual "Stream configuration procedure")
  if (!adc_disable_dma(DMAx, stream))
      return false;

  // Clear Interrupt flags
  adc_dma_clear_flags(DMAx, stream);

  // Clear ADC status register
  ADCx->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);

  // Enable DMA
  LL_DMA_EnableStream(DMAx, stream);

  // Trigger ADC start
  ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;  
  return true;
}

static bool adc_disable_dma(DMA_TypeDef* DMAx, uint32_t stream)
{
  LL_DMA_DisableStream(DMAx, stream);

  // wait until DMA EN bit gets cleared by hardware
  uint16_t uiTimeout = 1000;
  while (LL_DMA_IsEnabledStream(DMAx, stream)) {
    if (--uiTimeout == 0) {
      // Timeout. Failed to disable DMA
      return false;
    }
  }
  return true;
}

static void adc_start_single_conversion(ADC_TypeDef* ADCx)
{
  ADCx->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;  
}

bool stm32_hal_adc_start_read(const stm32_adc_t* ADCs, uint8_t n_ADC)
{
  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {
    uint8_t nconv = adc->n_channels;
    if (nconv > 0) {
      if (adc->DMAx) {
        if (!adc_start_dma_conversion(adc->ADCx, adc->DMAx, adc->DMA_Stream))
          return false;
      } else if (nconv == 1) {
        adc_start_single_conversion(adc->ADCx);
      }
    }
    // move to next ADC definition
    adc++; n_ADC--;
  }
  return true;
}

static void copy_adc_values(uint16_t* dst, uint16_t* src,
                            const stm32_adc_t* adc,
                            const stm32_adc_input_t* inputs)
{
  for (uint8_t i=0; i < adc->n_channels; i++) {
    uint8_t channel = adc->channels[i];
    const stm32_adc_input_t* input = &inputs[channel];

    uint32_t pin_mode = LL_GPIO_GetPinMode(input->GPIOx, input->GPIO_Pin);
    if (pin_mode != LL_GPIO_MODE_ANALOG) {
      continue;
    }

    if (input->inverted)
      dst[channel] = ADCMAXVALUE - *src;
    else
      dst[channel] = *src;

    src++;
  }
}

void stm32_hal_adc_wait_completion(const stm32_adc_t* ADCs, uint8_t n_ADC,
                                   const stm32_adc_input_t* inputs)
{
  //TODO:
  // - replace with IRQ trigger (both)
  // - move RTC batt reading somewhere else

  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    switch(adc->DMA_Stream){

    case LL_DMA_STREAM_0:
      // for (unsigned int i=0; i<10000; i++) {
      //   if (!LL_DMA_IsActiveFlag_TC0(adc->DMAx)) break;
      // }
      while(!LL_DMA_IsActiveFlag_TC0(adc->DMAx));
      break;

    case LL_DMA_STREAM_4:
      // for (unsigned int i=0; i<10000; i++) {
      //   if (!LL_DMA_IsActiveFlag_TC4(adc->DMAx)) break;
      // }
      while(!LL_DMA_IsActiveFlag_TC4(adc->DMAx));
      break;
    }

    adc_disable_dma(adc->DMAx, adc->DMA_Stream);
    copy_adc_values(adcValues, _adc_dma_buffer, adc, inputs);
    
    // move to next ADC definition
    adc++; n_ADC--;
  }  

  // TODO: this hack needs to go away...
  // #if defined(ADC_EXT) && !defined(ADC_EXT_DMA_Stream)
  if (isVBatBridgeEnabled()) {
  //     rtcBatteryVoltage = ADC_EXT->DR;
     disableVBatBridge();
  }
  // #endif

  // TODO: copy internal DMA buffer into analog value array
  //       while mapping the channels back in order.
}
