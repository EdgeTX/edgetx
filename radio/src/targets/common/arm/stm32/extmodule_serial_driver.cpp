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
#include "stm32_usart_driver.h"
#include "extmodule_serial_driver.h"

#include "board.h"

#if 0
//#if defined(EXTMODULE_USART)

struct RxFifo {
  uint8_t* buf;
  uint16_t size;
  uint16_t widx;
  uint16_t ridx;
};

struct ExtmoduleCtx {
  RxFifo rxFifo;
  const stm32_usart_t* usart;
};

static etx_serial_callbacks_t extmodule_callbacks = {
  nullptr, nullptr, nullptr
};

static void _fifo_clear(const stm32_usart_t* usart, RxFifo& fifo)
{
  if (usart->rxDMA) {
    auto dma_len = LL_DMA_GetDataLength(usart->rxDMA, usart->rxDMA_Stream);
    fifo.ridx = fifo.size - dma_len;
  } else {
    fifo.widx = fifo.ridx = 0;
  }
}

static int _fifo_get_byte(const stm32_usart_t* usart, RxFifo& fifo, uint8_t* data)
{
  if (usart->rxDMA) {
    auto DMAx = usart->rxDMA;
    auto stream = usart->rxDMA_Stream;

    // DMA stream enabled?
    if (!LL_DMA_IsEnabledStream(DMAx, stream)) return 0;

    // Anything to read?
    auto dma_len = LL_DMA_GetDataLength(DMAx, stream);
    if (fifo.size - dma_len == fifo.ridx) return 0;
  } else {
    if (fifo.ridx == fifo.widx) return 0;
  }

  *data = fifo.buf[fifo.ridx];
  fifo.ridx = (fifo.ridx + 1) & (fifo.size - 1);

  return 1;
}

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
  .rxDMA = EXTMODULE_USART_TX_DMA,
  .rxDMA_Stream = EXTMODULE_USART_RX_DMA_STREAM_LL,
  .rxDMA_Channel = EXTMODULE_USART_RX_DMA_CHANNEL,
};

static uint8_t _rx_buffer[INTMODULE_FIFO_SIZE] __DMA;

static ExtmoduleCtx extmoduleCtx = {
  .rxFifo = { _rx_buffer, sizeof(_rx_buffer), 0, 0 },
  .usart = &extmoduleUSART,
};

static void* extmoduleSerialInit(const etx_serial_init* params)
{
  if (!params) return nullptr;
    
  extmodule_callbacks.on_receive = nullptr;
  extmodule_callbacks.on_error = nullptr;

  // UART config
  auto usart = extmoduleCtx.usart;
  stm32_usart_init(usart, params);

  auto& fifo = extmoduleCtx.rxFifo;
  if (params->rx_enable && usart->rxDMA) {
    stm32_usart_init_rx_dma(usart, fifo.buf, fifo.size);
  }
  _fifo_clear(usart, fifo);

  return (void*)&extmoduleCtx;
}

void extmoduleSerialStop(void* ctx)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  stm32_usart_deinit(modCtx->usart);

  // reset callbacks
  extmodule_callbacks.on_receive = nullptr;
  extmodule_callbacks.on_error = nullptr;
}

static void extmoduleSerialSendByte(void* ctx, uint8_t byte)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  stm32_usart_send_byte(modCtx->usart, byte);
}

static void extmoduleSerialSendBuffer(void* ctx, const uint8_t * data, uint8_t size)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  if (size == 0) return;
  stm32_usart_send_buffer(modCtx->usart, data, size);
}

static void extmoduleSerialWaitForTxCompleted(void* ctx)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  stm32_usart_wait_for_tx_dma(modCtx->usart);
}

static int extmoduleSerialGetByte(void* ctx, uint8_t* data)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  return _fifo_get_byte(modCtx->usart, modCtx->rxFifo, data);
}

static void extmoduleSerialClearRxBuffer(void* ctx)
{
  auto modCtx = (ExtmoduleCtx*)ctx;
  _fifo_clear(modCtx->usart, modCtx->rxFifo);
}

static const etx_serial_driver_t extmoduleSerialDriver = {
  .init = extmoduleSerialInit,
  .deinit = extmoduleSerialStop,
  .sendByte = extmoduleSerialSendByte,
  .sendBuffer = extmoduleSerialSendBuffer,
  .waitForTxCompleted = extmoduleSerialWaitForTxCompleted,
  .getByte = extmoduleSerialGetByte,
  .clearRxBuffer = extmoduleSerialClearRxBuffer,
  .getBaudrate = nullptr,
  .setReceiveCb = nullptr,
  .setBaudrateCb = nullptr,
};

constexpr const etx_serial_driver_t* _default_driver = &extmoduleSerialDriver;

// IRQ based RX/TX: probably obsolete now
extern "C" void EXTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(extmoduleCtx.usart, &extmodule_callbacks);
}

#else

constexpr const etx_serial_driver_t* _default_driver = nullptr;

#endif // defined(EXTMODULE_USART)

const etx_serial_driver_t* _extmodule_driver = _default_driver;
void* _extmodule_hw_def = nullptr; // TODO

void extmoduleSetSerialPort(const etx_serial_driver_t* drv, void* hw_def)
{
  if (drv) {
    _extmodule_driver = drv;
    _extmodule_hw_def = hw_def;
  } else {
    _extmodule_driver = _default_driver;
    _extmodule_hw_def = nullptr;
  }
}

bool extmoduleGetSerialPort(const etx_serial_driver_t*& drv, void*& hw_def)
{
  drv    = _extmodule_driver;
  hw_def = _extmodule_hw_def;

  return _extmodule_driver != nullptr;
}
