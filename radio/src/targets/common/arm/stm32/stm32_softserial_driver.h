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

extern const etx_serial_driver_t STM32SoftSerialDriver;
