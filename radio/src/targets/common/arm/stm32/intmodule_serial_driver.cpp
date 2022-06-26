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

#include "fifo.h"
#include "dmafifo.h"

#if defined(INTMODULE_RX_DMA)
typedef DMAFifo<INTMODULE_FIFO_SIZE> RxFifo;
static RxFifo intmoduleFifo __DMA(
    __LL_DMA_GET_STREAM_INSTANCE(INTMODULE_RX_DMA, INTMODULE_RX_DMA_STREAM));
#else
typedef Fifo<uint8_t, INTMODULE_FIFO_SIZE> RxFifo;
static RxFifo intmoduleFifo;
#endif

struct IntmoduleCtx
{
  RxFifo* rxFifo;
  const stm32_usart_t* usart;
};

#if !defined(INTMODULE_DMA_STREAM)
static uint8_t * intmoduleTxBufferData;
static volatile uint8_t intmoduleTxBufferRemaining;

uint8_t intmoduleTxBufferSend(uint8_t* data)
{
  if (intmoduleTxBufferRemaining) {
    *data = *(intmoduleTxBufferData++);
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

#if !defined(INTMODULE_RX_DMA)
// TODO: move this somewhere else
static void intmoduleFifoReceive(uint8_t data)
{
  intmoduleFifo.push(data);
}
#endif

static const LL_GPIO_InitTypeDef intmoduleUSART_PinDef = {
  .Pin = INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = INTMODULE_GPIO_AF,
};

static const stm32_usart_t intmoduleUSART = {
  .USARTx = INTMODULE_USART,
  .GPIOx = INTMODULE_GPIO,
  .pinInit = &intmoduleUSART_PinDef,
  .IRQn = INTMODULE_USART_IRQn,
  .IRQ_Prio = INTMODULE_USART_IRQ_PRIORITY,
#if defined(INTMODULE_DMA)
  .txDMA = INTMODULE_DMA,
  .txDMA_Stream = INTMODULE_DMA_STREAM,
  .txDMA_Channel = INTMODULE_DMA_CHANNEL,
#else
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
#endif
#if defined(INTMODULE_RX_DMA)
  .rxDMA = INTMODULE_RX_DMA,
  .rxDMA_Stream = INTMODULE_RX_DMA_STREAM,
  .rxDMA_Channel = INTMODULE_RX_DMA_CHANNEL,
#else
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
#endif
};

static const IntmoduleCtx intmoduleCtx = {
  .rxFifo = &intmoduleFifo,
  .usart = &intmoduleUSART,
};

void intmoduleStop()
{
  stm32_usart_deinit(&intmoduleUSART);

  // reset callbacks
  intmodule_driver.on_receive = nullptr;
  intmodule_driver.on_error = nullptr;
}

static void intmoduleStop(void* ctx)
{
  (void)ctx;
  intmoduleStop();
}

void* intmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return nullptr;

  // TODO: sanity check parameters
  //  - the UART seems to block when initialised with baudrate = 0

  // init callbacks
#if !defined(INTMODULE_RX_DMA)
  intmodule_driver.on_receive = intmoduleFifoReceive;
#else
  intmodule_driver.on_receive = nullptr;
#endif
  intmodule_driver.on_error = nullptr;

  stm32_usart_init(&intmoduleUSART, params);

  intmoduleCtx.rxFifo->clear();
  if (params->rx_enable && intmoduleUSART.rxDMA) {
    stm32_usart_init_rx_dma(&intmoduleUSART, intmoduleFifo.buffer(), intmoduleFifo.size());
  }  
  
  return (void*)&intmoduleCtx;
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void INTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(&intmoduleUSART, &intmodule_driver);
}

void intmoduleSendByte(void* ctx, uint8_t byte)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  stm32_usart_send_byte(modCtx->usart, byte);
}

void intmoduleSendBuffer(void* ctx, const uint8_t * data, uint8_t size)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (size == 0)
    return;

#if !defined(INTMODULE_DMA_STREAM)
  intmoduleTxBufferData = (uint8_t *)data;
  intmoduleTxBufferRemaining = size;
#endif
  stm32_usart_send_buffer(modCtx->usart, data, size);
}

void intmoduleWaitForTxCompleted(void* ctx)
{
  auto modCtx = (IntmoduleCtx*)ctx;
#if defined(INTMODULE_DMA_STREAM)
  stm32_usart_wait_for_tx_dma(modCtx->usart);
#else
  while (intmoduleTxBufferRemaining > 0);
#endif
}

static int intmoduleGetByte(void* ctx, uint8_t* data)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return -1;
  return modCtx->rxFifo->pop(*data);
}

static void intmoduleClearRxBuffer(void* ctx)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return;
  modCtx->rxFifo->clear();
}

const etx_serial_driver_t IntmoduleSerialDriver = {
  .init = intmoduleSerialStart,
  .deinit = intmoduleStop,
  .sendByte = intmoduleSendByte,
  .sendBuffer = intmoduleSendBuffer,
  .waitForTxCompleted = intmoduleWaitForTxCompleted,
  .getByte = intmoduleGetByte,
  .clearRxBuffer = intmoduleClearRxBuffer,
  .getBaudrate = nullptr,
  .setReceiveCb = nullptr,
  .setBaudrateCb = nullptr,
};
