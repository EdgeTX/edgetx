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

#define BSP_KEY_OUT_MASK                                      \
  (BSP_KEY_OUT1 | BSP_KEY_OUT2 | BSP_KEY_OUT3 | BSP_KEY_OUT4)

#define KEYS_GPIO_ENTER GPIO_PIN(GPIOG, 13) // PG.13

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

extern bool suspendI2CTasks;

static bool fct_state[4] = {false, false, false, false};
static uint32_t keyState = 0;

#define SCAN_COLS 4
#define IDLE_FORCE_SCAN 25 // cycles (250ms)

#if !defined(BOOT)
static uint8_t scan_col = 0;
static uint8_t read_col = 0xFF;
static uint8_t scan_pending = SCAN_COLS;
static uint8_t idle_cycles = 0;
static uint32_t col_cache[SCAN_COLS] = {0, 0, 0, 0};
#endif

static const uint16_t col_drive[SCAN_COLS] = {
  (uint16_t)~BSP_KEY_OUT1, (uint16_t)~BSP_KEY_OUT2,
  (uint16_t)~BSP_KEY_OUT3, (uint16_t)~BSP_KEY_OUT4
};

static uint32_t read_col_to_keys(uint8_t col, uint16_t inputs)
{
  uint32_t result = 0;
  if (col == 0) {
    if ((inputs & BSP_KEY_IN1) == 0) result |= 1 << TR1U;
    if ((inputs & BSP_KEY_IN2) == 0) result |= 1 << TR1D;
    if ((inputs & BSP_KEY_IN3) == 0) result |= 1 << TR2U;
    if ((inputs & BSP_KEY_IN4) == 0) result |= 1 << TR2D;
  } else if (col == 1) {
    if ((inputs & BSP_KEY_IN1) == 0) result |= 1 << TR3L;
    if ((inputs & BSP_KEY_IN2) == 0) result |= 1 << TR3R;
    if ((inputs & BSP_KEY_IN3) == 0) result |= 1 << TR4L;
    if ((inputs & BSP_KEY_IN4) == 0) result |= 1 << TR4R;
  } else if (col == 2) {
    if ((inputs & BSP_KEY_IN1) == 0) result |= 1 << PGDN;
    if ((inputs & BSP_KEY_IN2) == 0) result |= 1 << PGUP;
    if ((inputs & BSP_KEY_IN3) == 0) result |= 1 << RTN;
    if ((inputs & BSP_KEY_IN4) == 0) result |= 1 << MODEL;
  } else {
    if ((inputs & BSP_KEY_IN1) == 0) result |= 1 << KEY1;
    if ((inputs & BSP_KEY_IN2) == 0) result |= 1 << KEY2;
    if ((inputs & BSP_KEY_IN3) == 0) result |= 1 << KEY3;
    if ((inputs & BSP_KEY_IN4) == 0) result |= 1 << KEY4;
  }
  return result;
}

void pollKeys()
{
#if defined(BOOT)
  uint32_t ent_mask = 0;
  if (gpio_read(KEYS_GPIO_ENTER) == 0)
    ent_mask = (1 << ENT);

  if (suspendI2CTasks) {
    keyState = ent_mask;
    return;
  }

  uint32_t result = 0;
  for (uint8_t col = 0; col < SCAN_COLS; col++) {
    bsp_output_set(BSP_KEY_OUT_MASK, col_drive[col]);
    delay_us(10);
    result |= read_col_to_keys(col, bsp_input_get());
  }

  result |= ent_mask;
  bsp_output_set(BSP_KEY_OUT_MASK, 0);
  bsp_get_shouldReadKeys();

  fct_state[0] = (result & (1 << KEY1)) ? true : false;
  fct_state[1] = (result & (1 << KEY2)) ? true : false;
  fct_state[2] = (result & (1 << KEY3)) ? true : false;
  fct_state[3] = (result & (1 << KEY4)) ? true : false;

  keyState = result;
#else
  uint32_t ent_mask = 0;
  if (gpio_read(KEYS_GPIO_ENTER) == 0)
    ent_mask = (1 << ENT);

  if (suspendI2CTasks) {
    keyState = col_cache[0] | col_cache[1] | col_cache[2] | col_cache[3] | ent_mask;
    return;
  }

  volatile static struct
  {
    uint32_t oldResult = 0;
    uint8_t ui8ReadInProgress = 0;
  } syncelem;

  if (syncelem.ui8ReadInProgress != 0) {
    keyState = syncelem.oldResult;
    return;
  }

  syncelem.ui8ReadInProgress++;
  if (syncelem.ui8ReadInProgress > 1) {
    keyState = syncelem.oldResult;
    syncelem.ui8ReadInProgress--;
    return;
  }

  if (bsp_get_shouldReadKeys()) {
    scan_pending = SCAN_COLS;
    idle_cycles = 0;
  }

  if (scan_pending == 0) {
    idle_cycles++;
    if (idle_cycles >= IDLE_FORCE_SCAN) {
      scan_pending = SCAN_COLS;
      idle_cycles = 0;
    }
  }

  if (read_col < SCAN_COLS && scan_pending > 0) {
    uint16_t inputs = bsp_input_get();
    col_cache[read_col] = read_col_to_keys(read_col, inputs);
    scan_pending--;
  }

  read_col = scan_col;
  bsp_output_set(BSP_KEY_OUT_MASK, col_drive[scan_col]);
  scan_col = (scan_col + 1) % SCAN_COLS;

  uint32_t result = col_cache[0] | col_cache[1] | col_cache[2] | col_cache[3] | ent_mask;

  syncelem.oldResult = result;
  syncelem.ui8ReadInProgress = 0;

  fct_state[0] = (result & (1 << KEY1)) ? true : false;
  fct_state[1] = (result & (1 << KEY2)) ? true : false;
  fct_state[2] = (result & (1 << KEY3)) ? true : false;
  fct_state[3] = (result & (1 << KEY4)) ? true : false;

  keyState = result;
#endif
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
