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
#include "intmodule_serial_driver.h"

ModuleFifo intmoduleFifo;
#if !defined(INTMODULE_DMA_STREAM)
uint8_t * intmoduleTxBufferData;
volatile uint8_t intmoduleTxBufferRemaining;
#endif

struct etx_serial_driver {
  void (*on_receive)(uint8_t data);
  void (*on_error)();
};

static etx_serial_driver intmodule_driver = { nullptr, nullptr };

// TODO: move this somewhere else
static void intmoduleFifoReceive(uint8_t data)
{
  intmoduleFifo.push(data);
}

static void intmoduleFifoError()
{
  intmoduleFifo.errors++;
}

etx_serial_init::etx_serial_init():
  baudrate(0),
  parity(USART_Parity_No),
  stop_bits(USART_StopBits_1),
  word_length(USART_WordLength_8b),
  rx_enable(false),
  // TODO: this should not be needed
  on_receive(intmoduleFifoReceive),
  on_error(intmoduleFifoError)
{
}

void intmoduleStop()
{
  GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN);

#if defined(INTMODULE_DMA_STREAM)
  INTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN; // Disable DMA
#endif

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(INTMODULE_GPIO, &GPIO_InitStructure);

  USART_DeInit(INTMODULE_USART);

  GPIO_ResetBits(INTMODULE_GPIO, INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN);

  // reset callbacks
  intmodule_driver.on_receive = nullptr;
  intmodule_driver.on_error = nullptr;
}

void intmodulePxx1SerialStart()
{
  etx_serial_init params;
  params.baudrate = INTMODULE_PXX1_SERIAL_BAUDRATE;
  intmoduleSerialStart(&params);
}

void intmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return;
  
  INTERNAL_MODULE_ON();

  // init callbacks
  intmodule_driver.on_receive = params->on_receive;
  intmodule_driver.on_error = params->on_error;

  uint32_t baudrate = params->baudrate;

#if !defined(INTMODULE_DMA_STREAM)
  // IRQ based TX
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = INTMODULE_USART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // rco: workaround for high baudrate not supported with
  //      IRQ based TX.
  if (baudrate > 1870000) {
    TRACE("limiting baudrate to 1.87 MBit/s");
    baudrate = 1870000;
  }
#endif

  GPIO_PinAFConfig(INTMODULE_GPIO, INTMODULE_GPIO_PinSource_TX, INTMODULE_GPIO_AF);
  GPIO_PinAFConfig(INTMODULE_GPIO, INTMODULE_GPIO_PinSource_RX, INTMODULE_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(INTMODULE_GPIO, &GPIO_InitStructure);

  USART_DeInit(INTMODULE_USART);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_Parity = params->parity;
  USART_InitStructure.USART_StopBits = params->stop_bits;
  USART_InitStructure.USART_WordLength = params->word_length;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(INTMODULE_USART, &USART_InitStructure);
  USART_Cmd(INTMODULE_USART, ENABLE);

  if (params->rx_enable) {
    USART_ITConfig(INTMODULE_USART, USART_IT_RXNE, ENABLE);
    NVIC_SetPriority(INTMODULE_USART_IRQn, 6);
    NVIC_EnableIRQ(INTMODULE_USART_IRQn);
  }
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void INTMODULE_USART_IRQHandler(void)
{
#if !defined(INTMODULE_DMA_STREAM)
  // Send
  if (USART_GetITStatus(INTMODULE_USART, USART_IT_TXE) != RESET) {
    if (intmoduleTxBufferRemaining) {
      USART_SendData(INTMODULE_USART, intmoduleTxBufferData[0]);
      intmoduleTxBufferData++;
      intmoduleTxBufferRemaining--;
    }
    else {
      USART_ITConfig(INTMODULE_USART, USART_IT_TXE, DISABLE);
    }
  }
#endif

  // Receive
  uint32_t status = INTMODULE_USART->SR;
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS)) {
    uint8_t data = INTMODULE_USART->DR;
    if (status & USART_FLAG_ERRORS) {
      if (intmodule_driver.on_error)
        intmodule_driver.on_error();
    }
    else {
      if (intmodule_driver.on_receive)
        intmodule_driver.on_receive(data);
    }
    status = INTMODULE_USART->SR;
  }
}

void intmoduleSendByte(uint8_t byte)
{
  while (!(INTMODULE_USART->SR & USART_SR_TXE));
  USART_SendData(INTMODULE_USART, byte);
}

void intmoduleSendBuffer(const uint8_t * data, uint8_t size)
{
  if (size == 0)
    return;

#if defined(INTMODULE_DMA_STREAM)
  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(INTMODULE_DMA_STREAM);
  DMA_InitStructure.DMA_Channel = INTMODULE_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&INTMODULE_USART->DR);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(data);
  DMA_InitStructure.DMA_BufferSize = size;
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
  DMA_Init(INTMODULE_DMA_STREAM, &DMA_InitStructure);
  DMA_Cmd(INTMODULE_DMA_STREAM, ENABLE);
  USART_DMACmd(INTMODULE_USART, USART_DMAReq_Tx, ENABLE);
#else
  intmoduleTxBufferData = (uint8_t *)data;
  intmoduleTxBufferRemaining = size;
  USART_ITConfig(INTMODULE_USART, USART_IT_TXE, ENABLE);
#endif
}

void intmoduleWaitForTxCompleted()
{
#if !defined(SIMU)
#if defined(INTMODULE_DMA_STREAM)
  if (DMA_GetCmdStatus(INTMODULE_DMA_STREAM) == ENABLE) {
    while(DMA_GetFlagStatus(INTMODULE_DMA_STREAM, INTMODULE_DMA_FLAG_TC) == RESET);
    DMA_ClearFlag(INTMODULE_DMA_STREAM, INTMODULE_DMA_FLAG_TC);
  }
#else
  while (intmoduleTxBufferRemaining > 0);
#endif
#endif
}
