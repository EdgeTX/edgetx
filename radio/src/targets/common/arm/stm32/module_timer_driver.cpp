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

#include "module_timer_driver.h"
#include "stm32_pulse_driver.h"

static void* module_timer_init(void* hw_def, const etx_timer_config_t* cfg)
{
  auto timer = (const stm32_pulse_timer_t*)hw_def;

  bool polarity = cfg->polarity;
  uint32_t ocval = cfg->cmp_val;
  uint32_t ocmode = LL_TIM_OCMODE_FORCED_INACTIVE;

  stm32_pulse_init(timer, 0);
  stm32_pulse_config_output(timer, polarity, ocmode, ocval);

  return (void*)timer;
}

static void module_timer_deinit(void* ctx)
{
  auto timer = (const stm32_pulse_timer_t*)ctx;
  stm32_pulse_deinit(timer);
}

static void module_timer_send(void* ctx, const etx_timer_config_t* cfg,
                              const void* pulses, uint16_t length)
{
  auto timer = (const stm32_pulse_timer_t*)ctx;
  if (!stm32_pulse_if_not_running_disable(timer))
    return;

  // Set polarity
  stm32_pulse_set_polarity(timer, cfg->polarity);
  
  // Start DMA request and re-enable timer
  uint32_t ocmode = LL_TIM_OCMODE_PWM1;
  uint32_t ocval = cfg->cmp_val;
  stm32_pulse_start_dma_req(timer, pulses, length, ocmode, ocval);  
}

const etx_timer_driver_t STM32ModuleTimerDriver = {
  .init = module_timer_init,
  .deinit = module_timer_deinit,
  .send = module_timer_send,
};
