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

Fifo<uint8_t, TELEMETRY_FIFO_SIZE> telemetryNoDMAFifo;
uint32_t telemetryErrors = 0;

#if defined(PCBX12S)
DMAFifo<TELEMETRY_FIFO_SIZE> telemetryDMAFifo __DMA (TELEMETRY_DMA_Stream_RX);
uint8_t telemetryFifoMode;
#endif

void telemetryPortInitCommon(uint32_t baudrate, uint8_t mode, uint8_t noinv = 0)
{
  if (baudrate == 0) {
    USART_DeInit(TELEMETRY_USART);
    return;
  }

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
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(TELEMETRY_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = TELEMETRY_DIR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TELEMETRY_DIR_GPIO, &GPIO_InitStructure);
  TELEMETRY_DIR_INPUT();
  
  GPIO_InitStructure.GPIO_Pin = TELEMETRY_TX_REV_GPIO_PIN | TELEMETRY_RX_REV_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TELEMETRY_REV_GPIO, &GPIO_InitStructure);

  if (noinv != 0) {
    TELEMETRY_TX_POL_NORM();
    TELEMETRY_RX_POL_NORM();
  } else {
    TELEMETRY_TX_POL_INV();
    TELEMETRY_RX_POL_INV();
  }

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
  USART_Init(TELEMETRY_USART, &USART_InitStructure);

  USART_Cmd(TELEMETRY_USART, ENABLE);
  USART_ITConfig(TELEMETRY_USART, USART_IT_RXNE, ENABLE);
  NVIC_SetPriority(TELEMETRY_USART_IRQn, 6);
  NVIC_EnableIRQ(TELEMETRY_USART_IRQn);
}

void telemetryPortInit(uint32_t baudrate, uint8_t mode)
{
  telemetryPortInitCommon(baudrate, mode, 0);
}

void telemetryPortInvertedInit(uint32_t baudrate)
{
  telemetryPortInitCommon(baudrate, TELEMETRY_SERIAL_DEFAULT, 1);
}

/*void extmoduleSendInvertedByte(uint8_t byte)
{
#warning "TODO extmoduleSendInvertedByte";
}*/

void telemetryPortSetDirectionOutput()
{
  TELEMETRY_DIR_OUTPUT();
  TELEMETRY_USART->CR1 &= ~USART_CR1_RE;                  // turn off receiver
}

void telemetryPortSetDirectionInput()
{
  TELEMETRY_DIR_INPUT();
  TELEMETRY_USART->CR1 |= USART_CR1_RE;                   // turn on receiver
}

void sportSendByte(uint8_t byte)
{
  telemetryPortSetDirectionOutput();

  while (!(TELEMETRY_USART->SR & USART_SR_TXE));
  USART_SendData(TELEMETRY_USART, byte);
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

// TODO we should have telemetry in an higher layer, functions above should move to a sport_driver.cpp
bool telemetryGetByte(uint8_t * byte)
{
#if defined(PCBX12S)
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
#if defined(PCBX12S)
  telemetryDMAFifo.clear();
#endif

  telemetryNoDMAFifo.clear();
}

