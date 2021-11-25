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

#include "opentx.h"
#include "targets/horus/board.h"
#include "aux_serial_driver.h"

#if defined(AUX_SERIAL)
uint8_t auxSerialMode = UART_MODE_COUNT;  // Prevent debug output before port is setup
Fifo<uint8_t, 512> auxSerialTxFifo;

#if defined(AUX_SERIAL_DMA_Stream_RX)
AuxSerialRxFifo auxSerialRxFifo __DMA (AUX_SERIAL_DMA_Stream_RX);
#else
AuxSerialRxFifo auxSerialRxFifo;
#endif

const LL_GPIO_InitTypeDef auxUSARTPinInit = {
  .Pin = AUX_SERIAL_GPIO_PIN_TX | AUX_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX_SERIAL_GPIO_AF_LL,
};

const stm32_usart_t auxUSART = {
  .USARTx = AUX_SERIAL_USART,
  .GPIOx = AUX_SERIAL_GPIO,
  .pinInit = &auxUSARTPinInit,
  .IRQn = AUX_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
#if defined(AUX_SERIAL_DMA_Stream_RX)
  .rxDMA = AUX_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX_SERIAL_DMA_Channel_RX,
#else
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
#endif
};

static void auxSerialOnSend(uint8_t& data)
{
  return auxSerialTxFifo.pop(data);
}

static etx_serial_callbacks auxSerialCb = {
  .on_send = auxSerialOnSend,
  .on_receive = nullptr,
  .on_error = nullptr,
};

void auxSerialSetup(unsigned int baudrate, bool rx_enable, uint8_t word_length,
                    uint8_t parity, uint8_t stop_bits)
{
#if defined(AUX_SERIAL_PWR_GPIO)
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = AUX_SERIAL_PWR_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(AUX_SERIAL_PWR_GPIO, &pinInit);
#endif

#if defined(AUX_SERIAL_TX_INVERT_GPIO_PIN) // NV14
  // TODO
  GPIO_InitStructure.GPIO_Pin = AUX_SERIAL_TX_INVERT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(AUX_SERIAL_TX_INVERT_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(AUX_SERIAL_TX_INVERT_GPIO, AUX_SERIAL_TX_INVERT_GPIO_PIN);
#endif

#if defined(AUX_SERIAL_RX_INVERT_GPIO_PIN) // NV14
  // TODO
  GPIO_InitStructure.GPIO_Pin = AUX_SERIAL_RX_INVERT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(AUX_SERIAL_RX_INVERT_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(AUX_SERIAL_RX_INVERT_GPIO, AUX_SERIAL_RX_INVERT_GPIO_PIN);
#endif

  etx_serial_init serialInit = {
    .baudrate = baudrate,
    .parity = parity,
    .stop_bits = stop_bits,
    .word_length = word_length,
    .rx_enable = rx_enable,
    .rx_dma_buf = nullptr,
    .rx_dma_buf_len = 0,
    .on_receive = nullptr, // TODO
    .on_error = nullptr,   // TODO
  };

#if defined(AUX_SERIAL_DMA_Stream_RX)
  if (rx_enable) {
    auxSerialRxFifo.clear();
    serialInit.rx_dma_buf = auxSerialRxFifo.buffer();
    serialInit.rx_dma_buf_len = auxSerialRxFifo.size();
  }
#endif
  
  stm32_usart_init(&auxUSART, &serialInit);
}

extern "C" void AUX_SERIAL_USART_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_SER2);
  stm32_usart_isr(&auxUSART, &auxSerialCb);
}

void auxSerialPutc(char c)
{
  if (auxSerialTxFifo.isFull()) return;
  auxSerialTxFifo.push(c);
  stm32_usart_send_buffer(&auxUSART, &c, 1);
}

void auxSerialStop()
{
  stm32_usart_deinit(&auxUSART);
}

static void serialPushLua(uint8_t data)
{
  if (luaRxFifo)
    luaRxFifo->push(data);
}

void auxSerialInit(unsigned int mode, unsigned int protocol)
{
  auxSerialStop();

  auxSerialMode = mode;
  auxSerialCb.on_receive = nullptr;

  switch (mode) {
    case UART_MODE_TELEMETRY_MIRROR:
      // TODO: add a setting for the telemetry protocol ???
      // -> how do we know the baudrate?
#if defined(CROSSFIRE)
      if (protocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
        auxSerialSetup(CROSSFIRE_TELEM_MIRROR_BAUDRATE, false);
        AUX_SERIAL_POWER_ON();
        break;
      }
#endif
      auxSerialSetup(FRSKY_TELEM_MIRROR_BAUDRATE, false);
      AUX_SERIAL_POWER_ON();
      break;

#if defined(DEBUG) || defined(CLI)
    case UART_MODE_DEBUG:
      auxSerialSetup(DEBUG_BAUDRATE, false);
      AUX_SERIAL_POWER_OFF();
      break;
#endif

    case UART_MODE_TELEMETRY:
      if (protocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
        auxSerialSetup(FRSKY_D_BAUDRATE, true);
        AUX_SERIAL_POWER_ON();
      }
      break;

    case UART_MODE_SBUS_TRAINER:
      auxSerialSetup(
          SBUS_BAUDRATE, true, ETX_WordLength_9, ETX_Parity_Even,
          ETX_StopBits_Two);  // USART_WordLength_9b due to parity bit
      AUX_SERIAL_POWER_ON();
      break;

#if defined(LUA)
    case UART_MODE_LUA:
      auxSerialSetup(LUA_DEFAULT_BAUDRATE, false);
      auxSerialCb.on_receive = serialPushLua;
      AUX_SERIAL_POWER_ON();
      break;
#endif
  }
}

