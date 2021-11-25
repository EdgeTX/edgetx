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

#include "io/frsky_pxx2.h"
ModuleFifo extmoduleFifo;

static etx_serial_callbacks_t extmodule_driver = {
  nullptr, nullptr, nullptr
};

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
  .rx_dma_buf = nullptr,
  .rx_dma_buf_len = 0,
  .on_receive = extmoduleFifoReceive,
  .on_error = extmoduleFifoError,
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

static void extmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return;
    
  extmodule_driver.on_receive = params->on_receive;
  extmodule_driver.on_error = params->on_error;

  // UART config
  stm32_usart_init(&extmoduleUSART, params);
  extmoduleFifo.clear();
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
  stm32_usart_deinit(&extmoduleUSART);

  // reset callbacks
  extmodule_driver.on_receive = nullptr;
  extmodule_driver.on_error = nullptr;
}

static void extmoduleSendByte(uint8_t byte)
{
  stm32_usart_send_byte(&extmoduleUSART, byte);
}

static void extmoduleSendBuffer(const uint8_t * data, uint8_t size)
{
  if (size == 0) return;
  stm32_usart_send_buffer(&extmoduleUSART, data, size);
}

static void extmoduleWaitForTxCompleted()
{
  stm32_usart_wait_for_tx_dma(&extmoduleUSART);
}

const etx_serial_driver_t ExtmoduleSerialDriver = {
  .init = extmoduleSerialStart,
  .deinit = extmoduleSerialStop,
  .sendByte = extmoduleSendByte,
  .sendBuffer = extmoduleSendBuffer,
  .waitForTxCompleted = extmoduleWaitForTxCompleted,
};

extern "C" void EXTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(&extmoduleUSART, &extmodule_driver);
}

#endif // defined(EXTMODULE_USART)
