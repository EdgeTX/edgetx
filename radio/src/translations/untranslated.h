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

#if defined(TRANSLATIONS_CZ) || defined(TRANSLATIONS_PL)
  #define MAX_PLURAL2 4
  #define USE_PLURAL2_SPECIAL_CASE 1
#else
  #define MAX_PLURAL2 2
  #define USE_PLURAL2_SPECIAL_CASE 0
#endif

// For this number of minute in the last decimal place singular form is used in plural
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_DA) || defined(TRANSLATIONS_DE) || defined(TRANSLATIONS_ES) || \
    defined(TRANSLATIONS_FI) || defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_JP) || defined(TRANSLATIONS_NL) || \
    defined(TRANSLATIONS_PT) || defined(TRANSLATIONS_SE) || defined(TRANSLATIONS_TW)
  #define USE_SINGULAR_IN_PLURAL 1
#else
  #define USE_SINGULAR_IN_PLURAL 0
#endif

// If the number of minutes is above this value PLURAL2 is used
#if defined(TRANSLATIONS_CZ)
  #define USE_PLURAL2 20
#else
  #define USE_PLURAL2 INT_MAX
#endif

constexpr int g_min_plural2 = 2;
constexpr int g_max_plural2 = MAX_PLURAL2;
constexpr int g_use_singular_in_plural = USE_SINGULAR_IN_PLURAL;
constexpr int g_use_plural2_special_case = USE_PLURAL2_SPECIAL_CASE;
constexpr int g_use_plural2 = USE_PLURAL2;

#define CHAR_BW_GREATEREQUAL '}'
#define CHAR_BW_DEGREE       '@'

#define STR_UPDATE_LIST STR_DELAYDOWN

#define STR_RX          "Rx"

#define CHAR_RIGHT      "\302\200"
#define CHAR_LEFT       "\302\201"
#define CHAR_UP         "\302\202"
#define CHAR_DOWN       "\302\203"

#define CHAR_DELTA      "\302\210"
#define CHAR_STICK      "\302\211"
#define CHAR_POT        "\302\212"
#define CHAR_SLIDER     "\302\213"
#define CHAR_SWITCH     "\302\214"
#define CHAR_TRIM       "\302\215"
#define CHAR_INPUT      "\302\216"
#define CHAR_FUNCTION   "\302\217"
#define CHAR_CYC        "\302\220"
#define CHAR_TRAINER    "\302\221"
#define CHAR_CHANNEL    "\302\222"
#define CHAR_TELEMETRY  "\302\223"
#define CHAR_LUA        "\302\224"
#define CHAR_LS         "\302\225"
#define CHAR_CURVE      "\302\226"

// ACCESS STUFF
#define STR_SBUSIN      "SBUS in"
#define STR_SBUSOUT     "SBUS out"
#define STR_SPORT       "S.PORT"
#define STR_FBUS        "FBUS"
#define STR_SBUS24      "SBUS24"
