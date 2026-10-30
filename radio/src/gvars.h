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

#define LEN_GVAR_NAME                3
#define GVAR_MAX                     1024
#define GVAR_MIN                     -GVAR_MAX

#if defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5)
#define MAX_GVARS                    15
#else
#define MAX_GVARS                    9
#endif

// GVars have one value per flight mode
#define GVAR_VALUE(gv, fm)           g_model.flightModeData[fm].gvars[gv]

#if defined(GVARS)
    uint8_t getGVarFlightMode(uint8_t fm, uint8_t gv);
    int16_t getGVarValue(int8_t gv, int8_t fm);
    void setGVarValue(uint8_t x, int16_t value, int8_t fm);
    #define SET_GVAR(idx, val, fm)     setGVarValue(idx, val, fm)
    #define GVAR_DISPLAY_TIME          100 /*1 second*/;
    extern uint8_t gvarDisplayTimer;
    extern uint8_t gvarLastChanged;
#endif

// limits for mixer weight and offset
constexpr int32_t MIX_WEIGHT_MAX = 500;
constexpr int32_t MIX_WEIGHT_MIN = -500;
constexpr int32_t MIX_OFFSET_MAX = 500;       
constexpr int32_t MIX_OFFSET_MIN = -500;      

void getGVarIncDecRange(int16_t & valMin, int16_t & valMax);
