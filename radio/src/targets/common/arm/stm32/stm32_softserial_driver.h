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

#include "stm32_serial_driver.h"
#include "stm32_pulse_driver.h"

#if 0
// Single direction soft-serial (either RX or TX)
//
// etx_serial_driver.init() shall be passed
// this struct cast as (void*)
//
struct stm32_softserial_port {

  GPIO_TypeDef* GPIOx;
  uint32_t      GPIO_Pin;

  // only required for RX
  TIM_TypeDef*  TIMx;
  uint16_t      TIM_Prescaler;
  IRQn_Type     TIM_IRQn;
  uint32_t      EXTI_Port;
  uint32_t      EXTI_SysLine;
  uint32_t      EXTI_Line;

  const stm32_serial_buffer buffer;
};

void stm32_softserial_timer_isr(const stm32_softserial_port* port);
#endif

#define STM32_SOFTSERIAL_BUFFERED_PULSES 8

// Max 12 toggles per serial byte
#define STM32_SOFTSERIAL_MAX_PULSES_TRANSITIONS \
  (12 * STM32_SOFTSERIAL_BUFFERED_PULSES)

#if defined(STM32_SUPPORT_32BIT_TIMERS)
typedef uint32_t stm32_softserial_pulse_t;
#else
typedef uint16_t stm32_softserial_pulse_t;
#endif

struct stm32_softserial_tx_state;
typedef void (*stm32_softserial_conv_byte_fct)(stm32_softserial_tx_state*, uint8_t);

struct stm32_softserial_tx_state {

  stm32_softserial_conv_byte_fct conv_byte;

  stm32_softserial_pulse_t pulse_buffer[STM32_SOFTSERIAL_MAX_PULSES_TRANSITIONS];
  uint16_t*                pulse_ptr;
#if defined(STM32_SUPPORT_32BIT_TIMERS)
  uint8_t                  pulse_inc;
#endif

  const uint8_t*           serial_data;
  uint8_t                  serial_size;
};

struct stm32_softserial_tx_port {
  const stm32_pulse_timer_t* tim;
  stm32_softserial_tx_state* st;
};

#define DEFINE_STM32_SOFTSERIAL_PORT(p,timer)                           \
  static stm32_softserial_tx_state p ## _SoftserialState;               \
  static const stm32_softserial_tx_port p ## _STM32Softserial = {       \
    &timer,                                                             \
    & p ## _SoftserialState,                                            \
  }

#define REF_STM32_SOFTSERIAL_PORT(p) ((void*)& p ## _STM32Softserial)

extern const etx_serial_driver_t STM32SoftSerialTxDriver;
