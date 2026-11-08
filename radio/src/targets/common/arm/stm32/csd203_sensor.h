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

#pragma once

#include <stdint.h>

void IICcsd203init(void);
void initCSD203(void);
void readCSD203(void);

// CSD203_ExtSensorCFG: device supply battery
uint16_t getBatteryVoltage();           // 10mV steps
int16_t getExtModuleCurrent();          // mA

// CSD203_MainSensorCFG: 2S pack mid-cell tap / system consumption
uint16_t getBatteryMidVoltage();        // 10mV steps
int16_t getSystemCurrent();             // mA

// CSD203_InSensorCFG: external RF module supply / internal RF module
uint16_t getExtModuleVoltage();         // 10mV steps
int16_t getIntModuleCurrent();          // mA
