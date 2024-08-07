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

#include "edgetx.h"

uint8_t g_beepCnt;
uint8_t beepAgain = 0;
uint8_t beepAgainOrig = 0;
uint8_t beepOn = false;
bool warble = false;
bool warbleC;

// The various "beep" tone lengths
static const uint8_t beepTab[]  = {
    // key, trim, warn2, warn1, error
    1,  1,  2, 10,  60, //xShort
    1,  1,  4, 20,  80, //short
    1,  1,  8, 30, 100, //normal
    2,  2, 15, 40, 120, //long
    5,  5, 30, 50, 150, //xLong
};

void beep(uint8_t val)
{
#if defined(HAPTIC) && !defined(AUDIO)
  // completely untested
  if (val == 0)
    haptic.play(5, 0, PLAY_NOW);
  else
    haptic.event(AU_ERROR);
#endif

#if !defined(AUDIO)
  if (g_eeGeneral.alarmsFlash && val>1) {
    flashCounter = FLASH_DURATION;
  }
#endif

  if (g_eeGeneral.beepMode>0 || (g_eeGeneral.beepMode==0 && val!=0) || (g_eeGeneral.beepMode==-1 && val>=3)) {
    _beep(*(beepTab+5*(2+g_eeGeneral.beepLength)+val));
  }
}

