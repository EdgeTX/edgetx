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

enum ItalianPrompts {
  IT_PROMPT_NUMBERS_BASE = 0,
  IT_PROMPT_ZERO = IT_PROMPT_NUMBERS_BASE+0,
  IT_PROMPT_CENT = IT_PROMPT_NUMBERS_BASE+100,
  IT_PROMPT_MILA = IT_PROMPT_NUMBERS_BASE+101,
  IT_PROMPT_MILLE = IT_PROMPT_NUMBERS_BASE+102,
  IT_PROMPT_VIRGOLA = 103,
  IT_PROMPT_UN,
  IT_PROMPT_E,
  IT_PROMPT_MENO,
  IT_PROMPT_ORA,
  IT_PROMPT_ORE,
  IT_PROMPT_MINUTO,
  IT_PROMPT_MINUTI,
  IT_PROMPT_SECONDO,
  IT_PROMPT_SECONDI,

  IT_PROMPT_UNITS_BASE = 113,
  IT_PROMPT_VOLTS = IT_PROMPT_UNITS_BASE+(UNIT_VOLTS*2),
  IT_PROMPT_AMPS = IT_PROMPT_UNITS_BASE+(UNIT_AMPS*2),
  IT_PROMPT_METERS_PER_SECOND = IT_PROMPT_UNITS_BASE+(UNIT_METERS_PER_SECOND*2),
  IT_PROMPT_SPARE1 = IT_PROMPT_UNITS_BASE+(UNIT_RAW*2),
  IT_PROMPT_KMH = IT_PROMPT_UNITS_BASE+(UNIT_SPEED*2),
  IT_PROMPT_METERS = IT_PROMPT_UNITS_BASE+(UNIT_DIST*2),
  IT_PROMPT_DEGREES = IT_PROMPT_UNITS_BASE+(UNIT_TEMPERATURE*2),
  IT_PROMPT_PERCENT = IT_PROMPT_UNITS_BASE+(UNIT_PERCENT*2),
  IT_PROMPT_MILLIAMPS = IT_PROMPT_UNITS_BASE+(UNIT_MILLIAMPS*2),
  IT_PROMPT_MAH = IT_PROMPT_UNITS_BASE+(UNIT_MAH*2),
  IT_PROMPT_WATTS = IT_PROMPT_UNITS_BASE+(UNIT_WATTS*2),
  IT_PROMPT_FEET = IT_PROMPT_UNITS_BASE+(UNIT_FEET*2),
  IT_PROMPT_KTS = IT_PROMPT_UNITS_BASE+(UNIT_KTS*2),
  IT_PROMPT_MILLILITERS = IT_PROMPT_UNITS_BASE+(UNIT_MILLILITERS*2),
  IT_PROMPT_FLOZ = IT_PROMPT_UNITS_BASE+(UNIT_FLOZ*2),
  IT_PROMPT_FEET_PER_SECOND = IT_PROMPT_UNITS_BASE+(UNIT_FEET_PER_SECOND*2),

};

  #define IT_PUSH_UNIT_PROMPT(u, p) it_pushUnitPrompt((u), (p), id)

I18N_PLAY_FUNCTION(it, pushUnitPrompt, uint8_t unitprompt, int16_t number)
{
  if (number == 1)
    PUSH_UNIT_PROMPT(unitprompt, 0);
  else
    PUSH_UNIT_PROMPT(unitprompt, 1);
}


I18N_PLAY_FUNCTION(it, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
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
  getvalue_t orignumber;
  if (number < 0) {
    PUSH_NUMBER_PROMPT(IT_PROMPT_MENO);
    number = -number;
  }
  orignumber=number;

  int8_t mode = MODE(att);
  if (mode > 0) {
    if (mode == 2) {
      number /= 10;
    }
    div_t qr = div((int)number, 10);
    if (qr.rem > 0) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(IT_PROMPT_VIRGOLA);
      if (mode==2 && qr.rem < 10)
        PUSH_NUMBER_PROMPT(IT_PROMPT_ZERO);
      PLAY_NUMBER(qr.rem, 0, 0);
    }
    else {
      if (qr.quot==1) {
        PUSH_NUMBER_PROMPT(IT_PROMPT_UN);
        if (unit) {
          PUSH_NUMBER_PROMPT(IT_PROMPT_UNITS_BASE+(unit*2));
        }
        return;
      } else {
        PLAY_NUMBER(qr.quot, 0, 0);
      }
    }
  } else {
    if (orignumber == 1 && unit) {
      PUSH_NUMBER_PROMPT(IT_PROMPT_UN);
    } else {
      if (number >= 1000) {
        if (number >= 2000) {
          PLAY_NUMBER(number / 1000, 0, 0);
          PUSH_NUMBER_PROMPT(IT_PROMPT_MILA);
        } else {
          PUSH_NUMBER_PROMPT(IT_PROMPT_MILLE);
        }
        number %= 1000;
        if (number == 0)
          number = -1;
      }
      if (number >= 100) {
        if (number >= 200)
          PUSH_NUMBER_PROMPT(IT_PROMPT_ZERO + number/100);
        PUSH_NUMBER_PROMPT(IT_PROMPT_CENT);
        number %= 100;
        if (number == 0)
          number = -1;
      }
      PUSH_NUMBER_PROMPT(IT_PROMPT_ZERO+number);
    }
  }
  if (unit) {
    IT_PUSH_UNIT_PROMPT(unit, orignumber);
  }
}

I18N_PLAY_FUNCTION(it, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(IT_PROMPT_MENO);
    seconds = -seconds;
  }

  uint8_t tmp;
  if (IS_PLAY_LONG_TIMER()) {
    tmp = seconds / 60;
    if (seconds % 60 >= 30) tmp += 1;
    if (tmp > 0) PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
  } else {
    tmp = seconds / 3600;
    seconds %= 3600;
    if (tmp > 0 || IS_PLAY_TIME()) {
      PLAY_NUMBER(tmp, UNIT_HOURS, 0);
    }

    tmp = seconds / 60;
    seconds %= 60;
    if (tmp > 0) {
      PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
      if (seconds > 0) PUSH_NUMBER_PROMPT(IT_PROMPT_E);
    }

    if (seconds > 0) {
      PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
    }
  }
}

LANGUAGE_PACK_DECLARE(it, "Italiano");

