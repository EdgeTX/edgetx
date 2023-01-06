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

#include "stm32_hal_ll.h"
#include "stm32_exti_driver.h"
#include "aux_serial_driver.h"

#include "opentx.h"

#if defined(GHOST)
  #include "telemetry/ghost.h"
#endif

Fifo<uint8_t, TELEMETRY_FIFO_SIZE> telemetryNoDMAFifo;
uint32_t telemetryErrors = 0;

#if defined(PCBX12S) || defined (PCBNV14) || defined (PCBPL18)
#include "dmafifo.h"

DMAFifo<TELEMETRY_FIFO_SIZE> telemetryDMAFifo __DMA (TELEMETRY_DMA_Stream_RX);
uint8_t telemetryFifoMode;
#endif

static void telemetryInitDirPin()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin   = TELEMETRY_DIR_GPIO_PIN;
  GPIO_Init(TELEMETRY_DIR_GPIO, &GPIO_InitStructure);
  TELEMETRY_DIR_INPUT();
}

void telemetryPortInitCommon(uint32_t baudrate, uint8_t mode, uint8_t noinv = 0)
{
  if (baudrate == 0) {
    USART_DeInit(TELEMETRY_USART);
    return;
  }
  //deinit inverted mode
#if !(defined(PCBNV14) || defined(PCBPL18))
  telemetryPortInvertedInit(0);
#endif
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TELEMETRY_DMA_TX_Stream_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pre-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(TELEMETRY_GPIO, TELEMETRY_GPIO_PinSource_RX, TELEMETRY_GPIO_AF);
  GPIO_PinAFConfig(TELEMETRY_GPIO, TELEMETRY_GPIO_PinSource_TX, TELEMETRY_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = TELEMETRY_TX_GPIO_PIN | TELEMETRY_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = baudrate <= 400000 ? GPIO_Speed_2MHz : GPIO_Speed_25MHz;
  GPIO_Init(TELEMETRY_GPIO, &GPIO_InitStructure);

  telemetryInitDirPin();

#if defined(PCBNV14) || defined(PCBPL18)
  GPIO_InitStructure.GPIO_Pin = TELEMETRY_TX_INV_GPIO_PIN | TELEMETRY_RX_INV_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TELEMETRY_INV_GPIO, &GPIO_InitStructure);

  if (noinv != 0) {
    TELEMETRY_TX_NORM();
    TELEMETRY_RX_NORM();
  } else {
    TELEMETRY_TX_INV();
    TELEMETRY_RX_INV();
  }
#endif
  USART_DeInit(TELEMETRY_USART);

  USART_OverSampling8Cmd(TELEMETRY_USART, baudrate <= 400000 ? DISABLE : ENABLE);
  
  USART_InitStructure.USART_BaudRate = baudrate;
  if (mode & TELEMETRY_SERIAL_8E2) {
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_InitStructure.USART_Parity = USART_Parity_Even;
  }
  else {
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
  }
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  if (g_eeGeneral.uartSampleMode == UART_SAMPLE_MODE_ONEBIT) {
    USART_OneBitMethodCmd(TELEMETRY_USART, ENABLE);
  }
  USART_Init(TELEMETRY_USART, &USART_InitStructure);

#if defined(PCBX12S) || defined(PCBNV14) || defined(PCBPL18)
  telemetryFifoMode = mode;
  
  DMA_Cmd(TELEMETRY_DMA_Stream_RX, DISABLE);
  USART_DMACmd(TELEMETRY_USART, USART_DMAReq_Rx, DISABLE);
  DMA_DeInit(TELEMETRY_DMA_Stream_RX);

  if (mode & TELEMETRY_SERIAL_WITHOUT_DMA) {
    USART_Cmd(TELEMETRY_USART, ENABLE);
    USART_ITConfig(TELEMETRY_USART, USART_IT_RXNE, ENABLE);
    NVIC_SetPriority(TELEMETRY_USART_IRQn, 6);
    NVIC_EnableIRQ(TELEMETRY_USART_IRQn);
  }
  else {
    DMA_InitTypeDef DMA_InitStructure;
    telemetryDMAFifo.clear();
  
    USART_ITConfig(TELEMETRY_USART, USART_IT_RXNE, DISABLE);
    USART_ITConfig(TELEMETRY_USART, USART_IT_TXE, DISABLE);
    NVIC_SetPriority(TELEMETRY_USART_IRQn, 6);
    NVIC_EnableIRQ(TELEMETRY_USART_IRQn);
  
    DMA_InitStructure.DMA_Channel = TELEMETRY_DMA_Channel_RX;
    DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&TELEMETRY_USART->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(telemetryDMAFifo.buffer());
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = telemetryDMAFifo.size();
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
    DMA_Init(TELEMETRY_DMA_Stream_RX, &DMA_InitStructure);
    USART_DMACmd(TELEMETRY_USART, USART_DMAReq_Rx, ENABLE);
    USART_Cmd(TELEMETRY_USART, ENABLE);
    DMA_Cmd(TELEMETRY_DMA_Stream_RX, ENABLE);
  }
#else
  USART_Cmd(TELEMETRY_USART, ENABLE);
  USART_ITConfig(TELEMETRY_USART, USART_IT_RXNE, ENABLE);
  NVIC_SetPriority(TELEMETRY_USART_IRQn, 6);
  NVIC_EnableIRQ(TELEMETRY_USART_IRQn);
#endif
}

