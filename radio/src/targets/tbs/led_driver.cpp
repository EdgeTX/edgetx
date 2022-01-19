/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#define RGB_LED_CODE_1_TIME             66
#define RGB_LED_CODE_0_TIME             24
#define RGB_LED_RGB_BITS                24    
#define RGB_LED_BIT_BUF_SIZE            (RGB_LED_RGB_BITS + 1)

#define RGB_COLOUR_OFF      0,0,0
#define RGB_COLOUR_RED      25,0,0
#define RGB_COLOUR_GREEN    0,25,0
#define RGB_COLOUR_BLUE     0,0,25
#define RGB_COLOUR_WHITE    25,25,25

#define RGB2BUFFER(Red, Green, Blue)    (((uint32_t) Green << 16) |  ((uint32_t) Red << 8) | (uint32_t)Blue)

static uint32_t rgb_led_bit_buffer[RGB_LED_BIT_BUF_SIZE] __DMA;

void ledInit()
{
  GPIO_PinAFConfig(LED_GPIO, LED_GPIO_PinSource, LED_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(LED_GPIO, &GPIO_InitStructure);

  LED_DMA_STREAM->CR &= ~DMA_SxCR_EN;
  LED_TIMER->CR1 &= ~TIM_CR1_CEN;
  LED_TIMER->PSC = 0;
  LED_TIMER->ARR = 92;
  
  LED_TIMER->CCER |= TIM_CCER_CC4E;
  LED_TIMER->BDTR = TIM_BDTR_MOE; // Enable outputs
  LED_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0; // Force O/P high
  LED_TIMER->EGR = 1; // Restart
  LED_TIMER->DIER |= TIM_DIER_CC4DE; // Enable DMA on CC4
  LED_TIMER->CCMR2 = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;

  LED_DMA_STREAM->CR = LED_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1;

  memset(rgb_led_bit_buffer, 0, sizeof(uint32_t) * RGB_LED_BIT_BUF_SIZE);
}

void ledSetColour(uint8_t Red, uint8_t Green, uint8_t Blue)
{
  uint8_t i; 
  uint32_t Shift_Reg_Mask;
  uint32_t RGB_Buffer = 0;
  volatile uint32_t ResetTmr = getTmr2MHz();

  LED_DMA_STREAM->CR &= ~DMA_SxCR_EN;
  LED_TIMER->CCR4 = 0;

  RGB_Buffer = RGB2BUFFER(Red,Green,Blue);
  Shift_Reg_Mask = 1 << (RGB_LED_RGB_BITS - 1);
  for (i = 0; i < RGB_LED_RGB_BITS; i++) {
    if ((RGB_Buffer & Shift_Reg_Mask) != 0)
      rgb_led_bit_buffer[i] = RGB_LED_CODE_1_TIME;
    else
      rgb_led_bit_buffer[i] = RGB_LED_CODE_0_TIME;
    
    Shift_Reg_Mask >>= 1;      
  }

  // 280us reset time gap
  while(getTmr2MHz() - ResetTmr <= 600);
  LED_TIMER->CR1 &= ~TIM_CR1_CEN;

  DMA1->HIFCR = LED_DMA_FLAG_TC | LED_DMA_FLAG_ERRORS;
  LED_DMA_STREAM->PAR = CONVERT_PTR_UINT(&LED_TIMER->CCR4);
  LED_DMA_STREAM->M0AR = CONVERT_PTR_UINT(rgb_led_bit_buffer);
  LED_DMA_STREAM->NDTR = RGB_LED_BIT_BUF_SIZE;

  LED_DMA_STREAM->CR |= DMA_SxCR_EN;
  LED_TIMER->CR1 |= TIM_CR1_CEN;
}

void ledOff()
{
  ledSetColour(RGB_COLOUR_OFF);
}

void ledRed()
{
  ledSetColour(RGB_COLOUR_RED);
}

void ledGreen()
{
  ledSetColour(RGB_COLOUR_GREEN);
}

void ledBlue()
{
  ledSetColour(RGB_COLOUR_BLUE);
}
