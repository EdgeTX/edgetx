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
#include "stm32_dma.h"
#include "stm32_gpio_driver.h"

#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "timers_driver.h"
#include "debug.h"

#include <string.h>
#include "FreeRTOSConfig.h"


#define OVERSAMPLING 4

#define SAMPLING_TIMEOUT_US 500

// Please note that we use the same prio for DMA TC and ADC IRQs
// to avoid issues with preemption between these 2
#define ADC_IRQ_PRIO   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#if defined(STM32H7) || defined(STM32H7RS)
#  define _ENABLE_EOCIE(ADCx) SET_BIT(ADCx->IER, ADC_IER_EOCIE)
#  define _DISABLE_EOCIE(ADCx) CLEAR_BIT(ADCx->IER, ADC_IER_EOCIE)
#  define _START_ADC_SINGLE(ADCx) SET_BIT(ADCx->CR, ADC_CR_ADSTART)
#  define _START_ADC_DMA(ADCx) _START_ADC_SINGLE(ADCx)
#  define _CLEAR_ADC_STATUS(ADCx) CLEAR_BIT(ADCx->ISR, ADC_ISR_EOC | ADC_ISR_OVR)
#else
#  define _ENABLE_EOCIE(ADCx) SET_BIT(ADCx->CR1, ADC_CR1_EOCIE)
#  define _DISABLE_EOCIE(ADCx) CLEAR_BIT(ADCx->CR1, ADC_CR1_EOCIE)
#  define _START_ADC_SINGLE(ADCx) SET_BIT(ADCx->CR2, ADC_CR2_SWSTART)
#  define _START_ADC_DMA(ADCx) SET_BIT(ADCx->CR2, ADC_CR2_SWSTART | ADC_CR2_DMA)
#  define _CLEAR_ADC_STATUS(ADCx) CLEAR_BIT(ADCx->SR, ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR)
#endif

// Max 32 inputs supported
static uint32_t _adc_input_mask;
static uint32_t _adc_input_inhibt_mask = 0;
static volatile uint32_t _adc_inhibit_mask;

// DMA buffers
static uint16_t _adc_dma_buffer[MAX_ADC_INPUTS] __DMA;

// ADCs started
static volatile uint8_t _adc_started_mask;
static volatile uint8_t _adc_completed;

static const stm32_adc_t* _adc_ADCs;
static uint8_t _adc_n_ADC;
static const stm32_adc_input_t* _adc_inputs;
static uint8_t _adc_n_inputs;

// Need for oversampling and decimation
static uint8_t _adc_run;
static uint8_t _adc_oversampling_disabled;
static uint16_t _adc_oversampling[MAX_ADC_INPUTS];

void stm32_hal_set_inputs_mask(uint32_t inputs)
{
  _adc_input_inhibt_mask |= inputs;
}

uint32_t stm32_hal_get_inputs_mask()
{
  return _adc_input_inhibt_mask;
}

// STM32 uses a 25K+25K voltage divider bridge to measure the battery voltage
// Measuring VBAT puts considerable drain (22 µA) on the battery instead of
// normal drain (~10 nA)
void enableVBatBridge()
{
  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) < 1) return;

  // Set internal measurement path for vbat sensor
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VBAT);

  auto channel = adcGetInputOffset(ADC_INPUT_RTC_BAT);
  _adc_inhibit_mask &= ~(1 << channel);
}

void disableVBatBridge()
{
  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) < 1) return;

  auto channel = adcGetInputOffset(ADC_INPUT_RTC_BAT);
  _adc_inhibit_mask |= (1 << channel);

  // Set internal measurement path to none
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);
}

bool isVBatBridgeEnabled()
{
  // && !(_adc_inhibit_mask & (1 << channel));
  return LL_ADC_GetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1)) == LL_ADC_PATH_INTERNAL_VBAT;
}

static void adc_enable_clock(ADC_TypeDef* ADCx)
{
#if defined(RCC_AHB1ENR_ADC12EN)
# if defined(ADC3)
  if (ADCx == ADC3) {
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_ADC3);
    return;
  }
