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
#include "extmodule_serial_driver.h"
#include "board.h"

#if defined(EXTMODULE_USART)
#include "stm32_usart_driver.h"
#include "fifo.h"

typedef Fifo<uint8_t, INTMODULE_FIFO_SIZE> RxFifo;
static RxFifo extmoduleFifo;

struct ExtmoduleCtx
{
  RxFifo* rxFifo;
  const stm32_usart_t* usart;
};

static etx_serial_callbacks_t extmodule_driver = {
  nullptr, nullptr, nullptr
};

void extmoduleFifoReceive(uint8_t data)
{
  extmoduleFifo.push(data);
}

static const etx_serial_init extmoduleSerialParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
};

static const LL_GPIO_InitTypeDef extmoduleUSART_PinDef = {
  .Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = EXTMODULE_USART_GPIO_AF_LL,
};

static const stm32_usart_t extmoduleUSART = {
  .USARTx = EXTMODULE_USART,
  .GPIOx = EXTMODULE_TX_GPIO,
  .pinInit = &extmoduleUSART_PinDef,
  .IRQn = EXTMODULE_USART_IRQn,
  .IRQ_Prio = EXTMODULE_USART_IRQ_PRIORITY,
  .txDMA = EXTMODULE_USART_TX_DMA,
  .txDMA_Stream = EXTMODULE_USART_TX_DMA_STREAM_LL,
  .txDMA_Channel = EXTMODULE_USART_TX_DMA_CHANNEL,
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
};

static const ExtmoduleCtx extmoduleCtx = {
  .rxFifo = &extmoduleFifo,
  .usart = &extmoduleUSART,
};

static void* extmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return nullptr;
    
  extmodule_driver.on_receive = extmoduleFifoReceive;
  extmodule_driver.on_error = nullptr;

  // UART config
  stm32_usart_init(&extmoduleUSART, params);
  extmoduleCtx.rxFifo->clear();

  return (void*)&extmoduleCtx;
}

void extmoduleInvertedSerialStart(uint32_t baudrate)
{
  EXTERNAL_MODULE_ON();
  etx_serial_init params(extmoduleSerialParams);
  params.baudrate = baudrate;
  extmoduleSerialStart(&params);
}

void extmoduleSerialStop(void*)
{
  stm32_usart_deinit(&extmoduleUSART);

  // reset callbacks
  extmodule_driver.on_receive = nullptr;
  extmodule_driver.on_error = nullptr;
}

static void extmoduleSendByte(void* ctx, uint8_t byte)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  stm32_usart_send_byte(modCtx->usart, byte);
}

static void extmoduleSendBuffer(void* ctx, const uint8_t * data, uint8_t size)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  if (size == 0) return;
  stm32_usart_send_buffer(modCtx->usart, data, size);
}

static void extmoduleWaitForTxCompleted(void* ctx)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  stm32_usart_wait_for_tx_dma(modCtx->usart);
}

static int extmoduleGetByte(void* ctx, uint8_t* data)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return -1;
  return modCtx->rxFifo->pop(*data);
}

static void extmoduleClearRxBuffer(void* ctx)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return;
  modCtx->rxFifo->clear();
}

const etx_serial_driver_t ExtmoduleSerialDriver = {
  .init = extmoduleSerialStart,
  .deinit = extmoduleSerialStop,
  .sendByte = extmoduleSendByte,
  .sendBuffer = extmoduleSendBuffer,
  .waitForTxCompleted = extmoduleWaitForTxCompleted,
  .getByte = extmoduleGetByte,
  .clearRxBuffer = extmoduleClearRxBuffer,
  .getBaudrate = nullptr,
  .setReceiveCb = nullptr,
  .setBaudrateCb = nullptr,
};

extern "C" void EXTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(&extmoduleUSART, &extmodule_driver);
}

#endif // defined(EXTMODULE_USART)
