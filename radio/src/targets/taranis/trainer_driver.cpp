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

#include "opentx.h"

void trainerSendNextFrame();

#if defined(TRAINER_GPIO)
void init_trainer_ppm()
{
  GPIO_PinAFConfig(TRAINER_GPIO, TRAINER_OUT_GPIO_PinSource, TRAINER_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TRAINER_OUT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_GPIO, &GPIO_InitStructure);

  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN;
  TRAINER_TIMER->PSC = TRAINER_TIMER_FREQ / 2000000 - 1; // 0.5uS
  TRAINER_TIMER->ARR = 45000;
#if defined(TRAINER_OUT_CCMR1)
  TRAINER_TIMER->CCMR1 = TRAINER_OUT_CCMR1;
#elif defined(TRAINER_OUT_CCMR2)
  TRAINER_TIMER->CCMR2 = TRAINER_OUT_CCMR2;
#endif
  TRAINER_TIMER->BDTR = TIM_BDTR_MOE;
  TRAINER_TIMER->EGR = 1;
  TRAINER_TIMER->DIER |= TIM_DIER_UDE;
  TRAINER_TIMER->CR1 |= TIM_CR1_CEN;

  setupPulsesPPMTrainer();
  trainerSendNextFrame();

#if defined(TRAINER_DMA_STREAM)
  NVIC_EnableIRQ(TRAINER_DMA_IRQn);
  NVIC_SetPriority(TRAINER_DMA_IRQn, 7);
#endif

  NVIC_EnableIRQ(TRAINER_TIMER_IRQn);
  NVIC_SetPriority(TRAINER_TIMER_IRQn, 7);
}

void stop_trainer_ppm()
{
#if defined(TRAINER_DMA_STREAM)
  NVIC_DisableIRQ(TRAINER_DMA_IRQn);
  TRAINER_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
#endif

  NVIC_DisableIRQ(TRAINER_TIMER_IRQn);
  TRAINER_TIMER->DIER = 0;
  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop counter
}

void init_trainer_capture()
{
  GPIO_PinAFConfig(TRAINER_GPIO, TRAINER_IN_GPIO_PinSource, TRAINER_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TRAINER_IN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_GPIO, &GPIO_InitStructure);

  TRAINER_TIMER->ARR = 0xFFFF;
  TRAINER_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 2000000 - 1; // 0.5uS
  TRAINER_TIMER->CR2 = 0;
#if defined(TRAINER_IN_CCMR1)
  TRAINER_TIMER->CCMR1 = TRAINER_IN_CCMR1;
#elif defined(TRAINER_IN_CCMR2)
  TRAINER_TIMER->CCMR2 = TRAINER_IN_CCMR2;
#endif
  TRAINER_TIMER->CCER = TRAINER_IN_CCER;
  TRAINER_TIMER->SR &= ~TIM_SR_CC3IF & ~TIM_SR_CC2IF & ~TIM_SR_UIF; // Clear flags
  TRAINER_TIMER->DIER |= TRAINER_IN_INTERRUPT_ENABLE;
  TRAINER_TIMER->CR1 = TIM_CR1_CEN;

  NVIC_EnableIRQ(TRAINER_TIMER_IRQn);
  NVIC_SetPriority(TRAINER_TIMER_IRQn, 7);
}

void stop_trainer_capture()
{
  NVIC_DisableIRQ(TRAINER_TIMER_IRQn); // Stop Interrupt
  TRAINER_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop counter
  TRAINER_TIMER->DIER = 0; // Stop Interrupt
}

void trainerSendNextFrame()
{
  TRAINER_OUT_COUNTER_REGISTER = GET_TRAINER_PPM_DELAY() * 2;
  TRAINER_TIMER->CCER = TRAINER_OUT_CCER | (GET_TRAINER_PPM_POLARITY() ? 0 : TRAINER_CCER_POLARYTY);
  TRAINER_SETUP_REGISTER = *(trainerPulsesData.ppm.ptr - 1) - 4000; // 2mS in advance

#if defined(TRAINER_DMA_STREAM)
  TRAINER_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
  TRAINER_DMA_STREAM->CR |= TRAINER_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1;
  TRAINER_DMA_STREAM->PAR = CONVERT_PTR_UINT(&TRAINER_TIMER->ARR);
  TRAINER_DMA_STREAM->M0AR = CONVERT_PTR_UINT(trainerPulsesData.ppm.pulses);
  TRAINER_DMA_STREAM->NDTR = trainerPulsesData.ppm.ptr - trainerPulsesData.ppm.pulses;
  TRAINER_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA
#else
  trainerPulsesData.ppm.ptr = trainerPulsesData.ppm.pulses;
  TRAINER_TIMER->DIER |= TIM_DIER_UDE;
  TRAINER_TIMER->SR &= ~TIM_SR_UIF; // Clear this flag
  TRAINER_TIMER->DIER |= TIM_DIER_UIE; // Enable this interrupt
#endif
}
#endif

