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

#include "stm32_pulse_driver.h"

#include "hal.h"
#include "board.h"
#include "timers_driver.h"

static const stm32_pulse_timer_t intmoduleTimer = {
  .GPIOx = INTMODULE_TX_GPIO,
  .GPIO_Pin = INTMODULE_TX_GPIO_PIN,
  .GPIO_Alternate = INTMODULE_TX_GPIO_AF,
  .TIMx = INTMODULE_TIMER,
  .TIM_Prescaler = __LL_TIM_CALC_PSC(INTMODULE_TIMER_FREQ, 2000000),
  .TIM_Channel = INTMODULE_TIMER_Channel,
  .TIM_IRQn = INTMODULE_TIMER_IRQn,
  .DMAx = INTMODULE_TIMER_DMA,
  .DMA_Stream = INTMODULE_TIMER_DMA_STREAM,
  .DMA_Channel = INTMODULE_TIMER_DMA_CHANNEL,
  .DMA_IRQn = INTMODULE_TIMER_DMA_STREAM_IRQn,
};

// Make sure the timer channel is supported
static_assert(__STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(INTMODULE_TIMER_Channel),
              "Unsupported timer channel");

// Make sure the DMA channel is supported
static_assert(__STM32_PULSE_IS_DMA_STREAM_SUPPORTED(INTMODULE_TIMER_DMA_STREAM),
              "Unsupported DMA stream");

#if !defined(INTMODULE_TIMER_DMA_IRQHandler)
#error "Missing INTMODULE_TIMER_DMA_IRQHandler definition"
#endif

extern "C" void INTMODULE_TIMER_DMA_IRQHandler()
{
  stm32_pulse_dma_tc_isr(&intmoduleTimer);
}

#if !defined(INTMODULE_TIMER_IRQHandler)
#error "Missing INTMODULE_TIMER_IRQHandler definition"
#endif

extern "C" void INTMODULE_TIMER_IRQHandler()
{
  stm32_pulse_tim_update_isr(&intmoduleTimer);
}

void intmoduleStop()
{
  INTERNAL_MODULE_OFF();
  stm32_pulse_deinit(&intmoduleTimer);
}

#if defined(PXX1)
void intmodulePxx1PulsesStart()
{
  INTERNAL_MODULE_ON();
  stm32_pulse_config_output(&intmoduleTimer, false, LL_TIM_OCMODE_PWM1, 9 * 2);
  stm32_pulse_init(&intmoduleTimer);
}

void intmoduleSendNextFramePxx1(const uint16_t* data, uint8_t size)
{
  if (!stm32_pulse_if_not_running_disable(&intmoduleTimer)) return;

  // Start DMA request and re-enable timer
  stm32_pulse_start_dma_req(&intmoduleTimer, data, size, LL_TIM_OCMODE_PWM1,
                            9 * 2);
}
#endif
