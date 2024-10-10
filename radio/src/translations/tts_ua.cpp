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

enum UAPrompts {
  UA_PROMPT_NUMBERS_BASE = 0,
  UA_PROMPT_ZERO = UA_PROMPT_NUMBERS_BASE+0,       //02-99
  UA_PROMPT_HUNDRED = UA_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900

  UA_PROMPT_AND = UA_PROMPT_NUMBERS_BASE+110, // и
  UA_PROMPT_MINUS = UA_PROMPT_NUMBERS_BASE+111, // минус
  UA_PROMPT_UNITS_BASE = UA_PROMPT_NUMBERS_BASE + 113,
  UA_PROMPT_POINT_BASE = UA_PROMPT_NUMBERS_BASE + 165, //.0 - .9
  UA_PROMPT_FEMALE_ONE = UA_PROMPT_NUMBERS_BASE + 180,
  UA_PROMPT_FEMALE_TWO = UA_PROMPT_NUMBERS_BASE + 190,

  UA_PROMPT_THOUSAND1 = UA_PROMPT_NUMBERS_BASE+200, //1000
  UA_PROMPT_THOUSAND2 = UA_PROMPT_NUMBERS_BASE+201, //2000
  UA_PROMPT_THOUSAND5 = UA_PROMPT_NUMBERS_BASE+202, //5000
};
#define MALE     0x00
#define FEMALE     0x01
#define UA_FEMALE_UNIT 0xFF

#define UA_PUSH_UNIT_PROMPT(u, p) ua_pushUnitPrompt((u), (p), id, fragmentVolume)

I18N_PLAY_FUNCTION(ua, pushUnitPrompt,  uint8_t unitprompt, int16_t number)
{
    if (number < 0){ // if negative number, we have to use 2 units form (for example value = 1.3)
       PUSH_UNIT_PROMPT(unitprompt, 2);
    }
    else{
      int16_t mod10 = number % 10;
      if (number == 0)
        PUSH_UNIT_PROMPT(unitprompt, 0);
      else if (number == 1)
        PUSH_UNIT_PROMPT(unitprompt, 1);
      else if (number>=2 && number <=4)
        PUSH_UNIT_PROMPT(unitprompt, 2);
      else if (number>=5 && number <=20)
        PUSH_UNIT_PROMPT(unitprompt, 5);
      else if (mod10 == 1)
        PUSH_UNIT_PROMPT(unitprompt, 1);
      else if (mod10 >= 2 && mod10 <=4)
        PUSH_UNIT_PROMPT(unitprompt, 2);
      else
        PUSH_UNIT_PROMPT(unitprompt, 5);
    }
}

I18N_PLAY_FUNCTION(ua, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(UA_PROMPT_MINUS);
    number = -number;
  }

  div_t qr = div((int)number, 10);
  int8_t mode = MODE(att);
  if (mode > 0 && att != UA_FEMALE_UNIT) {
    if (mode == 2) {
      number /= 10;
    }
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(UA_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;

  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, UA_FEMALE_UNIT, 0); // female
    uint8_t thousands = number / 1000;
    int16_t mod10 = thousands % 10;
    if (thousands == 1)
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND1);
    else if (thousands>=2 && thousands <=4)
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND2);
    else if (thousands>=5 && thousands <=20)
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND5);
    else if (mod10 == 1)
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND1);
    else if (mod10 >= 2 && mod10 <=4)
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND2);
    else
      PUSH_NUMBER_PROMPT(UA_PROMPT_THOUSAND5);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(UA_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
      uint8_t attr = MALE;
      switch(unit) {
        case UA_FEMALE_UNIT:
        case UNIT_MPH:
        case UNIT_FLOZ:
        case UNIT_MINUTES:
        case UNIT_SECONDS:
          attr = FEMALE;
          break;
        default:
          attr = MALE;
          break;
      }
      uint8_t lastDigit = number % 10;
      uint8_t ten=(number - (number % 10))/10;
      if (lastDigit == 1 && number != 11 && attr == FEMALE)
        PUSH_NUMBER_PROMPT(UA_PROMPT_FEMALE_ONE + ten);
      else if (lastDigit == 2 && number !=12 && attr == FEMALE)
        PUSH_NUMBER_PROMPT(UA_PROMPT_FEMALE_TWO + ten);
      else
        PUSH_NUMBER_PROMPT(UA_PROMPT_ZERO + number);
  }

  if (unit) {
    if (mode > 0 && qr.rem) // number with decimal point
      UA_PUSH_UNIT_PROMPT(unit, -1); // force 2 units form, if float value
    else
      UA_PUSH_UNIT_PROMPT(unit, tmp);
  }
}

I18N_PLAY_FUNCTION(ua, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(UA_PROMPT_MINUS);
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
      PUSH_NUMBER_PROMPT(UA_PROMPT_AND);
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(ua, TR_VOICE_UKRAINIAN);
