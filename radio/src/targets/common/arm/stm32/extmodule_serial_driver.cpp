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
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Pull = LL_GPIO_PULL_UP;
  pinInit.Alternate = EXTMODULE_USART_GPIO_AF_LL;
  LL_GPIO_Init(EXTMODULE_USART_GPIO, &pinInit);

  // UART config
  stm32_usart_init(EXTMODULE_USART, params);
  extmoduleFifo.clear();

  // Enable TX DMA request
  LL_USART_EnableDMAReq_TX(EXTMODULE_USART);

  // Enable RX IRQ
  LL_USART_EnableIT_RXNE(EXTMODULE_USART);

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

  // Reconfigure pin as output
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(EXTMODULE_TX_GPIO, &pinInit);

  LL_USART_DeInit(EXTMODULE_USART);
  LL_GPIO_ResetOutputPin(EXTMODULE_USART_GPIO,
                         EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN);

  // reset callbacks
  extmodule_driver.on_receive = nullptr;
  extmodule_driver.on_error = nullptr;
}

static void extmoduleSendByte(uint8_t byte)
{
  while (!LL_USART_IsActiveFlag_TXE(EXTMODULE_USART));
  LL_USART_TransmitData8(EXTMODULE_USART, byte);
}

static void extmoduleSendBuffer(const uint8_t * data, uint8_t size)
{
  if (size == 0) return;
  LL_DMA_DeInit(EXTMODULE_USART_TX_DMA, EXTMODULE_USART_TX_DMA_STREAM_LL);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);
  dmaInit.Channel = EXTMODULE_USART_TX_DMA_CHANNEL;
  dmaInit.PeriphOrM2MSrcAddress = CONVERT_PTR_UINT(&EXTMODULE_USART->DR);
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.MemoryOrM2MDstAddress = CONVERT_PTR_UINT(data);
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.NbData = size;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;

  LL_DMA_Init(EXTMODULE_USART_TX_DMA, EXTMODULE_USART_TX_DMA_STREAM_LL, &dmaInit);
  LL_DMA_EnableStream(EXTMODULE_USART_TX_DMA, EXTMODULE_USART_TX_DMA_STREAM_LL);
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void EXTMODULE_USART_IRQHandler(void)
{
  // No ISR based send loop: DMA only

  // Receive
  uint32_t status = LL_USART_ReadReg(EXTMODULE_USART, SR);
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS)) {
    uint8_t data = LL_USART_ReceiveData8(EXTMODULE_USART);
    if (status & USART_FLAG_ERRORS) {
      if (extmodule_driver.on_error)
        extmodule_driver.on_error();
    }
    else {
      if (extmodule_driver.on_receive)
        extmodule_driver.on_receive(data);
    }
    status = LL_USART_ReadReg(EXTMODULE_USART, SR);
  }
}

static void extmoduleWaitForTxCompleted()
{
  // TODO: check if everything is properly initialised, this seems to block when
  //       the port has been initialised with a zero baudrate
  if (LL_DMA_IsEnabledStream(EXTMODULE_USART_TX_DMA,
                             EXTMODULE_USART_TX_DMA_STREAM_LL)) {

    static_assert(EXTMODULE_USART_TX_DMA_STREAM_LL == LL_DMA_STREAM_3 ||
                  EXTMODULE_USART_TX_DMA_STREAM_LL == LL_DMA_STREAM_6 ||
                  EXTMODULE_USART_TX_DMA_STREAM_LL == LL_DMA_STREAM_7, "");    

    switch(EXTMODULE_USART_TX_DMA_STREAM_LL) {
    case LL_DMA_STREAM_3:
      while (LL_DMA_IsActiveFlag_TC3(EXTMODULE_USART_TX_DMA));
      LL_DMA_ClearFlag_TC3(EXTMODULE_USART_TX_DMA);
      break;
    case LL_DMA_STREAM_6:
      while (LL_DMA_IsActiveFlag_TC6(EXTMODULE_USART_TX_DMA));
      LL_DMA_ClearFlag_TC6(EXTMODULE_USART_TX_DMA);
      break;
    case LL_DMA_STREAM_7:
      while (LL_DMA_IsActiveFlag_TC7(EXTMODULE_USART_TX_DMA));
      LL_DMA_ClearFlag_TC7(EXTMODULE_USART_TX_DMA);
      break;
    }
  }
}

const etx_serial_driver_t ExtmoduleSerialDriver = {
  .init = extmoduleSerialStart,
  .deinit = extmoduleSerialStop,
  .sendByte = extmoduleSendByte,
  .sendBuffer = extmoduleSendBuffer,
  .waitForTxCompleted = extmoduleWaitForTxCompleted,
};

#endif // defined(EXTMODULE_USART)