# endif
  // ADC1 / ADC2
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_ADC12);
#elif defined(RCC_APB2ENR_ADC1EN)
  uint32_t adc_idx = (((uint32_t) ADCx) - ADC1_BASE) / 0x100UL;
  uint32_t adc_msk = RCC_APB2ENR_ADC1EN << adc_idx;
  LL_APB2_GRP1_EnableClock(adc_msk);
#else
# warning "Unknown ADC clock enable"
#endif
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


static void adc_setup_scan_mode(ADC_TypeDef* ADCx, uint8_t nconv)
{
  // ADC must be disabled for the functions used here

  LL_ADC_InitTypeDef adcInit;
  LL_ADC_StructInit(&adcInit);

  LL_ADC_REG_InitTypeDef adcRegInit;
  LL_ADC_REG_StructInit(&adcRegInit);

#if !defined(STM32H7) && !defined(STM32H7RS)
  if (nconv > 1) {
    adcInit.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
  } else {
    adcInit.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  }
#endif

#if !defined(STM32H7)
  adcInit.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
#endif


#if defined(STM32H7) || defined(STM32H7RS)
  /* - Exit from deep-power-down mode and ADC voltage regulator enable        */
  if (LL_ADC_IsDeepPowerDownEnabled(ADCx) != 0UL)
  {
    /* Disable ADC deep power down mode */
    LL_ADC_DisableDeepPowerDown(ADCx);

    /* System was in deep power down mode, calibration must
     be relaunched or a previously saved calibration factor
     re-applied once the ADC voltage regulator is enabled */
  }

  if (LL_ADC_IsInternalRegulatorEnabled(ADCx) == 0UL)
  {
    /* Enable ADC internal voltage regulator */
    LL_ADC_EnableInternalRegulator(ADCx);

    /* Note: Variable divided by 2 to compensate partially              */
    /*       CPU processing cycles, scaling in us split to not          */
    /*       exceed 32 bits register capacity and handle low frequency. */
    uint32_t wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US / 10UL) *
                                ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
    while (wait_loop_index != 0UL) {
      wait_loop_index--;
    }
  }
#endif

#if defined(LL_ADC_RESOLUTION_12B_OPT)
  adcInit.Resolution = LL_ADC_RESOLUTION_12B_OPT;
#else
  adcInit.Resolution = LL_ADC_RESOLUTION_12B;
#endif
  LL_ADC_Init(ADCx, &adcInit);

  adcRegInit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  adcRegInit.ContinuousMode = LL_ADC_REG_CONV_SINGLE;

  if (nconv > 1) {
    adcRegInit.SequencerLength = (nconv - 1) << ADC_SQR1_L_Pos;
#if defined(STM32H7)
    adcRegInit.DataTransferMode = LL_ADC_REG_DMA_TRANSFER_LIMITED;
#else
    adcRegInit.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
#endif
  }

  LL_ADC_REG_Init(ADCx, &adcRegInit);

  // Enable ADC
#if defined(STM32H7RS) || defined(STM32H7)
  LL_ADC_ClearFlag_ADRDY(ADCx);
  LL_ADC_Enable(ADCx);
  while(!LL_ADC_IsActiveFlag_ADRDY(ADCx)) {
    if (!LL_ADC_IsEnabled(ADCx)) LL_ADC_Enable(ADCx);
  }
#else
  LL_ADC_Enable(ADCx);  
