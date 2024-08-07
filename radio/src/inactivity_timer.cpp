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

#include "inactivity_timer.h"
#include "hal/adc_driver.h"
#include "edgetx.h"

void inactivityTimerReset(ActivitySource src)
{
  bool reset_backlight_timer = false;

  switch(src) {
    
  case ActivitySource::Keys:
    if (g_eeGeneral.backlightMode & e_backlight_mode_keys) {
      reset_backlight_timer = true;
    }
    break;

  case ActivitySource::MainControls:
    if (g_eeGeneral.backlightMode & e_backlight_mode_sticks) {
      reset_backlight_timer = true;
    }
    break;
  }

  if (reset_backlight_timer) {
    resetBacklightTimeout();
  }
  
  inactivity.counter = 0;
}

#define INAC_STICKS_SHIFT   7
#define INAC_SWITCHES_SHIFT 8

bool inactivityCheckInputs()
{
  uint8_t sum = 0;

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  auto max_inputs = max_sticks + adcGetMaxInputs(ADC_INPUT_FLEX);

  for (uint8_t i = 0; i < max_inputs; i++) {
    if (i < max_sticks || IS_POT_AVAILABLE(i - max_sticks))
      sum += getAnalogValue(i) >> INAC_STICKS_SHIFT;
  }

  for (uint8_t i = 0; i < getSwitchCount(); i++)
    sum += getValue(MIXSRC_FIRST_SWITCH + i) >> INAC_SWITCHES_SHIFT;

#if defined(IMU)
  for (uint8_t i = 0; i < 2; i++)
    sum += getValue(MIXSRC_TILT_X + i) >> INAC_STICKS_SHIFT;
#endif

#if defined(SPACEMOUSE)
  for (uint8_t i = 0; i < (MIXSRC_LAST_SPACEMOUSE - MIXSRC_FIRST_SPACEMOUSE + 1);
       i++)
    sum += get_spacemouse_value(i) >> INAC_STICKS_SHIFT;
#endif

  if (abs((int8_t)(sum - inactivity.sum)) > 1) {
    inactivity.sum = sum;
    return true;
  } else {
    return false;
  }  
}
