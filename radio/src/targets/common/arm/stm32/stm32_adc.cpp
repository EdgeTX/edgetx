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

#include "stm32_adc.h"
#include "stm32_gpio_driver.h"
#include "opentx.h"

#define ADC_COMMON     ((ADC_Common_TypeDef *) ADC_BASE)
#define MAX_ADC_INPUTS 32
#define OVERSAMPLING   4

// Please note that we use the same prio for DMA TC and ADC IRQs
// to avoid issues with preemption between these 2
#define ADC_IRQ_PRIO   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

// Max 32 inputs supported
static uint32_t _adc_input_mask;
static volatile uint32_t _adc_inhibit_mask;

// DMA buffers
static uint16_t _adc_dma_buffer[MAX_ADC_INPUTS] __DMA;

// ADCs started
static uint8_t _adc_started_mask;
static volatile uint8_t _adc_completed;

static const stm32_adc_t* _adc_ADCs;
static uint8_t _adc_n_ADC;
static const stm32_adc_input_t* _adc_inputs;
static uint8_t _adc_n_inputs;

// Need for oversampling and decimation
static uint8_t _adc_run;
static uint8_t _adc_oversampling_disabled;
static uint16_t _adc_oversampling[MAX_ADC_INPUTS];

// STM32 uses a 25K+25K voltage divider bridge to measure the battery voltage
// Measuring VBAT puts considerable drain (22 µA) on the battery instead of
// normal drain (~10 nA)
void enableVBatBridge()
{
  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) < 1) return;

  // Set internal measurement path for vbat sensor
  LL_ADC_SetCommonPathInternalCh(ADC_COMMON, LL_ADC_PATH_INTERNAL_VBAT);

  auto channel = adcGetInputOffset(ADC_INPUT_RTC_BAT);
  _adc_inhibit_mask &= ~(1 << channel);
}

void disableVBatBridge()
{
  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) < 1) return;

  auto channel = adcGetInputOffset(ADC_INPUT_RTC_BAT);
  _adc_inhibit_mask |= (1 << channel);

  // Set internal measurement path to none
  LL_ADC_SetCommonPathInternalCh(ADC_COMMON, LL_ADC_PATH_INTERNAL_NONE);
}

bool isVBatBridgeEnabled()
{
  // && !(_adc_inhibit_mask & (1 << channel));
  return LL_ADC_GetCommonPathInternalCh(ADC_COMMON) == LL_ADC_PATH_INTERNAL_VBAT;
}

