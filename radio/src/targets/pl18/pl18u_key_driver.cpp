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

#include "hal/key_driver.h"

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "keys.h"

#define REPEAT_DELAY 20

/* The output bit-order has to be (D = L, U = R):
           PL18U
   0  LHL  TR4L
   1  LHR  TR4R
   2  LVD  TR3D
   3  LVU  TR3U
   4  RVD  TR2D
   5  RVU  TR2U
   6  RHL  TR1L
   7  RHR  TR1R
   8  LSD  TR6D
   9  LSU  TR6U 
   10 RSD  TR5D
   11 RSU  TR5U

   12 EX1D KLD
   13 EX1U KLU
   14 EX2D KRD
   15 EX2U KRU
*/

enum PhysicalTrims
{
  TR4L = 0,
  TR4R,
  TR3D = 2,
  TR3U,
  TR2D = 4,
  TR2U,
  TR1L = 6,
  TR1R,
  TR6D = 8,
  TR6U,
  TR5D = 10,
  TR5U,
  KLD = 12,
  KLU,
  KRD = 14,
  KRU,
};

volatile uint32_t rotencDt = 0;
static rotenc_t rotencValue = 0;
static uint8_t lastRotState = 0;
static uint8_t stateCount = 0;

void keysInit()
{
  stm32_gpio_enable_clock(GPIOB);
  stm32_gpio_enable_clock(GPIOC);
  stm32_gpio_enable_clock(GPIOD);
  stm32_gpio_enable_clock(GPIOG);
  stm32_gpio_enable_clock(GPIOH);
  stm32_gpio_enable_clock(GPIOI);
  stm32_gpio_enable_clock(GPIOJ);

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_NO;

  pinInit.Pin = KEYS_GPIOB_PINS;
  LL_GPIO_Init(GPIOB, &pinInit);

  pinInit.Pin = KEYS_GPIOC_PINS;
  LL_GPIO_Init(GPIOC, &pinInit);

  pinInit.Pin = KEYS_GPIOD_PINS;
  LL_GPIO_Init(GPIOD, &pinInit);

  pinInit.Pin = KEYS_GPIOJ_PINS;
  LL_GPIO_Init(GPIOJ, &pinInit);

  // Matrix outputs
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;

  pinInit.Pin = KEYS_OUT_GPIOG_PINS;
  LL_GPIO_Init(GPIOG, &pinInit);

  pinInit.Pin = KEYS_OUT_GPIOH_PINS;
  LL_GPIO_Init(GPIOH, &pinInit);

  pinInit.Pin = KEYS_OUT_GPIOI_PINS;
  LL_GPIO_Init(GPIOI, &pinInit);
}

static uint32_t _readKeyMatrix()
{
    // This function avoids concurrent matrix agitation

    uint32_t result = 0;

    volatile static struct
    {
        uint32_t oldResult = 0;
        uint8_t ui8ReadInProgress = 0;
    } syncelem;

    if (syncelem.ui8ReadInProgress != 0) return syncelem.oldResult;

    // ui8ReadInProgress was 0, increment it
    syncelem.ui8ReadInProgress++;
    // Double check before continuing, as non-atomic, non-blocking so far
    // If ui8ReadInProgress is above 1, then there was concurrent task calling it, exit
    if (syncelem.ui8ReadInProgress > 1) return syncelem.oldResult;

    // If we land here, we have exclusive access to Matrix
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);

    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << KLD;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << KLU;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR4L;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR4R;

    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR3D;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR3U;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR2U;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR2D;

    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR1R;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR1L;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << KRD;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << KRU;
    
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR6U;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR6D;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR5D;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR5U;    
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);

    syncelem.oldResult = result;
    syncelem.ui8ReadInProgress = 0;

    return result;
}

uint32_t readKeys()
{
  uint32_t result = 0;

  uint32_t mkeys = _readKeyMatrix();
  if (mkeys & (1 << KRD)) result |= 1 << KEY_ENTER;
  if (mkeys & (1 << KRU)) result |= 1 << KEY_EXIT;

  uint8_t rotState = 0;
  if (mkeys & (1 << KLD)) rotState |= 1;
  if (mkeys & (1 << KLU)) rotState |= 2;

  if (rotState != lastRotState) {
    lastRotState = rotState;
    stateCount = 0;
  } else {
    stateCount++;
    if (stateCount == 3) {
      if (rotState == 1)
        rotencValue++;
      else if (rotState == 2)
        rotencValue--;
    } else if (stateCount >= REPEAT_DELAY) {
      stateCount = 0;
    }
  }

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  result |= _readKeyMatrix();
  result &= 0xfff;  // Only 12 bits for 6 trims

  return result;
}

rotenc_t rotaryEncoderGetValue()
{
  return rotencValue;
}

void rotaryEncoderInit()
{  
}

