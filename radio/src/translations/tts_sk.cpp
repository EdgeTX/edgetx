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

enum SlovakPrompts {
  SK_PROMPT_NUMBERS_BASE = 0,
  SK_PROMPT_NULA = SK_PROMPT_NUMBERS_BASE+0,    //0-99
  SK_PROMPT_STO = SK_PROMPT_NUMBERS_BASE+100,   //100,200 .. 900
  SK_PROMPT_TISIC = SK_PROMPT_NUMBERS_BASE+109, //1000
  SK_PROMPT_DVETISIC = SK_PROMPT_NUMBERS_BASE+110,
  SK_PROMPT_JEDEN = SK_PROMPT_NUMBERS_BASE+111,
  SK_PROMPT_JEDNO = SK_PROMPT_NUMBERS_BASE+112,
  SK_PROMPT_DVE = SK_PROMPT_NUMBERS_BASE+113,
  SK_PROMPT_CELA = SK_PROMPT_NUMBERS_BASE+114,
  SK_PROMPT_CELE = SK_PROMPT_NUMBERS_BASE+115,
  SK_PROMPT_CELYCH = SK_PROMPT_NUMBERS_BASE+116,
  SK_PROMPT_MINUS = SK_PROMPT_NUMBERS_BASE+117,

  SK_PROMPT_UNITS_BASE = 118, //(jeden)volt,(dva)volty,(pet)voltu,(desetina)voltu

};


  #define SK_PUSH_UNIT_PROMPT(u, p) sk_pushUnitPrompt((u), (p), id)

#define MUZSKY 0x80
#define ZENSKY 0x81
#define STREDNI 0x82

I18N_PLAY_FUNCTION(sk, pushUnitPrompt, uint8_t unitprompt, int16_t number)
{
  if (number == 1)
    PUSH_UNIT_PROMPT(unitprompt, 0);
  else if (number > 1 && number < 5)
    PUSH_UNIT_PROMPT(unitprompt, 1);
  else
    PUSH_UNIT_PROMPT(unitprompt, 2);
}

I18N_PLAY_FUNCTION(sk, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{

  if (number < 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_MINUS);
    number = -number;
  }


  int8_t mode = MODE(att);
  if (mode > 0) {
    if (mode == 2) {
      number /= 10;
    }
    div_t qr = div((int)number, 10);
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, ZENSKY);
      if (qr.quot == 0)
        PUSH_NUMBER_PROMPT(SK_PROMPT_CELA);
      else
        SK_PUSH_UNIT_PROMPT(SK_PROMPT_CELA, qr.quot);
      PLAY_NUMBER(qr.rem, 0, ZENSKY);
      PUSH_NUMBER_PROMPT(SK_PROMPT_UNITS_BASE+((unit-1)*4)+3);
      return;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;

  switch(unit) {
    case 0:
      break;
    case 6:
    case 8:
    case 10:
    case 14:
    case 17:
    case 21:
    case 22:
    case 23:
    case 24:
      att = ZENSKY;
      break;
    case 13:
    case 18:
      att = STREDNI;
      break;
    default:
      att = MUZSKY;
      break;
  }

  if ((number == 1) && (att == MUZSKY)) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_JEDEN);
    number = -1;
  }

  if ((number == 1) && (att == STREDNI)) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_JEDNO);
    number = -1;
  }

  if ((number == 2) && ((att == ZENSKY) || (att == STREDNI))) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_DVE);
    number = -1;
  }

  if (number >= 1000) {
    if (number >= 3000)
      PLAY_NUMBER(number / 1000, 0, 0);
    if (number >= 2000 && number < 3000)
      PUSH_NUMBER_PROMPT(SK_PROMPT_DVETISIC);
    else
      PUSH_NUMBER_PROMPT(SK_PROMPT_TISIC);

    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_STO + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }

  if (number >= 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_NULA+number);
  }

  if (unit) {
    SK_PUSH_UNIT_PROMPT(unit, tmp);
  }
}

I18N_PLAY_FUNCTION(sk, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_MINUS);
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
      PLAY_NUMBER(tmp, UNIT_HOURS, ZENSKY);
    }

    tmp = seconds / 60;
    seconds %= 60;
    if (tmp > 0) {
      PLAY_NUMBER(tmp, UNIT_MINUTES, ZENSKY);
    }

    if (seconds > 0) {
      PLAY_NUMBER(seconds, UNIT_SECONDS, ZENSKY);
    }
  }
}

LANGUAGE_PACK_DECLARE(sk, "Slovak");

