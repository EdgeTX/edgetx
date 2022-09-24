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

enum FrenchPrompts {
  FR_PROMPT_NUMBERS_BASE = 0,
  FR_PROMPT_ZERO = FR_PROMPT_NUMBERS_BASE+0,
  FR_PROMPT_CENT = FR_PROMPT_NUMBERS_BASE+100,
  FR_PROMPT_DEUX_CENT = FR_PROMPT_CENT+1,
  FR_PROMPT_MILLE = FR_PROMPT_CENT+9,

  FR_PROMPT_UNE = 110,
  FR_PROMPT_ONZE,
  FR_PROMPT_VINGT_ET_UNE,
  FR_PROMPT_TRENTE_ET_UNE,
  FR_PROMPT_QUARANTE_ET_UNE,
  FR_PROMPT_CINQUANTE_ET_UNE,
  FR_PROMPT_SOIXANTE_ET_UNE,
  FR_PROMPT_SOIXANTE_ET_ONZE,
  FR_PROMPT_QUATRE_VINGT_UNE,

  FR_PROMPT_VIRGULE = 119,
  FR_PROMPT_ET = 120,
  FR_PROMPT_MOINS = 121,
  FR_PROMPT_MINUIT = 122,
  FR_PROMPT_MIDI = 123,

  FR_PROMPT_UNITS_BASE = 124,
  FR_PROMPT_HEURE = FR_PROMPT_UNITS_BASE+UNIT_HOURS,
  FR_PROMPT_MINUTE = FR_PROMPT_UNITS_BASE+UNIT_MINUTES,
  FR_PROMPT_SECONDE = FR_PROMPT_UNITS_BASE+UNIT_SECONDS,
  FR_PROMPT_VIRGULE_BASE = 180, //,0 - ,9
};


  #define FR_PUSH_UNIT_PROMPT(u) fr_pushUnitPrompt((u), id)

I18N_PLAY_FUNCTION(fr, pushUnitPrompt, uint8_t unitprompt)
{
  PUSH_UNIT_PROMPT(unitprompt, 0);
}

#define FEMININ 0x80

I18N_PLAY_FUNCTION(fr, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
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
    PUSH_NUMBER_PROMPT(FR_PROMPT_MOINS);
    number = -number;
  }


  int8_t mode = MODE(att);
  if (mode > 0) {
    if (mode == 2) {
      number /= 10;
    }
    div_t qr = div((int)number, 10);
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(FR_PROMPT_VIRGULE_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  if (number >= 1000) {
    if (number >= 2000)
      PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(FR_PROMPT_MILLE);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    if (number >= 200)
      PUSH_NUMBER_PROMPT(FR_PROMPT_ZERO + number/100);
    PUSH_NUMBER_PROMPT(FR_PROMPT_CENT);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (((number % 10) == 1) && number < 90 && (att & FEMININ)) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_UNE+(number/10));
  }
  else if (number >= 0) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_ZERO+number);
  }

  if (unit) {
    FR_PUSH_UNIT_PROMPT(unit);
  }
}

I18N_PLAY_FUNCTION(fr, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(0, 0, FEMININ);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_MOINS);
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
    if (IS_PLAY_TIME() && tmp == 0) {
      PUSH_NUMBER_PROMPT(FR_PROMPT_MINUIT);
    } else if (IS_PLAY_TIME() && tmp == 12) {
      PUSH_NUMBER_PROMPT(FR_PROMPT_MIDI);
    } else if (tmp > 0) {
      PLAY_NUMBER(tmp, UNIT_HOURS, FEMININ);
    }

    tmp = seconds / 60;
    seconds %= 60;
    if (tmp > 0) {
      if (IS_PLAY_TIME()) {
        PLAY_NUMBER(tmp, 0, tmp == 1 ? FEMININ : 0);
      } else {
        PLAY_NUMBER(tmp, UNIT_MINUTES, FEMININ);
        if (seconds > 0) PUSH_NUMBER_PROMPT(FR_PROMPT_ET);
      }
    }

    if (!IS_PLAY_TIME() && seconds > 0) {
      PLAY_NUMBER(seconds, UNIT_SECONDS, FEMININ);
    }
  }
}

LANGUAGE_PACK_DECLARE(fr, "Francais");

