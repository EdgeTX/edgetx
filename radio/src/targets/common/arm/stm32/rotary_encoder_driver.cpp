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

#include "stm32_exti_driver.h"
#include "board.h"

#include "hal.h"
#include "board_common.h"

#if !defined(BOOT)
  #include "opentx.h"
#endif

static uint8_t rotencPosition;
volatile rotenc_t rotencValue;

#if defined(BOOT)
#define INC_ROT        1
#define INC_ROT_2      2
#else
#define INC_ROT        (g_eeGeneral.rotEncDirection ? -1 : 1);
#define INC_ROT_2      (g_eeGeneral.rotEncDirection ? -2 : 2);
#endif

void rotaryEncoderCheck()
{
#if (defined(RADIO_FAMILY_T16) && !defined(RADIO_T18)) || defined(RADIO_TX12)
  static uint8_t state = 0;
  uint8_t pins = ROTARY_ENCODER_POSITION();

  if (pins != (state & 0x03) && !(readKeys() & (1 << KEY_ENTER))) {
	if ((pins ^ (state & 0x03)) == 0x03)
	{
		if (pins == 3)
		{
			rotencValue += INC_ROT_2;
		}
		else
		{
			rotencValue -= INC_ROT_2;
		}
	}
	else
	{
		if ((state & 0x01) ^ ((pins & 0x02) >> 1))
		{
			rotencValue -= INC_ROT;
		}
		else
		{
			rotencValue += INC_ROT;
		}
	}
    state &= ~0x03;
    state |= pins;
#else
  uint8_t newPosition = ROTARY_ENCODER_POSITION();
  if (newPosition != rotencPosition && !(readKeys() & (1 << KEY_ENTER))) {
#if defined(RADIO_ZORRO) // zorro def. rotation dir is inverse of other radios
    if (!(rotencPosition & 0x01) ^ ((newPosition & 0x02) >> 1)) {
#else
    if ((rotencPosition & 0x01) ^ ((newPosition & 0x02) >> 1)) {
#endif
      rotencValue -= INC_ROT;
    } else {
      rotencValue += INC_ROT;
    }
    rotencPosition = newPosition;
#endif
  }
}

void rotaryEncoderStartDelay()
{
  ROTARY_ENCODER_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_URS;
}

void rotaryEncoderInit()
{
  rotencPosition = ROTARY_ENCODER_POSITION();

  ROTARY_ENCODER_TIMER->ARR = 99; // 100uS
  ROTARY_ENCODER_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1; // 1uS
  ROTARY_ENCODER_TIMER->CCER = 0;
  ROTARY_ENCODER_TIMER->CCMR1 = 0;
  ROTARY_ENCODER_TIMER->EGR = 0;
  ROTARY_ENCODER_TIMER->CR1 = 0;
  ROTARY_ENCODER_TIMER->DIER |= TIM_DIER_UIE;

  SYSCFG_EXTILineConfig(ROTARY_ENCODER_EXTI_PortSource, ROTARY_ENCODER_EXTI_PinSource1);
  SYSCFG_EXTILineConfig(ROTARY_ENCODER_EXTI_PortSource, ROTARY_ENCODER_EXTI_PinSource2);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = ROTARY_ENCODER_EXTI_LINE1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  EXTI_InitStructure.EXTI_Line = ROTARY_ENCODER_EXTI_LINE2;
  EXTI_Init(&EXTI_InitStructure);

  stm32_exti_enable(ROTARY_ENCODER_EXTI_LINE1, rotaryEncoderStartDelay);
  stm32_exti_enable(ROTARY_ENCODER_EXTI_LINE2, rotaryEncoderStartDelay);
    
  NVIC_EnableIRQ(ROTARY_ENCODER_TIMER_IRQn);
  NVIC_SetPriority(ROTARY_ENCODER_TIMER_IRQn, 7);
}

extern "C" void ROTARY_ENCODER_TIMER_IRQHandler(void)
{
  ROTARY_ENCODER_TIMER->SR &= ~TIM_SR_UIF;
  ROTARY_ENCODER_TIMER->CR1 = 0;
  rotaryEncoderCheck();
}
