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

enum GermanPrompts {
  DE_PROMPT_NUMBERS_BASE = 0,
  DE_PROMPT_NULL = DE_PROMPT_NUMBERS_BASE+0,
  DE_PROMPT_EIN = DE_PROMPT_NUMBERS_BASE+100,
  DE_PROMPT_EINE = DE_PROMPT_NUMBERS_BASE+101,
  DE_PROMPT_HUNDERT = DE_PROMPT_NUMBERS_BASE+102,
  DE_PROMPT_TAUSEND = DE_PROMPT_NUMBERS_BASE+103,
  DE_PROMPT_COMMA = 104,
  DE_PROMPT_UND,
  DE_PROMPT_MINUS,
};

static inline bool hasDualUnits(uint8_t unit) { // true if unit has a secondary unit for value 1 
  return (unit == UNIT_HOURS ||                 // example with sec. unit: "null Stunden", "eine Stunde", "zwei Stunden", ...
          unit == UNIT_MINUTES ||               // exmple without sec. unit "null Volt", "ein Volt", "zwei Volt", ...
          unit == UNIT_SECONDS ||              
          unit == UNIT_FLOZ ||                
          unit == UNIT_MS ||
          unit == UNIT_US ||
          unit == UNIT_MPH ||
          unit == UNIT_RPMS ||
          unit == UNIT_MAH ||
          unit == UNIT_RADIANS);                // sepcial case for value 1: "ein Radiant", not "eine Radiant"
}

#define DE_PUSH_UNIT_PROMPT(u, p) de_pushUnitPrompt((u), (p), id, fragmentVolume)

I18N_PLAY_FUNCTION(de, pushUnitPrompt, uint8_t unitprompt, int16_t number)
{
  if(number != 1 && hasDualUnits(unitprompt))
    PUSH_UNIT_PROMPT(unitprompt, 1); // value is not 1 and unit has secondary unit -> <unit>1.wav for secondary unit (plural)
  else
    PUSH_UNIT_PROMPT(unitprompt, 0); // value is 1 or unit has no secondary unit -> push <unit>0.wav for primary unit (singular)
}

I18N_PLAY_FUNCTION(de, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_MINUS);
    number = -number;
  }

  int8_t mode = MODE(att);

  if (mode > 0) {
    if (mode == 2) {
      number /= 10;
    }

    div_t qr = div((int)number, 10);

    if (qr.rem > 0) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(DE_PROMPT_COMMA);
      PUSH_NUMBER_PROMPT(DE_PROMPT_NUMBERS_BASE + qr.rem);
      
      number = -1;      // force secondary unit if unit has one
                        // skips the following integer processing 
    }
    else {
      number = qr.quot;   // no remainder, continue with integer part
    }
  }

  if (number >= 2000) {
    PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(DE_PROMPT_TAUSEND);
    number %= 1000;
    if (number == 0)
      number = -1;       
  }

  if ((number >= 1000) && (number < 2000)) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_EIN);
    PUSH_NUMBER_PROMPT(DE_PROMPT_TAUSEND);
    number %= 1000;
    if (number == 0)
      number = -1;    
  }

  if ((number >= 200) && (number < 1000)) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_NULL + number / 100);
    PUSH_NUMBER_PROMPT(DE_PROMPT_HUNDERT);	
    number %= 100;
    if (number == 0)
      number = -1;    
  }

  if ((number >= 100) && (number < 200)) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_EIN);
    PUSH_NUMBER_PROMPT(DE_PROMPT_HUNDERT);
    number %= 100;
    if (number == 0)
      number = -1;
  }

  if (number >= 0) {
    if (number == 1) {
      if(unit) {
        if(hasDualUnits(unit) && unit != UNIT_RADIANS) 
          PUSH_NUMBER_PROMPT(DE_PROMPT_EINE);           // value is 1, has unit and unit has secondary unit -> "eine",
                                                        // except radians -> "ein"
        else
          PUSH_NUMBER_PROMPT(DE_PROMPT_EIN);            // value is 1, has unit and unit has no secondary unit -> "ein"
                                                        // or unit is radians (which has secondary unit but German ...)
      } else
        PUSH_NUMBER_PROMPT(DE_PROMPT_NULL + 1);         // value is 1, has no unit -> regular number "eins"
    } else
      PUSH_NUMBER_PROMPT(DE_PROMPT_NULL + number);      // value is not 1, has no unit or unit has 
                                                        // no secondary unit -> regular number
  }

  if(unit)
    DE_PUSH_UNIT_PROMPT(unit, number);
}

I18N_PLAY_FUNCTION(de, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_MINUS);
    seconds = -seconds;
  }

  int hours, minutes;
  hours = seconds / 3600;
  seconds = seconds % 3600;
  minutes = seconds / 60;
  seconds = seconds % 60;

  if (IS_PLAY_LONG_TIMER() && seconds >= 30) {
    minutes += 1;
  }

  if (hours > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(hours, UNIT_HOURS, 0);
  }

  if (minutes > 0) {
    PLAY_NUMBER(minutes, UNIT_MINUTES, 0);
  }

  if (!IS_PLAY_LONG_TIMER() && seconds > 0) {
    if (minutes)
      PUSH_NUMBER_PROMPT(DE_PROMPT_UND);
      
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(de, TR_VOICE_DEUTSCH);

