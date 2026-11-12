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

/*
 * Arabic number and duration announcements.
 *
 * Voice pack layout (SOUNDS/ar/SYSTEM/). Recordings must follow this table;
 * the voice pack CSV in edgetx-sdcard-sounds is generated from it.
 *
 *   0000-0099  0-99, form used with MASCULINE nouns (also bare counting)
 *   0100-0108  100, 200 (مئتان), 300 ... 900
 *   0109       ألف    (1000, and after 100+ / 1-2 remainders)
 *   0110       ألفان  (2000)
 *   0111       آلاف   (after 3-10)
 *   0112       ألفًا  (after 11-99)
 *   0113       و      (and)
 *   0114       سالب   (minus)
 *   0170-0179  decimals .0-.9 ("فاصلة صفر" ... "فاصلة تسعة")
 *   0200-0299  0-99, form used with FEMININE nouns
 *
 * Both 0-99 blocks are labelled by the gender of the noun being counted, so
 * gender polarity (3-10 take the opposite gender) is part of the recording,
 * e.g. 0003 = ثلاثة (with a masculine unit), 0203 = ثلاث (with a feminine
 * unit).
 *
 * Units: <unit>0..3.wav
 *   0 = singular            1, decimals, exact hundreds/thousands
 *   1 = dual                2
 *   2 = plural              3-10, and 0
 *   3 = accusative singular 11-99
 * For numbers above 100 the last two digits pick the form (103 -> plural).
 *
 * Word order: 1 is "unit + one" (ثانية واحدة), 2 is the dual unit alone
 * (ثانيتان), everything else is "number + unit". Number parts are joined
 * with و (ألف ومئتان وخمسون).
 *
 * NOTE: grammar choices and the unit genders below need review by a native
 * speaker.
 */

enum ArabicPrompts {
  AR_PROMPT_NUMBERS_BASE = 0,
  AR_PROMPT_ZERO = AR_PROMPT_NUMBERS_BASE + 0,            // 0-99 masculine
  AR_PROMPT_HUNDRED = AR_PROMPT_NUMBERS_BASE + 100,       // 100..900
  AR_PROMPT_THOUSAND = AR_PROMPT_NUMBERS_BASE + 109,      // ألف
  AR_PROMPT_THOUSAND_DUAL = AR_PROMPT_NUMBERS_BASE + 110, // ألفان
  AR_PROMPT_THOUSANDS = AR_PROMPT_NUMBERS_BASE + 111,     // آلاف
  AR_PROMPT_THOUSAND_ACC = AR_PROMPT_NUMBERS_BASE + 112,  // ألفًا
  AR_PROMPT_AND = AR_PROMPT_NUMBERS_BASE + 113,
  AR_PROMPT_MINUS = AR_PROMPT_NUMBERS_BASE + 114,
  AR_PROMPT_POINT_BASE = AR_PROMPT_NUMBERS_BASE + 170,    // .0 - .9
  AR_PROMPT_FEMININE_ZERO = AR_PROMPT_NUMBERS_BASE + 200, // 0-99 feminine
};

enum ArabicUnitForms {
  AR_UNIT_SINGULAR = 0,
  AR_UNIT_DUAL = 1,
  AR_UNIT_PLURAL = 2,
  AR_UNIT_ACCUSATIVE = 3,
};

// Gender of the Arabic noun used for each unit recording.
static bool ar_isFeminineUnit(uint8_t unit)
{
  switch (unit) {
    case UNIT_KTS:                // عقدة
    case UNIT_FEET_PER_SECOND:    // قدم في الثانية
    case UNIT_FEET:               // قدم
    case UNIT_CELSIUS:            // درجة مئوية
    case UNIT_FAHRENHEIT:         // درجة فهرنهايت
    case UNIT_RPMS:               // دورة في الدقيقة
    case UNIT_DEGREE:             // درجة
    case UNIT_FLOZ:               // أونصة
    case UNIT_MS:                 // ملي ثانية
    case UNIT_US:                 // ميكرو ثانية
    case UNIT_HOURS:              // ساعة
    case UNIT_MINUTES:            // دقيقة
    case UNIT_SECONDS:            // ثانية
      return true;
    default:
      return false;
  }
}

// Unit form for a whole number of units (number >= 0)
static uint8_t ar_unitForm(int32_t number)
{
  if (number == 0) return AR_UNIT_PLURAL;
  if (number == 2) return AR_UNIT_DUAL;
  int32_t lastTwo = number % 100;
  if (lastTwo >= 3 && lastTwo <= 10) return AR_UNIT_PLURAL;
  if (lastTwo >= 11) return AR_UNIT_ACCUSATIVE;
  return AR_UNIT_SINGULAR;  // 1, exact hundreds/thousands, x01, x02
}

