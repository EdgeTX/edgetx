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

#include "sticks_pwm_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_timer.h"
#include "hal/adc_driver.h"
#include "stm32_hal_ll.h"
#include "delays_driver.h"

#include "dataconstants.h"

static volatile uint32_t _pwm_interrupt_count;

static void sticks_pwm_init(const stick_pwm_timer_t* tim)
{
  stm32_gpio_enable_clock(tim->GPIOx);

  LL_GPIO_InitTypeDef pinInit;
  pinInit.Pin = tim->GPIO_Pin;
  pinInit.Alternate = tim->GPIO_Alternate;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(tim->GPIOx, &pinInit);

  auto TIMx = tim->TIMx;

  stm32_timer_enable_clock(TIMx);
  TIMx->CR1 &= ~TIM_CR1_CEN; // Stop timer
  TIMx->PSC = 80;
  TIMx->ARR = 0xffff;
  TIMx->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
  TIMx->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0;
  TIMx->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  TIMx->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE;
  TIMx->CR1 = TIM_CR1_CEN; // Start timer

  NVIC_EnableIRQ(tim->TIM_IRQn);
  NVIC_SetPriority(tim->TIM_IRQn, 10);
}

static void sticks_pwm_deinit(const stick_pwm_timer_t* tim)
{
  NVIC_DisableIRQ(tim->TIM_IRQn);

  // Stop timer
  auto TIMx = tim->TIMx;
  TIMx->CR1 &= ~TIM_CR1_CEN;
  stm32_timer_disable_clock(TIMx);

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = tim->GPIO_Pin;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  LL_GPIO_Init(tim->GPIOx, &pinInit);
}

bool sticks_pwm_detect(const stick_pwm_timer_t* timer,
		       const stick_pwm_input_t* inputs,
		       uint8_t n_inputs)
{
  if (!timer || !inputs || n_inputs == 0)
    return false;
  
  _pwm_interrupt_count = 0;
  sticks_pwm_init(timer);
  delay_ms(20);

  if (_pwm_interrupt_count < 32) {
    sticks_pwm_deinit(timer);
    return false;
  }

  return true;
}

static inline uint32_t tim_get_capture(TIM_TypeDef *TIMx, uint8_t channel)
{
  auto* base_CCRx = &(TIMx->CCR1);
  return READ_REG(*(base_CCRx + channel));
}

static inline void tim_set_polarity_rising(TIM_TypeDef *TIMx, uint8_t channel)
{
  CLEAR_BIT(TIMx->CCER, TIM_CCER_CC1P << (channel * 4));
}

static inline void tim_set_polarity_falling(TIM_TypeDef *TIMx, uint8_t channel)
{
  SET_BIT(TIMx->CCER, TIM_CCER_CC1P << (channel * 4));
}

static inline uint32_t tim_is_active_flag_CCx(TIM_TypeDef *TIMx, uint8_t channel)
{
  return READ_BIT(TIMx->SR, TIM_SR_CC1IF << channel);
}

static inline void tim_clear_flag_CCx(TIM_TypeDef *TIMx, uint8_t channel)
{
  CLEAR_BIT(TIMx->SR, TIM_SR_CC1IF << channel);
}

static inline uint32_t diff_with_16bits_overflow(uint32_t a, uint32_t b)
{
  if (b > a) {
    return b - a;
  }

  return b + 0xffff - a;
}

void sticks_pwm_isr(const stick_pwm_timer_t* tim,
		    const stick_pwm_input_t* inputs,
		    uint8_t n_inputs)
{
  static uint8_t  timer_capture_states[MAX_STICKS];
  static uint32_t timer_capture_rising_time[MAX_STICKS];

  auto TIMx = tim->TIMx;
  auto adcValues = getAnalogValues();

  for (uint8_t i = 0; i < n_inputs; i++) {

    const auto& input = inputs[i];
    auto channel = input.channel;
    
    if (tim_is_active_flag_CCx(TIMx, channel)) {

      uint32_t capture = tim_get_capture(TIMx, channel);
      tim_clear_flag_CCx(TIMx, channel);

      // overflow may happen but we only use this to detect PWM / ADC on radio startup
      _pwm_interrupt_count++;

      if (timer_capture_states[i] != 0) {
        uint32_t value = diff_with_16bits_overflow(timer_capture_rising_time[i], capture);

        if (value <= ADC_MAX_VALUE) {
	  uint16_t v16 = (uint16_t)value;
          adcValues[i] = input.inverted ? ADC_INVERT_VALUE(v16) : v16;
        }

        tim_set_polarity_rising(TIMx, channel);
        timer_capture_states[i] = 0;
      }
      else {
        timer_capture_rising_time[i] = capture;

        tim_set_polarity_falling(TIMx, channel);
        timer_capture_states[i] = 0x80;
      }
    }
  }
}
