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

#include "hal/gpio.h"
#include "hal/audio_driver.h"

#include "stm32_gpio.h"
#include "stm32_timer.h"
#include "stm32_dma.h"

#include "edgetx.h"


const AudioBuffer * nextBuffer = nullptr;

#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)
// 16 bit, 1 channels
#define DMA_BUFFER_HALF_LEN AUDIO_BUFFER_SIZE

// 2 buffers
#define DMA_BUFFER_LEN (DMA_BUFFER_HALF_LEN * 2)

static uint16_t _dma_buffer[DMA_BUFFER_LEN] __DMA_NO_CACHE;

static volatile uint32_t _dma_buffer_offset = 0;

static inline uint32_t _calc_offset(uint8_t tc)
{
  return tc * DMA_BUFFER_HALF_LEN;
}

// the returned bool is true when there is no more data available
static bool audio_update_dma_buffer(uint8_t tc)
{
  _dma_buffer_offset = _calc_offset(tc);

  auto buffer = audioQueue.buffersFifo.getNextFilledBuffer();
  nextBuffer = buffer;
  if (!buffer) {
    unsigned idx = 0;
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = AUDIO_DATA_SILENCE;
    }
    return true;
  } else {
    unsigned idx = 0;
    for (; idx < buffer->size; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = (uint16_t)buffer->data[idx];
    }
    for (; idx < DMA_BUFFER_HALF_LEN; idx++) {
      uint32_t offset = _dma_buffer_offset + idx;
      _dma_buffer[offset] = AUDIO_DATA_SILENCE;
    }
    audioQueue.buffersFifo.freeNextFilledBuffer();
    return false;
  }
}
#else
#define AUDIO_DAC DAC
#endif

// Init timer that triggers the DAC conversion
// at sampling frequency
static void dac_trigger_init()
{
  stm32_timer_enable_clock(AUDIO_TIMER);

  // set timer to: 10 uS, 100 kHz
  AUDIO_TIMER->PSC = 0;
  AUDIO_TIMER->ARR = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 100000 - 1;

  // reset counter
  AUDIO_TIMER->CNT = 0;

  // set expiry to sample period (1 / sample_rate)
  AUDIO_TIMER->ARR =
      (PERI1_FREQUENCY * TIMER_MULT_APB1) / AUDIO_SAMPLE_RATE - 1;

  // Master mode selection
  //
  //  000: Reset
  AUDIO_TIMER->CR2 = 0;
  //  010: The update event is selected as trigger output (TRGO)
  //       (used to trigger DAC conversion)
  AUDIO_TIMER->CR2 = TIM_CR2_MMS_1;

  // enable timer
  AUDIO_TIMER->CR1 |= TIM_CR1_CEN;
}

#if defined(AUDIO_MUTE_GPIO)
static inline void set_mute_pin(bool enabled)
{
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  gpio_write(AUDIO_MUTE_GPIO, enabled);
}

static inline bool get_mute_pin(void)
{
  bool enabled = gpio_read(AUDIO_MUTE_GPIO) ? 1 : 0;
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif  
  return enabled;
}
#endif

// Configure DAC0
// Not sure why PB14 has not be allocated to the DAC, although it is an EXTRA function
// So maybe it is automatically done
void dacInit()
{  
#if defined(AUDIO_MUTE_GPIO)
  // Mute before init anything
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  set_mute_pin(true);
#endif

  dac_trigger_init();

  gpio_init_analog(AUDIO_OUTPUT_GPIO);
  stm32_dma_enable_clock(AUDIO_DMA);

#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)
  LL_DMA_DeInit(AUDIO_DMA, AUDIO_DMA_Stream);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

#if defined(STM32H7)
  dmaInit.Mode = LL_DMA_MODE_CIRCULAR;
  dmaInit.PeriphRequest = AUDIO_DMA_Channel;
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  dmaInit.PeriphOrM2MSrcAddress = LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_LEFT_ALIGNED);
  dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
#elif defined(STM32H7RS)
  #error not implemented
#endif
  LL_DMA_Init(AUDIO_DMA, AUDIO_DMA_Stream, &dmaInit);
#else
  // Disable DMA stream
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN;

  // clear DMA stream flags
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 |
                     DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

  // Chan 7, 16-bit wide, Medium priority, memory increments
  AUDIO_DMA_Stream->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 |
                         DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0 |
                         DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 |
                         DMA_SxCR_CIRC;

  // write to DAC channel 1 (12 bits, left-aligned)
  AUDIO_DMA_Stream->PAR = CONVERT_PTR_UINT(&AUDIO_DAC->DHR12L1);

  // disable direct mode and set FIFO threshold to half
  AUDIO_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;
#endif

#if defined(LL_APB1_GRP1_PERIPH_DAC12)
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC12);
#else
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
#endif

  // set data registre to silence
  AUDIO_DAC->DHR12L1 = AUDIO_DATA_SILENCE;
#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)
  LL_DAC_ClearFlag_DMAUDR1(AUDIO_DAC);
  LL_DAC_SetTriggerSource(AUDIO_DAC, LL_DAC_CHANNEL_1, LL_DAC_TRIG_EXT_TIM6_TRGO);
  LL_DAC_EnableTrigger(AUDIO_DAC, LL_DAC_CHANNEL_1);
  LL_DAC_Enable(AUDIO_DAC, LL_DAC_CHANNEL_1);

#else
  // clear underrun flag
  AUDIO_DAC->SR = DAC_SR_DMAUDR1;

  // use TIM6 TRGO as trigger (TSEL1 = TSEL2 = 000)
  // enable DAC & channel 1 trigger
  AUDIO_DAC->CR = DAC_CR_TEN1 | DAC_CR_EN1;
