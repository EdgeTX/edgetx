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

void flySkyIbus2SetDefault(int index, uint16_t id, uint8_t subId,
                           uint8_t instance);

void processFlySkyIbus2AFHDS3Sensor(const uint8_t* packet, uint8_t len);

void flySkyIbus2CalGpsGyro(uint8_t* packet, uint8_t* len);
void flySkyIbus2CalibIBC(uint8_t* packet, uint8_t* len, short voltags);
void flySkyIbus2CalGpsAlt();
void flySkyIbus2CalGpsDist();
void Ibus2ParamCheck(uint8_t* packet, uint8_t len);
void flySkyIbus2ReadParamRPM(uint8_t* packet, uint8_t* len);
bool getIbus2IbcState();
uint8_t flyskyIbus2SensorOnLine();
