/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

// Rotary encoder simulator: uses KEY_UP / KEY_DOWN buttons (PG.02 / PG.03)
// to drive the encoder position, with repeat and acceleration on hold.

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_timer.h"
#include "board.h"
#include "hal.h"
#include "timers_driver.h"

#include "hal/rotary_encoder.h"

// Repeat timing (units = 10ms ISR ticks)
#define REPEAT_INITIAL_DELAY  40   // 400ms before first repeat
#define REPEAT_INTERVAL_START 10   // 100ms between steps at start
#define REPEAT_INTERVAL_MIN    2   // 20ms between steps at max speed
#define REPEAT_ACCEL_TICKS   100   // ticks of hold until max speed

volatile rotenc_t rotencValue = 0;
volatile uint32_t rotencDt = 0;

rotenc_t rotaryEncoderGetValue()
{
  return rotencValue;
}

static uint8_t lastPins = 0;

// Per-button hold state
static uint16_t holdTicks[2] = {0, 0};
static uint16_t repeatCounter[2] = {0, 0};

static void stepEncoder(int8_t dir)
{
  static uint32_t last_tick = 0;
  rotencValue += dir;
  uint32_t now = timersGetMsTick();
  uint32_t dt = last_tick ? (now - last_tick) : 100;
  rotencDt += dt;
  last_tick = now;
}

static void updateFromButtons()
{
  uint8_t pins = 0;
  if (!LL_GPIO_IsInputPinSet(ROTARY_ENCODER_GPIO, ROTARY_ENCODER_GPIO_PIN_A))
    pins |= 0x01; // KEY_UP  (bit 0)
  if (!LL_GPIO_IsInputPinSet(ROTARY_ENCODER_GPIO, ROTARY_ENCODER_GPIO_PIN_B))
    pins |= 0x02; // KEY_DOWN (bit 1)

  static const int8_t dir[2] = {-1, +1}; // KEY_UP decrements, KEY_DOWN increments

  for (int i = 0; i < 2; i++) {
    uint8_t mask = (1 << i);
    bool pressed = (pins & mask) != 0;
    bool wasPressed = (lastPins & mask) != 0;

    if (!wasPressed && pressed) {
      // Fresh press: step immediately, start hold tracking
      stepEncoder(dir[i]);
      holdTicks[i] = 0;
      repeatCounter[i] = REPEAT_INITIAL_DELAY;
    } else if (pressed) {
      // Held: count ticks and fire on interval
      holdTicks[i]++;
      if (repeatCounter[i] > 0) {
        repeatCounter[i]--;
      } else {
        stepEncoder(dir[i]);
        // Compute next interval: linearly interpolate from START to MIN
        uint16_t hold = holdTicks[i];
        if (hold > REPEAT_ACCEL_TICKS) hold = REPEAT_ACCEL_TICKS;
        uint16_t interval = REPEAT_INTERVAL_START -
            (hold * (REPEAT_INTERVAL_START - REPEAT_INTERVAL_MIN)) / REPEAT_ACCEL_TICKS;
        repeatCounter[i] = interval;
      }
    } else {
      holdTicks[i] = 0;
      repeatCounter[i] = 0;
    }
  }

  lastPins = pins;
}

// Called from charger loop before main init
void rotaryEncoderCheck()
{
  updateFromButtons();
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

  // Periodic 10ms timer for button polling
  stm32_timer_enable_clock(ROTARY_ENCODER_TIMER);
  ROTARY_ENCODER_TIMER->ARR = 9999; // 10ms at 1µS tick
  ROTARY_ENCODER_TIMER->PSC =
      (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1; // 1µS
  ROTARY_ENCODER_TIMER->CCER = 0;
  ROTARY_ENCODER_TIMER->CCMR1 = 0;
  ROTARY_ENCODER_TIMER->EGR = TIM_EGR_UG; // reload PSC/ARR immediately
  ROTARY_ENCODER_TIMER->SR = 0;
  ROTARY_ENCODER_TIMER->CR1 = TIM_CR1_CEN; // continuous
  ROTARY_ENCODER_TIMER->DIER |= TIM_DIER_UIE;

  NVIC_EnableIRQ(ROTARY_ENCODER_TIMER_IRQn);
  NVIC_SetPriority(ROTARY_ENCODER_TIMER_IRQn, 7);

  lastPins = 0;
}

extern "C" void ROTARY_ENCODER_TIMER_IRQHandler(void)
{
  ROTARY_ENCODER_TIMER->SR &= ~TIM_SR_UIF;
  updateFromButtons();
}