void telemetryPortInit(uint32_t baudrate, uint8_t mode)
{
  telemetryPortInitCommon(baudrate, mode, 0);
}
// soft serial vars
static uint8_t rxBitCount;
static uint8_t rxByte;
// single bit length expresses in half us
static uint16_t bitLength;
static uint16_t probeTimeFromStartBit;

#if !(defined(PCBNV14) || defined(PCBPL18))
static void do_telemetry_exti()
{
  if (rxBitCount == 0) {

    TELEMETRY_TIMER->ARR = probeTimeFromStartBit; // 1,5 cycle from start at 57600bps
    TELEMETRY_TIMER->CR1 |= TIM_CR1_CEN;
    
    // disable start bit interrupt
    EXTI->IMR &= ~EXTI_IMR_MR6;
  }
}
#endif

void telemetryPortInvertedInit(uint32_t baudrate)
{
#if defined(PCBNV14) || defined(PCBPL18)
  telemetryPortInitCommon(baudrate, TELEMETRY_SERIAL_DEFAULT, 1);
#else
  if (baudrate == 0) {

    stm32_exti_disable(TELEMETRY_EXTI_LINE);
    NVIC_DisableIRQ(TELEMETRY_TIMER_IRQn);
    return;
  }

  rxBitCount = 0;

  switch(baudrate) {
    case 115200:
      bitLength = 17;
      probeTimeFromStartBit = 23; //because pin is not probed immediately
//      probeTimeFromStartBit = 25; // TODO: Tarnis used 25, how to merge?
      break;
    case 57600:
      bitLength = 35; //34 was used before - I prefer to use use 35 because of lower error
      probeTimeFromStartBit = 48; // 48 used in original implementation
      break;
    default:
      bitLength = 2000000/baudrate; //because of 0,5 us  tick
      probeTimeFromStartBit = 3000000/baudrate;
  }

#if defined(PCBX12S)
  telemetryFifoMode = TELEMETRY_SERIAL_WITHOUT_DMA;
#endif
  
  // configure bit sample timer
  TELEMETRY_TIMER->PSC = (PERI2_FREQUENCY * TIMER_MULT_APB2) / 2000000 - 1; // 0.5uS
  TELEMETRY_TIMER->CCER = 0;
  TELEMETRY_TIMER->CCMR1 = 0;
  TELEMETRY_TIMER->CR1 = TIM_CR1_CEN;
  TELEMETRY_TIMER->DIER = TIM_DIER_UIE;

  NVIC_SetPriority(TELEMETRY_TIMER_IRQn, 0);
  NVIC_EnableIRQ(TELEMETRY_TIMER_IRQn);

  // init TELEMETRY_RX_GPIO_PIN
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = TELEMETRY_RX_GPIO_PIN;
  GPIO_Init(TELEMETRY_GPIO, &GPIO_InitStructure);

  telemetryInitDirPin();

  // Connect EXTI line to TELEMETRY RX pin
  LL_SYSCFG_SetEXTISource(TELEMETRY_EXTI_PORT, TELEMETRY_EXTI_SYS_LINE);

  // Configure EXTI for raising edge (start bit)
  stm32_exti_enable(TELEMETRY_EXTI_LINE, TELEMETRY_EXTI_TRIGGER, do_telemetry_exti);
#endif // defined(PCBNV14)
}

