/*
 * Copyright (C) EdgeTx
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

#include "stm32_softserial_driver.h"
#include "stm32_exti_driver.h"

// getTmr2MHz()
#include "timers_driver.h"

// RX state
static uint8_t rxBitCount;
static uint8_t rxByte;

// RX FIFO
static volatile uint8_t rxRidx;
static volatile uint8_t rxWidx;
static uint8_t *rxBuffer;
static uint32_t rxBufLen;

// single bit length expresses in half us
static uint16_t bitLength;
static uint16_t probeTimeFromStartBit;

static const stm32_softserial_port* _softserialPort;

static void _softserial_exti()
{
  if (rxBitCount == 0) {

    // enable timer counter
    auto TIMx = _softserialPort->TIMx;
    LL_TIM_SetAutoReload(TIMx, probeTimeFromStartBit);
    LL_TIM_EnableCounter(TIMx);
    
    // disable start bit interrupt
    LL_EXTI_DisableIT_0_31(_softserialPort->EXTI_Line);
  }
}

static void _softserial_set_timings(uint32_t baudrate)
{
  switch(baudrate) {
    case 115200:
      bitLength = 17;
      probeTimeFromStartBit = 25;
      break;
    case 57600:
      bitLength = 35;
      probeTimeFromStartBit = 48;
      break;
    default:
      bitLength = 2000000 / baudrate; // because of 0,5 us tick
      probeTimeFromStartBit = 3000000 / baudrate;
  }
}

static inline void _fifo_clear()
{
  rxWidx = 0;
  rxRidx = 0;
}

static void _softserial_init_rx(const stm32_softserial_port* port, const etx_serial_init* params)
{
  rxBitCount = 0;
  rxBuffer = port->buffer.buffer;
  rxBufLen = port->buffer.length;
  _fifo_clear();
  
  _softserial_set_timings(params->baudrate);

  // configure bit sample timer
  LL_TIM_InitTypeDef timInit;
  LL_TIM_StructInit(&timInit);

  timInit.Prescaler = port->TIM_Prescaler;

  // enable_tim_clock(port->TIMx);
  LL_TIM_Init(port->TIMx, &timInit);

  NVIC_SetPriority(port->TIM_IRQn, 0);
  NVIC_EnableIRQ(port->TIM_IRQn);

  // init TELEMETRY_RX_GPIO_PIN
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = port->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_DOWN;

  LL_GPIO_Init(port->GPIOx, &pinInit);

  // TODO: telemetryInitDirPin();
  // -> should be done outside this driver!

  // Connect EXTI line to TELEMETRY RX pin
  LL_SYSCFG_SetEXTISource(port->EXTI_Port, port->EXTI_SysLine);

  // Configure EXTI for raising edge (start bit; assuming inverted serial)
  _softserialPort = port;
  stm32_exti_enable(port->EXTI_Line, LL_EXTI_TRIGGER_RISING, _softserial_exti);
}

static void _softserial_deinit_gpio(const stm32_softserial_port* port)
{
  // Reconfigure pin as input
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = port->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  LL_GPIO_Init(port->GPIOx, &pinInit);
}

static void _softserial_deinit_rx(const stm32_softserial_port* port)
{
  stm32_exti_disable(port->EXTI_Line);
  NVIC_DisableIRQ(port->TIM_IRQn);
  LL_TIM_DeInit(port->TIMx);
}

static void _softserial_init_tx(const stm32_softserial_port* port, const etx_serial_init* params)
{
  _softserial_set_timings(params->baudrate);
  _softserialPort = nullptr;

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = port->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP; // really ???

  LL_GPIO_Init(port->GPIOx, &pinInit);
}

static void* stm32_softserial_init(void* hw_def, const etx_serial_init* params)
{
  auto port = (const stm32_softserial_port*)hw_def;
  if (params->rx_enable) {
    _softserial_init_rx(port, params);
  } else {
    _softserial_init_tx(port, params);
  }

  return hw_def;
}

static void stm32_softserial_deinit(void* ctx)
{
  auto port = (const stm32_softserial_port*)ctx;

  if (port == _softserialPort) {
    _softserial_deinit_rx(port);
  }

  _softserial_deinit_gpio(port);
  _softserialPort = nullptr;
}

#define _WAIT_LOOP(len)                           \
  {                                               \
    while ((uint16_t)(getTmr2MHz() - time) < len) \
      ;                                           \
    time += len;                                  \
  }

static void stm32_softserial_send_byte(void* ctx, uint8_t byte)
{
  auto port = (const stm32_softserial_port*)ctx;
  uint16_t time;
  uint32_t i;
  
  __disable_irq();
  time = getTmr2MHz();
  LL_GPIO_SetOutputPin(port->GPIOx, port->GPIO_Pin);

  _WAIT_LOOP(bitLength);

  for (i = 0; i < 8; i++) {
    if (byte & 1) {
      LL_GPIO_ResetOutputPin(port->GPIOx, port->GPIO_Pin);
    }
    else {
      LL_GPIO_SetOutputPin(port->GPIOx, port->GPIO_Pin);
    }
    byte >>= 1 ;

    _WAIT_LOOP(bitLength);
  }
  LL_GPIO_ResetOutputPin(port->GPIOx, port->GPIO_Pin);
  __enable_irq(); // No need to wait for the stop bit to complete

  _WAIT_LOOP(bitLength);
}

static inline bool _fifo_full()
{
  return ((rxWidx + 1) & (rxBufLen - 1)) == rxRidx;
}

static inline void _fifo_push(uint8_t c)
{
  rxWidx = (rxWidx + 1) & (rxBufLen - 1);
  rxBuffer[rxWidx] = c;
}

static int stm32_softserial_get_byte(void* ctx, uint8_t* data)
{
  if (rxWidx == rxRidx) return 0;

  *data = rxBuffer[rxRidx];
  rxRidx = (rxWidx + 1) & (rxBufLen - 1);
  
  return 1;
}

void stm32_softserial_timer_isr(const stm32_softserial_port* port)
{
  auto TIMx = port->TIMx;
  LL_TIM_ClearFlag_UPDATE(TIMx);

  if (rxBitCount < 8) {
    if (rxBitCount == 0) {
      LL_TIM_SetAutoReload(TIMx, bitLength);
      rxByte = 0;
    }
    else {
      rxByte >>= 1;
    }

    if (LL_GPIO_IsInputPinSet(port->GPIOx, port->GPIO_Pin) == 0)
      rxByte |= 0x80;

    ++rxBitCount;
  }
  else if (rxBitCount == 8) {

    if (!_fifo_full()) _fifo_push(rxByte);
    rxBitCount = 0;

    // disable timer
    LL_TIM_DisableCounter(TIMx);

    // re-enable start bit interrupt
    LL_EXTI_EnableIT_0_31(port->EXTI_Line);
  }  
}

static void stm32_softserial_clear_rx_buffer(void* ctx)
{
  rxWidx = 0;
  rxRidx = 0;
}

const etx_serial_driver_t STM32SoftSerialDriver = {
  .init = stm32_softserial_init,
  .deinit = stm32_softserial_deinit,
  .sendByte = stm32_softserial_send_byte,
  .sendBuffer = nullptr,
  .waitForTxCompleted = nullptr,
  .enableRx = nullptr,
  .getByte = stm32_softserial_get_byte,
  .clearRxBuffer = stm32_softserial_clear_rx_buffer,
  .getBaudrate = nullptr,
  .setReceiveCb = nullptr,
  .setBaudrateCb = nullptr,
};
