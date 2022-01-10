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

#include "opentx.h"
#include "hal/adc_driver.h"

uint32_t readKeys()
{
  uint32_t result = 0;
  bool getKeys = true;

/* TODO! Uncomment, only for testing
#if defined(LUA)
  if (!isLuaStandaloneRunning()) {
    getKeys = false;
  }
#endif
*/

  if (getKeys) {
    GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
    GPIO_SetBits(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    delay_us(10);

    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << KEY_RADIO; // TR7 left

    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << KEY_MODEL; // TR7 right

    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << KEY_TELEM; // TR5 down

    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << KEY_PGUP; // TR5 up

    GPIO_SetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    delay_us(10);

    if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
       result |= 1 << KEY_DOWN; // TR3 down

    if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
       result |= 1 << KEY_UP; // TR3 up

    if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
       result |= 1 << KEY_LEFT; // TR4 up

    if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
       result |= 1 << KEY_RIGHT; // TR4 down

    GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
    GPIO_ResetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
  }

  // Enter and Exit are always supported
  GPIO_SetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
  GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
  GPIO_SetBits(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);
  GPIO_ResetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
  delay_us(10);

  if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
     result |= 1 << KEY_ENTER; // TR6 up

  if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
     result |= 1 << KEY_EXIT; // TR6 down

  GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
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
  if (~TRIMS_GPIO_REG_TR1U & TRIMS_GPIO_PIN_TR1U)
    result |= 1 << (TRM1_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_TR1D & TRIMS_GPIO_PIN_TR1D)
    result |= 1 << (TRM1_DWN - TRM_BASE);

  if (~TRIMS_GPIO_REG_TR2U & TRIMS_GPIO_PIN_TR2U)
    result |= 1 << (TRM2_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_TR2D & TRIMS_GPIO_PIN_TR2D)
    result |= 1 << (TRM2_DWN - TRM_BASE);

  // Extract the matrix trims
  GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
  GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
  GPIO_SetBits(TRIMS_GPIO_OUT4, TRIMS_GPIO_OUT4_PIN);
  GPIO_ResetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
  delay_us(10);
  if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
     result |= 1 << (TRM7_LEFT - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
     result |= 1 << (TRM7_RIGHT - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
     result |= 1 << (TRM5_DWN - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
     result |= 1 << (TRM5_UP - TRM_BASE);

  GPIO_SetBits(TRIMS_GPIO_OUT1, TRIMS_GPIO_OUT1_PIN);
  GPIO_ResetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
  delay_us(10);
  if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
     result |= 1 << (TRM3_DWN - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
     result |= 1 << (TRM3_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
     result |= 1 << (TRM4_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
     result |= 1 << (TRM4_DWN - TRM_BASE);

  GPIO_SetBits(TRIMS_GPIO_OUT2, TRIMS_GPIO_OUT2_PIN);
  GPIO_ResetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
  delay_us(10);
  if (~TRIMS_GPIO_REG_IN1 & TRIMS_GPIO_PIN_IN1)
     result |= 1 << (TRM6_UP - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN2 & TRIMS_GPIO_PIN_IN2)
     result |= 1 << (TRM6_DWN - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN3 & TRIMS_GPIO_PIN_IN3)
     result |= 1 << (TRM8_LEFT - TRM_BASE);
  if (~TRIMS_GPIO_REG_IN4 & TRIMS_GPIO_PIN_IN4)
     result |= 1 << (TRM8_RIGHT - TRM_BASE);
  GPIO_SetBits(TRIMS_GPIO_OUT3, TRIMS_GPIO_OUT3_PIN);
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

  if ((in || trims) && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
    // on keypress turn the light on
    resetBacklightTimeout();
  }
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
