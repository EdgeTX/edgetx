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

#include "hal/switch_driver.h"
#include "stm32_switch_driver.h"

#include "hal.h"
#include "hal/adc_driver.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_gpio_driver.h"

// G11P switches: SW1 is a digital switch (PH4), SW2/SW3/SW4 are analog lines
// (RAW3/RAW4/RAW5 - see hal_g11p.h for the input index map).
//
// TODO(hardware): switch types and analog thresholds must be validated.

// Analog input indices
#define G11P_ADC_SW2   5
#define G11P_ADC_SW3   6
#define G11P_ADC_SW4   7

static const char _switch_names[][4] = {"SW1", "SW2", "SW3", "SW4"};

void boardInitSwitches()
{
  gpio_init(GPIO_PIN(GPIOH, 4), GPIO_IN, GPIO_PIN_SPEED_LOW); // SW1
}

static SwitchHwPos _adc_switch_position(uint8_t adc_index)
{
  uint16_t value = getAnalogValue(adc_index);
  if (value > 2730) return SWITCH_HW_DOWN;
  if (value > 1365) return SWITCH_HW_MID;
  return SWITCH_HW_UP;
}

SwitchHwPos boardSwitchGetPosition(SwitchCategory cat, uint8_t idx)
{
  (void)cat;

  switch (idx) {
    case 0: // SW1 (digital)
      return gpio_read(GPIO_PIN(GPIOH, 4)) ? SWITCH_HW_UP : SWITCH_HW_DOWN;
    case 1:
      return _adc_switch_position(G11P_ADC_SW2);
    case 2:
      return _adc_switch_position(G11P_ADC_SW3);
    case 3:
      return _adc_switch_position(G11P_ADC_SW4);
  }

  return SWITCH_HW_UP;
}

const char* boardSwitchGetName(SwitchCategory cat, uint8_t idx)
{
  (void)cat;
  if (idx >= sizeof(_switch_names) / sizeof(_switch_names[0])) return nullptr;
  return _switch_names[idx];
}