#if !(defined(PCBNV14) || defined(PCBPL18))
void telemetryPortInvertedRxBit()
{
  if (rxBitCount < 8) {
    if (rxBitCount == 0) {
      TELEMETRY_TIMER->ARR = bitLength;
      rxByte = 0;
    }
    else {
      rxByte >>= 1;
    }

    if (GPIO_ReadInputDataBit(TELEMETRY_GPIO, TELEMETRY_RX_GPIO_PIN) == Bit_RESET)
      rxByte |= 0x80;

    ++rxBitCount;
  }
  else if (rxBitCount == 8) {
    telemetryNoDMAFifo.push(rxByte);
    rxBitCount = 0;

    // disable timer
    TELEMETRY_TIMER->CR1 &= ~TIM_CR1_CEN;

    // re-enable start bit interrupt
    EXTI->IMR |= EXTI_IMR_MR6;
  }
}
#endif

void telemetryPortSetDirectionOutput()
{
#if defined(GHOST) && SPORT_MAX_BAUDRATE < 400000
  if (isModuleGhost(EXTERNAL_MODULE)) {
    TELEMETRY_USART->BRR = BRR_400K;
  }
#endif
  TELEMETRY_DIR_OUTPUT();
  TELEMETRY_USART->CR1 &= ~USART_CR1_RE;                  // turn off receiver
}

void sportWaitTransmissionComplete()
{
  while (!(TELEMETRY_USART->SR & USART_SR_TC));
}

void telemetryPortSetDirectionInput()
{
  sportWaitTransmissionComplete();
#if defined(GHOST) && SPORT_MAX_BAUDRATE < 400000
  if (isModuleGhost(EXTERNAL_MODULE) && g_model.moduleData[EXTERNAL_MODULE].ghost.telemetryBaudrate == GHST_TELEMETRY_RATE_115K) {
    TELEMETRY_USART->BRR = BRR_115K;
  }
#endif
  TELEMETRY_DIR_INPUT();
  TELEMETRY_USART->CR1 |= USART_CR1_RE;                   // turn on receiver
}

void sportSendByte(uint8_t byte)
{
  telemetryPortSetDirectionOutput();

  while (!(TELEMETRY_USART->SR & USART_SR_TXE));
  USART_SendData(TELEMETRY_USART, byte);
}

void sportStopSendByteLoop()
{
  DMA_Cmd(TELEMETRY_DMA_Stream_TX, DISABLE);
  DMA_DeInit(TELEMETRY_DMA_Stream_TX);
}
void sportSendByteLoop(uint8_t byte)
{
  telemetryPortSetDirectionOutput();

  outputTelemetryBuffer.data[0] = byte;

  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(TELEMETRY_DMA_Stream_TX);
  DMA_InitStructure.DMA_Channel = TELEMETRY_DMA_Channel_TX;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&TELEMETRY_USART->DR);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(outputTelemetryBuffer.data);
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(TELEMETRY_DMA_Stream_TX, &DMA_InitStructure);
  DMA_Cmd(TELEMETRY_DMA_Stream_TX, ENABLE);
  USART_DMACmd(TELEMETRY_USART, USART_DMAReq_Tx, ENABLE);
}