#endif
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

    if (_adc_input_inhibt_mask & (1 << input_idx)) {
      // skip input
      nconv--; chan++;
      continue;      
    }
    
    // internal channel don't have a GPIO + pin defined
    uint32_t mask = 1 << __LL_ADC_CHANNEL_TO_DECIMAL_NB(input->ADC_Channel);
    if (!__LL_ADC_IS_CHANNEL_INTERNAL(input->ADC_Channel)) {
      uint32_t mode = LL_GPIO_GetPinMode(input->GPIOx, input->GPIO_Pin);
      if (mode != LL_GPIO_MODE_ANALOG) {
        // skip channel
        nconv--; chan++;
        continue;
      }
    } else {
      // Internal channels are inhibited until explicitely enabled
      _adc_inhibit_mask |= (1 << input_idx);
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

#if defined(STM32H7)
    LL_ADC_SetChannelPreselection(adc->ADCx, input->ADC_Channel);
#endif
    
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
  stm32_dma_enable_clock(DMAx);

  // Disable DMA before continuing (see ref. manual "Stream configuration procedure")
  if (!adc_disable_dma(DMAx, stream))
      return false;

  // Clear Interrupt flags
  adc_dma_clear_flags(DMAx, stream);

  // setup DMA request
#if defined(STM32H7RS)
  LL_DMA_SetSrcAddress(DMAx, stream, (intptr_t)&adc->DR);
  LL_DMA_ConfigTransfer(DMAx, stream,
                        LL_DMA_SRC_DATAWIDTH_HALFWORD |
                            LL_DMA_DEST_DATAWIDTH_HALFWORD |
                            LL_DMA_DEST_INCREMENT);
  LL_DMA_SetPeriphRequest(DMAx, stream, channel);
  LL_DMA_SetChannelPriorityLevel(DMAx, stream, LL_DMA_HIGH_PRIORITY);
  // TODO: check FIFO mode
#elif defined(STM32H7)
  if (DMAx != (DMA_TypeDef*)BDMA) {
    LL_DMA_ConfigAddresses(DMAx, stream, (intptr_t)&adc->DR, (intptr_t)dest,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMAx, stream, nconv);
    LL_DMA_SetPeriphRequest(DMAx, stream, channel);

    // Very high priority, half-word transfers, increment memory
    LL_DMA_ConfigTransfer(
        DMAx, stream,
        LL_DMA_PRIORITY_VERYHIGH | LL_DMA_MDATAALIGN_HALFWORD |
            LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    // disable direct mode, half full FIFO
    LL_DMA_EnableFifoMode(DMAx, stream);
    LL_DMA_SetFIFOThreshold(DMAx, stream, LL_DMA_FIFOTHRESHOLD_1_2);
    LL_DMA_SetMemoryBurstxfer(DMAx, stream, LL_DMA_MBURST_INC4);
  } else {
    BDMA_TypeDef* BDMAx = (BDMA_TypeDef*)DMAx;
    LL_BDMA_ConfigAddresses(BDMAx, stream, (intptr_t)&adc->DR, (intptr_t)dest,
                            LL_BDMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_BDMA_SetDataLength(BDMAx, stream, nconv);
    LL_BDMA_SetPeriphRequest(BDMAx, stream, channel);

    // Very high priority, half-word transfers, increment memory
    LL_BDMA_ConfigTransfer(
        BDMAx, stream,
        LL_BDMA_PRIORITY_VERYHIGH | LL_BDMA_MDATAALIGN_HALFWORD |
            LL_BDMA_PDATAALIGN_HALFWORD | LL_BDMA_MEMORY_INCREMENT |
            LL_BDMA_DIRECTION_PERIPH_TO_MEMORY);
  }
#elif defined(STM32F4) || defined(STM32F2)
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
  LL_DMA_SetMemoryBurstxfer(DMAx, stream, LL_DMA_MBURST_INC4);

#else
# warning "Unsupported DMA configuration for MCU type"
#endif

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
  memset(_adc_dma_buffer, 0, sizeof(_adc_dma_buffer));

  // Init common to all ADCs
  LL_ADC_CommonInitTypeDef commonInit;
  LL_ADC_CommonStructInit(&commonInit);

#if defined(LL_ADC_CLOCK_ASYNC_DIV4)
  // H7 and H7RS use their own clock
  commonInit.CommonClock = LL_ADC_CLOCK_ASYNC_DIV4;
#else
  // others can only use the peripheral clock
  commonInit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
#endif

  _adc_input_mask = 0;
  const stm32_adc_t* adc = ADCs;

  while (n_ADC > 0) {

    uint8_t nconv = adc->n_channels;
    if (nconv > 0) {

      // enable common instance
      LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(adc->ADCx), &commonInit);
  
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
#if defined(STM32H7)
        if (adc->ADCx == ADC3) {
          NVIC_SetPriority(ADC3_IRQn, ADC_IRQ_PRIO);
          NVIC_EnableIRQ(ADC3_IRQn);
        } else {
          NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIO);
          NVIC_EnableIRQ(ADC_IRQn);
        }
#else

#if defined(STM32H7RS)
#  define ADC_IRQn ADC1_2_IRQn
#endif
        NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIO);
        NVIC_EnableIRQ(ADC_IRQn);
#endif
      }
    }

    // move to next ADC definition
    adc++; n_ADC--;
  }

  return true;
}

