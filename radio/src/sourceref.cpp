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

#include "sourceref.h"
#include "dataconstants.h"
#include "board.h"

//
// Convert from legacy MixSources enum to SourceRef
//
SourceRef sourceRefFromMixSrc(int32_t mixsrc)
{
  SourceRef ref = {};

  // Handle inversion (negative values)
  if (mixsrc < 0) {
    ref.flags = SOURCE_FLAG_INVERTED;
    mixsrc = -mixsrc;
  }

  if (mixsrc == MIXSRC_NONE) {
    ref.type = SOURCE_TYPE_NONE;
  }
  else if (mixsrc >= MIXSRC_FIRST_INPUT && mixsrc <= MIXSRC_LAST_INPUT) {
    ref.type = SOURCE_TYPE_INPUT;
    ref.index = mixsrc - MIXSRC_FIRST_INPUT;
  }
#if defined(LUA_INPUTS)
  else if (mixsrc >= MIXSRC_FIRST_LUA && mixsrc <= MIXSRC_LAST_LUA) {
    ref.type = SOURCE_TYPE_LUA;
    ref.index = mixsrc - MIXSRC_FIRST_LUA;
  }
#endif
  else if (mixsrc >= MIXSRC_FIRST_STICK && mixsrc <= MIXSRC_LAST_STICK) {
    ref.type = SOURCE_TYPE_STICK;
    ref.index = mixsrc - MIXSRC_FIRST_STICK;
  }
  else if (mixsrc >= MIXSRC_FIRST_POT && mixsrc <= MIXSRC_LAST_POT) {
    ref.type = SOURCE_TYPE_POT;
    ref.index = mixsrc - MIXSRC_FIRST_POT;
  }
#if defined(IMU)
  else if (mixsrc == MIXSRC_TILT_X) {
    ref.type = SOURCE_TYPE_IMU;
    ref.index = 0;
  }
  else if (mixsrc == MIXSRC_TILT_Y) {
    ref.type = SOURCE_TYPE_IMU;
    ref.index = 1;
  }
#endif
#if defined(PCBHORUS)
  else if (mixsrc >= MIXSRC_FIRST_SPACEMOUSE && mixsrc <= MIXSRC_LAST_SPACEMOUSE) {
    ref.type = SOURCE_TYPE_SPACEMOUSE;
    ref.index = mixsrc - MIXSRC_FIRST_SPACEMOUSE;
  }
#endif
  else if (mixsrc == MIXSRC_MIN) {
    ref.type = SOURCE_TYPE_MIN;
  }
  else if (mixsrc == MIXSRC_MAX) {
    ref.type = SOURCE_TYPE_MAX;
  }
#if defined(LUMINOSITY_SENSOR)
  else if (mixsrc == MIXSRC_LIGHT) {
    ref.type = SOURCE_TYPE_LIGHT;
  }
#endif
  else if (mixsrc >= MIXSRC_FIRST_HELI && mixsrc <= MIXSRC_LAST_HELI) {
    ref.type = SOURCE_TYPE_HELI;
    ref.index = mixsrc - MIXSRC_FIRST_HELI;
  }
  else if (mixsrc >= MIXSRC_FIRST_TRIM && mixsrc <= MIXSRC_LAST_TRIM) {
    ref.type = SOURCE_TYPE_TRIM;
    ref.index = mixsrc - MIXSRC_FIRST_TRIM;
  }
  else if (mixsrc >= MIXSRC_FIRST_SWITCH && mixsrc <= MIXSRC_LAST_SWITCH) {
    ref.type = SOURCE_TYPE_SWITCH;
    ref.index = mixsrc - MIXSRC_FIRST_SWITCH;
  }
#if defined(FUNCTION_SWITCHES)
  else if (mixsrc >= MIXSRC_FIRST_CUSTOMSWITCH_GROUP && mixsrc <= MIXSRC_LAST_CUSTOMSWITCH_GROUP) {
    ref.type = SOURCE_TYPE_CUSTOM_SWITCH_GROUP;
    ref.index = mixsrc - MIXSRC_FIRST_CUSTOMSWITCH_GROUP;
  }
#endif
  else if (mixsrc >= MIXSRC_FIRST_LOGICAL_SWITCH && mixsrc <= MIXSRC_LAST_LOGICAL_SWITCH) {
    ref.type = SOURCE_TYPE_LOGICAL_SWITCH;
    ref.index = mixsrc - MIXSRC_FIRST_LOGICAL_SWITCH;
  }
  else if (mixsrc >= MIXSRC_FIRST_TRAINER && mixsrc <= MIXSRC_LAST_TRAINER) {
    ref.type = SOURCE_TYPE_TRAINER;
    ref.index = mixsrc - MIXSRC_FIRST_TRAINER;
  }
  else if (mixsrc >= MIXSRC_FIRST_CH && mixsrc <= MIXSRC_LAST_CH) {
    ref.type = SOURCE_TYPE_CHANNEL;
    ref.index = mixsrc - MIXSRC_FIRST_CH;
  }
  else if (mixsrc >= MIXSRC_FIRST_GVAR && mixsrc <= MIXSRC_LAST_GVAR) {
    ref.type = SOURCE_TYPE_GVAR;
    ref.index = mixsrc - MIXSRC_FIRST_GVAR;
  }
  else if (mixsrc == MIXSRC_TX_VOLTAGE) {
    ref.type = SOURCE_TYPE_TX_VOLTAGE;
  }
  else if (mixsrc == MIXSRC_TX_TIME) {
    ref.type = SOURCE_TYPE_TX_TIME;
  }
  else if (mixsrc == MIXSRC_TX_GPS) {
    ref.type = SOURCE_TYPE_TX_GPS;
  }
  else if (mixsrc >= MIXSRC_FIRST_TIMER && mixsrc <= MIXSRC_LAST_TIMER) {
    ref.type = SOURCE_TYPE_TIMER;
    ref.index = mixsrc - MIXSRC_FIRST_TIMER;
  }
  else if (mixsrc >= MIXSRC_FIRST_TELEM && mixsrc <= MIXSRC_LAST_TELEM) {
    ref.type = SOURCE_TYPE_TELEMETRY;
    ref.index = mixsrc - MIXSRC_FIRST_TELEM;
  }

  return ref;
}

