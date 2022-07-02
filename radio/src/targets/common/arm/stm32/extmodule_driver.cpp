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

#include "extmodule_driver.h"
#include "stm32_pulse_driver.h"

#include "hal.h"
#include "board.h"
#include "timers_driver.h"

static const stm32_pulse_timer_t extmoduleTimer = {
  .GPIOx = EXTMODULE_TX_GPIO,
  .GPIO_Pin = EXTMODULE_TX_GPIO_PIN,
  .GPIO_Alternate = EXTMODULE_TIMER_TX_GPIO_AF,
  .TIMx = EXTMODULE_TIMER,
  .TIM_Prescaler = __LL_TIM_CALC_PSC(EXTMODULE_TIMER_FREQ, 2000000),
  .TIM_Channel = EXTMODULE_TIMER_Channel,
  .TIM_IRQn = EXTMODULE_TIMER_IRQn,
  .DMAx = EXTMODULE_TIMER_DMA,
  .DMA_Stream = EXTMODULE_TIMER_DMA_STREAM_LL,
  .DMA_Channel = EXTMODULE_TIMER_DMA_CHANNEL,
  .DMA_IRQn = EXTMODULE_TIMER_DMA_STREAM_IRQn,
};

// Make sure the timer channel is supported
static_assert(__STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(EXTMODULE_TIMER_Channel),
              "Unsupported timer channel");

// Make sure the DMA channel is supported
static_assert(__STM32_PULSE_IS_DMA_STREAM_SUPPORTED(EXTMODULE_TIMER_DMA_STREAM_LL),
              "Unsupported DMA stream");

#if !defined(EXTMODULE_TIMER_DMA_IRQHandler)
#error "Missing EXTMODULE_TIMER_DMA_IRQHandler definition"
#endif

extern "C" void EXTMODULE_TIMER_DMA_IRQHandler()
{
  stm32_pulse_dma_tc_isr(&extmoduleTimer);
}

#if !defined(EXTMODULE_TIMER_IRQHandler)
#error "Missing EXTMODULE_TIMER_IRQHandler definition"
#endif

extern "C" void EXTMODULE_TIMER_IRQHandler()
{
  stm32_pulse_tim_update_isr(&extmoduleTimer);
}

void extmoduleStop()
{
  EXTERNAL_MODULE_OFF();
  stm32_pulse_deinit(&extmoduleTimer);
}

static void config_ppm_output(uint16_t ppm_delay, bool polarity)
{
  // PPM generation principle:
  //
  // Hardware timer in PWM mode is used for PPM generation
  // Output is OFF if CNT<CCR1(delay) and ON if bigger
  // CCR1 register defines duration of pulse length and is constant
  // AAR register defines duration of each pulse, it is
  // updated after every pulse in Update interrupt handler.

  stm32_pulse_config_output(&extmoduleTimer, !polarity, LL_TIM_OCMODE_FORCED_INACTIVE,
                            ppm_delay * 2);
}

void extmodulePpmStart(uint16_t ppm_delay, bool polarity)
{
  EXTERNAL_MODULE_ON();
  stm32_pulse_init(&extmoduleTimer);
  config_ppm_output(ppm_delay, polarity);
}

void extmoduleSendNextFramePpm(void* pulses, uint16_t length,
                               uint16_t ppm_delay, bool polarity)
{
  if (!stm32_pulse_if_not_running_disable(&extmoduleTimer))
    return;

  // Set polarity
  stm32_pulse_set_polarity(&extmoduleTimer, !polarity);

  // Start DMA request and re-enable timer
  stm32_pulse_start_dma_req(&extmoduleTimer, pulses, length,
                            LL_TIM_OCMODE_PWM1, ppm_delay * 2);
}

#if defined(PXX1)
void extmodulePxx1PulsesStart()
{
  EXTERNAL_MODULE_ON();
  stm32_pulse_config_output(&extmoduleTimer, false, LL_TIM_OCMODE_PWM1, 9 * 2);
  stm32_pulse_init(&extmoduleTimer);
}

void extmoduleSendNextFramePxx1(const void* pulses, uint16_t length)
{
  if (!stm32_pulse_if_not_running_disable(&extmoduleTimer)) return;

  // Start DMA request and re-enable timer
  stm32_pulse_start_dma_req(&extmoduleTimer, pulses, length, LL_TIM_OCMODE_PWM1,
                            9 * 2);
}
#endif

// TODO: polarity?
void extmoduleSerialStart()
{
  EXTERNAL_MODULE_ON();
  stm32_pulse_init(&extmoduleTimer);
  stm32_pulse_config_output(&extmoduleTimer, true, LL_TIM_OCMODE_TOGGLE, 0);
}

void extmoduleSendNextFrameSoftSerial(const void* pulses, uint16_t length, bool polarity)
{
  if (!stm32_pulse_if_not_running_disable(&extmoduleTimer))
    return;

  // Set polarity
  stm32_pulse_set_polarity(&extmoduleTimer, polarity);
  
  // Start DMA request and re-enable timer
  stm32_pulse_start_dma_req(&extmoduleTimer, pulses, length, LL_TIM_OCMODE_TOGGLE, 0);
}

void extmoduleInitTxPin()
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = extmoduleTimer.GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(extmoduleTimer.GPIOx, &pinInit);
}

// Delay based byte sending @ 57600 bps
void extmoduleSendInvertedByte(uint8_t byte)
{
  uint16_t time;
  uint32_t i;

  __disable_irq();
  time = getTmr2MHz();
  LL_GPIO_SetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  while ((uint16_t) (getTmr2MHz() - time) < 34)	{
    // wait
  }
  time += 34;
  for (i = 0; i < 8; i++) {
    if (byte & 1) {
      LL_GPIO_ResetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    else {
      LL_GPIO_SetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    byte >>= 1 ;
    while ((uint16_t) (getTmr2MHz() - time) < 35) {
      // wait
    }
    time += 35 ;
  }
  LL_GPIO_ResetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  __enable_irq();	// No need to wait for the stop bit to complete
  while ((uint16_t) (getTmr2MHz() - time) < 34) {
    // wait
  }
}
