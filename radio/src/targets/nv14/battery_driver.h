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

enum ChargeState
{
  CHARGE_UNKNOWN,
  CHARGE_NONE,
  CHARGE_STARTED,
  CHARGE_FINISHED
};


#define PWR_CHARGE_FINISHED_GPIO                 GPIO_PIN(GPIOB, 13) // PB.13
#define PWR_CHARGING_GPIO                        GPIO_PIN(GPIOB, 14) // PB.14

#define READ_CHARGE_FINISHED_STATE()             gpio_read(PWR_CHARGE_FINISHED_GPIO) ? 1 : 0
#define READ_CHARGING_STATE()                    gpio_read(PWR_CHARGING_GPIO) ? 1 : 0

extern void battery_charge_init();
extern void handle_battery_charge(uint32_t last_press_time);
extern uint16_t get_battery_charge_state();
extern uint16_t getBatteryVoltage();   // returns current battery voltage in 10mV steps

#endif
