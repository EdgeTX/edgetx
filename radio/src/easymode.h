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

// ---- Model Type ----
enum EasyModelType : uint8_t {
  EASYMODE_NONE = 0,
  EASYMODE_AIRPLANE,
  EASYMODE_HELICOPTER,
  EASYMODE_GLIDER,
  EASYMODE_MULTIROTOR,
  EASYMODE_CAR,
  EASYMODE_BOAT,
  EASYMODE_TYPE_COUNT
};

// ---- Wing Type (airplane/glider) ----
enum EasyWingType : uint8_t {
  EASYWING_SINGLE_AIL = 0,
  EASYWING_DUAL_AIL,
  EASYWING_FLAPERON,
  EASYWING_1AIL_1FLAP,
  EASYWING_2AIL_1FLAP,
  EASYWING_2AIL_2FLAP,
  EASYWING_2AIL_4FLAP,     // full house glider: ail+flap+brake per side
  EASYWING_ELEVON,         // flying wing
  EASYWING_DELTA,          // delta wing
  EASYWING_COUNT
};

// ---- Tail Type (airplane/glider) ----
enum EasyTailType : uint8_t {
  EASYTAIL_NORMAL = 0,     // separate elevator + rudder
  EASYTAIL_V_TAIL,         // ruddervators
  EASYTAIL_TAILLESS,       // use with elevon/delta wing
  EASYTAIL_DUAL_ELEVATOR,  // 2x elevator + rudder
  EASYTAIL_AILEVATOR,      // elevator halves act as ailerons
  EASYTAIL_COUNT
};

// ---- Motor Type ----
enum EasyMotorType : uint8_t {
  EASYMOTOR_NONE = 0,
  EASYMOTOR_SINGLE_ELECTRIC,
  EASYMOTOR_SINGLE_NITRO,
  EASYMOTOR_COUNT
};

// ---- Multirotor channel order ----
enum EasyMultiChannelOrder : uint8_t {
  EASYMULTI_AETR = 0,     // Ail Ele Thr Rud (Betaflight default)
  EASYMULTI_TAER,          // Thr Ail Ele Rud (Spektrum)
  EASYMULTI_RETA,          // Rud Ele Thr Ail
  EASYMULTI_ORDER_COUNT
};

#define EASYCH_NONE  (-1)
#define EASYCH_MAX   16

// ---- Channel assignment map ----
struct EasyModeChannelMap {
  int8_t throttle;
  int8_t aileron;
  int8_t aileron2;
  int8_t elevator;
  int8_t elevator2;
  int8_t rudder;
  int8_t flap;
  int8_t flap2;
  int8_t flap3;       // glider: inner flap L or brake L
  int8_t flap4;       // glider: inner flap R or brake R
  int8_t steering;    // surface vehicles
  int8_t aux1;        // arm switch (multi), gyro gain (heli)
  int8_t aux2;        // flight mode switch
};

// ---- Options ----
struct EasyModeOptions {
  int8_t  expoAileron;       // 0-100
  int8_t  expoElevator;      // 0-100
  int8_t  expoRudder;        // 0-100
  int8_t  dualRateLow;       // 0-100 (percent)
  int8_t  aileronDifferential; // 0-100
  int8_t  aileronToRudderMix;  // 0-100
  int8_t  flapToElevatorComp;  // -100 to 100
  uint8_t multiChannelOrder;   // EasyMultiChannelOrder
  uint8_t swashType;           // reuse firmware SwashType enum for heli
  uint8_t crowEnabled;         // for glider
};

// ---- Input sources for non-stick controls ----
struct EasyModeInputSources {
  uint16_t flapSource;    // MixSources - pot/slider/switch for flap control
  uint16_t motorSource;   // MixSources - switch for motor on/off (glider)
  uint16_t aux1Source;    // MixSources - switch for aux1 (gyro gain / arm)
  uint16_t aux2Source;    // MixSources - switch for aux2 (flight mode)
};

// ---- Custom free-form mixes ----
#define EASY_CUSTOM_MIXES 3

struct EasyModeCustomMix {
  uint16_t source;    // MixSources (0 = MIXSRC_NONE = disabled)
  int8_t   destCh;    // 0-based channel, EASYCH_NONE = disabled
  int8_t   weight;    // -100 to 100
};

// ---- Main EasyMode data ----
struct EasyModeData {
  EasyModelType   modelType;
  EasyWingType    wingType;
  EasyTailType    tailType;
  EasyMotorType   motorType;
  EasyModeChannelMap channels;
  EasyModeOptions options;
  EasyModeInputSources sources;
  EasyModeCustomMix customMix[EASY_CUSTOM_MIXES];
};

extern EasyModeData g_easyMode;

void easyModeClear();
bool easyModeActive();

// Set default channel map for a given model type / wing / tail
void easyModeSetDefaults(EasyModeData& em);

// String arrays for UI
extern const char* const easyModelTypeStrings[];
extern const char* const easyWingTypeStrings[];
extern const char* const easyTailTypeStrings[];
extern const char* const easyMotorTypeStrings[];
extern const char* const easyMultiOrderStrings[];
