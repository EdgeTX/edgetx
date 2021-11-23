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

extern "C" {
  #define USE_FULL_LL_DRIVER
  #include "CMSIS/Device/ST/STM32F4xx/Include/stm32f4xx.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_gpio.h"
  #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_tim.h"
}

#include "hal.h"
#include "extmodule_driver.h"
#include "timers_driver.h"

#if defined(CROSSFIRE)
#include "pulses/crossfire.h"
#endif

void extmoduleStop()
{
  EXTERNAL_MODULE_OFF();

  NVIC_DisableIRQ(EXTMODULE_TIMER_DMA_STREAM_IRQn);

  EXTMODULE_TIMER_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
  EXTMODULE_TIMER->DIER &= ~TIM_DIER_UDE;
  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(EXTMODULE_TX_GPIO, &GPIO_InitStructure);
}

void extmodulePpmStart(uint16_t ppm_delay, bool polarity)
{
  EXTERNAL_MODULE_ON();

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = EXTMODULE_TX_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Alternate = EXTMODULE_TIMER_TX_GPIO_AF;
  LL_GPIO_Init(EXTMODULE_TX_GPIO, &pinInit);

  // PPM generation principle:
  //
  // Hardware timer in PWM mode is used for PPM generation
  // Output is OFF if CNT<CCR1(delay) and ON if bigger
  // CCR1 register defines duration of pulse length and is constant
  // AAR register defines duration of each pulse, it is
  // updated after every pulse in Update interrupt handler.

  LL_TIM_InitTypeDef timInit;
  LL_TIM_StructInit(&timInit);

  // 0.5uS (2Mhz)
  timInit.Prescaler = __LL_TIM_CALC_PSC(EXTMODULE_TIMER_FREQ, 2000000);
  timInit.Autoreload = 65535;
  LL_TIM_Init(EXTMODULE_TIMER, &timInit);

  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = LL_TIM_OCMODE_PWM1;
  ocInit.OCState = LL_TIM_OCSTATE_ENABLE;

  ocInit.OCIdleState = LL_TIM_OCIDLESTATE_HIGH;

  if (polarity) {
    ocInit.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  } else {
    ocInit.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  }

  ocInit.CompareValue = ppm_delay * 2;

  LL_TIM_DisableCounter(EXTMODULE_TIMER);
  LL_TIM_OC_Init(EXTMODULE_TIMER, LL_TIM_CHANNEL_CH3, &ocInit);

  LL_TIM_OC_EnablePreload(EXTMODULE_TIMER, LL_TIM_CHANNEL_CH3);

  // BDTR = TIM_BDTR_MOE
  LL_TIM_EnableAllOutputs(EXTMODULE_TIMER);

  LL_TIM_EnableDMAReq_UPDATE(EXTMODULE_TIMER);
  LL_TIM_EnableCounter(EXTMODULE_TIMER);

  NVIC_EnableIRQ(EXTMODULE_TIMER_DMA_STREAM_IRQn);
  NVIC_SetPriority(EXTMODULE_TIMER_DMA_STREAM_IRQn, 7);
}