//
// Convert from SourceRef to legacy MixSources enum
//
int32_t mixSrcFromSourceRef(const SourceRef& ref)
{
  int32_t mixsrc = MIXSRC_NONE;

  switch (ref.type) {
    case SOURCE_TYPE_NONE:     mixsrc = MIXSRC_NONE; break;
    case SOURCE_TYPE_INPUT:    mixsrc = MIXSRC_FIRST_INPUT + ref.index; break;
#if defined(LUA_INPUTS)
    case SOURCE_TYPE_LUA:      mixsrc = MIXSRC_FIRST_LUA + ref.index; break;
#endif
    case SOURCE_TYPE_STICK:    mixsrc = MIXSRC_FIRST_STICK + ref.index; break;
    case SOURCE_TYPE_POT:      mixsrc = MIXSRC_FIRST_POT + ref.index; break;
#if defined(IMU)
    case SOURCE_TYPE_IMU:      mixsrc = (ref.index == 0) ? MIXSRC_TILT_X : MIXSRC_TILT_Y; break;
#endif
#if defined(PCBHORUS)
    case SOURCE_TYPE_SPACEMOUSE: mixsrc = MIXSRC_FIRST_SPACEMOUSE + ref.index; break;
#endif
    case SOURCE_TYPE_MIN:      mixsrc = MIXSRC_MIN; break;
    case SOURCE_TYPE_MAX:      mixsrc = MIXSRC_MAX; break;
#if defined(LUMINOSITY_SENSOR)
    case SOURCE_TYPE_LIGHT:    mixsrc = MIXSRC_LIGHT; break;
#endif
    case SOURCE_TYPE_HELI:     mixsrc = MIXSRC_FIRST_HELI + ref.index; break;
    case SOURCE_TYPE_TRIM:     mixsrc = MIXSRC_FIRST_TRIM + ref.index; break;
    case SOURCE_TYPE_SWITCH:   mixsrc = MIXSRC_FIRST_SWITCH + ref.index; break;
#if defined(FUNCTION_SWITCHES)
    case SOURCE_TYPE_CUSTOM_SWITCH_GROUP: mixsrc = MIXSRC_FIRST_CUSTOMSWITCH_GROUP + ref.index; break;
#endif
    case SOURCE_TYPE_LOGICAL_SWITCH: mixsrc = MIXSRC_FIRST_LOGICAL_SWITCH + ref.index; break;
    case SOURCE_TYPE_TRAINER:  mixsrc = MIXSRC_FIRST_TRAINER + ref.index; break;
    case SOURCE_TYPE_CHANNEL:  mixsrc = MIXSRC_FIRST_CH + ref.index; break;
    case SOURCE_TYPE_GVAR:     mixsrc = MIXSRC_FIRST_GVAR + ref.index; break;
    case SOURCE_TYPE_TX_VOLTAGE: mixsrc = MIXSRC_TX_VOLTAGE; break;
    case SOURCE_TYPE_TX_TIME:  mixsrc = MIXSRC_TX_TIME; break;
    case SOURCE_TYPE_TX_GPS:   mixsrc = MIXSRC_TX_GPS; break;
    case SOURCE_TYPE_TIMER:    mixsrc = MIXSRC_FIRST_TIMER + ref.index; break;
    case SOURCE_TYPE_TELEMETRY: mixsrc = MIXSRC_FIRST_TELEM + ref.index; break;
    default: break;
  }

  if (ref.flags & SOURCE_FLAG_INVERTED)
    mixsrc = -mixsrc;

  return mixsrc;
}

