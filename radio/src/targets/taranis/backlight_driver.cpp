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

#include "board.h"

#if !defined(BOOT)
#include "myeeprom.h"
#endif

#if !defined(BACKLIGHT_GPIO)
  // no backlight
  void backlightInit() {}
  void backlightEnable(uint8_t level) {}
  void backlightFullOn() {}
  void backlightDisable() {}
  uint8_t isBacklightEnabled() {return false;}
#elif defined(PCBX9E)
void backlightInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN_1|BACKLIGHT_GPIO_PIN_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource_1, BACKLIGHT_GPIO_AF_1);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource_2, BACKLIGHT_GPIO_AF_1);
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  BACKLIGHT_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
  BACKLIGHT_TIMER->CCR2 = 0;
  BACKLIGHT_TIMER->CCR1 = 100;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
}

void backlightEnable(uint8_t level)
{
#if defined(BOOT)
  uint8_t color = 0;
#else
  uint8_t color = g_eeGeneral.backlightColor;
#endif
  BACKLIGHT_TIMER->CCR1 = ((100-level)*(20-color))/20;
  BACKLIGHT_TIMER->CCR2 = ((100-level)*color)/20;
}

void backlightFullOn()
{
  backlightEnable(0);
}

void backlightDisable()
{
  BACKLIGHT_TIMER->CCR1 = 0;
  BACKLIGHT_TIMER->CCR2 = 0;
}

uint8_t isBacklightEnabled()
{
  return (BACKLIGHT_TIMER->CCR1 != 0 || BACKLIGHT_TIMER->CCR2 != 0);
}
#elif defined(PCBX9DP)
void backlightInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN_1 | BACKLIGHT_GPIO_PIN_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource_1, BACKLIGHT_GPIO_AF_1);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource_2, BACKLIGHT_GPIO_AF_1);
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  BACKLIGHT_TIMER->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM
  BACKLIGHT_TIMER->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; // PWM
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC4E | TIM_CCER_CC2E;
  BACKLIGHT_TIMER->CCR2 = 0;
  BACKLIGHT_TIMER->CCR4 = 0;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
}

void backlightEnable(uint8_t level)
{
#if defined(BOOT)
  uint8_t color = 0;
#else
  uint8_t color = g_eeGeneral.backlightColor;
#endif
  BACKLIGHT_TIMER->CCR4 = ((100-level)*(20-color))/20;
  BACKLIGHT_TIMER->CCR2 = ((100-level)*color)/20;
}

void backlightFullOn()
{
  backlightEnable(0);
}

void backlightDisable()
{
  BACKLIGHT_TIMER->CCR4 = 0;
  BACKLIGHT_TIMER->CCR2 = 0;
}

uint8_t isBacklightEnabled()
{
  return (BACKLIGHT_TIMER->CCR4 != 0 || BACKLIGHT_TIMER->CCR2 != 0);
}
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
void backlightInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource, BACKLIGHT_GPIO_AF);
#if defined(BACKLIGHT_BDTR) // TODO perhaps it can be always done
  BACKLIGHT_TIMER->BDTR = BACKLIGHT_BDTR; // Enable outputs
#endif
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  #if defined(BACKLIGHT_CCMR1)
    BACKLIGHT_TIMER->CCMR1 = BACKLIGHT_CCMR1;
  #elif defined(BACKLIGHT_CCMR2)
    BACKLIGHT_TIMER->CCMR2 = BACKLIGHT_CCMR2;
  #endif
  BACKLIGHT_TIMER->CCER = BACKLIGHT_CCER;
  BACKLIGHT_COUNTER_REGISTER = 100;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
}

void backlightEnable(uint8_t level)
{
  BACKLIGHT_COUNTER_REGISTER = 100 - level;
}

void backlightFullOn()
{
  backlightEnable(0);
}

void backlightDisable()
{
  BACKLIGHT_COUNTER_REGISTER = 0;
}

uint8_t isBacklightEnabled()
{
  return BACKLIGHT_COUNTER_REGISTER != 0;
}
#else
void backlightInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource, BACKLIGHT_GPIO_AF);
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  BACKLIGHT_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC1E;
  BACKLIGHT_TIMER->CCR1 = 80;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = 1;
}

void backlightEnable(uint8_t level)
{
  BACKLIGHT_TIMER->CCR1 = 100 - level;
}

void backlightFullOn()
{
  backlightEnable(0);
}

void backlightDisable()
{
  BACKLIGHT_TIMER->CCR1 = 0;
}

uint8_t isBacklightEnabled()
{
  return BACKLIGHT_TIMER->CCR1 != 0;
}
#endif