#define AR_PUSH_NUMBER(n, f) ar_pushNumber((n), (f), id, fragmentVolume)

// Plays a whole number >= 0 without unit
I18N_PLAY_FUNCTION(ar, pushNumber, int32_t number, bool feminine)
{
  if (number >= 1000) {
    int32_t thousands = number / 1000;
    if (thousands == 1) {
      PUSH_NUMBER_PROMPT(AR_PROMPT_THOUSAND);
    } else if (thousands == 2) {
      PUSH_NUMBER_PROMPT(AR_PROMPT_THOUSAND_DUAL);
    } else {
      // ألف is masculine
      AR_PUSH_NUMBER(thousands, false);
      int32_t lastTwo = thousands % 100;
      if (lastTwo >= 3 && lastTwo <= 10)
        PUSH_NUMBER_PROMPT(AR_PROMPT_THOUSANDS);
      else if (lastTwo >= 11)
        PUSH_NUMBER_PROMPT(AR_PROMPT_THOUSAND_ACC);
      else
        PUSH_NUMBER_PROMPT(AR_PROMPT_THOUSAND);
    }
    number %= 1000;
    if (number == 0) return;
    PUSH_NUMBER_PROMPT(AR_PROMPT_AND);
  }

  if (number >= 100) {
    PUSH_NUMBER_PROMPT(AR_PROMPT_HUNDRED + (number / 100) - 1);
    number %= 100;
    if (number == 0) return;
    PUSH_NUMBER_PROMPT(AR_PROMPT_AND);
  }

  PUSH_NUMBER_PROMPT(
      (feminine ? AR_PROMPT_FEMININE_ZERO : AR_PROMPT_ZERO) + number);
}

I18N_PLAY_FUNCTION(ar, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(AR_PROMPT_MINUS);
    number = -number;
  }

  int8_t mode = MODE(att);
  if (mode > 0) {
    uint8_t rem2 = 0;
    if (mode == 2) {
      div_t qr2 = div((int)number, 10);
      number = qr2.quot;
      rem2 = qr2.rem;
    }
    div_t qr = div((int)number, 10);
    if (qr.rem || (mode == 2 && rem2)) {
      // Decimal value: "number point digit(s)" followed by singular unit
      AR_PUSH_NUMBER(qr.quot, false);
      PUSH_NUMBER_PROMPT(AR_PROMPT_POINT_BASE + qr.rem);
      if (mode == 2 && rem2) {
        AR_PUSH_NUMBER(rem2, false);
      }
      if (unit) {
        PUSH_UNIT_PROMPT(unit, AR_UNIT_SINGULAR);
      }
      return;
    }
    number = qr.quot;
  }

  if (unit) {
    bool feminine = ar_isFeminineUnit(unit);
    if (number == 1) {
      // ثانية واحدة
      PUSH_UNIT_PROMPT(unit, AR_UNIT_SINGULAR);
      AR_PUSH_NUMBER(1, feminine);
      return;
    }
    if (number == 2) {
      // ثانيتان
      PUSH_UNIT_PROMPT(unit, AR_UNIT_DUAL);
      return;
    }
    AR_PUSH_NUMBER(number, feminine);
    PUSH_UNIT_PROMPT(unit, ar_unitForm(number));
  } else {
    AR_PUSH_NUMBER(number, false);
  }
}

I18N_PLAY_FUNCTION(ar, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(AR_PROMPT_MINUS);
    seconds = -seconds;
  }

  if (IS_PLAY_LONG_TIMER()) {
    seconds += 30;
  }

  int hours, minutes;
  hours = seconds / 3600;
  seconds = seconds % 3600;
  minutes = seconds / 60;
  seconds = seconds % 60;

  bool spoken = false;

  if (hours > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(hours, UNIT_HOURS, 0);
    spoken = true;
  }

  if (minutes > 0) {
    if (spoken) PUSH_NUMBER_PROMPT(AR_PROMPT_AND);
    PLAY_NUMBER(minutes, UNIT_MINUTES, 0);
    spoken = true;
  }

  if (!IS_PLAY_LONG_TIMER() && seconds > 0) {
    if (spoken) PUSH_NUMBER_PROMPT(AR_PROMPT_AND);
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(ar, STR_VOICE_ARABIC);