#if defined(PXX1)
void extmodulePxx1PulsesStart()
{
  EXTERNAL_MODULE_ON();

  GPIO_PinAFConfig(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PinSource, EXTMODULE_TIMER_TX_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(EXTMODULE_TX_GPIO, &GPIO_InitStructure);

  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  EXTMODULE_TIMER->PSC = EXTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

#if defined(PCBX10) || PCBREV >= 13
  EXTMODULE_TIMER->CCR3 = 18;
  EXTMODULE_TIMER->CCER = TIM_CCER_CC3E | TIM_CCER_CC3NE | TIM_CCER_CC3P | TIM_CCER_CC3NP;
  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0; // Force O/P high
  EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  EXTMODULE_TIMER->EGR = 1; // Restart
  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
#else
  EXTMODULE_TIMER->CCR1 = 18;
  EXTMODULE_TIMER->CCER =
    TIM_CCER_CC1E |
#if !defined(PCBNV14)
    TIM_CCER_CC1P |
#endif
    TIM_CCER_CC1NE | TIM_CCER_CC1NP;

  EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0; // Force O/P high
  EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  EXTMODULE_TIMER->EGR = 1; // Restart
  EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
#endif

  EXTMODULE_TIMER->ARR = 45000;
  EXTMODULE_TIMER->SR &= ~TIM_SR_CC2IF; // Clear flag
  EXTMODULE_TIMER->DIER |= TIM_DIER_UDE; // Enable DMA on update
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;

  NVIC_EnableIRQ(EXTMODULE_TIMER_DMA_STREAM_IRQn);
  NVIC_SetPriority(EXTMODULE_TIMER_DMA_STREAM_IRQn, 7);
}
#endif

void extmoduleSerialStart()
{
  EXTERNAL_MODULE_ON();

  GPIO_PinAFConfig(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PinSource, EXTMODULE_TIMER_TX_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(EXTMODULE_TX_GPIO, &GPIO_InitStructure);

  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  EXTMODULE_TIMER->PSC = EXTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

#if defined(PCBX10) || PCBREV >= 13
  EXTMODULE_TIMER->CCR3 = 0;
  EXTMODULE_TIMER->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P;
  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0; // Force O/P high
  EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  EXTMODULE_TIMER->EGR = 1; // Restart
  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0;
#else

#if defined(PCBNV14)
  EXTMODULE_TIMER->CCER = TIM_CCER_CC1E;
#else
  EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P;
#endif
  EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  EXTMODULE_TIMER->CCR1 = 0;
  EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0; // Force O/P high
  EXTMODULE_TIMER->EGR = 1; // Restart
  EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;
#endif

  EXTMODULE_TIMER->ARR = 40000; // dummy value until the DMA request kicks in
  EXTMODULE_TIMER->SR &= ~TIM_SR_CC2IF; // Clear flag
  EXTMODULE_TIMER->DIER |= TIM_DIER_UDE;
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;

  NVIC_EnableIRQ(EXTMODULE_TIMER_DMA_STREAM_IRQn);
  NVIC_SetPriority(EXTMODULE_TIMER_DMA_STREAM_IRQn, 7);
}

void extmoduleSendNextFramePpm(void* pulses, uint16_t length,
                               uint16_t ppm_delay, bool polarity)
{
  if (EXTMODULE_TIMER_DMA_STREAM->CR & DMA_SxCR_EN) return;

  // disable timer
  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;

#if defined(PCBX10) || PCBREV >= 13
  // Using timer channel 3
  EXTMODULE_TIMER->CCR3 = ppm_delay * 2;
  EXTMODULE_TIMER->CCER =
      TIM_CCER_CC3E | (polarity ? TIM_CCER_CC3P : 0);
#else
  // Using timer channel 1
  EXTMODULE_TIMER->CCR1 = ppm_delay * 2;
  EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | (polarity ?
#if defined(PCBNV14)
                                                    0
                                                    : TIM_CCER_CC1P
#else
                                                    TIM_CCER_CC1P
                                                    : 0
#endif
                                          );
#endif
  EXTMODULE_TIMER_DMA_STREAM->CR &= ~DMA_SxCR_EN;  // Disable DMA
  EXTMODULE_TIMER_DMA_STREAM->CR |=
      EXTMODULE_TIMER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC |
      EXTMODULE_TIMER_DMA_SIZE | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  EXTMODULE_TIMER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&EXTMODULE_TIMER->ARR);
  EXTMODULE_TIMER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(pulses);
  EXTMODULE_TIMER_DMA_STREAM->NDTR = length;
  EXTMODULE_TIMER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;  // Enable DMA

  // re-init timer
  EXTMODULE_TIMER->EGR = 1;
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;
}

#if defined(PXX1)
void extmoduleSendNextFramePxx1(const void* pulses, uint16_t length)
{
  if (EXTMODULE_TIMER_DMA_STREAM->CR & DMA_SxCR_EN) return;

  // disable timer
  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  EXTMODULE_TIMER_DMA_STREAM->CR |=
      EXTMODULE_TIMER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC |
      EXTMODULE_TIMER_DMA_SIZE | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  EXTMODULE_TIMER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&EXTMODULE_TIMER->ARR);
  EXTMODULE_TIMER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(pulses);
  EXTMODULE_TIMER_DMA_STREAM->NDTR = length;
  EXTMODULE_TIMER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;  // Enable DMA

  // re-init timer
  EXTMODULE_TIMER->EGR = 1;
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;
}
#endif

