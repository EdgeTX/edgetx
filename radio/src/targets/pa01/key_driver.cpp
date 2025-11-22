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

#include "bsp_io.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_i2c_driver.h"

#include "hal.h"
#include "delays_driver.h"
#include "keys.h"

#define BSP_READ_AFTER_WRITE_DELAY   10 // us
#define BSP_KEY_OUT_MASK                                      \
  (BSP_KEY_OUT1 | BSP_KEY_OUT2 | BSP_KEY_OUT3 | BSP_KEY_OUT4)

/* The output bit-order has to be:
   0  LHL  TR3L (Left equals down)
   1  LHR  TR3R
   2  LVD  TR1D
   3  LVU  TR1U
   4  RVD  TR2D
   5  RVU  TR2U
   6  RHL  TR4L
   7  RHR  TR4R
*/

enum PhysicalKeys
{
  // Trims
  TR3L = 0,
  TR3R,
  TR1D = 2,
  TR1U,
  TR2D = 4,
  TR2U,
  TR4L = 6,
  TR4R,
  // Keys
  PGUP = 8,
  PGDN,
  RTN = 10,
  MODEL,
  KEY1 = 12,
  KEY2,
  KEY3 = 14,
  KEY4,
  ENT  = 16
};

static bool fct_state[4] = {false, false, false, false};
static uint32_t keyState = 0;
#if !defined(BOOT)
static uint32_t nonReadCount = 0;
#endif

void pollKeys()
{
#if !defined(BOOT)
  if(!bsp_get_shouldReadKeys() && nonReadCount < 10)
  {
    if (gpio_read(KEYS_GPIO_ENTER) == 0)
      keyState |= 1<<ENT;

    nonReadCount++;
  }
  nonReadCount = 0;
#endif
  // This function avoids concurrent matrix agitation

  uint32_t result = 0;
  uint16_t bsp_input = 0;

  volatile static struct
  {
    uint32_t oldResult = 0;
    uint8_t ui8ReadInProgress = 0;
  } syncelem;

  if (syncelem.ui8ReadInProgress != 0) {
    keyState = syncelem.oldResult;
  }

  // ui8ReadInProgress was 0, increment it
  syncelem.ui8ReadInProgress++;
  // Double check before continuing, as non-atomic, non-blocking so far
  // If ui8ReadInProgress is above 1, then there was concurrent task calling it, exit
  if (syncelem.ui8ReadInProgress > 1) {
    keyState = syncelem.oldResult;
  }

  // If we land here, we have exclusive access to Matrix
  bsp_output_set(BSP_KEY_OUT_MASK, ~BSP_KEY_OUT1);
  delay_us(BSP_READ_AFTER_WRITE_DELAY);
  bsp_input = bsp_input_get();
  if ((bsp_input & BSP_KEY_IN1) == 0)
    result |= 1<<TR1U;
  if ((bsp_input & BSP_KEY_IN2) == 0)
    result |= 1<<TR1D;
  if ((bsp_input & BSP_KEY_IN3) == 0)
    result |= 1<<TR2U;
  if ((bsp_input & BSP_KEY_IN4) == 0)
    result |= 1<<TR2D;

  bsp_output_set(BSP_KEY_OUT_MASK, ~BSP_KEY_OUT2);
  delay_us(BSP_READ_AFTER_WRITE_DELAY);
  bsp_input = bsp_input_get();
  if ((bsp_input & BSP_KEY_IN1) == 0)
    result |= 1<<TR3L;
  if ((bsp_input & BSP_KEY_IN2) == 0)
    result |= 1<<TR3R;
  if ((bsp_input & BSP_KEY_IN3) == 0)
    result |= 1<<TR4L;
  if ((bsp_input & BSP_KEY_IN4) == 0)
    result |= 1<<TR4R;

  bsp_output_set(BSP_KEY_OUT_MASK, ~BSP_KEY_OUT3);
  delay_us(BSP_READ_AFTER_WRITE_DELAY);
  bsp_input = bsp_input_get();
  if ((bsp_input & BSP_KEY_IN1) == 0)
    result |= 1<<PGDN;
  if ((bsp_input & BSP_KEY_IN2) == 0)
    result |= 1<<PGUP;
  if ((bsp_input & BSP_KEY_IN3) == 0)
    result |= 1<<RTN;
  if ((bsp_input & BSP_KEY_IN4) == 0)
    result |= 1<<MODEL;

  bsp_output_set(BSP_KEY_OUT_MASK, ~BSP_KEY_OUT4);
  delay_us(BSP_READ_AFTER_WRITE_DELAY);
  bsp_input = bsp_input_get();
  if ((bsp_input & BSP_KEY_IN1) == 0)
    result |= 1<<KEY1;
  if ((bsp_input & BSP_KEY_IN2) == 0)
    result |= 1<<KEY2;
  if ((bsp_input & BSP_KEY_IN3) == 0)
    result |= 1<<KEY3;
  if ((bsp_input & BSP_KEY_IN4) == 0)
    result |= 1<<KEY4;

  if (gpio_read(KEYS_GPIO_ENTER) == 0)
    result |= 1<<ENT;

  syncelem.oldResult = result;
  syncelem.ui8ReadInProgress = 0;

  bsp_output_set(BSP_KEY_OUT_MASK, 0);
  bsp_get_shouldReadKeys();

  fct_state[0] = (result & 1<<KEY1)?true:false;
  fct_state[1] = (result & 1<<KEY2)?true:false;
  fct_state[2] = (result & 1<<KEY3)?true:false;
  fct_state[3] = (result & 1<<KEY4)?true:false;

  keyState = result;
}

void keysInit()
{
  gpio_init(KEYS_GPIO_ENTER, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
}

uint32_t readKeys()
{
  uint32_t result = 0;

  uint32_t mkeys = keyState;
  if (mkeys & (1 << PGUP)) result |= 1 << KEY_PAGEUP;
  if (mkeys & (1 << PGDN)) result |= 1 << KEY_PAGEDN;
  if (mkeys & (1 << RTN))  result |= 1 << KEY_EXIT;
  if (mkeys & (1 << MODEL)) result |= 1 << KEY_MODEL;
  if (mkeys & (1 << ENT))  result |= 1 << KEY_ENTER;

  return result;
}

uint32_t readTrims()
{
  uint32_t mkeys = keyState;

  return mkeys & 0xff;  // Mask only the trims output
}

bool getFctKeyState(int index)
{
  return fct_state[index];
}
