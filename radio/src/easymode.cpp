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

#include "easymode.h"
#include "edgetx.h"
#include "input_mapping.h"

#include <string.h>

EasyModeData g_easyMode;

void easyModeClear()
{
  memset(&g_easyMode, 0, sizeof(g_easyMode));
  g_easyMode.modelType = EASYMODE_NONE;

  // Set all channel assignments to "none"
  g_easyMode.channels.throttle = EASYCH_NONE;
  g_easyMode.channels.aileron = EASYCH_NONE;
  g_easyMode.channels.aileron2 = EASYCH_NONE;
  g_easyMode.channels.elevator = EASYCH_NONE;
  g_easyMode.channels.elevator2 = EASYCH_NONE;
  g_easyMode.channels.rudder = EASYCH_NONE;
  g_easyMode.channels.flap = EASYCH_NONE;
  g_easyMode.channels.flap2 = EASYCH_NONE;
  g_easyMode.channels.flap3 = EASYCH_NONE;
  g_easyMode.channels.flap4 = EASYCH_NONE;
  g_easyMode.channels.steering = EASYCH_NONE;
  g_easyMode.channels.aux1 = EASYCH_NONE;
  g_easyMode.channels.aux2 = EASYCH_NONE;

  for (int i = 0; i < EASY_CUSTOM_MIXES; i++) {
    g_easyMode.customMix[i].destCh = EASYCH_NONE;
  }
}

bool easyModeActive()
{
  return g_easyMode.modelType != EASYMODE_NONE;
}

void easyModeSetDefaults(EasyModeData& em)
{
  // Default options
  em.options.expoAileron = 30;
  em.options.expoElevator = 30;
  em.options.expoRudder = 20;
  em.options.dualRateLow = 75;
  em.options.aileronDifferential = 0;
  em.options.aileronToRudderMix = 0;
  em.options.flapToElevatorComp = 0;
  em.options.multiChannelOrder = EASYMULTI_AETR;
  em.options.swashType = 0;
  em.options.crowEnabled = 0;

  switch (em.modelType) {
    case EASYMODE_AIRPLANE:
      em.wingType = EASYWING_SINGLE_AIL;
      em.tailType = EASYTAIL_NORMAL;
      em.motorType = EASYMOTOR_SINGLE_ELECTRIC;
      em.channels.throttle = 2;   // CH3
      em.channels.aileron = 0;    // CH1
      em.channels.elevator = 1;   // CH2
      em.channels.rudder = 3;     // CH4
      em.sources.flapSource = MIXSRC_FIRST_POT;  // S1
      break;

    case EASYMODE_HELICOPTER:
      em.wingType = EASYWING_SINGLE_AIL;
      em.tailType = EASYTAIL_NORMAL;
      em.motorType = EASYMOTOR_SINGLE_ELECTRIC;
      em.options.swashType = 1;   // SWASH_TYPE_120
      em.channels.throttle = 2;   // CH3
      em.channels.aileron = 0;    // CH1
      em.channels.elevator = 1;   // CH2
      em.channels.rudder = 3;     // CH4
      em.channels.aux1 = 4;       // CH5 - gyro gain
      em.sources.aux1Source = MIXSRC_FIRST_POT;  // S1
      break;

    case EASYMODE_GLIDER:
      em.wingType = EASYWING_2AIL_2FLAP;
      em.tailType = EASYTAIL_NORMAL;
      em.motorType = EASYMOTOR_NONE;
      em.channels.aileron = 0;    // CH1
      em.channels.elevator = 1;   // CH2
      em.channels.throttle = 2;   // CH3 (motor if electric, else unused)
      em.channels.rudder = 3;     // CH4
      em.channels.aileron2 = 4;   // CH5
      em.channels.flap = 5;      // CH6
      em.channels.flap2 = 6;     // CH7
      // Throttle stick for flap/crow
      em.sources.flapSource = MIXSRC_FIRST_STICK + inputMappingChannelOrder(2);
      // Motor on/off switch (used when motor type != none)
      em.sources.motorSource = MIXSRC_FIRST_SWITCH;  // SA
      break;

    case EASYMODE_MULTIROTOR:
      em.wingType = EASYWING_SINGLE_AIL;
      em.tailType = EASYTAIL_NORMAL;
      em.motorType = EASYMOTOR_SINGLE_ELECTRIC;
      em.options.multiChannelOrder = EASYMULTI_AETR;
      // AETR: A=CH1, E=CH2, T=CH3, R=CH4
      em.channels.aileron = 0;
      em.channels.elevator = 1;
      em.channels.throttle = 2;
      em.channels.rudder = 3;
      em.channels.aux1 = 4;       // CH5 - arm switch
      em.channels.aux2 = 5;       // CH6 - flight mode
      em.sources.aux1Source = MIXSRC_FIRST_SWITCH;      // SA
      em.sources.aux2Source = MIXSRC_FIRST_SWITCH + 1;  // SB
      break;

    case EASYMODE_CAR:
    case EASYMODE_BOAT:
      em.wingType = EASYWING_SINGLE_AIL;
      em.tailType = EASYTAIL_NORMAL;
      em.motorType = EASYMOTOR_SINGLE_ELECTRIC;
      em.channels.steering = 0;   // CH1
      em.channels.throttle = 1;   // CH2
      break;

    default:
      break;
  }
}

const char* const easyModelTypeStrings[] = {
  "---",
  STR_EASYMODE_AIRPLANE,
  STR_EASYMODE_HELICOPTER,
  STR_EASYMODE_GLIDER,
  STR_EASYMODE_MULTIROTOR,
  STR_EASYMODE_CAR,
  STR_EASYMODE_BOAT,
  nullptr
};

const char* const easyWingTypeStrings[] = {
  STR_EASYMODE_WING_SINGLE_AIL,
  STR_EASYMODE_WING_DUAL_AIL,
  STR_EASYMODE_WING_FLAPERON,
  STR_EASYMODE_WING_1A1F,
  STR_EASYMODE_WING_2A1F,
  STR_EASYMODE_WING_2A2F,
  STR_EASYMODE_WING_2A4F,
  STR_EASYMODE_WING_ELEVON,
  STR_EASYMODE_WING_DELTA,
  nullptr
};

const char* const easyTailTypeStrings[] = {
  STR_EASYMODE_TAIL_NORMAL,
  STR_EASYMODE_TAIL_VTAIL,
  STR_EASYMODE_TAIL_TAILLESS,
  STR_EASYMODE_TAIL_DUAL_ELE,
  STR_EASYMODE_TAIL_AILEVATOR,
  nullptr
};

const char* const easyMotorTypeStrings[] = {
  STR_EASYMODE_MOTOR_NONE,
  STR_EASYMODE_MOTOR_ELECTRIC,
  STR_EASYMODE_MOTOR_NITRO,
  nullptr
};

const char* const easyMultiOrderStrings[] = {
  "AETR",
  "TAER",
  "RETA",
  nullptr
};