#if defined(STM32H7RS)

static inline DMA_Channel_TypeDef* _dma_get_stream(DMA_TypeDef *DMAx, uint32_t Channel)
{
  return ((DMA_Channel_TypeDef*)((uint32_t)((uint32_t)DMAx + LL_DMA_CH_OFFSET_TAB[Channel])));
}

#else // STM32H7RS

static inline DMA_Stream_TypeDef* _dma_get_stream(DMA_TypeDef *DMAx, uint32_t Stream)
{
#if defined(STM32H7)
#  define __STREAM_OFFSET_TAB LL_DMA_STR_OFFSET_TAB
#else
#  define __STREAM_OFFSET_TAB STREAM_OFFSET_TAB
#endif
  return ((DMA_Stream_TypeDef*)((uint32_t)((uint32_t)DMAx + __STREAM_OFFSET_TAB[Stream])));
#undef __STREAM_OFFSET_TAB
}

#endif // !STM32H7RS


#define DMA_Stream0_IT_MASK     (uint32_t)(DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 | \
                                           DMA_LISR_TEIF0 | DMA_LISR_HTIF0 | \
                                           DMA_LISR_TCIF0)
  
#define DMA_Stream4_IT_MASK     (uint32_t)(DMA_HISR_FEIF4 | DMA_HISR_DMEIF4 | \
                                           DMA_HISR_TEIF4 | DMA_HISR_HTIF4 | \
                                           DMA_HISR_TCIF4)

static void adc_dma_clear_flags(DMA_TypeDef* DMAx, uint32_t stream)
{
#if defined(STM32H7RS)
  DMA_Channel_TypeDef* ch = _dma_get_stream(DMAx, stream);
  WRITE_REG(ch->CFCR, DMA_CFCR_DTEF | DMA_CFCR_HTF | DMA_CFCR_TCF);
#else
  // no other choice, sorry for that...
  if (stream == LL_DMA_STREAM_4) {
    /* Reset interrupt pending bits for DMA2 Stream4 */
    WRITE_REG(DMAx->HIFCR, DMA_Stream4_IT_MASK);

  } else if (stream == LL_DMA_STREAM_0) {
    /* Reset interrupt pending bits for DMA2 Stream0 */
    WRITE_REG(DMAx->LIFCR, DMA_Stream0_IT_MASK);
  }
#endif
}

