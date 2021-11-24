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
#include "intmodule_serial_driver.h"
#include "stm32_usart_driver.h"
#include "board.h"

#include "io/frsky_pxx2.h"
ModuleFifo intmoduleFifo;

#if !defined(INTMODULE_DMA_STREAM)
uint8_t * intmoduleTxBufferData;
volatile uint8_t intmoduleTxBufferRemaining;

bool intmoduleTxBufferSend(uint8_t& data)
{
  if (intmoduleTxBufferRemaining) {
    data = *(intmoduleTxBufferData++);
    intmoduleTxBufferRemaining--;
    return true;
  }

  // buffer is empty
  return false;
}
#endif

static etx_serial_callbacks_t intmodule_driver = {
#if !defined(INTMODULE_DMA_STREAM)
  intmoduleTxBufferSend,
#else
  nullptr,
#endif
  nullptr, nullptr
};

// TODO: move this somewhere else
void intmoduleFifoReceive(uint8_t data)
{
  intmoduleFifo.push(data);
}

void intmoduleFifoError()
{
  intmoduleFifo.errors++;
}

static const LL_GPIO_InitTypeDef intmoduleUSART_PinDef = {
  .Pin = INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = INTMODULE_GPIO_AF_LL,
};

static const stm32_usart_t intmoduleUSART = {
  .USARTx = INTMODULE_USART,
  .GPIOx = INTMODULE_GPIO,
  .pinInit = &intmoduleUSART_PinDef,
  .IRQn = INTMODULE_USART_IRQn,
  .IRQ_Prio = INTMODULE_USART_IRQ_PRIORITY,
  .DMAx = INTMODULE_DMA,
  .DMA_Stream = INTMODULE_DMA_STREAM_LL,
  .DMA_Channel = INTMODULE_DMA_CHANNEL,
};

void intmoduleStop()
{
  stm32_usart_deinit(&intmoduleUSART);

  // reset callbacks
  intmodule_driver.on_receive = nullptr;
  intmodule_driver.on_error = nullptr;
}

void intmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return;

  // TODO: sanity check parameters
  //  - the UART seems to block when initialised with baudrate = 0

  // init callbacks
  intmodule_driver.on_receive = params->on_receive;
  intmodule_driver.on_error = params->on_error;

  stm32_usart_init(&intmoduleUSART, params);
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void INTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(&intmoduleUSART, &intmodule_driver);
}

void intmoduleSendByte(uint8_t byte)
{
  stm32_usart_send_byte(&intmoduleUSART, byte);
}

void intmoduleSendBuffer(const uint8_t * data, uint8_t size)
{
  if (size == 0)
    return;

#if !defined(INTMODULE_DMA_STREAM)
  intmoduleTxBufferData = (uint8_t *)data;
  intmoduleTxBufferRemaining = size;
#endif
  stm32_usart_send_buffer(&intmoduleUSART, data, size);
}

void intmoduleWaitForTxCompleted()
{
#if defined(INTMODULE_DMA_STREAM)
  stm32_usart_wait_for_tx_dma(&intmoduleUSART);
#else
  while (intmoduleTxBufferRemaining > 0);
#endif
}

const etx_serial_driver_t IntmoduleSerialDriver = {
  .init = intmoduleSerialStart,
  .deinit = intmoduleStop,
  .sendByte = intmoduleSendByte,
  .sendBuffer = intmoduleSendBuffer,
  .waitForTxCompleted = intmoduleWaitForTxCompleted,
};
