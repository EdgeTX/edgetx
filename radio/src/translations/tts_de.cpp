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

#include "opentx.h"

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
  DE_PROMPT_UHR,
  DE_PROMPT_MINUTE,
  DE_PROMPT_MINUTEN,
  DE_PROMPT_SEKUNDE,
  DE_PROMPT_SEKUNDEN,
  DE_PROMPT_STUNDE,
  DE_PROMPT_STUNDEN,

  DE_PROMPT_UNITS_BASE = 114,
  DE_PROMPT_VOLTS = DE_PROMPT_UNITS_BASE+UNIT_VOLTS,
  DE_PROMPT_AMPS = DE_PROMPT_UNITS_BASE+UNIT_AMPS,
  DE_PROMPT_METERS_PER_SECOND = DE_PROMPT_UNITS_BASE+UNIT_METERS_PER_SECOND,
  DE_PROMPT_SPARE1 = DE_PROMPT_UNITS_BASE+UNIT_RAW,
  DE_PROMPT_KMH = DE_PROMPT_UNITS_BASE+UNIT_SPEED,
  DE_PROMPT_METERS = DE_PROMPT_UNITS_BASE+UNIT_DIST,
  DE_PROMPT_DEGREES = DE_PROMPT_UNITS_BASE+UNIT_TEMPERATURE,
  DE_PROMPT_PERCENT = DE_PROMPT_UNITS_BASE+UNIT_PERCENT,
  DE_PROMPT_MILLIAMPS = DE_PROMPT_UNITS_BASE+UNIT_MILLIAMPS,
  DE_PROMPT_MAH = DE_PROMPT_UNITS_BASE+UNIT_MAH,
  DE_PROMPT_WATTS = DE_PROMPT_UNITS_BASE+UNIT_WATTS,
  DE_PROMPT_FEET = DE_PROMPT_UNITS_BASE+UNIT_FEET,
  DE_PROMPT_KTS = DE_PROMPT_UNITS_BASE+UNIT_KTS,
  DE_PROMPT_HOURS = DE_PROMPT_UNITS_BASE+UNIT_HOURS,
  DE_PROMPT_MINUTES = DE_PROMPT_UNITS_BASE+UNIT_MINUTES,
  DE_PROMPT_SECONDS = DE_PROMPT_UNITS_BASE+UNIT_SECONDS,
  DE_PROMPT_RPMS = DE_PROMPT_UNITS_BASE+UNIT_RPMS,
  DE_PROMPT_G = DE_PROMPT_UNITS_BASE+UNIT_G,
  DE_PROMPT_MILLILITERS = DE_PROMPT_UNITS_BASE+UNIT_MILLILITERS,
  DE_PROMPT_FLOZ = DE_PROMPT_UNITS_BASE+UNIT_FLOZ,
  DE_PROMPT_FEET_PER_SECOND = DE_PROMPT_UNITS_BASE+UNIT_FEET_PER_SECOND,

};

  #define DE_PUSH_UNIT_PROMPT(u) de_pushUnitPrompt((u), id)

I18N_PLAY_FUNCTION(de, pushUnitPrompt, uint8_t unitprompt)
{
  PUSH_UNIT_PROMPT(unitprompt, 0);
}

I18N_PLAY_FUNCTION(de, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
/*  if digit >= 1000000000:
      temp_digit, digit = divmod(digit, 1000000000)
      prompts.extend(self.getNumberPrompt(temp_digit))
      prompts.append(Prompt(GUIDE_00_BILLION, dir=2))
  if digit >= 1000000:
      temp_digit, digit = divmod(digit, 1000000)
      prompts.extend(self.getNumberPrompt(temp_digit))
      prompts.append(Prompt(GUIDE_00_MILLION, dir=2))
*/

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
    }
    else {
      if (qr.quot == 1) {
        PUSH_NUMBER_PROMPT(DE_PROMPT_EIN);
      }
      else {
        PUSH_NUMBER_PROMPT(qr.quot);
      }
    }
    if(unit)
      DE_PUSH_UNIT_PROMPT(unit);
    return;
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
      PUSH_NUMBER_PROMPT(DE_PROMPT_NULL + number / 1);
  }

  if (unit) {
    DE_PUSH_UNIT_PROMPT(unit);
  }
}

I18N_PLAY_FUNCTION(de, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp;
 if (IS_PLAY_LONG_TIMER())
  {
    tmp = seconds / 60;
    if (seconds % 60 >= 30) tmp += 1;
    if (tmp > 0) PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
  }
  else
  {
    tmp = seconds / 3600;
    seconds %= 3600;

    if (tmp > 0 || IS_PLAY_TIME()) {
      if (tmp > 1) {
        PLAY_NUMBER(tmp, 0, 0);
        PUSH_NUMBER_PROMPT(DE_PROMPT_STUNDEN);
      } else {
        PUSH_NUMBER_PROMPT(DE_PROMPT_EINE);
        PUSH_NUMBER_PROMPT(DE_PROMPT_STUNDE);
      }
      if (seconds > 0) {
        PUSH_NUMBER_PROMPT(DE_PROMPT_UND);
      }
    }

    tmp = seconds / 60;
    seconds %= 60;

    if (tmp > 0) {
      if (tmp > 1) {
        PLAY_NUMBER(tmp, 0, 0);
        PUSH_NUMBER_PROMPT(DE_PROMPT_MINUTEN);
      } else {
        PUSH_NUMBER_PROMPT(DE_PROMPT_EINE);
        PUSH_NUMBER_PROMPT(DE_PROMPT_MINUTE);
      }
      if (seconds > 0) {
        PUSH_NUMBER_PROMPT(DE_PROMPT_UND);
      }
    }

    if (seconds > 1) {
      PLAY_NUMBER(seconds, 0, 0);
      PUSH_NUMBER_PROMPT(DE_PROMPT_SEKUNDEN);
    } else {
      if (seconds == 1) {
        PUSH_NUMBER_PROMPT(DE_PROMPT_EINE);
        PUSH_NUMBER_PROMPT(DE_PROMPT_SEKUNDE);
      }
    }
  }
}

LANGUAGE_PACK_DECLARE(de, "Deutsch");

