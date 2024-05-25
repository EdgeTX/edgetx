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
#include "stm32_exti_driver.h"
#include "stm32_gpio_driver.h"
#include "board.h"

#include "hal.h"
#include "hal/key_driver.h"
#include "hal/rotary_encoder.h"

#include "board_common.h"

#if !defined(BOOT)
  #include "opentx.h"
#endif

volatile rotenc_t rotencValue = 0;
volatile uint32_t rotencDt = 0;

#if defined(BOOT)
#define INC_ROT        1
#define INC_ROT_2      2
#else
#define INC_ROT \
  (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_BOTH ? -1 : 1);
#define INC_ROT_2 \
  (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_BOTH ? -2 : 2);
#endif

rotenc_t rotaryEncoderGetValue()
{
  return rotencValue / ROTARY_ENCODER_GRANULARITY;
}

rotenc_t rotaryEncoderGetRawValue()
{
  return rotencValue;
}

void rotaryEncoderCheck()
{
  static uint8_t state = 0;
  static uint8_t re_count = 0;
  uint8_t pins = ROTARY_ENCODER_POSITION();

#if defined(ROTARY_ENCODER_SUPPORT_BUGGY_WIRING)
  if (pins != (state & 0x03) && !(readKeys() & (1 << KEY_ENTER))) {
    if (re_count == 0) {
      // Need at least 2 values to correctly determine initial direction
      re_count = 1;
    } else {
      if ((pins ^ (state & 0x03)) == 0x03) {
        if (pins == 3) {
          rotencValue += INC_ROT_2;
        } else {
          rotencValue -= INC_ROT_2;
        }
      } else {
        if ((state & 0x01) ^ ((pins & 0x02) >> 1)) {
          rotencValue -= INC_ROT;
        } else {
          rotencValue += INC_ROT;
        }
      }

      if (re_count == 1)
      {
        re_count = 2;
        // Assume 1st value is same direction as 2nd value
        rotencValue = rotencValue * 2;
      }
    }
    state &= ~0x03;
    state |= pins;
  }
#else
  if (pins != state && !(readKeys() & (1 << KEY_ENTER))) {
    if (re_count == 0) {
      // Need at least 2 values to correctly determine initial direction
      re_count = 1;
    } else {
#if defined(ROTARY_ENCODER_INVERTED)
      if (!(state & 0x01) ^ ((pins & 0x02) >> 1)) {
#else
      if ((state & 0x01) ^ ((pins & 0x02) >> 1)) {
#endif
        rotencValue -= INC_ROT;
      } else {
        rotencValue += INC_ROT;
      }

      if (re_count == 1)
      {
        re_count = 2;
        // Assume 1st value is same direction as 2nd value
        rotencValue = rotencValue * 2;
      }
    }
    state = pins;
  }
#endif

#if !defined(BOOT) && defined(COLORLCD)
  static uint32_t last_tick = 0;
  static rotenc_t last_value = 0;

  rotenc_t value = rotencValue;
  rotenc_t diff = (value - last_value) / ROTARY_ENCODER_GRANULARITY;

  if (diff != 0) {
    uint32_t now = RTOS_GET_MS();
    uint32_t dt = now - last_tick;
    // pre-compute accumulated dt (dx/dt is done later in LVGL driver)
    rotencDt += dt;
    last_tick = now;
    last_value += diff * ROTARY_ENCODER_GRANULARITY;
  }
#endif
}

void rotaryEncoderStartDelay()
{
  ROTARY_ENCODER_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_URS;
}

void rotaryEncoderInit()
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  pinInit.Pin = ROTARY_ENCODER_GPIO_PIN_A | ROTARY_ENCODER_GPIO_PIN_B;

  stm32_gpio_enable_clock(ROTARY_ENCODER_GPIO);
  LL_GPIO_Init(ROTARY_ENCODER_GPIO, &pinInit);
  
  ROTARY_ENCODER_TIMER->ARR = 99; // 100uS
  ROTARY_ENCODER_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1; // 1uS
  ROTARY_ENCODER_TIMER->CCER = 0;
  ROTARY_ENCODER_TIMER->CCMR1 = 0;
  ROTARY_ENCODER_TIMER->EGR = 0;
  ROTARY_ENCODER_TIMER->CR1 = 0;
  ROTARY_ENCODER_TIMER->DIER |= TIM_DIER_UIE;

#if defined(LL_APB4_GRP1_PERIPH_SYSCFG)
  LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);
#elif defined(LL_APB2_GRP1_PERIPH_SYSCFG)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
#else
  #error "Unsupported SYSCFG clock"
#endif

  LL_SYSCFG_SetEXTISource(ROTARY_ENCODER_EXTI_PORT, ROTARY_ENCODER_EXTI_SYS_LINE1);
  LL_SYSCFG_SetEXTISource(ROTARY_ENCODER_EXTI_PORT, ROTARY_ENCODER_EXTI_SYS_LINE2);

  uint32_t trigger = LL_EXTI_TRIGGER_RISING_FALLING;
  stm32_exti_enable(ROTARY_ENCODER_EXTI_LINE1, trigger, rotaryEncoderStartDelay);
  stm32_exti_enable(ROTARY_ENCODER_EXTI_LINE2, trigger, rotaryEncoderStartDelay);
    
  NVIC_EnableIRQ(ROTARY_ENCODER_TIMER_IRQn);
  NVIC_SetPriority(ROTARY_ENCODER_TIMER_IRQn, 7);
}

extern "C" void ROTARY_ENCODER_TIMER_IRQHandler(void)
{
  ROTARY_ENCODER_TIMER->SR &= ~TIM_SR_UIF;
  ROTARY_ENCODER_TIMER->CR1 = 0;
  rotaryEncoderCheck();
}
