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

#ifndef _TRAINER_H_
#define _TRAINER_H_

#include "dataconstants.h"

// Trainer input channels
extern int16_t ppmInput[MAX_TRAINER_CHANNELS];

// Timer gets decremented in per10ms()
#define PPM_IN_VALID_TIMEOUT 100 // 1s
extern uint8_t ppmInputValidityTimer;

extern uint8_t currentTrainerMode;
#define IS_TRAINER_INPUT_VALID() (ppmInputValidityTimer != 0)

void checkTrainerSignalWarning();
void checkTrainerSettings();
void stopTrainer();
void forceResetTrainerSettings();

// Needs to be inlined to avoid slow function calls in ISR routines
inline void captureTrainerPulses(uint16_t capture)
{
  static uint16_t lastCapt = 0;
  static int8_t channelNumber = -1;

  uint16_t val = (uint16_t)(capture - lastCapt) / 2;
  lastCapt = capture;

  // We process ppmInput right here to make servo movement as smooth as possible
  //    while under trainee control
  //
  // G: Prioritize reset pulse. (Needed when less than 16 incoming pulses)
  //
  if (val > 4000 && val < 19000) {
    channelNumber = 0; // triggered
  }
  else {
    if (channelNumber >= 0 && channelNumber < MAX_TRAINER_CHANNELS) {
      if (val > 800 && val < 2200) {
        ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;
        ppmInput[channelNumber++] =
          // +-500 != 512, but close enough.
          (int16_t)(val - 1500) * (g_eeGeneral.PPM_Multiplier+10) / 10;
      }
      else {
        channelNumber = -1; // not triggered
      }
    }
  }
}

#endif // _TRAINER_H_
