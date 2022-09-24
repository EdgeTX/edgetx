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

enum HungarianPrompts {
  HU_PROMPT_NUMBERS_BASE = 0,
  HU_PROMPT_ZERO = HU_PROMPT_NUMBERS_BASE+0,       //02-99
  HU_PROMPT_HUNDRED = HU_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900
  HU_PROMPT_THOUSAND = HU_PROMPT_NUMBERS_BASE+109, //1000
  HU_PROMPT_AND = HU_PROMPT_NUMBERS_BASE+110,
  HU_PROMPT_MINUS = HU_PROMPT_NUMBERS_BASE+111,
  HU_PROMPT_POINT = HU_PROMPT_NUMBERS_BASE+112,
  HU_PROMPT_UNITS_BASE = 113,
  HU_PROMPT_POINT_BASE = 165, //.0 - .9
};


  #define HU_PUSH_UNIT_PROMPT(u, p) hu_pushUnitPrompt((u), (p), id)

I18N_PLAY_FUNCTION(hu, pushUnitPrompt, uint8_t unitprompt, int16_t number)
{
  if (number == 1)
    PUSH_UNIT_PROMPT(unitprompt, 0);
  else
    PUSH_UNIT_PROMPT(unitprompt, 1);
}

I18N_PLAY_FUNCTION(hu, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_MINUS);
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
      PUSH_NUMBER_PROMPT(HU_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;

  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(HU_PROMPT_THOUSAND);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_ZERO + number);
  }

  if (unit) {
    HU_PUSH_UNIT_PROMPT(unit, tmp);
  }
}

I18N_PLAY_FUNCTION(hu, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_MINUS);
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
      // This is not necessary in the Hungarian
      // if (seconds > 0)
      // PUSH_NUMBER_PROMPT(HU_PROMPT_AND);
    }

    if (seconds > 0) {
      PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
    }
  }
}

LANGUAGE_PACK_DECLARE(hu, "Hungarian");