void auxSerialSbusInit()
{
  auxSerialInit(UART_MODE_SBUS_TRAINER, 0);
}

uint8_t auxSerialTracesEnabled()
{
#if defined(DEBUG)
  return (auxSerialMode == UART_MODE_DEBUG);
#else
  return false;
#endif
}
#endif // AUX_SERIAL

#if defined(AUX2_SERIAL)
uint8_t aux2SerialMode = UART_MODE_COUNT;  // Prevent debug output before port is setup
Fifo<uint8_t, 512> aux2SerialTxFifo;
AuxSerialRxFifo aux2SerialRxFifo __DMA (AUX2_SERIAL_DMA_Stream_RX);

const LL_GPIO_InitTypeDef auxUSARTPinInit = {
  .Pin = AUX2_SERIAL_GPIO_PIN_TX | AUX2_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX2_SERIAL_GPIO_AF_LL,
};

const stm32_usart_t aux2USART = {
  .USARTx = AUX2_SERIAL_USART,
  .GPIOx = AUX2_SERIAL_GPIO,
  .pinInit = &auxUSARTPinInit,
  .IRQn = AUX2_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = AUX2_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX2_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX2_SERIAL_DMA_Channel_RX,
};

static void aux2SerialOnSend(uint8_t& data)
{
  return aux2SerialTxFifo.pop(data);
}

static etx_serial_callbacks aux2SerialCb = {
  .on_send = aux2SerialOnSend,
  .on_receive = nullptr,
  .on_error = nullptr,
};

void aux2SerialSetup(unsigned int baudrate, bool rx_enable, uint8_t word_length,
                     uint8_t parity, uint8_t stop_bits)
{
  etx_serial_init serialInit = {
    .baudrate = baudrate,
    .parity = parity,
    .stop_bits = stop_bits,
    .word_length = word_length,
    .rx_enable = rx_enable,
    .rx_dma_buf = nullptr,
    .rx_dma_buf_len = 0,
    .on_receive = nullptr, // TODO
    .on_error = nullptr,   // TODO
  };

  if (rx_enable) {
    aux2SerialRxFifo.clear();
    serialInit.rx_dma_buf = aux2SerialRxFifo.buffer();
    serialInit.rx_dma_buf_len = aux2SerialRxFifo.size();
  }

  stm32_usart_init(&aux2USART, &serialInit);
  
#if defined(AUX2_SERIAL_PWR_GPIO)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = AUX2_SERIAL_PWR_GPIO_PIN;
  GPIO_Init(AUX2_SERIAL_PWR_GPIO, &GPIO_InitStructure);
#endif
}

extern "C" void AUX2_SERIAL_USART_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_SER2);
  stm32_usart_isr(&aux2USART, &aux2SerialCb);
}

void aux2SerialPutc(char c)
{
  if (aux2SerialTxFifo.isFull()) return;
  aux2SerialTxFifo.push(c);
  stm32_usart_send_buffer(&aux2USART, &c, 1);
}

void aux2SerialStop()
{
  stm32_usart_deinit(&aux2USART);
}

void aux2SerialInit(unsigned int mode, unsigned int protocol)
{
  aux2SerialStop();

  aux2SerialMode = mode;
  aux2SerialCb.on_receive = nullptr;

  switch (mode) {
    case UART_MODE_TELEMETRY_MIRROR:
#if defined(CROSSFIRE)
      if (protocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
        aux2SerialSetup(CROSSFIRE_TELEM_MIRROR_BAUDRATE, false);
        AUX2_SERIAL_POWER_ON();
        break;
      }
#endif
      aux2SerialSetup(FRSKY_TELEM_MIRROR_BAUDRATE, false);
      AUX2_SERIAL_POWER_ON();
      break;

#if defined(DEBUG) || defined(CLI)
    case UART_MODE_DEBUG:
      aux2SerialSetup(DEBUG_BAUDRATE, false);
      AUX2_SERIAL_POWER_OFF();
      break;
#endif

    case UART_MODE_TELEMETRY:
      if (protocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
        aux2SerialSetup(FRSKY_D_BAUDRATE, true);
        AUX2_SERIAL_POWER_ON();
      }
      break;

    case UART_MODE_SBUS_TRAINER:
      aux2SerialSetup(
          SBUS_BAUDRATE, true, ETX_WordLength_9, ETX_Parity_Even,
          ETX_StopBits_Two);  // 2 stop bits requires USART_WordLength_9b
      AUX2_SERIAL_POWER_ON();
      break;

#if defined(LUA)
    case UART_MODE_LUA:
      aux2SerialSetup(LUA_DEFAULT_BAUDRATE, false);
      aux2SerialCb.on_receive = serialPushLua;
      AUX2_SERIAL_POWER_ON();
      break;
#endif
  }
}

void aux2SerialSbusInit()
{
  aux2SerialInit(UART_MODE_SBUS_TRAINER, 0);
}

uint8_t aux2SerialTracesEnabled()
{
#if defined(DEBUG)
  return (aux2SerialMode == UART_MODE_DEBUG);
#else
  return false;
#endif
}

#endif // AUX2_SERIAL
