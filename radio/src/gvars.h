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

#define MAX_GVARS                    9

// GVars have one value per flight mode
#define GVAR_VALUE(gv, fm)           g_model.flightModeData[fm].gvars[gv]

#if defined(GVARS)
    uint8_t getGVarFlightMode(uint8_t fm, uint8_t gv);
    int16_t getGVarFieldValue(int16_t x, int16_t min, int16_t max, int8_t fm);
    int32_t getGVarFieldValuePrec1(int16_t x, int16_t min, int16_t max, int8_t fm);
    int16_t getGVarValue(int8_t gv, int8_t fm);
    int32_t getGVarValuePrec1(int8_t gv, int8_t fm);
    void setGVarValue(uint8_t x, int16_t value, int8_t fm);
    #define GET_GVAR(x, min, max, fm)  getGVarFieldValue(x, min, max, fm)
    #define SET_GVAR(idx, val, fm)     setGVarValue(idx, val, fm)
    #define GVAR_DISPLAY_TIME          100 /*1 second*/;
    #define GET_GVAR_PREC1(x, min, max, fm) getGVarFieldValuePrec1(x, min, max, fm)
    extern uint8_t gvarDisplayTimer;
    extern uint8_t gvarLastChanged;
#else
  #define GET_GVAR(x, ...)             (x)
  #define GET_GVAR_PREC1(x, ...)       (x*10)
#endif

// we reserve the space inside the range of values, like offset, weight, etc.
#define GV_RANGE_MAX                1024
#define GV_RANGE_POS                (GV_RANGE_MAX - 1 - MAX_GVARS)
#define GV_RANGE_NEG                (-GV_RANGE_MAX + MAX_GVARS)

#define GV_IS_GV_VALUE(x)           (x > GV_RANGE_POS || x < GV_RANGE_NEG)
#define GV_INDEX_FROM_VALUE(x)      ((x & (GV_RANGE_MAX * 2 - 1)) - GV_RANGE_MAX)
#define GV_VALUE_FROM_INDEX(idx)    ((idx < 0) ? GV_RANGE_MAX + idx : -GV_RANGE_MAX + idx)

// the define GV_RANGE_MAX marks the highest bit value used for this variables
// because this would give too big numbers for ARM, we limit it further for
// offset and weight
constexpr int32_t MIX_WEIGHT_MAX = 500;
constexpr int32_t MIX_WEIGHT_MIN = -500;
constexpr int32_t MIX_OFFSET_MAX = 500;       
constexpr int32_t MIX_OFFSET_MIN = -500;      

void getGVarIncDecRange(int16_t & valMin, int16_t & valMax);