static void adc_enable_clock(ADC_TypeDef* ADCx)
{
  uint32_t adc_idx = (((uint32_t) ADCx) - ADC1_BASE) / 0x100UL;
  uint32_t adc_msk = RCC_APB2ENR_ADC1EN << adc_idx;
  LL_APB2_GRP1_EnableClock(adc_msk);
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

    stm32_gpio_enable_clock(gpio->GPIOx);
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
  } else {
    adcInit.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  }

  adcInit.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;  
  LL_ADC_Init(ADCx, &adcInit);

  LL_ADC_REG_InitTypeDef adcRegInit;
  LL_ADC_REG_StructInit(&adcRegInit);

  adcRegInit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  adcRegInit.ContinuousMode = LL_ADC_REG_CONV_SINGLE;

  if (nconv > 1) {
    adcRegInit.SequencerLength = _seq_length_lookup[nconv - 1];
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
  uint32_t channel_mask = 0;
  
  while (nconv > 0) {

    uint8_t input_idx = *chan;
    const stm32_adc_input_t* input = &inputs[input_idx];

    // internal channel don't have a GPIO + pin defined
    uint32_t mask = (1 << (ADC_CHANNEL_ID_MASK & input->ADC_Channel));
    if (!__LL_ADC_IS_CHANNEL_INTERNAL(input->ADC_Channel)) {
      uint32_t mode = LL_GPIO_GetPinMode(input->GPIOx, input->GPIO_Pin);
      if (mode != LL_GPIO_MODE_ANALOG) {
        // skip channel
        nconv--; chan++;
        continue;
      }
    } else {
      // Internal channels are inhibited until explicitely enabled
      _adc_inhibit_mask |= mask;
    }

    // channel is already used, probably a secondary input
    // using the same ADC channel
    if (channel_mask & mask) {
      nconv--; chan++;
      continue;
    }

    // update mask for used channels
    channel_mask |= mask;

    // update mask for valid inputs
    _adc_input_mask |= (1 << input_idx);
    
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

bool stm32_hal_adc_init(const stm32_adc_t* ADCs, uint8_t n_ADC,
                        const stm32_adc_input_t* inputs,
                        const stm32_adc_gpio_t* ADC_GPIOs, uint8_t n_GPIO)
{
  _adc_input_mask = 0;
  _adc_inhibit_mask = 0;
  _adc_oversampling_disabled = 0;

  adc_init_pins(ADC_GPIOs, n_GPIO);

  // Init common to all ADCs
  LL_ADC_CommonInitTypeDef commonInit;
  LL_ADC_CommonStructInit(&commonInit);

  commonInit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
  LL_ADC_CommonInit(ADC_COMMON, &commonInit);

  _adc_input_mask = 0;
  // uint16_t* dma_buffer = _adc_dma_buffer;
  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    uint8_t nconv = adc->n_channels;
    if (nconv > 0) {

      // enable periph clock
      adc_enable_clock(adc->ADCx);
  
      // configure each channel
      const uint8_t* chan = adc->channels;
      nconv = adc_init_channels(adc, inputs, chan, nconv);
      adc_setup_scan_mode(adc->ADCx, nconv);

      if (nconv > 1) {
        if (adc->DMAx) {
          uint16_t* dma_buffer = _adc_dma_buffer + adc->offset;
          if (!adc_init_dma_stream(adc->ADCx, adc->DMAx, adc->DMA_Stream,
                                   adc->DMA_Channel, dma_buffer, nconv))
            return false;

          NVIC_SetPriority(adc->DMA_Stream_IRQn, ADC_IRQ_PRIO);
          NVIC_EnableIRQ(adc->DMA_Stream_IRQn);
        } else {
          // multiple channels on the same ADC
          // without a DMA is NOT supported
          return false;
        }
      } else {
        // single conversion
        NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIO);
        NVIC_EnableIRQ(ADC_IRQn);
      }
    }

    // move to next ADC definition
    adc++; n_ADC--;
  }

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
    WRITE_REG(DMAx->HIFCR, DMA_Stream4_IT_MASK);

  } else if (stream == LL_DMA_STREAM_0) {
    /* Reset interrupt pending bits for DMA2 Stream0 */
    WRITE_REG(DMAx->LIFCR, DMA_Stream0_IT_MASK);
  }
}

static inline DMA_Stream_TypeDef* _dma_get_stream(DMA_TypeDef *DMAx, uint32_t Stream)
{
  return ((DMA_Stream_TypeDef*)((uint32_t)((uint32_t)DMAx + STREAM_OFFSET_TAB[Stream])));
}

