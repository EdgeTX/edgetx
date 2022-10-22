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

static bool adc_disable_dma(DMA_TypeDef* DMAx, uint32_t stream);
static void adc_dma_clear_flags(DMA_TypeDef* DMAx, uint32_t stream);

static void adc_init_pins(const stm32_adc_gpio_t* GPIOs, uint8_t n_GPIO)
{
  (void)GPIOs;
  (void)n_GPIO;
  
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  // TODO: replace with a loop over _ADC_GPIOs
  // TODO: check if pin is already initialised
#if defined(ADC_GPIOA_PINS)
#if defined(RADIO_FAMILY_T16) || defined(PCBNV14)
  if (globalData.flyskygimbals)
  {
      GPIO_InitStructure.GPIO_Pin = ADC_GPIOA_PINS_FS;
  }
  else
#endif
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

  ADC_InitStructure.ADC_ScanConvMode = ENABLE; // Sets ADC_CR1_SCAN
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Clears ADC_CR2_CONT
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // Software trigger
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = nconv; // Channel count
  
  ADC_Init(ADCx, &ADC_InitStructure);

  // enable ADC_MAIN
  ADC_Cmd(ADCx, ENABLE);                            // ADC_CR2_ADON
  // enable DMA for ADC
  ADC_DMACmd(ADCx, ENABLE);                         // ADC_CR2_DMA
  ADC_DMARequestAfterLastTransferCmd(ADCx, ENABLE); // ADC_CR2_DDS
}

static void adc_init_channels(const stm32_adc_t* adc,
                              const uint8_t* chan,
{
  if (!chan || !nconv) return;

  uint8_t rank = 1;
  while (nconv > 0) {
    ADC_RegularChannelConfig(adc->ADCx, *chan, rank, adc->sample_time);
    nconv--; rank++; chan++;
  }
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

  // Very high priority, 1 byte transfers, increment memory
  LL_DMA_SetChannelSelection(DMAx, stream, channel);

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

bool stm32_hal_adc_init(const stm32_adc_t* ADCs,
                        const stm32_adc_gpio_t* ADC_GPIOs, uint8_t n_ADC)
{
  adc_init_pins(ADC_GPIOs, n_ADC);

  // Init common to all ADCs
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  ADC_CommonStructInit(&ADC_CommonInitStruct);
  ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInit(&ADC_CommonInitStruct);

  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    // TODO: compute this thing based on initialised ports
    uint8_t nconv = adc->n_channels;
    if (nconv > 0) {
      adc_setup_scan_mode(adc->ADCx, nconv);

      // configure each channel
      const uint8_t* chan = adc->channels;
      adc_init_channels(adc, chan, nconv);

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

  // Enable vbat sensor
  ADC_VBATCmd(ENABLE);

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

void stm32_hal_adc_wait_completion(const stm32_adc_t* ADCs, uint8_t n_ADC)
{
  //TODO:
  // - replace with IRQ trigger (both)
  // - move RTC batt reading somewhere else

  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    switch(adc->DMA_Stream){

    case LL_DMA_STREAM_0:
      for (unsigned int i=0; i<10000; i++) {
        if (!LL_DMA_IsActiveFlag_TC0(adc->DMAx)) break;
      }
      break;

    case LL_DMA_STREAM_4:
      for (unsigned int i=0; i<10000; i++) {
        if (!LL_DMA_IsActiveFlag_TC4(adc->DMAx)) break;
      }
      break;
    }

    adc_disable_dma(adc->DMAx, adc->DMA_Stream);
    
    // move to next ADC definition
    adc++; n_ADC--;
  }  

  // TODO: this hack needs to go away...
#if defined(ADC_EXT) && !defined(ADC_EXT_DMA_Stream)
  if (isVBatBridgeEnabled()) {
    rtcBatteryVoltage = ADC_EXT->DR;
    disableVBatBridge();
  }
#endif
}
