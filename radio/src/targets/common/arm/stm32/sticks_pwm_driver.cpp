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

#include "opentx.h"
#include "hal/adc_driver.h"

volatile static uint32_t pwm_interrupt_count = 0;

void sticksPwmInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWM_GPIOA_PINS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PWM_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(PWM_GPIO, GPIO_PinSource0, PWM_GPIO_AF);
  GPIO_PinAFConfig(PWM_GPIO, GPIO_PinSource1, PWM_GPIO_AF);
  GPIO_PinAFConfig(PWM_GPIO, GPIO_PinSource2, PWM_GPIO_AF);
  GPIO_PinAFConfig(PWM_GPIO, GPIO_PinSource3, PWM_GPIO_AF);

  PWM_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  PWM_TIMER->PSC = 80;
  PWM_TIMER->ARR = 0xffff;
  PWM_TIMER->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
  PWM_TIMER->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0;
  PWM_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  PWM_TIMER->DIER |= TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4;
  PWM_TIMER->CR1 = TIM_CR1_CEN; // Start timer

  NVIC_EnableIRQ(PWM_IRQn);
  NVIC_SetPriority(PWM_IRQn, 10);
}

void sticksPwmDeInit()
{
  NVIC_DisableIRQ(PWM_IRQn);
  PWM_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = PWM_GPIOA_PINS;
  GPIO_Init(PWM_GPIO, &GPIO_InitStructure);
}

void sticksPwmDetect()
{
  sticksPwmInit();
  delay_ms(20);
  if (pwm_interrupt_count < 32) {
    hardwareOptions.sticksPwmDisabled = true;
    sticksPwmDeInit();
  }
}

inline uint32_t TIM_GetCapture_Stick(uint8_t n)
{
  switch (n) {
    case 0:
      return PWM_TIMER->CCR1;
    case 1:
      return PWM_TIMER->CCR2;
    case 2:
      return PWM_TIMER->CCR3;
    case 3:
      return PWM_TIMER->CCR4;
    default:
      return 0;
  }
}

inline void TIM_SetPolarityRising(uint8_t n)
{
  PWM_TIMER->CCER &= ~(TIM_CCER_CC1P << (n * 4));
}

inline void TIM_SetPolarityFalling(uint8_t n)
{
  PWM_TIMER->CCER |= (TIM_CCER_CC1P << (n * 4));
}

inline void TIM_ClearITPendingBit(uint8_t n)
{
  PWM_TIMER->SR = ~(TIM_IT_CC1 << n);
}

inline uint32_t diff_with_16bits_overflow(uint32_t a, uint32_t b)
{
  if (b > a)
    return b - a;
  else
    return b + 0xffff - a;
}

#if !defined(STICK_PWM_CHANNEL_0)
  #define STICK_PWM_CHANNEL_0 0
#endif

#if !defined(STICK_PWM_CHANNEL_1)
  #define STICK_PWM_CHANNEL_1 1
#endif

#if !defined(STICK_PWM_CHANNEL_2)
  #define STICK_PWM_CHANNEL_2 2
#endif

#if !defined(STICK_PWM_CHANNEL_3)
  #define STICK_PWM_CHANNEL_3 3
#endif

static const uint8_t _channel_map[] = {
  STICK_PWM_CHANNEL_0,
  STICK_PWM_CHANNEL_1,
  STICK_PWM_CHANNEL_2,
  STICK_PWM_CHANNEL_3,
};

extern "C" void PWM_IRQHandler(void)
{
  static uint8_t  timer_capture_states[MAX_STICKS];
  static uint32_t timer_capture_rising_time[MAX_STICKS];

  auto adcValues = getAnalogValues();

  for (uint8_t i=0; i<MAX_STICKS; i++) {
    if (PWM_TIMER->SR & (TIM_DIER_CC1IE << i)) {
      uint32_t capture = TIM_GetCapture_Stick(i);
      // overflow may happen but we only use this to detect PWM / ADC on radio startup
      pwm_interrupt_count++; 
      if (timer_capture_states[i] != 0) {
        uint32_t value = diff_with_16bits_overflow(timer_capture_rising_time[i], capture);
        if (value < 10000) {
          adcValues[_channel_map[i]] = (uint16_t) value;
        }
        TIM_SetPolarityRising(i);
        timer_capture_states[i] = 0;
      }
      else {
        timer_capture_rising_time[i] = capture;
        TIM_SetPolarityFalling(i);
        timer_capture_states[i] = 0x80;
      }
      TIM_ClearITPendingBit(i);
    }
  }
}