#if defined(AFHDS3) && !(defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO))
void extmoduleSendNextFrameAFHDS3(const void* dataPtr, uint16_t dataSize)
{
  if (EXTMODULE_TIMER_DMA_STREAM->CR & DMA_SxCR_EN) return;

  EXTMODULE_TIMER_DMA_STREAM->CR &= ~DMA_SxCR_EN;  // Disable DMA
  EXTMODULE_TIMER_DMA_STREAM->CR |=
      EXTMODULE_TIMER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC |
      DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  EXTMODULE_TIMER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&EXTMODULE_TIMER->ARR);
  EXTMODULE_TIMER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(dataPtr);
  EXTMODULE_TIMER_DMA_STREAM->NDTR = dataSize;
  EXTMODULE_TIMER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;  // Enable DMA

  // re-init timer
  EXTMODULE_TIMER->EGR = TIM_PSCReloadMode_Immediate;
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;
}
#endif

void extmoduleSendNextFrameSoftSerial100kbit(const void* pulses, uint16_t length,
                                             bool polarity)
{
  if (EXTMODULE_TIMER_DMA_STREAM->CR & DMA_SxCR_EN) return;

  //if (PROTOCOL_CHANNELS_SBUS == moduleState[EXTERNAL_MODULE].protocol) {
    // reverse polarity for Sbus if needed
    EXTMODULE_TIMER->CCER =
#if defined(PCBX10) || PCBREV >= 13
        TIM_CCER_CC3E | (polarity ? TIM_CCER_CC3P : 0)
#elif defined(PCBNV14)
        TIM_CCER_CC1E | (polarity ? 0 : TIM_CCER_CC1P)
#else
        TIM_CCER_CC1E | (polarity ? TIM_CCER_CC1P : 0)
#endif
        ;  //
    //}

  // disable timer
  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;

  // send DMA request
  EXTMODULE_TIMER_DMA_STREAM->CR &= ~DMA_SxCR_EN;  // Disable DMA
  EXTMODULE_TIMER_DMA_STREAM->CR |=
      EXTMODULE_TIMER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC |
      EXTMODULE_TIMER_DMA_SIZE | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  EXTMODULE_TIMER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&EXTMODULE_TIMER->ARR);
  EXTMODULE_TIMER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(pulses);
  EXTMODULE_TIMER_DMA_STREAM->NDTR = length;
  EXTMODULE_TIMER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE;  // Enable DMA

  // re-init timer
  EXTMODULE_TIMER->EGR = 1;
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN;
}

void extmoduleSendInvertedByte(uint8_t byte)
{
  uint16_t time;
  uint32_t i;

  __disable_irq();
  time = getTmr2MHz();
  GPIO_SetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  while ((uint16_t) (getTmr2MHz() - time) < 34)	{
    // wait
  }
  time += 34;
  for (i = 0; i < 8; i++) {
    if (byte & 1) {
      GPIO_ResetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    else {
      GPIO_SetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    byte >>= 1 ;
    while ((uint16_t) (getTmr2MHz() - time) < 35) {
      // wait
    }
    time += 35 ;
  }
  GPIO_ResetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  __enable_irq();	// No need to wait for the stop bit to complete
  while ((uint16_t) (getTmr2MHz() - time) < 34) {
    // wait
  }
}

extern "C" void EXTMODULE_TIMER_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(EXTMODULE_TIMER_DMA_STREAM, EXTMODULE_TIMER_DMA_FLAG_TC))
    return;

  DMA_ClearITPendingBit(EXTMODULE_TIMER_DMA_STREAM, EXTMODULE_TIMER_DMA_FLAG_TC);


}
