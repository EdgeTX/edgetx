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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "definitions.h"
#include "dataconstants.h"
#include "opentx_types.h"

PACK(struct GlobalData {
  uint8_t unexpectedShutdown:1;
  uint8_t externalAntennaEnabled:1;
  uint8_t authenticationCount:2;
  uint8_t upgradeModulePopup:1;
  uint8_t internalModuleVersionChecked:1;
  uint8_t flyskygimbals:1;
  uint8_t spare:1;
});

extern GlobalData globalData;

extern uint16_t sessionTimer;
extern uint16_t s_timeCumThr;
extern uint16_t s_timeCum16ThrP;

#if defined(OVERRIDE_CHANNEL_FUNCTION)
#define OVERRIDE_CHANNEL_UNDEFINED -4096
extern safetych_t safetyCh[MAX_OUTPUT_CHANNELS];
#endif

extern uint8_t trimsCheckTimer;
extern uint8_t trimsDisplayTimer;
extern uint8_t trimsDisplayMask;
extern uint16_t maxMixerDuration;

extern uint8_t requiredSpeakerVolume;
extern bool forceBacklightOn;

enum MainRequest {
  REQUEST_SCREENSHOT,
  REQUEST_FLIGHT_RESET,
};

extern uint8_t mainRequestFlags;

#define DELAY_POS_MARGIN   3

PACK(struct SwOn {
  uint16_t delay:14; // max = 2550
  uint8_t  activeMix:1;
  uint8_t  activeExpo:1;
  int16_t  now;  // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  int16_t  prev;
});

extern SwOn   swOn[MAX_MIXERS];
extern int32_t act[MAX_MIXERS];

// static variables used in evalFlightModeMixes - moved here so they don't interfere with the stack
// It's also easier to initialize them here.
extern int8_t  virtualInputsTrims[MAX_INPUTS];

extern int16_t anas [MAX_INPUTS];
extern int16_t trims[NUM_TRIMS];
extern int32_t chans[MAX_OUTPUT_CHANNELS];
extern int16_t ex_chans[MAX_OUTPUT_CHANNELS]; // Outputs (before LIMITS) of the last perMain
extern int16_t channelOutputs[MAX_OUTPUT_CHANNELS];

typedef uint16_t BeepANACenter;
extern BeepANACenter bpanaCenter;

extern uint8_t s_mixer_first_run_done;

extern int16_t calibratedAnalogs[NUM_CALIBRATED_ANALOGS];

extern uint8_t g_beepCnt;
extern uint8_t beepAgain;
extern uint16_t lightOffCounter;
extern uint8_t flashCounter;
extern uint8_t mixWarning;

#endif
