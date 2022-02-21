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

#include "opentx_types.h"
#include "board.h"
#include "keys.h"

#include "hal/adc_driver.h"

#if defined(LUA)
#include "lua/lua_api.h"
#endif

enum PhysicalTrims
{
    TR3D,
    TR3U,
    TR4D,
    TR4U,
    TR5D,
    TR5U,
    TR6D,
    TR6U,
    TR7L,
    TR7R,
    TR8L,
    TR8R
};

uint32_t readKeyMatrix()
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
    GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    GPIO_SetBits(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR7L;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR7R;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR5D;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR5U;

    GPIO_SetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR3D;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR3U;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR4U;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR4D;

    GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    delay_us(10);
    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << TR6U;
    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << TR6D;
    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << TR8L;
    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << TR8R;
    GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    syncelem.oldResult = result;
    syncelem.ui8ReadInProgress = 0;
    return result;
}

uint32_t readKeys()
{
  uint32_t result = 0;
  bool getKeys = true;

/*
#if defined(LUA)
  if (!isLuaStandaloneRunning()) {
    getKeys = false;
  }
#endif
*/

  uint32_t mkeys = readKeyMatrix();
  if (getKeys) {
    if (~TRIMS_GPIO_REG_TR1U & TRIMS_GPIO_PIN_TR1U)
      result |= 1 << KEY_RADIO;
    if (~TRIMS_GPIO_REG_TR1D & TRIMS_GPIO_PIN_TR1D)
      result |= 1 << KEY_MODEL;
    if (~TRIMS_GPIO_REG_TR2U & TRIMS_GPIO_PIN_TR2U)
      result |= 1 << KEY_TELEM;

    if (mkeys & (1 << TR3U)) result |= 1 << KEY_PGUP;
    if (mkeys & (1 << TR3D)) result |= 1 << KEY_PGDN;
  }

  // Enter and Exit are always supported
  if (mkeys & (1 << TR4D)) result |= 1 << KEY_ENTER;
  if (mkeys & (1 << TR4U)) result |= 1 << KEY_EXIT;

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  bool getTrim = true;
#if defined(LUA)
  if (isLuaStandaloneRunning()) {
    getTrim = false;
  }
#endif
  if(!getTrim) return result;
  /* The output bit-order has to be:
      0 LHL  TR7L (Left equals down)
      1 LHR  TR7R
      2 LVD  TR5D
      3 LVU  TR5U
      4 RVD  TR6D
      5 RVU  TR6U
      6 RHL  TR8L
      7 RHR  TR8R
      8 LSD  TR1D
      9 LSU  TR1U
     10 RSD  TR2D
     11 RSU  TR2U
     12 EX1D TR3D
     13 EX1U TR3U
     14 EX2D TR4D
     15 EX2U TR4U
     */

  if (~TRIMS_GPIO_REG_TR1U & TRIMS_GPIO_PIN_TR1U)
    result |= 1 << (TRM_LS_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_TR1D & TRIMS_GPIO_PIN_TR1D)
    result |= 1 << (TRM_LS_DWN - TRM_BASE);

  if (~TRIMS_GPIO_REG_TR2U & TRIMS_GPIO_PIN_TR2U)
    result |= 1 << (TRM_RS_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_TR2D & TRIMS_GPIO_PIN_TR2D)
    result |= 1 << (TRM_RS_DWN - TRM_BASE);

  uint32_t mkeys = readKeyMatrix();
  if (mkeys & (1 << TR3D)) result |= (1 << (TRM_EX1_DWN - TRM_BASE));
  if (mkeys & (1 << TR3U)) result |= (1 << (TRM_EX1_UP  - TRM_BASE));
  if (mkeys & (1 << TR4D)) result |= (1 << (TRM_EX2_DWN - TRM_BASE));
  if (mkeys & (1 << TR4U)) result |= (1 << (TRM_EX2_UP  - TRM_BASE));
  if (mkeys & (1 << TR5D)) result |= (1 << (TRM_LV_DWN  - TRM_BASE));
  if (mkeys & (1 << TR5U)) result |= (1 << (TRM_LV_UP   - TRM_BASE));
  if (mkeys & (1 << TR6D)) result |= (1 << (TRM_RV_DWN  - TRM_BASE));
  if (mkeys & (1 << TR6U)) result |= (1 << (TRM_RV_UP   - TRM_BASE));
  if (mkeys & (1 << TR7L)) result |= (1 << (TRM_LH_DWN  - TRM_BASE));
  if (mkeys & (1 << TR7R)) result |= (1 << (TRM_LH_UP   - TRM_BASE));
  if (mkeys & (1 << TR8L)) result |= (1 << (TRM_RH_DWN  - TRM_BASE));
  if (mkeys & (1 << TR8R)) result |= (1 << (TRM_RH_UP   - TRM_BASE));
  return result;
}

bool trimDown(uint8_t idx)
{
  return readTrims() & (1 << idx);
}

bool keyDown()
{
  return readKeys() || readTrims();
}

/* TODO common to ARM */
void readKeysAndTrims()
{
  int i;

  uint8_t index = 0;
  uint32_t in = readKeys();
  uint32_t trims = readTrims();

  for (i = 0; i < TRM_BASE; i++) {
    keys[index++].input(in & (1 << i));
  }

  for (i = 1; i <= 1 << (TRM_LAST-TRM_BASE); i <<= 1) {
    keys[index++].input(trims & i);
  }
/*
  if ((in || trims) && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
    // on keypress turn the light on
    resetBacklightTimeout();
  }
  */
}

#if !defined(BOOT)
uint32_t switchState(uint8_t index)
{
  uint8_t analogIdx = 0;
  // Switches A and C are wired to digital inputs, other switches are sampled via analog inputs.
  switch (index)
  {
    // Switch A:
    case SW_SA0: return (SWITCHES_GPIO_REG_A_L & SWITCHES_GPIO_PIN_A_L); break;
    case SW_SA2: return (~SWITCHES_GPIO_REG_A_L & SWITCHES_GPIO_PIN_A_L); break;
    // Switch B:
    case SW_SB0:
    case SW_SB1:
    case SW_SB2: analogIdx = 9; break;
    // Switch C:
    case SW_SC0: return (SWITCHES_GPIO_REG_C_L & SWITCHES_GPIO_PIN_C_L); break;
    case SW_SC2: return (~SWITCHES_GPIO_REG_C_L & SWITCHES_GPIO_PIN_C_L); break;
    default:
      // all further cases are analog switches (SWD, SWE, SWF, SWG, SWH) and follow a pattern
      analogIdx = ((index - SW_SD0)/3) + 10; // SWD is analog 10, SWE 11 and so on
      break;
  }

  uint16_t value = getAnalogValue(analogIdx);
  uint8_t position;

  if (value < 1024)
    position = 0;
  else if (value > 3 * 1024)
    position = 2;
  else
    position = 1;
  return position == (index % 3);
}
#endif

void monitorInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

  GPIO_InitStructure.GPIO_Pin = VBUS_MONITOR_PIN;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);
}

void keysInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOB_PINS;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOC_PINS;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOD_PINS;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOF_PINS;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOH_PINS;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOJ_PINS;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  // Matrix outputs
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = KEYS_OUT_GPIOG_PINS;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_OUT_GPIOH_PINS;
  GPIO_Init(GPIOH, &GPIO_InitStructure);
}
