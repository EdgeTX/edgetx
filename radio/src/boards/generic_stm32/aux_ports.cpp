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

#include "hal/serial_port.h"
#include "stm32_serial_driver.h"

#include "board.h"
#include "dataconstants.h"

#define AUX_SERIAL_TX_BUFFER 512
#if defined(SDRAM)
  #define AUX_SERIAL_RX_BUFFER 512
#else
  #define AUX_SERIAL_RX_BUFFER 32
#endif

#if defined(AUX_SERIAL_PWR_GPIO) || defined(AUX2_SERIAL_PWR_GPIO)
static void _aux_pwr(GPIO_TypeDef *GPIOx, uint32_t pin, uint8_t on)
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOx, &pinInit);

  if (on) {
    LL_GPIO_SetOutputPin(GPIOx, pin);
  } else {
    LL_GPIO_ResetOutputPin(GPIOx, pin);
  }
}
#endif

#if defined(AUX_SERIAL)

static const LL_GPIO_InitTypeDef auxUSARTPinInit = {
  .Pin = AUX_SERIAL_GPIO_PIN_TX | AUX_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX_SERIAL_GPIO_AF_LL,
};

#if !defined(AUX_SERIAL_DMA_TX)
  #define AUX_SERIAL_DMA_TX                   nullptr
  #define AUX_SERIAL_DMA_Stream_TX            nullptr
  #define AUX_SERIAL_DMA_Stream_TX_LL         0
  #define AUX_SERIAL_DMA_Channel_TX           0
#endif

static const stm32_usart_t auxUSART = {
  .USARTx = AUX_SERIAL_USART,
  .GPIOx = AUX_SERIAL_GPIO,
  .pinInit = &auxUSARTPinInit,
  .IRQn = AUX_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = AUX_SERIAL_DMA_TX,
  .txDMA_Stream = AUX_SERIAL_DMA_Stream_TX_LL,
  .txDMA_Channel = AUX_SERIAL_DMA_Channel_TX,
  .rxDMA = AUX_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX_SERIAL_DMA_Channel_RX,
};

DEFINE_STM32_SERIAL_PORT(Aux, auxUSART, AUX_SERIAL_RX_BUFFER, AUX_SERIAL_TX_BUFFER);

#if defined(AUX_SERIAL_PWR_GPIO)
void set_aux_pwr(uint8_t on)
{
  _aux_pwr(AUX_SERIAL_PWR_GPIO, AUX_SERIAL_PWR_GPIO_PIN, on);
}
#define AUX_PWR_FCT set_aux_pwr
#else
#define AUX_PWR_FCT nullptr
#endif

const etx_serial_port_t auxSerialPort = {
  .name = "AUX1",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(Aux),
  AUX_PWR_FCT
};
#define AUX_SERIAL_PORT &auxSerialPort
#else
#define AUX_SERIAL_PORT nullptr
#endif

#if defined(AUX2_SERIAL)

static const LL_GPIO_InitTypeDef aux2USARTPinInit = {
  .Pin = AUX2_SERIAL_GPIO_PIN_TX | AUX2_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX2_SERIAL_GPIO_AF_LL,
};

static const stm32_usart_t aux2USART = {
  .USARTx = AUX2_SERIAL_USART,
  .GPIOx = AUX2_SERIAL_GPIO,
  .pinInit = &aux2USARTPinInit,
  .IRQn = AUX2_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = AUX2_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX2_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX2_SERIAL_DMA_Channel_RX,
};

DEFINE_STM32_SERIAL_PORT(Aux2, aux2USART, AUX_SERIAL_RX_BUFFER, AUX_SERIAL_TX_BUFFER);

#if defined(AUX2_SERIAL_PWR_GPIO)
void set_aux2_pwr(uint8_t on)
{
  _aux_pwr(AUX2_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO_PIN, on);
}
#define AUX2_PWR_FCT set_aux2_pwr
#else
#define AUX2_PWR_FCT nullptr
#endif

const etx_serial_port_t aux2SerialPort = {
  .name = "AUX2",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(Aux2),
  AUX2_PWR_FCT
};
#define AUX2_SERIAL_PORT &aux2SerialPort
#else
#define AUX2_SERIAL_PORT nullptr
#endif

static const etx_serial_port_t* serialPorts[MAX_AUX_SERIAL] = {
  AUX_SERIAL_PORT,
  AUX2_SERIAL_PORT,
};

const etx_serial_port_t* auxSerialGetPort(int port_nr)
{
  if (port_nr >= MAX_AUX_SERIAL) return nullptr;
  return serialPorts[port_nr];
}