#if defined(TRAINER_DMA_STREAM)
extern "C" void TRAINER_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(TRAINER_DMA_STREAM, TRAINER_DMA_FLAG_TC))
    return;

  DMA_ClearITPendingBit(TRAINER_DMA_STREAM, TRAINER_DMA_FLAG_TC);

  TRAINER_TIMER->SR &= ~TRAINER_OUT_INTERRUPT_FLAG; // Clear flag
  TRAINER_TIMER->DIER |= TRAINER_OUT_INTERRUPT_ENABLE; // Enable this interrupt
}
#endif

extern "C" void TRAINER_TIMER_IRQHandler()
{
  DEBUG_INTERRUPT(INT_TRAINER);

  uint16_t capture = 0;
  bool doCapture = false;

#if defined(TRAINER_GPIO)
  // What mode? in or out?
  if ((TRAINER_TIMER->DIER & TRAINER_IN_INTERRUPT_ENABLE) && (TRAINER_TIMER->SR & TRAINER_IN_INTERRUPT_FLAG)) {
    // capture mode on trainer jack
    capture = TRAINER_IN_COUNTER_REGISTER;
    if (TRAINER_CONNECTED() && currentTrainerMode == TRAINER_MODE_MASTER_TRAINER_JACK) {
      doCapture = true;
    }
  }
#endif

#if defined(TRAINER_MODULE_CPPM) && !defined(TRAINER_MODULE_CPPM_TIMER_IRQHandler)
  if ((TRAINER_TIMER->DIER & TRAINER_MODULE_CPPM_INTERRUPT_ENABLE) && (TRAINER_TIMER->SR & TRAINER_MODULE_CPPM_INTERRUPT_FLAG)) {
    // capture mode on heartbeat pin (external module)
    capture = TRAINER_MODULE_CPPM_COUNTER_REGISTER;
    if (currentTrainerMode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) {
      doCapture = true;
    }
  }
#endif

  if (doCapture) {
    captureTrainerPulses(capture);
  }

#if defined(TRAINER_GPIO)
  // PPM out compare interrupt
  if ((TRAINER_TIMER->DIER & TRAINER_OUT_INTERRUPT_ENABLE) && (TRAINER_TIMER->SR & TRAINER_OUT_INTERRUPT_FLAG)) {
    // compare interrupt
    TRAINER_TIMER->DIER &= ~TRAINER_OUT_INTERRUPT_ENABLE; // stop this interrupt
    TRAINER_TIMER->SR &= ~TRAINER_OUT_INTERRUPT_FLAG; // Clear flag
    setupPulsesPPMTrainer();
    trainerSendNextFrame();
  }
#endif

#if defined(TRAINER_GPIO) && !defined(TRAINER_DMA_STREAM)  // PPM out update interrupt
  if ((TRAINER_TIMER->DIER & TIM_DIER_UIE) && (TRAINER_TIMER->SR & TIM_SR_UIF)) {
    TRAINER_TIMER->SR &= ~TIM_SR_UIF; // Clear flag
    TRAINER_TIMER->ARR = *trainerPulsesData.ppm.ptr++;
    if (*trainerPulsesData.ppm.ptr == 0) {
      TRAINER_TIMER->SR &= ~TRAINER_OUT_INTERRUPT_FLAG; // Clear this flag
      TRAINER_TIMER->DIER |= TRAINER_OUT_INTERRUPT_ENABLE; // Enable this interrupt
    }
  }
#endif
}

#if defined(TRAINER_MODULE_CPPM_TIMER_IRQHandler)
extern "C" void TRAINER_MODULE_CPPM_TIMER_IRQHandler()
{
  uint16_t capture = 0;
  bool doCapture = false;

  if ((TRAINER_MODULE_CPPM_TIMER->DIER & TRAINER_MODULE_CPPM_INTERRUPT_ENABLE) && (TRAINER_MODULE_CPPM_TIMER->SR & TRAINER_MODULE_CPPM_INTERRUPT_FLAG)) {
    // capture mode on external module
    capture = TRAINER_MODULE_CPPM_COUNTER_REGISTER;
    if (currentTrainerMode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) {
      doCapture = true;
    }
  }

  if (doCapture) {
    captureTrainerPulses(capture);
  }
}
#endif

#if defined(TRAINER_MODULE_CPPM)
void init_trainer_module_cppm()
{
  EXTERNAL_MODULE_ON();

  GPIO_PinAFConfig(TRAINER_MODULE_CPPM_GPIO, TRAINER_MODULE_CPPM_GPIO_PinSource, TRAINER_MODULE_CPPM_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TRAINER_MODULE_CPPM_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TRAINER_MODULE_CPPM_GPIO, &GPIO_InitStructure);

  TRAINER_MODULE_CPPM_TIMER->ARR = 0xFFFF;
  TRAINER_MODULE_CPPM_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 2000000 - 1; // 0.5uS
  TRAINER_MODULE_CPPM_TIMER->CR2 = 0;
#if defined(TRAINER_MODULE_CPPM_CCMR1)
  TRAINER_MODULE_CPPM_TIMER->CCMR1 = TRAINER_MODULE_CPPM_CCMR1;
#elif defined(TRAINER_MODULE_CPPM_CCMR2)
  TRAINER_MODULE_CPPM_TIMER->CCMR2 = TRAINER_MODULE_CPPM_CCMR2;
#endif
  TRAINER_MODULE_CPPM_TIMER->CCER = TRAINER_MODULE_CPPM_CCER;
  TRAINER_MODULE_CPPM_TIMER->SR &= ~TRAINER_MODULE_CPPM_INTERRUPT_FLAG; // Clear flag
  TRAINER_MODULE_CPPM_TIMER->DIER |= TRAINER_MODULE_CPPM_INTERRUPT_ENABLE;
  TRAINER_MODULE_CPPM_TIMER->CR1 = TIM_CR1_CEN;

  NVIC_SetPriority(TRAINER_MODULE_CPPM_TIMER_IRQn, 7);
  NVIC_EnableIRQ(TRAINER_MODULE_CPPM_TIMER_IRQn);
}