//
// Convert from legacy SwitchSources enum to SwitchRef
//
SwitchRef switchRefFromSwSrc(int32_t swsrc)
{
  SwitchRef ref = {};

  if (swsrc < 0) {
    ref.flags = SWITCH_FLAG_INVERTED;
    swsrc = -swsrc;
  }

  if (swsrc == SWSRC_NONE) {
    ref.type = SWITCH_TYPE_NONE;
  }
  else if (swsrc >= SWSRC_FIRST_SWITCH && swsrc <= SWSRC_LAST_SWITCH) {
    ref.type = SWITCH_TYPE_SWITCH;
    ref.index = swsrc - SWSRC_FIRST_SWITCH;
  }
#if defined(MULTIPOS_SWITCH)
  else if (swsrc >= SWSRC_FIRST_MULTIPOS_SWITCH && swsrc <= SWSRC_LAST_MULTIPOS_SWITCH) {
    ref.type = SWITCH_TYPE_MULTIPOS;
    ref.index = swsrc - SWSRC_FIRST_MULTIPOS_SWITCH;
  }
#endif
  else if (swsrc >= SWSRC_FIRST_TRIM && swsrc <= SWSRC_LAST_TRIM) {
    ref.type = SWITCH_TYPE_TRIM;
    ref.index = swsrc - SWSRC_FIRST_TRIM;
  }
  else if (swsrc >= SWSRC_FIRST_LOGICAL_SWITCH && swsrc <= SWSRC_LAST_LOGICAL_SWITCH) {
    ref.type = SWITCH_TYPE_LOGICAL;
    ref.index = swsrc - SWSRC_FIRST_LOGICAL_SWITCH;
  }
  else if (swsrc == SWSRC_ON) {
    ref.type = SWITCH_TYPE_ON;
  }
  else if (swsrc == SWSRC_ONE) {
    ref.type = SWITCH_TYPE_ONE;
  }
  else if (swsrc >= SWSRC_FIRST_FLIGHT_MODE && swsrc <= SWSRC_LAST_FLIGHT_MODE) {
    ref.type = SWITCH_TYPE_FLIGHT_MODE;
    ref.index = swsrc - SWSRC_FIRST_FLIGHT_MODE;
  }
  else if (swsrc == SWSRC_TELEMETRY_STREAMING) {
    ref.type = SWITCH_TYPE_TELEMETRY;
  }
  else if (swsrc >= SWSRC_FIRST_SENSOR && swsrc <= SWSRC_LAST_SENSOR) {
    ref.type = SWITCH_TYPE_SENSOR;
    ref.index = swsrc - SWSRC_FIRST_SENSOR;
  }
  else if (swsrc == SWSRC_RADIO_ACTIVITY) {
    ref.type = SWITCH_TYPE_RADIO_ACTIVITY;
  }
  else if (swsrc == SWSRC_TRAINER_CONNECTED) {
    ref.type = SWITCH_TYPE_TRAINER;
  }

  return ref;
}

//
// Convert from SwitchRef to legacy SwitchSources enum
//
int32_t swSrcFromSwitchRef(const SwitchRef& ref)
{
  int32_t swsrc = SWSRC_NONE;

  switch (ref.type) {
    case SWITCH_TYPE_NONE:        swsrc = SWSRC_NONE; break;
    case SWITCH_TYPE_SWITCH:      swsrc = SWSRC_FIRST_SWITCH + ref.index; break;
#if defined(MULTIPOS_SWITCH)
    case SWITCH_TYPE_MULTIPOS:    swsrc = SWSRC_FIRST_MULTIPOS_SWITCH + ref.index; break;
#endif
    case SWITCH_TYPE_TRIM:        swsrc = SWSRC_FIRST_TRIM + ref.index; break;
    case SWITCH_TYPE_LOGICAL:     swsrc = SWSRC_FIRST_LOGICAL_SWITCH + ref.index; break;
    case SWITCH_TYPE_ON:          swsrc = SWSRC_ON; break;
    case SWITCH_TYPE_ONE:         swsrc = SWSRC_ONE; break;
    case SWITCH_TYPE_FLIGHT_MODE: swsrc = SWSRC_FIRST_FLIGHT_MODE + ref.index; break;
    case SWITCH_TYPE_TELEMETRY:   swsrc = SWSRC_TELEMETRY_STREAMING; break;
    case SWITCH_TYPE_SENSOR:      swsrc = SWSRC_FIRST_SENSOR + ref.index; break;
    case SWITCH_TYPE_RADIO_ACTIVITY: swsrc = SWSRC_RADIO_ACTIVITY; break;
    case SWITCH_TYPE_TRAINER:     swsrc = SWSRC_TRAINER_CONNECTED; break;
    default: break;
  }

  if (ref.flags & SWITCH_FLAG_INVERTED)
    swsrc = -swsrc;

  return swsrc;
}