static void adc_start_dma_conversion(const stm32_adc_t* adc)
{
  ADC_TypeDef* ADCx = adc->ADCx;
  DMA_TypeDef* DMAx = adc->DMAx;
  uint32_t stream = adc->DMA_Stream;

  // Clear Interrupt flags
  adc_dma_clear_flags(DMAx, stream);

  // Clear ADC status register & disable ADC IRQ
  _CLEAR_ADC_STATUS(ADCx);
  _DISABLE_EOCIE(ADCx);

  // Enable DMA
#if defined(STM32H7RS)
  // transfer length must be configured for every transfer
  uint32_t seq_len = LL_ADC_REG_GetSequencerLength(ADCx) + 1;
  LL_DMA_SetBlkDataLength(DMAx, stream, seq_len * 2);

  uint16_t* dest_addr = _adc_dma_buffer + adc->offset;
  LL_DMA_SetDestAddress(DMAx, stream, (intptr_t)dest_addr);

  DMA_Channel_TypeDef* ch = _dma_get_stream(DMAx, stream);
  SET_BIT(ch->CCR, DMA_CCR_TCIE | DMA_CCR_DTEIE);
  SET_BIT(ch->CCR, DMA_CCR_EN);
#else
  DMA_Stream_TypeDef* dma_stream = _dma_get_stream(DMAx, stream);
  SET_BIT(dma_stream->CR, DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
  SET_BIT(dma_stream->CR, DMA_SxCR_EN);
#endif

  // Trigger ADC start
  _START_ADC_DMA(ADCx);
}

static bool adc_disable_dma(DMA_TypeDef* DMAx, uint32_t stream)
{
#if defined(STM32H7RS)
  DMA_Channel_TypeDef* ch = _dma_get_stream(DMAx, stream);
  CLEAR_BIT(ch->CCR, DMA_CCR_TCIE | DMA_CCR_DTEIE);

  LL_DMA_DisableChannel(DMAx, stream);

  // wait until DMA EN bit gets cleared by hardware
  uint16_t timeout = 1000;
  while (LL_DMA_IsEnabledChannel(DMAx, stream)) {
    if (--timeout == 0) {
      // Timeout. Failed to disable DMA
      return false;
    }
  }
#else
  auto dma_stream = _dma_get_stream(DMAx, stream);
  CLEAR_BIT(dma_stream->CR, DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
  
  LL_DMA_DisableStream(DMAx, stream);

  // wait until DMA EN bit gets cleared by hardware
  uint16_t timeout = 1000;
  while (LL_DMA_IsEnabledStream(DMAx, stream)) {
    if (--timeout == 0) {
      // Timeout. Failed to disable DMA
      return false;
    }
  }
#endif
  
  return true;
}

static void adc_start_normal_conversion(ADC_TypeDef* ADCx)
{
  // clear status flags
  _CLEAR_ADC_STATUS(ADCx);

  // enble ADC IRQ
  _ENABLE_EOCIE(ADCx);

  // and start!
  _START_ADC_SINGLE(ADCx);
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
    bool seq_mode_enabled = false;
#if defined(STM32H7) || defined(STM32H7RS)
    seq_mode_enabled = (LL_ADC_REG_GetSequencerLength(ADCx) != LL_ADC_REG_SEQ_SCAN_DISABLE);
#else
    seq_mode_enabled = (LL_ADC_GetSequencersScanMode(ADCx) == LL_ADC_SEQ_SCAN_ENABLE);
#endif

    if (seq_mode_enabled) {
      // Disable DMA before continuing (see ref. manual "Stream configuration procedure")
      auto DMAx = adc->DMAx;
      auto stream = adc->DMA_Stream;
      if (DMAx && adc_disable_dma(DMAx, stream)) {
        _adc_started_mask |= adc_mask;
        adc_start_dma_conversion(adc);
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

  memset(_adc_oversampling, 0, sizeof(_adc_oversampling));
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

  auto timeout = timersGetUsTick();
  while(!_adc_completed) {
    // busy wait
    if ((uint32_t)(timersGetUsTick() - timeout) >= SAMPLING_TIMEOUT_US) {
      TRACE("ADC timeout");
      _adc_started_mask = 0;
      return;
    }
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
  adc_dma_clear_flags(adc->DMAx, adc->DMA_Stream);

#if !defined(STM32H7) && !defined(STM32H7RS)
  // Disable DMA
  CLEAR_BIT(adc->ADCx->CR2, ADC_CR2_DMA);
#endif

  uint16_t* dma_buffer = _adc_dma_buffer + adc->offset;
  copy_adc_values(dma_buffer, adc, _adc_inputs);

  _adc_chain_conversions(adc);
}

void stm32_hal_adc_isr(const stm32_adc_t* adc)
{
  // check if this ADC triggered the IRQ
  auto ADCx = adc->ADCx;
#if defined(STM32H7) || defined(STM32H7RS)
  if (!LL_ADC_IsActiveFlag_EOC(ADCx) || !LL_ADC_IsEnabledIT_EOC(ADCx))
    return;
#else
  if (!LL_ADC_IsActiveFlag_EOCS(ADCx) || !LL_ADC_IsEnabledIT_EOCS(ADCx))
    return;
#endif

  // Disable end-of-conversion IRQ
  _DISABLE_EOCIE(ADCx);

  uint16_t* dma_buffer = _adc_dma_buffer + adc->offset;
  *dma_buffer = adc->ADCx->DR;
  copy_adc_values(dma_buffer, adc, _adc_inputs);

  _adc_chain_conversions(adc);
}
