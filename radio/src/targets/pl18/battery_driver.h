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

enum ChargeState
{
  CHARGE_UNKNOWN,
  CHARGE_NONE,
  CHARGE_STARTED,
  CHARGE_FINISHED
};

#define IS_UCHARGER_ACTIVE()              GPIO_ReadInputDataBit(UCHARGER_GPIO, UCHARGER_GPIO_PIN)
#define IS_UCHARGER_CHARGE_END_ACTIVE()   GPIO_ReadInputDataBit(UCHARGER_CHARGE_END_GPIO, UCHARGER_CHARGE_END_GPIO_PIN) 
#define ENABLE_UCHARGER()                 GPIO_SetBits(UCHARGER_EN_GPIO, UCHARGER_EN_GPIO_PIN)
#define DISABLE_UCHARGER()                GPIO_ResetBits(UCHARGER_EN_GPIO, UCHARGER_EN_GPIO_PIN)

#define IS_WCHARGER_ACTIVE()              GPIO_ReadInputDataBit(WCHARGER_GPIO, WCHARGER_GPIO_PIN)
#define IS_WCHARGER_CHARGE_END_ACTIVE()   GPIO_ReadInputDataBit(WCHARGER_CHARGE_END_GPIO, WCHARGER_CHARGE_END_GPIO_PIN) 
#define ENABLE_WCHARGER()                 GPIO_SetBits(WCHARGER_EN_GPIO, WCHARGER_EN_GPIO_PIN)
#define DISABLE_WCHARGER()                GPIO_ResetBits(WCHARGER_EN_GPIO, WCHARGER_EN_GPIO_PIN)
#define WCHARGER_CURRENT_LOW()            GPIO_ResetBits(WCHARGER_I_CONTROL_GPIO, WCHARGER_I_CONTROL_GPIO_PIN)
#define WCHARGER_CURRENT_HIGH()           GPIO_SetBits(WCHARGER_I_CONTROL_GPIO, WCHARGER_I_CONTROL_GPIO_PIN)

extern void battery_charge_init();
extern void handle_battery_charge(uint32_t last_press_time);
extern uint16_t get_battery_charge_state();
extern uint16_t getBatteryVoltage();   // returns current battery voltage in 10mV steps
extern bool isChargerActive();

#endif
