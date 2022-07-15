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

enum CzechPrompts {
  CZ_PROMPT_NUMBERS_BASE = 0,
  CZ_PROMPT_NULA = CZ_PROMPT_NUMBERS_BASE+0,    // 0-99
  CZ_PROMPT_STO = CZ_PROMPT_NUMBERS_BASE+100,   // 100,200 .. 900
  CZ_PROMPT_TISIC = CZ_PROMPT_NUMBERS_BASE+109, // 1000
  CZ_PROMPT_TISICE = CZ_PROMPT_NUMBERS_BASE+110,
  CZ_PROMPT_JEDEN = CZ_PROMPT_NUMBERS_BASE+111,
  CZ_PROMPT_JEDNO = CZ_PROMPT_NUMBERS_BASE+112,
  CZ_PROMPT_DVE = CZ_PROMPT_NUMBERS_BASE+113,
  CZ_PROMPT_CELA = CZ_PROMPT_NUMBERS_BASE+114,
  CZ_PROMPT_CELE = CZ_PROMPT_NUMBERS_BASE+115,
  CZ_PROMPT_CELYCH = CZ_PROMPT_NUMBERS_BASE+116,
  CZ_PROMPT_MINUS = CZ_PROMPT_NUMBERS_BASE+117,

  CZ_PROMPT_UNITS_BASE = 118, // (jeden)volt,(dva)volty,(pet)voltu,(desetina)voltu
};


  #define CZ_PUSH_UNIT_PROMPT(u, p) cz_pushUnitPrompt((u), (p), id)

#define MALE     0x80
#define FEMALE     0x81
#define NEUTRAL    0x82

I18N_PLAY_FUNCTION(cz, pushUnitPrompt, uint8_t unit, int16_t number)
{
  TRACE("CZSAY unit:%d number:%d", unit, number);
  if (number == 1)
    PUSH_UNIT_PROMPT(unit, 0);
  else if (number > 1 && number < 5)
    PUSH_UNIT_PROMPT(unit, 1);
  else
    PUSH_UNIT_PROMPT(unit, 2);
}

I18N_PLAY_FUNCTION(cz, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_MINUS);
    number = -number;
  }


  int8_t mode = MODE(att);
  if (mode > 0) {
    if (mode == 2) {
      number /= 10;
    }
    div_t qr = div((int)number, 10);
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, FEMALE);
      if (qr.quot < 2) {
        PUSH_NUMBER_PROMPT(CZ_PROMPT_CELA);
      }
      else if (qr.quot > 1 && qr.quot < 5) {
        PUSH_NUMBER_PROMPT(CZ_PROMPT_CELE);
      }
      else {
        PUSH_NUMBER_PROMPT(CZ_PROMPT_CELYCH);
      };
      PLAY_NUMBER(qr.rem, 0, FEMALE);
      PUSH_UNIT_PROMPT(unit, 3);
      return;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;
  switch(unit) {
    case UNIT_RAW:
      break;
    case UNIT_FEET_PER_SECOND:
    case UNIT_MPH:
    case UNIT_FEET:
    case UNIT_MAH:
    case UNIT_RPMS:
    case UNIT_FLOZ:
    case UNIT_HOURS:
    case UNIT_MINUTES:
    case UNIT_SECONDS:
      att = FEMALE;
      break;
    case UNIT_PERCENT:
    case UNIT_G:
      att = NEUTRAL;
      break;
    default:
      att = MALE;
      break;
  }

  if ((number == 1) && (att == MALE)) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_JEDEN);
    number = -1;
  }

  if ((number == 1) && (att == NEUTRAL)) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_JEDNO);
    number = -1;
  }

  if ((number == 2) && ((att == FEMALE) || (att == NEUTRAL))) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_DVE);
    number = -1;
  }

  if (number >= 1000) {
    if (number >= 2000)
      PLAY_NUMBER(number / 1000, 0, 0);
    if (number >= 2000 && number < 5000)
      PUSH_NUMBER_PROMPT(CZ_PROMPT_TISICE);
    else
      PUSH_NUMBER_PROMPT(CZ_PROMPT_TISIC);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_STO + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }

  if (number >= 0) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_NULA+number);
  }

  if (unit) {
    CZ_PUSH_UNIT_PROMPT(unit, tmp);
  }
}

I18N_PLAY_FUNCTION(cz, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(CZ_PROMPT_MINUS);
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
      PLAY_NUMBER(tmp, UNIT_HOURS, FEMALE);
    }

    tmp = seconds / 60;
    seconds %= 60;
    if (tmp > 0) {
      PLAY_NUMBER(tmp, UNIT_MINUTES, FEMALE);
    }

    if (seconds > 0) {
      PLAY_NUMBER(seconds, UNIT_SECONDS, FEMALE);
    }
  }
}

LANGUAGE_PACK_DECLARE(cz, "Czech");

