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

/* The output bit-order has to be:
   0  LHL  TR7L (Left equals down)
   1  LHR  TR7R
   2  LVD  TR5D
   3  LVU  TR5U
   4  RVD  TR6D
   5  RVU  TR6U
   6  RHL  TR8L
   7  RHR  TR8R
   8  LSD  TR1D
   9  LSU  TR1U
   10 RSD  TR2D
   11 RSU  TR2U
   12 EX1D TR3D
   13 EX1U TR3U
   14 EX2D TR4D
   15 EX2U TR4U
*/

enum PhysicalTrims
{
    TR7L = 0,
    TR7R,
    TR5D = 2,
    TR5U,
    TR6D = 4,
    TR6U,
    TR8L = 6,
    TR8R,
    TR1D = 8,
    TR1U,
    TR2D = 10,
    TR2U,
    TR3D = 12,
    TR3U,
    TR4D = 14,
    TR4U,
};

void keysInit()
{
  stm32_gpio_enable_clock(GPIOB);
  stm32_gpio_enable_clock(GPIOC);
  stm32_gpio_enable_clock(GPIOD);
  stm32_gpio_enable_clock(GPIOG);
  stm32_gpio_enable_clock(GPIOH);
  stm32_gpio_enable_clock(GPIOJ);

  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_DOWN;

  pinInit.Pin = KEYS_GPIOB_PINS;
  LL_GPIO_Init(GPIOB, &pinInit);

  pinInit.Pin = KEYS_GPIOC_PINS;
  LL_GPIO_Init(GPIOC, &pinInit);

  pinInit.Pin = KEYS_GPIOD_PINS;
  LL_GPIO_Init(GPIOD, &pinInit);

  pinInit.Pin = KEYS_GPIOH_PINS;
  LL_GPIO_Init(GPIOH, &pinInit);

  pinInit.Pin = KEYS_GPIOJ_PINS;
  LL_GPIO_Init(GPIOJ, &pinInit);

  // Matrix outputs
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_NO;

  pinInit.Pin = KEYS_OUT_GPIOG_PINS;
  LL_GPIO_Init(GPIOG, &pinInit);

  pinInit.Pin = KEYS_OUT_GPIOH_PINS;
  LL_GPIO_Init(GPIOH, &pinInit);
}

static uint32_t _readKeyMatrix()
{
    // This function avoids concurrent matrix agitation

    uint32_t result = 0;
    /* Bit  0 - TR3 down
     * Bit  1 - TR3 up
     * Bit  2 - TR4 down
     * Bit  3 - TR4 up
     * Bit  4 - TR5 down
     * Bit  5 - TR5 up
     * Bit  6 - TR6 down
     * Bit  7 - TR6 up
     * Bit  8 - TR7 left
     * Bit  9 - TR7 right
     * Bit 10 - TR8 left
     * Bit 11 - TR8 right
     */

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
       result |= 1 << TR7L;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR7R;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR5D;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR5U;

    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR3D;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR3U;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR4U;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR4D;

    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    LL_GPIO_ResetOutputPin(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR6U;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR6D;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR8L;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR8R;
    
    LL_GPIO_SetOutputPin(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    
    syncelem.oldResult = result;
    syncelem.ui8ReadInProgress = 0;

    return result;
}

uint32_t readKeys()
{
  uint32_t result = 0;

  if (getHatsAsKeys()) {
    uint32_t mkeys = _readKeyMatrix();
    if (mkeys & (1 << TR4D)) result |= 1 << KEY_ENTER;
    if (mkeys & (1 << TR4U)) result |= 1 << KEY_EXIT;
  }

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  result |= _readKeyMatrix();

  if (~TRIMS_GPIO_REG_TR1U & TRIMS_GPIO_PIN_TR1U)
    result |= 1 << (TR1U);
  if (~TRIMS_GPIO_REG_TR1D & TRIMS_GPIO_PIN_TR1D)
    result |= 1 << (TR1D);

  if (~TRIMS_GPIO_REG_TR2U & TRIMS_GPIO_PIN_TR2U)
    result |= 1 << (TR2U);
  if (~TRIMS_GPIO_REG_TR2D & TRIMS_GPIO_PIN_TR2D)
    result |= 1 << (TR2D);

  return result;
}
