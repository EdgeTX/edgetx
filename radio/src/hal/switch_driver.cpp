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

#include "switch_driver.h"

#include "definitions.h"
#include "hal/adc_driver.h"

#include "dataconstants.h"
#include "edgetx_helpers.h"
#include "edgetx.h"

// Board API
void boardInitSwitches();

SwitchHwPos boardSwitchGetPosition(uint8_t cat, uint8_t idx);

const char* boardSwitchGetName(uint8_t cat, uint8_t idx);
SwitchHwType boardSwitchGetType(uint8_t cat, uint8_t idx);

uint8_t boardGetMaxSwitches();
uint8_t boardGetMaxFctSwitches();

swconfig_t boardSwitchGetDefaultConfig();

constexpr uint8_t _INVALID_ADC_CH = 0xFF;

static uint8_t _flex_switches[MAX_FLEX_SWITCHES];

void switchInit()
{
  memset(_flex_switches, _INVALID_ADC_CH, sizeof(_flex_switches));
  boardInitSwitches();
}

swconfig_t switchGetDefaultConfig() { return boardSwitchGetDefaultConfig(); }

uint8_t switchGetMaxSwitches()
{
  return boardGetMaxSwitches() + MAX_FLEX_SWITCHES;
}

uint8_t switchGetMaxFctSwitches() { return boardGetMaxFctSwitches(); }

uint32_t switchState(uint8_t pos_idx)
{
  auto d = div(pos_idx, 3);
  auto idx = (uint8_t)d.quot;
  auto pos = (SwitchHwPos)d.rem;

  return switchGetPosition(idx) == pos;
}

SwitchHwPos switchGetPosition(uint8_t sw_idx)
{
  auto idx = (int)sw_idx;
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) {
    return boardSwitchGetPosition(SWITCH_PHYSICAL, idx);
  }

  idx -= max_switches;
  if (MAX_FLEX_SWITCHES > 0) {
    if (idx < MAX_FLEX_SWITCHES) {
      auto channel = _flex_switches[idx];
      SwitchHwPos ret = SWITCH_HW_UP;

      if (channel != _INVALID_ADC_CH) {
        auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
        uint16_t value = anaIn(channel + offset);
        if (value > 3 * 512) {
          ret = SWITCH_HW_DOWN;
        } else if (value >= 512) {
          ret = SWITCH_HW_MID;
        }
      }

      return ret;
    }

    idx -= MAX_FLEX_SWITCHES;
  }

  if (idx < boardGetMaxFctSwitches()) {
    return boardSwitchGetPosition(SWITCH_FUNCTION, idx);
  }
  
  return SWITCH_HW_UP;
}

static const char * const _flex_sw_canon_names[] = {
  "FL1", "FL2", "FL3", "FL4"
};

static_assert(DIM(_flex_sw_canon_names) >= MAX_FLEX_SWITCHES,
	      "Missing canonical name for flex switches");

const char* switchGetName(uint8_t sw_idx)
{
  auto idx = (int)sw_idx;
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) {
    return boardSwitchGetName(SWITCH_PHYSICAL, idx);
  }

  idx -= max_switches;
  if (MAX_FLEX_SWITCHES > 0) {
    if (idx < MAX_FLEX_SWITCHES) {
      return _flex_sw_canon_names[idx];
    }

    idx -= MAX_FLEX_SWITCHES;
  }

  if (idx < boardGetMaxFctSwitches()) {
    return boardSwitchGetName(SWITCH_FUNCTION, idx);
  }
  
  return nullptr;
}

SwitchHwType switchGetHwType(uint8_t sw_idx)
{
  auto idx = (int)sw_idx;
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) {
    return boardSwitchGetType(SWITCH_PHYSICAL, idx);
  }

  idx -= max_switches;
  if (idx < MAX_FLEX_SWITCHES) {
    return SWITCH_HW_3POS;
  }

  return SWITCH_HW_2POS;
}

bool switchIsFlex(uint8_t idx)
{
  auto max_switches = boardGetMaxSwitches();
  return idx >= max_switches && idx < max_switches + MAX_FLEX_SWITCHES;
}

void switchConfigFlex_raw(uint8_t idx, int8_t channel)
{
  if (!MAX_FLEX_SWITCHES || idx > MAX_FLEX_SWITCHES) return;
  if ((int)channel >= (int)adcGetMaxInputs(ADC_INPUT_FLEX)) return;
  
  auto& sw_channel = _flex_switches[idx];
  if (channel < 0) {
    sw_channel = _INVALID_ADC_CH;
  } else if (POT_CONFIG((uint8_t)channel) == FLEX_SWITCH) {
    sw_channel = (uint8_t)channel;
  }
}

uint8_t switchGetFlexChannel(uint8_t idx)
{
  return _flex_switches[idx];
}

void switchConfigFlex(uint8_t idx, int8_t channel)
{
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) return;
  idx -= max_switches;
  switchConfigFlex_raw(idx, channel);
}

int8_t switchGetFlexConfig_raw(uint8_t idx)
{
  if (!MAX_FLEX_SWITCHES || idx > MAX_FLEX_SWITCHES) return _INVALID_ADC_CH;
  return (int8_t)_flex_switches[idx];
}

int8_t switchGetFlexConfig(uint8_t idx)
{
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) return _INVALID_ADC_CH;
  idx -= max_switches;
  return switchGetFlexConfig_raw(idx);
}

bool switchIsFlexValid_raw(uint8_t sw_idx)
{
  auto idx = (int)sw_idx;
  return MAX_FLEX_SWITCHES > 0 && idx < MAX_FLEX_SWITCHES &&
         _flex_switches[idx] != _INVALID_ADC_CH;
}

bool switchIsFlexValid(uint8_t idx)
{
  auto max_switches = boardGetMaxSwitches();
  if (idx < max_switches) return false;
  idx -= max_switches;
  return switchIsFlexValid_raw(idx);
}

static bool is_flex_input_available(uint8_t flex_idx, uint8_t channel)
{
  for (int i = 0; i < MAX_FLEX_SWITCHES; i++) {
    if (_flex_switches[i] == channel && (int)flex_idx != i)
      return false;
  }

  return true;
}

bool switchIsFlexInputAvailable(uint8_t idx, uint8_t channel)
{
  if (MAX_FLEX_SWITCHES == 0) return false;
  if (POT_CONFIG(channel) != FLEX_SWITCH) return false;

  auto phy_switches = boardGetMaxSwitches();
  if (idx < phy_switches) return false;

  return is_flex_input_available(idx - phy_switches, channel);
}

static void invalidate_flex_config(uint8_t flex_idx)
{
  auto sw_idx = flex_idx + boardGetMaxSwitches();
  swconfig_t mask = (swconfig_t)SWITCH_CONFIG_MASK(sw_idx);
  g_eeGeneral.switchConfig &= ~mask;

  _flex_switches[flex_idx] = _INVALID_ADC_CH;
}

void switchFixFlexConfig()
{
  for (int i = 0; i < MAX_FLEX_SWITCHES; i++) {
    auto channel = _flex_switches[i];
    if (channel != _INVALID_ADC_CH && POT_CONFIG(channel) != FLEX_SWITCH) {
      invalidate_flex_config(i);
    }
  }
}
