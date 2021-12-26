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

#define MAX_TIMERS_219                     3
#define MAX_GVARS_219                      9

#if defined(PCBHORUS)
  #define NUM_SWITCHES_219                 10
#elif defined(PCBXLITE) ||defined(PCBXLITES)
  #define NUM_SWITCHES_219                  6
#elif  defined(RADIO_TLITE) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_T8) || defined(PCBNV14) || defined(PCBPL18)
  #define NUM_SWITCHES_219                  8
#elif defined(PCBX7) || defined(PCBX7ACCESS)
  #define NUM_SWITCHES_219                  8
#elif defined(PCBX9LITES)
  #define NUM_SWITCHES_219                  7
#elif defined(PCBX9LITE)
  #define NUM_SWITCHES_219                  5
#elif defined(PCBX9E)
  #define NUM_SWITCHES_219                 18
#elif defined(RADIO_X9DP2019) || defined(PCBX9D) || defined(PCBX9DP)
  #define NUM_SWITCHES_219                  9
#else
  #error "Unsupported Conversion for Radio"
#endif

#if defined(PCBHORUS)
  #define NUM_POTS_219                      5
  #define NUM_SLIDERS_219                   4
#elif defined(PCBX9LITE)
  #define NUM_POTS_219                      1
  #define NUM_SLIDERS_219                   0
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(PCBXLITE) || defined(PCBX7) || defined(PCBNV14) || defined(PCBPL18)
  #define NUM_POTS_219                      2
  #define NUM_SLIDERS_219                   0
#elif defined(PCBX9E)
  #define NUM_POTS_219                      4
  #define NUM_SLIDERS_219                   4
#else
  #define NUM_POTS_219                      3
  #define NUM_SLIDERS_219                   2
#endif

#define NUM_STICKS_219                      4

#if defined(PCBHORUS)
  #define LEN_SWITCH_NAME_219               3
  #define LEN_ANA_NAME_219                  3
  #define LEN_MODEL_FILENAME_219           16
  #define LEN_BLUETOOTH_NAME_219           10
#else
  #define LEN_SWITCH_NAME_219               3
  #define LEN_ANA_NAME_219                  3
  #define LEN_BLUETOOTH_NAME_219           10
#endif

#define LEN_GVAR_NAME_219                   3
#define TELEM_LABEL_LEN_219                 4

#if defined(PCBHORUS)
  #define LEN_MODEL_NAME_219               15
  #define LEN_TIMER_NAME_219                8
  #define LEN_FLIGHT_MODE_NAME_219         10
  #define LEN_BITMAP_NAME_219              14
  #define LEN_EXPOMIX_NAME_219              6
  #define LEN_CHANNEL_NAME_219              6
  #define LEN_INPUT_NAME_219                4
  #define LEN_CURVE_NAME_219                3
  #define LEN_FUNCTION_NAME_219             6
  #define MAX_CURVES_219                   32
  #define MAX_CURVE_POINTS_219            512
#elif LCD_W == 212
  #define LEN_MODEL_NAME_219               12
  #define LEN_TIMER_NAME_219                8
  #define LEN_FLIGHT_MODE_NAME_219         10
  #define LEN_BITMAP_NAME_219              10
  #define LEN_EXPOMIX_NAME_219              6
  #define LEN_CHANNEL_NAME_219              6
  #define LEN_INPUT_NAME_219                4
  #define LEN_CURVE_NAME_219                3
  #define LEN_FUNCTION_NAME_219             8
  #define MAX_CURVES_219                   32
  #define MAX_CURVE_POINTS_219            512
#else
  #define LEN_MODEL_NAME_219               10
  #define LEN_TIMER_NAME_219                3
  #define LEN_FLIGHT_MODE_NAME_219          6
  #define LEN_EXPOMIX_NAME_219              6
  #define LEN_CHANNEL_NAME_219              4
  #define LEN_INPUT_NAME_219                3
  #define LEN_CURVE_NAME_219                3
  #define LEN_FUNCTION_NAME_219             6
  #define MAX_CURVES_219                   32
  #define MAX_CURVE_POINTS_219            512
#endif

#if defined(PCBHORUS) || defined(PCBNV14) || defined(PCBPL18)
  #define MAX_MODELS_219                   60
  #define MAX_OUTPUT_CHANNELS_219          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES_219              9
  #define MAX_MIXERS_219                   64
  #define MAX_EXPOS_219                    64
  #define MAX_LOGICAL_SWITCHES_219         64
  #define MAX_SPECIAL_FUNCTIONS_219        64 // number of functions assigned to switches
  #define MAX_SCRIPTS_219                   9
  #define MAX_INPUTS_219                   32
  #define MAX_TRAINER_CHANNELS_219         16
  #define MAX_TELEMETRY_SENSORS_219        60
  #define MAX_CUSTOM_SCREENS_219            5
#elif defined(PCBTARANIS)
  #define MAX_MODELS_219                   60
  #define MAX_OUTPUT_CHANNELS_219          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES_219              9
  #define MAX_MIXERS_219                   64
  #define MAX_EXPOS_219                    64
  #define MAX_LOGICAL_SWITCHES_219         64
  #define MAX_SPECIAL_FUNCTIONS_219        64 // number of functions assigned to switches
  #define MAX_SCRIPTS_219                   7
  #define MAX_INPUTS_219                   32
  #define MAX_TRAINER_CHANNELS_219         16
  #if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E)
    #define MAX_TELEMETRY_SENSORS_219      60
  #else
    #define MAX_TELEMETRY_SENSORS_219      40
  #endif
#endif

PACK(typedef struct {
  int32_t  mode:9; // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint32_t start:23;
  int32_t  value:24;
  uint32_t countdownBeep:2;
  uint32_t minuteBeep:1;
  uint32_t persistent:2;
  int32_t  countdownStart:2;
  uint32_t direction:1;
  char     name[LEN_TIMER_NAME_219];
}) TimerData_v219;

// #include "chksize.h"

// #define CHKSIZE(x, y) check_size<struct x, y>()

// static inline void check_struct_219()
// {
// #if defined(PCBHORUS)
//   CHKSIZE(ModelData_v219, 10664);
// #elif defined(PCBX9E)
//   CHKSIZE(ModelData_v219, 6520);
// #elif defined(PCBX9D)
//   CHKSIZE(RadioData_v219, 872);
// #endif
// }

// #undef CHKSIZE