static void adc_start_dma_conversion(ADC_TypeDef* ADCx, DMA_TypeDef* DMAx, uint32_t stream)
{
  // Clear Interrupt flags
  adc_dma_clear_flags(DMAx, stream);

  // Clear ADC status register & disable ADC IRQ
  CLEAR_BIT(ADCx->SR, ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  CLEAR_BIT(ADCx->CR1, ADC_CR1_EOCIE);

  // Enable DMA
  auto dma_stream = _dma_get_stream(DMAx, stream);
  SET_BIT(dma_stream->CR, DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
  SET_BIT(dma_stream->CR, DMA_SxCR_EN);

  // Trigger ADC start
  SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
}

static bool adc_disable_dma(DMA_TypeDef* DMAx, uint32_t stream)
{
  LL_DMA_DisableStream(DMAx, stream);

  // wait until DMA EN bit gets cleared by hardware
  uint16_t timeout = 1000;
  while (LL_DMA_IsEnabledStream(DMAx, stream)) {
    if (--timeout == 0) {
      // Timeout. Failed to disable DMA
      return false;
    }
  }
  return true;
}

static void adc_start_normal_conversion(ADC_TypeDef* ADCx)
{
  // clear status flags
  CLEAR_BIT(ADCx->SR, ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);

  // enble ADC IRQ
  SET_BIT(ADCx->CR1, ADC_CR1_EOCIE);

  // and start!
  SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
}

static void adc_start_read(const stm32_adc_t* ADCs, uint8_t n_ADC)
{
  // Start all ADCs in parallel

  uint8_t adc_mask = 1;
  _adc_started_mask = 0;

  const stm32_adc_t* adc = ADCs;
  while (n_ADC > 0) {

    // if the ADC has no active channels,
    // it has not been enabled at all
    auto ADCx = adc->ADCx;
    if (!LL_ADC_IsEnabled(ADCx)) {
      adc++; n_ADC--; adc_mask <<= 1;
      continue;
    }

    // more than one channel enabled on this ADC
    if (LL_ADC_GetSequencersScanMode(ADCx) == LL_ADC_SEQ_SCAN_ENABLE) {

      // Disable DMA before continuing (see ref. manual "Stream configuration procedure")
      auto DMAx = adc->DMAx;
      auto stream = adc->DMA_Stream;
      if (DMAx && adc_disable_dma(DMAx, stream)) {
        _adc_started_mask |= adc_mask;
        adc_start_dma_conversion(ADCx, DMAx, stream);
      }

    } else {
      // only one channel
      _adc_started_mask |= adc_mask;
      adc_start_normal_conversion(ADCx);
    }

    // move to next ADC
    adc++; n_ADC--; adc_mask <<= 1;
  }
}

bool stm32_hal_adc_start_read(const stm32_adc_t* ADCs, uint8_t n_ADC,
                              const stm32_adc_input_t* inputs, uint8_t n_inputs)
{
  _adc_completed = 0;
  _adc_run = 0;

  _adc_ADCs = ADCs;
  _adc_n_ADC = n_ADC;
  _adc_inputs = inputs;
  _adc_n_inputs = n_inputs;

  memclear(_adc_oversampling, sizeof(_adc_oversampling));
  adc_start_read(_adc_ADCs, _adc_n_ADC);

  return true;
}

static void copy_adc_values(uint16_t* src, const stm32_adc_t* adc,
                            const stm32_adc_input_t* inputs)
{
  for (uint8_t i=0; i < adc->n_channels; i++) {
    uint8_t channel = adc->channels[i];

    // if input disabled, skip
    if (~_adc_input_mask & (1 << channel)) {
      continue;
    }

    // skip sampled but inhibited channels
    if (_adc_inhibit_mask & (1 << channel)) {
      src++;
      continue;
    }

    // TODO: move inversion to the generic ADC driver?
    if (inputs[channel].inverted)
      _adc_oversampling[channel] += ADC_INVERT_VALUE(*src);
    else
      _adc_oversampling[channel] += *src;

#if defined(JITTER_MEASURE)
    if (JITTER_MEASURE_ACTIVE()) {
      rawJitter[channel].measure(dst[channel]);
    }
#endif

    src++;
  }
}

void stm32_hal_adc_wait_completion(const stm32_adc_t* ADCs, uint8_t n_ADC,
                                   const stm32_adc_input_t* inputs, uint8_t n_inputs)
{
  (void)ADCs;
  (void)n_ADC;
  (void)inputs;
  (void)n_inputs;

  while(!_adc_completed) {
    // busy wait
  }
}

void stm32_hal_adc_disable_oversampling()
{
  _adc_oversampling_disabled = 1;
}

static void _adc_mark_completed(const stm32_adc_t* adc)
{
  uint8_t adc_idx = (adc - _adc_ADCs);
  _adc_started_mask &= ~(1 << adc_idx);
}

static void _adc_chain_conversions(const stm32_adc_t* adc)
{
  _adc_mark_completed(adc);
  if (_adc_started_mask != 0) return;

  if (!_adc_oversampling_disabled && (++_adc_run < OVERSAMPLING)) {
    adc_start_read(_adc_ADCs, _adc_n_ADC);
    return;
  }

  auto adcValues = getAnalogValues();
  for (uint8_t i = 0; i < _adc_n_inputs; i++) {
    if (~_adc_input_mask & (1 << i)) continue;
    if (_adc_inhibit_mask & (1 << i)) continue;
    adcValues[i] = _adc_oversampling[i] / OVERSAMPLING;
  }

  // we're done!
  _adc_completed = 1;
}

void stm32_hal_adc_dma_isr(const stm32_adc_t* adc)
{
  // Disable IRQ
  auto dma_stream = _dma_get_stream(adc->DMAx, adc->DMA_Stream);
  CLEAR_BIT(dma_stream->CR, DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);

  uint16_t* dma_buffer = _adc_dma_buffer + adc->offset;
  copy_adc_values(dma_buffer, adc, _adc_inputs);

  _adc_chain_conversions(adc);
}

void stm32_hal_adc_isr(const stm32_adc_t* adc)
{
  // check if this ADC triggered the IRQ
  auto ADCx = adc->ADCx;
  if (!LL_ADC_IsActiveFlag_EOCS(ADCx) || !LL_ADC_IsEnabledIT_EOCS(ADCx))
    return;

  // Disable end-of-conversion IRQ
  CLEAR_BIT(ADCx->CR1, ADC_CR1_EOCIE);

  uint16_t* dma_buffer = _adc_dma_buffer + adc->offset;
  *dma_buffer = adc->ADCx->DR;
  copy_adc_values(dma_buffer, adc, _adc_inputs);

  _adc_chain_conversions(adc);
}