#endif
  NVIC_EnableIRQ(AUDIO_DMA_Stream_IRQn);
  NVIC_SetPriority(AUDIO_DMA_Stream_IRQn, 7);
}

#if defined(AUDIO_MUTE_GPIO)
void audioMute()
{
  if (!g_eeGeneral.audioMuteEnable) return;
#if defined(AUDIO_UNMUTE_DELAY)
  tmr10ms_t now = get_tmr10ms();
  if (!audioQueue.lastAudioPlayTime) {
    // we start the mute delay now
    audioQueue.lastAudioPlayTime = now;
  }
  else if (now - audioQueue.lastAudioPlayTime > AUDIO_MUTE_DELAY / 10) {
    // delay expired, we may mute
    set_mute_pin(true);
  }
#else
  // mute
  set_mute_pin(true);
#endif
}

void audioUnmute()
{
  if (isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
    set_mute_pin(true);
    return;
  }

#if defined(AUDIO_UNMUTE_DELAY)
  // if muted
  if (get_mute_pin()) {
    // ..un-mute
    set_mute_pin(false);
    RTOS_WAIT_MS(AUDIO_UNMUTE_DELAY);
  }
  // reset the mute delay
  audioQueue.lastAudioPlayTime = 0;
#else
  set_mute_pin(false);
#endif
}
#endif

void audioConsumeCurrentBuffer()
{
#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)
  if (!LL_DMA_IsEnabledStream(AUDIO_DMA, AUDIO_DMA_Stream)) {
    if (!audio_update_dma_buffer(0)) {
#if defined(AUDIO_MUTE_GPIO)
      audioUnmute();
#endif

      LL_DMA_DisableStream(AUDIO_DMA, AUDIO_DMA_Stream);
      stm32_dma_check_tc_flag(AUDIO_DMA, AUDIO_DMA_Stream);
      stm32_dma_check_ht_flag(AUDIO_DMA, AUDIO_DMA_Stream);
      LL_DAC_ClearFlag_DMAUDR1(AUDIO_DAC);
      LL_DAC_EnableDMAReq(AUDIO_DAC, LL_DAC_CHANNEL_1);
      LL_DAC_Enable(AUDIO_DAC, LL_DAC_CHANNEL_1);
      LL_DMA_SetMemoryAddress(AUDIO_DMA, AUDIO_DMA_Stream, (intptr_t)_dma_buffer);
      LL_DMA_SetDataLength(AUDIO_DMA, AUDIO_DMA_Stream, DMA_BUFFER_LEN);
      LL_DMA_EnableStream(AUDIO_DMA, AUDIO_DMA_Stream);
      LL_DMA_EnableIT_HT(AUDIO_DMA, AUDIO_DMA_Stream);
      LL_DMA_EnableIT_TC(AUDIO_DMA, AUDIO_DMA_Stream);
#else
  if (!nextBuffer) {
    nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (nextBuffer) {

#if defined(AUDIO_MUTE_GPIO)
      audioUnmute();
#endif
      // Disable DMA stream
      AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN;

      // reset DMA flags
      AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 |
                         DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 |
                         DMA_HIFCR_CFEIF5;

      // set DMA buffer
      AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(nextBuffer->data);
      AUDIO_DMA_Stream->NDTR = nextBuffer->size;

      // enable DMA stream and transfer complete interrupt
      AUDIO_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;

      // clear underrun flag
      AUDIO_DAC->SR = DAC_SR_DMAUDR1;

      // enable DAC
      AUDIO_DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1;
#endif
    } else {
#if defined(AUDIO_MUTE_GPIO)
      audioMute();
#endif
    }
  }
}

// Sound routines
void audioInit()
{
  dacInit();
}

void audioEnd()
{
  AUDIO_DAC->CR = 0;
  AUDIO_TIMER->CR1 = 0;

  // Also need to turn off any possible interrupts
  NVIC_DisableIRQ(AUDIO_TIM_IRQn);
  NVIC_DisableIRQ(AUDIO_DMA_Stream_IRQn);
}

extern "C" void AUDIO_DMA_Stream_IRQHandler()
{
#if defined(STM32H5) || defined(STM32H7) || defined(STM32H7RS)
  bool stopDMA = false;
  if(stm32_dma_check_ht_flag(AUDIO_DMA, AUDIO_DMA_Stream)) {
    stopDMA = audio_update_dma_buffer(0);
  }

  if(stm32_dma_check_tc_flag(AUDIO_DMA, AUDIO_DMA_Stream)) {
    stopDMA |= audio_update_dma_buffer(1);
  }

  if(stopDMA) {
    LL_DMA_DisableIT_TC(AUDIO_DMA, AUDIO_DMA_Stream);
    LL_DMA_DisableIT_HT(AUDIO_DMA, AUDIO_DMA_Stream);
    LL_DMA_DisableStream(AUDIO_DMA, AUDIO_DMA_Stream);
  }

#else
  // disable transfer complete interrupt
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_TCIE;

  // reset DMA flags
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 |
                     DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

  // Disable DMA stream
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN;

  // get next buffer if any
  if (nextBuffer) audioQueue.buffersFifo.freeNextFilledBuffer();
  nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();

  if (nextBuffer) {
    AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(nextBuffer->data);
    AUDIO_DMA_Stream->NDTR = nextBuffer->size;

    // reset DMA flags (again?)
    AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 |
                       DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;

    // enable DMA stream and transfer complete interrupt
    AUDIO_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;

    // clear underrun flag
    AUDIO_DAC->SR = DAC_SR_DMAUDR1;
  }
#endif
}
