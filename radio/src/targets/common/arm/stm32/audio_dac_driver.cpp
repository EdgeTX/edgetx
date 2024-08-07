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
#include "stm32_gpio.h"
#include "stm32_timer.h"
#include "stm32_dma.h"

#include "edgetx.h"

#if !defined(SIMU)
const AudioBuffer * nextBuffer = nullptr;

void setSampleRate(uint32_t frequency)
{
  uint32_t timer = (PERI1_FREQUENCY * TIMER_MULT_APB1) / frequency - 1 ;         // MCK/8 and 100 000 Hz
  AUDIO_TIMER->CR1 &= ~TIM_CR1_CEN ;
  AUDIO_TIMER->CNT = 0 ;
  AUDIO_TIMER->ARR = limit<uint32_t>(2, timer, 65535) ;
  AUDIO_TIMER->CR1 |= TIM_CR1_CEN ;
}

// Start TIMER6 at 100000Hz, used for DAC trigger
void dacTimerInit()
{
  stm32_timer_enable_clock(AUDIO_TIMER);
  AUDIO_TIMER->PSC = 0 ;                                                       // Max speed
  AUDIO_TIMER->ARR = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 100000 - 1 ;        // 10 uS, 100 kHz
  AUDIO_TIMER->CR2 = 0 ;
  AUDIO_TIMER->CR2 = 0x20 ;
  AUDIO_TIMER->CR1 = TIM_CR1_CEN ;
}

#if defined(AUDIO_MUTE_GPIO)
static inline void setMutePin(bool enabled)
{
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  gpio_write(AUDIO_MUTE_GPIO, enabled);
}

static inline bool getMutePin(void)
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
  setMutePin(true);
#endif

  dacTimerInit();

  gpio_init_analog(AUDIO_OUTPUT_GPIO);
  stm32_dma_enable_clock(AUDIO_DMA);

  // Chan 7, 16-bit wide, Medium priority, memory increments
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5 ; // Write ones to clear bits
  AUDIO_DMA_Stream->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 |
                     DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_CIRC ;
  AUDIO_DMA_Stream->PAR = CONVERT_PTR_UINT(&DAC->DHR12R1);
  // AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(Sine_values);
  AUDIO_DMA_Stream->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
  // AUDIO_DMA_Stream->NDTR = 100 ;

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
  DAC->DHR12R1 = 2010 ;
  DAC->SR = DAC_SR_DMAUDR1 ;              // Write 1 to clear flag
  DAC->CR = DAC_CR_TEN1 | DAC_CR_EN1 ;                    // Enable DAC
  NVIC_EnableIRQ(AUDIO_TIM_IRQn); // TODO needed?
  NVIC_SetPriority(AUDIO_TIM_IRQn, 7);
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
    setMutePin(true);
  }
#else
  // mute
  setMutePin(true);
#endif
}

void audioUnmute()
{
if(isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
  setMutePin(true);
  return;
}

#if defined(AUDIO_UNMUTE_DELAY)
  // if muted
  if (getMutePin()) {
    // ..un-mute
    setMutePin(false);
    RTOS_WAIT_MS(AUDIO_UNMUTE_DELAY);
  }
  // reset the mute delay
  audioQueue.lastAudioPlayTime = 0;
#else
  setMutePin(false);
#endif
}
#endif

void audioConsumeCurrentBuffer()
{
  if (!nextBuffer) {
    nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();
    if (nextBuffer) {
#if defined(AUDIO_MUTE_GPIO)
      audioUnmute();
#endif
      AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA channel
      AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5; // Write ones to clear bits
      AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(nextBuffer->data);
      AUDIO_DMA_Stream->NDTR = nextBuffer->size;
      AUDIO_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA channel and interrupt
      DAC->SR = DAC_SR_DMAUDR1; // Write 1 to clear flag
      DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1; // Enable DAC
    }
#if defined(AUDIO_MUTE_GPIO)
    else {
      audioMute();
    }
#endif
  }
}

void dacStart()
{
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5 ; // Write ones to clear bits
  AUDIO_DMA_Stream->CR |= DMA_SxCR_CIRC | DMA_SxCR_EN ;                               // Enable DMA channel
  DAC->SR = DAC_SR_DMAUDR1 ;                      // Write 1 to clear flag
  DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1 ;                 // Enable DAC
}

void dacStop()
{
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_CIRC ;
}


// Sound routines
void audioInit()
{
  dacInit();
}

void audioEnd()
{
  DAC->CR = 0 ;
  AUDIO_TIMER->CR1 = 0 ;
  // Also need to turn off any possible interrupts
  NVIC_DisableIRQ(AUDIO_TIM_IRQn) ;
  NVIC_DisableIRQ(AUDIO_DMA_Stream_IRQn) ;
}

extern "C" void AUDIO_TIM_IRQHandler()
{
  DEBUG_INTERRUPT(INT_AUDIO);
  DAC->CR &= ~DAC_CR_DMAEN1 ;     // Stop DMA requests
#if defined(STM32F2)
  DAC->CR &= ~DAC_CR_DMAUDRIE1 ;  // Stop underrun interrupt
#endif
  DAC->SR = DAC_SR_DMAUDR1 ;      // Write 1 to clear flag
}

extern "C" void AUDIO_DMA_Stream_IRQHandler()
{
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_TCIE ;            // Stop interrupt
  AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5 ; // Write ones to clear flags
  AUDIO_DMA_Stream->CR &= ~DMA_SxCR_EN ;                              // Disable DMA channel

  if (nextBuffer) audioQueue.buffersFifo.freeNextFilledBuffer();

  nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();
  if (nextBuffer) {
    AUDIO_DMA_Stream->M0AR = CONVERT_PTR_UINT(nextBuffer->data);
    AUDIO_DMA_Stream->NDTR = nextBuffer->size;
    AUDIO_DMA->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5 ; // Write ones to clear bits
    AUDIO_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE ;       // Enable DMA channel
    DAC->SR = DAC_SR_DMAUDR1;                      // Write 1 to clear flag
  }
}
#endif  // #if !defined(SIMU)
