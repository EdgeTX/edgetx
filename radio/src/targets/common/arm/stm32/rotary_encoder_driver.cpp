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
#include "stm32_timer.h"

#if !defined(BOOT)
  #include "edgetx.h"
#endif

#if ROTARY_ENCODER_GRANULARITY == 2
  #define ON_DETENT(p) ((p == 3) || (p == 0))
#elif ROTARY_ENCODER_GRANULARITY == 4
  #define ON_DETENT(p) (p == 3)
#elif
#error "Unknown ROTARY_ENCODER_GRANULARITY"
#endif

volatile rotenc_t rotencValue = 0;
volatile uint32_t rotencDt = 0;

// Last encoder pins state
static uint8_t lastPins = 0;
// Record encoder position change between detents
int8_t reChgPos = 0;
// Used on start to ignore movement until encoder position on detent
bool skipUntilDetent = false;

rotenc_t rotaryEncoderGetValue()
{
  return rotencValue;
}

void rotaryEncoderCheck()
{
  // Value increment for each state transition of the RE pins
#if defined(ROTARY_ENCODER_INVERTED)
  static int8_t reInc[4][4] = {
    // Prev = 0
    {  0, -1,  1, -2 },
    // Prev = 1
    {  1,  0,  0, -1 },
    // Prev = 2
    { -1,  0,  0,  1 },
    // Prev = 3
    {  2,  1, -1,  0 },
  };
#else
  static int8_t reInc[4][4] = {
    // Prev = 0
    {  0,  1, -1,  2 },
    // Prev = 1
    { -1,  0,  0,  1 },
    // Prev = 2
    {  1,  0,  0, -1 },
    // Prev = 3
    { -2, -1,  1,  0 },
  };
#endif

  uint8_t pins = ROTARY_ENCODER_POSITION();

  // No change - do nothing
  if (pins == lastPins) {
    return;
  }

  // Handle case where radio started with encoder not on detent position
  if (skipUntilDetent) {
    if (ON_DETENT(pins)) {
      lastPins = pins;
      skipUntilDetent = false;
    }
    return;
  }

  // Get increment value for pin state transition
  int inc = reInc[lastPins][pins];

#if !defined(BOOT)
  if (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_BOTH)
    inc = -inc;
#endif

  // Update position change between detents
  reChgPos += inc;

  // Update reported value on full detent change
  if (reChgPos >= ROTARY_ENCODER_GRANULARITY) {
    // If ENTER pressed - ignore scrolling
    if ((readKeys() & (1 << KEY_ENTER)) == 0) {
      rotencValue += 1;
    }
    reChgPos -= ROTARY_ENCODER_GRANULARITY;
  } else if (reChgPos <= -ROTARY_ENCODER_GRANULARITY) {
    // If ENTER pressed - ignore scrolling
    if ((readKeys() & (1 << KEY_ENTER)) == 0) {
      rotencValue -= 1;
    }
    reChgPos += ROTARY_ENCODER_GRANULARITY;
  }

  lastPins = pins;

#if !defined(BOOT) && defined(COLORLCD)
  static uint32_t last_tick = 0;
  static rotenc_t last_value = 0;

  rotenc_t value = rotencValue;
  rotenc_t diff = (value - last_value);

  if (diff != 0) {
    uint32_t now = RTOS_GET_MS();
    uint32_t dt = now - last_tick;
    // pre-compute accumulated dt (dx/dt is done later in LVGL driver)
    rotencDt += dt;
    last_tick = now;
    last_value = value;
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

  stm32_timer_enable_clock(ROTARY_ENCODER_TIMER);
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

  // Get initial position
  lastPins = ROTARY_ENCODER_POSITION();
  skipUntilDetent = !ON_DETENT(lastPins);
}

extern "C" void ROTARY_ENCODER_TIMER_IRQHandler(void)
{
  ROTARY_ENCODER_TIMER->SR &= ~TIM_SR_UIF;
  ROTARY_ENCODER_TIMER->CR1 = 0;
  rotaryEncoderCheck();
}
