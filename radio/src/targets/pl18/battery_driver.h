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

/***************************************************************************************************

***************************************************************************************************/
#ifndef      __BATTERY_DRIVER_H__
    #define  __BATTERY_DRIVER_H__
/***************************************************************************************************

***************************************************************************************************/

#include "board.h"
#include "hal.h"

#include "hal/gpio.h"
#include "stm32_gpio.h"

enum ChargeState
{
  CHARGE_UNKNOWN,
  CHARGE_NONE,
  CHARGE_STARTED,
  CHARGE_FINISHED
};

#define IS_UCHARGER_ACTIVE()              gpio_read(UCHARGER_GPIO) ? 1 : 0
#define IS_UCHARGER_CHARGE_END_ACTIVE()   gpio_read(UCHARGER_CHARGE_END_GPIO) ? 1 : 0
#if defined(UCHARGER_EN_GPIO)
#define ENABLE_UCHARGER()                 gpio_set(UCHARGER_EN_GPIO)
#define DISABLE_UCHARGER()                gpio_clear(UCHARGER_EN_GPIO)
#else
#define ENABLE_UCHARGER()
#define DISABLE_UCHARGER()
#endif
#define IS_WCHARGER_ACTIVE()              gpio_read(WCHARGER_GPIO) ? 1 : 0
#define IS_WCHARGER_CHARGE_END_ACTIVE()   gpio_read(WCHARGER_CHARGE_END_GPIO) ? 1 : 0
#define ENABLE_WCHARGER()                 gpio_set(WCHARGER_EN_GPIO)
#define DISABLE_WCHARGER()                gpio_clear(WCHARGER_EN_GPIO)
#define WCHARGER_CURRENT_LOW()            gpio_set(WCHARGER_I_CONTROL_GPIO)
#define WCHARGER_CURRENT_HIGH()           gpio_clear(WCHARGER_I_CONTROL_GPIO)

extern void battery_charge_init();
extern void handle_battery_charge(uint32_t last_press_time);
extern uint16_t get_battery_charge_state();
extern uint16_t getBatteryVoltage();   // returns current battery voltage in 10mV steps
extern bool isChargerActive();

#endif