void sportSendBuffer(const uint8_t * buffer, uint32_t count)
{
  telemetryPortSetDirectionOutput();

  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(TELEMETRY_DMA_Stream_TX);
  DMA_InitStructure.DMA_Channel = TELEMETRY_DMA_Channel_TX;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&TELEMETRY_USART->DR);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(buffer);
  DMA_InitStructure.DMA_BufferSize = count;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(TELEMETRY_DMA_Stream_TX, &DMA_InitStructure);
  DMA_Cmd(TELEMETRY_DMA_Stream_TX, ENABLE);
  USART_DMACmd(TELEMETRY_USART, USART_DMAReq_Tx, ENABLE);
  DMA_ITConfig(TELEMETRY_DMA_Stream_TX, DMA_IT_TC, ENABLE);
//  USART_ClearITPendingBit(TELEMETRY_USART, USART_IT_TC);  // TODO: This requires?  Only appear in tarnis driver

  // enable interrupt and set it's priority
  NVIC_EnableIRQ(TELEMETRY_DMA_TX_Stream_IRQ) ;
  NVIC_SetPriority(TELEMETRY_DMA_TX_Stream_IRQ, 7);
}

extern "C" void TELEMETRY_DMA_TX_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_TELEM_DMA);
  if (DMA_GetITStatus(TELEMETRY_DMA_Stream_TX, TELEMETRY_DMA_TX_FLAG_TC)) {
    DMA_ClearITPendingBit(TELEMETRY_DMA_Stream_TX, TELEMETRY_DMA_TX_FLAG_TC);

    // clear TC flag before enabling interrupt
    TELEMETRY_USART->SR &= ~USART_SR_TC;
    TELEMETRY_USART->CR1 |= USART_CR1_TCIE;

    if (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT) {
      outputTelemetryBuffer.reset();
    }
  }
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void TELEMETRY_USART_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_TELEM_USART);
  uint32_t status = TELEMETRY_USART->SR;

  if ((status & USART_SR_TC) && (TELEMETRY_USART->CR1 & USART_CR1_TCIE)) {
    TELEMETRY_USART->CR1 &= ~USART_CR1_TCIE;
    telemetryPortSetDirectionInput();
    while (status & (USART_FLAG_RXNE)) {
      status = TELEMETRY_USART->DR;
      status = TELEMETRY_USART->SR;
    }
  }

  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS)) {
    uint8_t data = TELEMETRY_USART->DR;
    if (status & USART_FLAG_ERRORS) {
      telemetryErrors++;
    }
    else {
      telemetryNoDMAFifo.push(data);
#if defined(LUA)
      if (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_SPORT) {
        static uint8_t prevdata;
        if (prevdata == 0x7E && outputTelemetryBuffer.size > 0 && outputTelemetryBuffer.destination == TELEMETRY_ENDPOINT_SPORT && data == outputTelemetryBuffer.sport.physicalId) {
          sportSendBuffer(outputTelemetryBuffer.data + 1, outputTelemetryBuffer.size - 1);
        }
        prevdata = data;
      }
#endif
    }
    status = TELEMETRY_USART->SR;
  }
}

#if !(defined(PCBNV14) || defined(PCBPL18))
extern "C" void TELEMETRY_TIMER_IRQHandler()
{
  TELEMETRY_TIMER->SR &= ~TIM_SR_UIF;
  telemetryPortInvertedRxBit();
}
#endif

// TODO we should have telemetry in an higher layer, functions above should move to a sport_driver.cpp
bool sportGetByte(uint8_t * byte)
{
#if defined(PCBX12S) || defined(PCBNV14) || defined(PCBPL18)
  if (telemetryFifoMode & TELEMETRY_SERIAL_WITHOUT_DMA)
    return telemetryNoDMAFifo.pop(*byte);
  else
    return telemetryDMAFifo.pop(*byte);
#else
  return telemetryNoDMAFifo.pop(*byte);
#endif
}

void telemetryClearFifo()
{
#if defined(PCBX12S) || defined(PCBNV14) || defined(PCBPL18)
  telemetryDMAFifo.clear();
#endif

  telemetryNoDMAFifo.clear();
}

