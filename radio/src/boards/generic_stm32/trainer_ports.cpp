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
#include "stm32_gpio_driver.h"
#include "trainer_driver.h"

#include "hal/module_port.h"
#include "dataconstants.h"

#include "hal.h"

void board_trainer_init()
{
#if defined(TRAINER_DETECT_GPIO_PIN)
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = TRAINER_DETECT_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  stm32_gpio_enable_clock(TRAINER_DETECT_GPIO);
  LL_GPIO_Init(TRAINER_DETECT_GPIO, &pinInit);
#endif  

  trainer_init();
}

#if defined(TRAINER_GPIO)

bool trainer_dsc_available() { return true; }

static_assert(__IS_TRAINER_TIMER_OUT_CHANNEL_SUPPORTED(TRAINER_OUT_TIMER_Channel),
              "Unsupported trainer timer output channel");

static_assert(__IS_TRAINER_TIMER_IN_CHANNEL_SUPPORTED(TRAINER_IN_TIMER_Channel),
              "Unsupported trainer timer input channel");

static const stm32_pulse_timer_t trainerOutputTimer = {
  .GPIOx = TRAINER_GPIO,
  .GPIO_Pin = TRAINER_OUT_GPIO_PIN,
  .GPIO_Alternate = TRAINER_GPIO_AF,
  .TIMx = TRAINER_TIMER,
  .TIM_Freq = TRAINER_TIMER_FREQ,
  .TIM_Channel = TRAINER_OUT_TIMER_Channel,
  .TIM_IRQn = TRAINER_TIMER_IRQn,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)0,
  .DMA_TC_CallbackPtr = nullptr,
};

void trainer_init_dsc_out()
{
  trainer_init_output(&trainerOutputTimer);
}

static const stm32_pulse_timer_t trainerInputTimer = {
  .GPIOx = TRAINER_GPIO,
  .GPIO_Pin = TRAINER_IN_GPIO_PIN,
  .GPIO_Alternate = TRAINER_GPIO_AF,
  .TIMx = TRAINER_TIMER,
  .TIM_Freq = TRAINER_TIMER_FREQ,
  .TIM_Channel = TRAINER_IN_TIMER_Channel,
  .TIM_IRQn = TRAINER_TIMER_IRQn,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)0,
  .DMA_TC_CallbackPtr = nullptr,
};

void trainer_init_dsc_in()
{
  trainer_init_capture(&trainerInputTimer);
}

void trainer_stop_dsc() { trainer_stop(); }

#else
bool trainer_dsc_available() { return false; }
void trainer_init_dsc_out() {}
void trainer_init_dsc_in() {}
void trainer_stop_dsc() {}
#endif

bool is_trainer_dsc_connected()
{
#if defined(TRAINER_DETECT_GPIO_PIN)
  bool set = LL_GPIO_IsInputPinSet(TRAINER_DETECT_GPIO, TRAINER_DETECT_GPIO_PIN);
#if defined(TRAINER_DETECT_INVERTED)
  return !set;
#else
  return set;
#endif
#else // TRAINER_DETECT_GPIO_PIN
  return true;
#endif
}

void trainer_init_module_cppm()
{
  auto port =  modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_TIMER,
                              ETX_MOD_PORT_TIMER, ETX_Pol_Normal,
                              ETX_MOD_DIR_RX);
  if (!port) return;

  auto tim = (const stm32_pulse_timer_t*)port->hw_def;
  if (!tim) return;

  modulePortSetPower(EXTERNAL_MODULE,true);
  trainer_init_capture(tim);
}

void trainer_stop_module_cppm()
{
  trainer_stop();
  modulePortSetPower(EXTERNAL_MODULE,false);
}

#if defined(TRAINER_TIMER_IRQHandler)
extern "C" void TRAINER_TIMER_IRQHandler() { trainer_timer_isr(); }
#endif

#if defined(TRAINER_MODULE_CPPM_TIMER_IRQHandler) && \
  TRAINER_TIMER_IRQHandler != TRAINER_MODULE_CPPM_TIMER_IRQHandler
extern "C" void TRAINER_MODULE_CPPM_TIMER_IRQHandler() { trainer_timer_isr(); }
#endif
