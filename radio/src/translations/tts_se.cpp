/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Kjell Kernen
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "edgetx.h"

enum SwedishPrompts {
  SE_PROMPT_NUMBERS_BASE = 0,
  SE_PROMPT_ZERO = SE_PROMPT_NUMBERS_BASE+0,       //02-99
  SE_PROMPT_HUNDRED = SE_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900
  SE_PROMPT_THOUSAND = SE_PROMPT_NUMBERS_BASE+109, //1000
  SE_PROMPT_AND = SE_PROMPT_NUMBERS_BASE+110,
  SE_PROMPT_MINUS = SE_PROMPT_NUMBERS_BASE+111,
  SE_PROMPT_POINT = SE_PROMPT_NUMBERS_BASE+112,
  SE_PROMPT_UNITS_BASE = 113,
  SE_PROMPT_POINT_BASE = 165, //.0 - .9
};


  #define SE_PUSH_UNIT_PROMPT(u, p) se_pushUnitPrompt((u), (p), id)

I18N_PLAY_FUNCTION(se, pushUnitPrompt, uint8_t unitprompt, int16_t number)
{
  if (number == 1)
    PUSH_UNIT_PROMPT(unitprompt, 0);
  else
    PUSH_UNIT_PROMPT(unitprompt, 1);
}

I18N_PLAY_FUNCTION(se, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_MINUS);
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
      PUSH_NUMBER_PROMPT(SE_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmpNumber = number;

  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(SE_PROMPT_THOUSAND);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_ZERO + number);
  }

  if (unit) {
    SE_PUSH_UNIT_PROMPT(unit, tmpNumber);
  }
}

I18N_PLAY_FUNCTION(se, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, UNIT_HOURS, 0);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
    if (seconds > 0)
      PUSH_NUMBER_PROMPT(SE_PROMPT_AND);
  }

  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(se, "Swedish");

