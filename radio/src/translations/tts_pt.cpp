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

enum PortuguesePrompts {
  PT_PROMPT_NUMBERS_BASE = 0,
  PT_PROMPT_ZERO = PT_PROMPT_NUMBERS_BASE+0,
  PT_PROMPT_CEM = PT_PROMPT_NUMBERS_BASE+100,
  PT_PROMPT_CENTO = PT_PROMPT_NUMBERS_BASE+101,
  PT_PROMPT_DUZENTOS = PT_PROMPT_NUMBERS_BASE+102,
  PT_PROMPT_TREZCENTOS,
  PT_PROMPT_QUATROCENTOS,
  PT_PROMPT_QUINHENTOS,
  PT_PROMPT_SEISCENTOS,
  PT_PROMPT_SETECENTOS,
  PT_PROMPT_OITOCENTOS,
  PT_PROMPT_NUEVECENTOS,
  PT_PROMPT_MIL = PT_PROMPT_NUMBERS_BASE+110,
  PT_PROMPT_VIRGULA = 111,
  PT_PROMPT_UMA,
  PT_PROMPT_DUAS,
  PT_PROMPT_E,
  PT_PROMPT_MENOS,
  PT_PROMPT_HORA,
  PT_PROMPT_HORAS,
  PT_PROMPT_MINUTO,
  PT_PROMPT_MINUTOS,
  PT_PROMPT_SEGUNDO,
  PT_PROMPT_SEGUNDOS,

  PT_PROMPT_UNITS_BASE = 122,
  PT_PROMPT_VOLTS = PT_PROMPT_UNITS_BASE+UNIT_VOLTS,
  PT_PROMPT_AMPS = PT_PROMPT_UNITS_BASE+UNIT_AMPS,
  PT_PROMPT_METERS_PER_SECOND = PT_PROMPT_UNITS_BASE+UNIT_METERS_PER_SECOND,
  PT_PROMPT_SPARE1 = PT_PROMPT_UNITS_BASE+UNIT_RAW,
  PT_PROMPT_KMH = PT_PROMPT_UNITS_BASE+UNIT_SPEED,
  PT_PROMPT_METERS = PT_PROMPT_UNITS_BASE+UNIT_DIST,
  PT_PROMPT_DEGREES = PT_PROMPT_UNITS_BASE+UNIT_TEMPERATURE,
  PT_PROMPT_PERCENT = PT_PROMPT_UNITS_BASE+UNIT_PERCENT,
  PT_PROMPT_MILLIAMPS = PT_PROMPT_UNITS_BASE+UNIT_MILLIAMPS,
  PT_PROMPT_MAH = PT_PROMPT_UNITS_BASE+UNIT_MAH,
  PT_PROMPT_WATTS = PT_PROMPT_UNITS_BASE+UNIT_WATTS,
  PT_PROMPT_FEET = PT_PROMPT_UNITS_BASE+UNIT_FEET,
  PT_PROMPT_KTS = PT_PROMPT_UNITS_BASE+UNIT_KTS,
  PT_PROMPT_MILLILITERS = PT_PROMPT_UNITS_BASE+UNIT_MILLILITERS,
  PT_PROMPT_FLOZ = PT_PROMPT_UNITS_BASE+UNIT_FLOZ,
  PT_PROMPT_FEET_PER_SECOND = PT_PROMPT_UNITS_BASE+UNIT_FEET_PER_SECOND,
};


  #define PT_PUSH_UNIT_PROMPT(u) pt_pushUnitPrompt((u), id)

I18N_PLAY_FUNCTION(pt, pushUnitPrompt, uint8_t unitprompt)
{
    PUSH_UNIT_PROMPT(unitprompt, 0);
}

I18N_PLAY_FUNCTION(pt, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(PT_PROMPT_MENOS);
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
      PUSH_NUMBER_PROMPT(PT_PROMPT_VIRGULA);
      if (mode==2 && qr.rem < 10)
        PUSH_NUMBER_PROMPT(PT_PROMPT_ZERO);
      PLAY_NUMBER(qr.rem, unit, 0);
    }
    else {
      PLAY_NUMBER(qr.quot, unit, 0);
    }
    return;
  }

  if (number >= 1000) {
    if (number >= 2000) {
      PLAY_NUMBER(number / 1000, 0, 0);
      PUSH_NUMBER_PROMPT(PT_PROMPT_MIL);
    } else {
      PUSH_NUMBER_PROMPT(PT_PROMPT_MIL);
    }
    number %= 1000;
    if (number == 0)
      number = -1;
  }

  if (number > 100) {
    PUSH_NUMBER_PROMPT(PT_PROMPT_CEM + number/100);
    number %= 100;
    if (number == 0)
      number = -1;
  }

  if (number == 100) {
    PUSH_NUMBER_PROMPT(PT_PROMPT_CEM);
  }
  else {
    PUSH_NUMBER_PROMPT(PT_PROMPT_ZERO + number);
  }

  if (unit) {
    PT_PUSH_UNIT_PROMPT(unit);
  }
}

I18N_PLAY_FUNCTION(pt, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(PT_PROMPT_MENOS);
    seconds = -seconds;
  }

  uint8_t ore = 0;
  uint8_t tmp;
  if (IS_PLAY_LONG_TIMER()) {
    tmp = seconds / 60;
    if (seconds % 60 >= 30) tmp += 1;
    if (tmp > 0) PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
  } else {
    tmp = seconds / 3600;
    seconds %= 3600;
    if (tmp > 0 || IS_PLAY_TIME()) {
      ore = tmp;
      if (tmp > 2) {
        PLAY_NUMBER(tmp, 0, 0);
        PUSH_UNIT_PROMPT(UNIT_HOURS, 1);
      } else if (tmp == 2) {
        PUSH_NUMBER_PROMPT(PT_PROMPT_DUAS);
        PUSH_UNIT_PROMPT(UNIT_HOURS, 1);
      } else if (tmp == 1) {
        PUSH_NUMBER_PROMPT(PT_PROMPT_UMA);
        PUSH_UNIT_PROMPT(UNIT_HOURS, 0);
      }
    }

    tmp = seconds / 60;
    seconds %= 60;
    if (tmp > 0 || ore > 0) {
      if (tmp != 1) {
        PLAY_NUMBER(tmp, 0, 0);
        PUSH_UNIT_PROMPT(UNIT_MINUTES, 1);
      } else {
        PUSH_NUMBER_PROMPT(PT_PROMPT_NUMBERS_BASE + 1);
        PUSH_UNIT_PROMPT(UNIT_MINUTES, 0);
      }
      PUSH_NUMBER_PROMPT(PT_PROMPT_E);
    }

    if (seconds != 1) {
      PLAY_NUMBER(seconds, 0, 0);
      PUSH_UNIT_PROMPT(UNIT_SECONDS, 1);
    } else {
      PUSH_NUMBER_PROMPT(PT_PROMPT_NUMBERS_BASE + 1);
      PUSH_UNIT_PROMPT(UNIT_SECONDS, 0);
    }
  }
}

LANGUAGE_PACK_DECLARE(pt, "Portugues");

