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

#include "extmodule_serial_driver.h"
#include "board.h"

#if defined(EXTMODULE_USART)
#include "stm32_usart_driver.h"

#include "io/frsky_pxx2.h"
ModuleFifo extmoduleFifo;

struct etx_serial_callbacks_t {
  void (*on_receive)(uint8_t data);
  void (*on_error)();
};

static etx_serial_callbacks_t extmodule_driver = { nullptr, nullptr };

void extmoduleFifoReceive(uint8_t data)
{
  extmoduleFifo.push(data);
}

void extmoduleFifoError()
{
  extmoduleFifo.errors++;
}

static const etx_serial_init extmoduleSerialParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
  .on_receive = extmoduleFifoReceive,
  .on_error = extmoduleFifoError,
};

static void extmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return;
    
  extmodule_driver.on_receive = params->on_receive;
  extmodule_driver.on_error = params->on_error;

  // TX + RX Pins
  GPIO_PinAFConfig(EXTMODULE_USART_GPIO, EXTMODULE_TX_GPIO_PinSource,
                   EXTMODULE_USART_GPIO_AF);
  GPIO_PinAFConfig(EXTMODULE_USART_GPIO, EXTMODULE_RX_GPIO_PinSource,
                   EXTMODULE_USART_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(EXTMODULE_USART_GPIO, &GPIO_InitStructure);

  // UART config
  stm32_usart_init(EXTMODULE_USART, params);
  extmoduleFifo.clear();

  USART_ITConfig(EXTMODULE_USART, USART_IT_RXNE, ENABLE);
  NVIC_SetPriority(EXTMODULE_USART_IRQn, 6);
  NVIC_EnableIRQ(EXTMODULE_USART_IRQn);
}

void extmoduleInvertedSerialStart(uint32_t baudrate)
{
  EXTERNAL_MODULE_ON();
  etx_serial_init params(extmoduleSerialParams);
  params.baudrate = baudrate;
  extmoduleSerialStart(&params);
}

#if defined(PXX1)
void extmodulePxx1SerialStart()
{
  EXTERNAL_MODULE_ON();
  etx_serial_init params(extmoduleSerialParams);
  params.baudrate = EXTMODULE_PXX1_SERIAL_BAUDRATE;
  ExtmoduleSerialDriver.init(&params);
}
#endif

void extmoduleSerialStop()
{
  DMA_DeInit(EXTMODULE_USART_TX_DMA_STREAM);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(EXTMODULE_USART_GPIO, &GPIO_InitStructure);

  USART_DeInit(EXTMODULE_USART);

  GPIO_ResetBits(EXTMODULE_USART_GPIO, EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN);

  // reset callbacks
  extmodule_driver.on_receive = nullptr;
  extmodule_driver.on_error = nullptr;
}

static void extmoduleSendByte(uint8_t byte)
{
  while (!(EXTMODULE_USART->SR & USART_SR_TXE));
  USART_SendData(EXTMODULE_USART, byte);
}

static void extmoduleSendBuffer(const uint8_t * data, uint8_t size)
{
  if (size == 0)
    return;

  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(EXTMODULE_USART_TX_DMA_STREAM);
  DMA_InitStructure.DMA_Channel = EXTMODULE_USART_TX_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&EXTMODULE_USART->DR);
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
  DMA_Init(EXTMODULE_USART_TX_DMA_STREAM, &DMA_InitStructure);
  DMA_Cmd(EXTMODULE_USART_TX_DMA_STREAM, ENABLE);
  USART_DMACmd(EXTMODULE_USART, USART_DMAReq_Tx, ENABLE);
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void EXTMODULE_USART_IRQHandler(void)
{
  // No ISR based send loop: DMA only

  // Receive
  uint32_t status = EXTMODULE_USART->SR;
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS)) {
    uint8_t data = EXTMODULE_USART->DR;
    if (status & USART_FLAG_ERRORS) {
      if (extmodule_driver.on_error)
        extmodule_driver.on_error();
    }
    else {
      if (extmodule_driver.on_receive)
        extmodule_driver.on_receive(data);
    }
    status = EXTMODULE_USART->SR;
  }
}

static void extmoduleWaitForTxCompleted()
{
#if !defined(SIMU)
  // TODO: check if everything is properly initialised, this seems to block when
  //       the port has been initialised with a zero baudrate
  if (DMA_GetCmdStatus(EXTMODULE_USART_TX_DMA_STREAM) == ENABLE) {
    while (DMA_GetFlagStatus(EXTMODULE_USART_TX_DMA_STREAM,
                             EXTMODULE_USART_TX_DMA_FLAG_TC) == RESET)
      ;
    DMA_ClearFlag(EXTMODULE_USART_TX_DMA_STREAM,
                  EXTMODULE_USART_TX_DMA_FLAG_TC);
  }
#endif
}

etx_serial_driver_t ExtmoduleSerialDriver = {
  .init = extmoduleSerialStart,
  .deinit = extmoduleSerialStop,
  .sendByte = extmoduleSendByte,
  .sendBuffer = extmoduleSendBuffer,
  .waitForTxCompleted = extmoduleWaitForTxCompleted,
};

#endif // defined(EXTMODULE_USART)