void stop_trainer_module_cppm()
{
  TRAINER_MODULE_CPPM_TIMER->DIER = 0;
  TRAINER_MODULE_CPPM_TIMER->CR1 &= ~TIM_CR1_CEN;                             // Stop counter
  NVIC_DisableIRQ(TRAINER_MODULE_CPPM_TIMER_IRQn);                            // Stop Interrupt

  if (!IS_EXTERNAL_MODULE_ENABLED()) {
    EXTERNAL_MODULE_OFF();
  }
}
#endif

// TODO: use external module serial driver
#if defined(TRAINER_MODULE_SBUS)

static DMAFifo<32> trainerSbusFifo __DMA (TRAINER_MODULE_SBUS_DMA_STREAM);

void init_trainer_module_sbus()
{
  EXTERNAL_MODULE_ON();

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(TRAINER_MODULE_SBUS_GPIO, TRAINER_MODULE_SBUS_GPIO_PinSource, TRAINER_MODULE_SBUS_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = TRAINER_MODULE_SBUS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TRAINER_MODULE_SBUS_GPIO, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = SBUS_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1; // this should be 2 stop bits
  USART_InitStructure.USART_Parity = USART_Parity_Even;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(TRAINER_MODULE_SBUS_USART, &USART_InitStructure);

  DMA_InitTypeDef DMA_InitStructure;
  trainerSbusFifo.clear();
  USART_ITConfig(TRAINER_MODULE_SBUS_USART, USART_IT_RXNE, DISABLE);
  USART_ITConfig(TRAINER_MODULE_SBUS_USART, USART_IT_TXE, DISABLE);
  DMA_InitStructure.DMA_Channel = TRAINER_MODULE_SBUS_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&TRAINER_MODULE_SBUS_USART->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(trainerSbusFifo.buffer());
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = trainerSbusFifo.size();
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(TRAINER_MODULE_SBUS_DMA_STREAM, &DMA_InitStructure);
  USART_DMACmd(TRAINER_MODULE_SBUS_USART, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(TRAINER_MODULE_SBUS_USART, ENABLE);
  DMA_Cmd(TRAINER_MODULE_SBUS_DMA_STREAM, ENABLE);
}

void init_trainer_module_ibus()
{
  EXTERNAL_MODULE_ON();

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(TRAINER_MODULE_SBUS_GPIO, TRAINER_MODULE_SBUS_GPIO_PinSource, TRAINER_MODULE_SBUS_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = TRAINER_MODULE_SBUS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TRAINER_MODULE_SBUS_GPIO, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = IBUS_BAUDRATE; 
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(TRAINER_MODULE_SBUS_USART, &USART_InitStructure);

  DMA_InitTypeDef DMA_InitStructure;
  trainerSbusFifo.clear();
  USART_ITConfig(TRAINER_MODULE_SBUS_USART, USART_IT_RXNE, DISABLE);
  USART_ITConfig(TRAINER_MODULE_SBUS_USART, USART_IT_TXE, DISABLE);
  DMA_InitStructure.DMA_Channel = TRAINER_MODULE_SBUS_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&TRAINER_MODULE_SBUS_USART->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(trainerSbusFifo.buffer());
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = trainerSbusFifo.size();
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(TRAINER_MODULE_SBUS_DMA_STREAM, &DMA_InitStructure);
  USART_DMACmd(TRAINER_MODULE_SBUS_USART, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(TRAINER_MODULE_SBUS_USART, ENABLE);
  DMA_Cmd(TRAINER_MODULE_SBUS_DMA_STREAM, ENABLE);
}

void stop_trainer_module_sbus()
{
  DMA_Cmd(TRAINER_MODULE_SBUS_DMA_STREAM, DISABLE);
  USART_Cmd(TRAINER_MODULE_SBUS_USART, DISABLE);
  USART_DMACmd(TRAINER_MODULE_SBUS_USART, USART_DMAReq_Rx, DISABLE);
  DMA_DeInit(TRAINER_MODULE_SBUS_DMA_STREAM);

  if (!IS_EXTERNAL_MODULE_ENABLED()) {
    EXTERNAL_MODULE_OFF();
  }
}

int trainerModuleSbusGetByte(uint8_t* byte)
{
  return trainerSbusFifo.pop(*byte);
}
#endif
