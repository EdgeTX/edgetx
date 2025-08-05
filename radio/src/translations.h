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

#pragma once

#include <limits.h>
#include "edgetx_types.h"
#include "translations/untranslated.h"
#include "audio.h"

#if defined(TRANSLATIONS_FR)
#include "translations/fr.h"
#define LEN_SPECIAL_CHARS 3
#elif defined(TRANSLATIONS_IT)
#include "translations/it.h"
#define LEN_SPECIAL_CHARS 1
#elif defined(TRANSLATIONS_DA)
#include "translations/da.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_SE)
#include "translations/se.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_FI)
#include "translations/fi.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_DE)
#include "translations/de.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_CZ)
#include "translations/cz.h"
#define LEN_SPECIAL_CHARS 17
#elif defined(TRANSLATIONS_ES)
#include "translations/es.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_PL)
#include "translations/pl.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_PT)
#include "translations/pt.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_NL)
#include "translations/nl.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_CN)
#include "translations/cn.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_TW)
#include "translations/tw.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_JP)
#include "translations/jp.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_RU)
#include "translations/ru.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_HE)
#include "translations/he.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_KO)
#include "translations/ko.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_UA)
#include "translations/ua.h"
#define LEN_SPECIAL_CHARS 0
#else
#include "translations/en.h"
#define LEN_SPECIAL_CHARS 0
#endif

#define PSIZE(x) ( sizeof(x) - 1 )

#if NARROW_LAYOUT
  #define TR3(x, y, z) z
  #define TR(x, y) y
#elif LCD_W >= 480
  #define TR3(x, y, z) z
  #define TR(x, y) y
#elif LCD_W >= 212
  #define TR3(x, y, z) y
  #define TR(x, y) y
#else
  #define TR3(x, y, z) x
  #define TR(x, y) x
#endif

#if defined(COLORLCD)
  #define BUTTON(x) x
  #define TRC(x, y) y
#else
  #define BUTTON(x)    "[" x "]"
  #define TRC(x, y) x
#endif

#if defined(SURFACE_RADIO)
  #define TRSA(x, y) x
#else
  #define TRSA(x, y) y
#endif

#if (LCD_W == 212) || defined(COLORLCD)
 #define LCDW_128_LINEBREAK
#else
 #define LCDW_128_LINEBREAK        "\036"
#endif

constexpr int g_max_plural2 = TR_MAX_PLURAL2;
constexpr int g_min_plural2 = TR_MIN_PLURAL2;
constexpr int g_use_singular_in_plural = TR_USE_SINGULAR_IN_PLURAL;
constexpr int g_use_plural2_special_case = TR_USE_PLURAL2_SPECIAL_CASE;
constexpr int g_use_plural2 = TR_USE_PLURAL2;

extern const char CHR_HOUR;
extern const char CHR_INPUT;

#define LEN_MULTIPLIER PSIZE(TR_MULTIPLIER)
#define STR_UPDATE_LIST STR_DELAYDOWN

struct LanguagePack {
  const char * id;
  const char * name;
  void (*playNumber)(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id, int8_t fragmentVolume);
  void (*playDuration)(int seconds, uint8_t flags, uint8_t id, int8_t fragmentVolume);
};

extern const LanguagePack * currentLanguagePack;
extern uint8_t currentLanguagePackIdx;

extern const LanguagePack czLanguagePack;
extern const LanguagePack daLanguagePack;
extern const LanguagePack deLanguagePack;
extern const LanguagePack enLanguagePack;
extern const LanguagePack esLanguagePack;
extern const LanguagePack frLanguagePack;
extern const LanguagePack huLanguagePack;
extern const LanguagePack itLanguagePack;
extern const LanguagePack nlLanguagePack;
extern const LanguagePack plLanguagePack;
extern const LanguagePack ptLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack seLanguagePack;
extern const LanguagePack skLanguagePack;
extern const LanguagePack cnLanguagePack;
extern const LanguagePack jpLanguagePack;
extern const LanguagePack koLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack heLanguagePack;
extern const LanguagePack uaLanguagePack;
extern const LanguagePack * const languagePacks[];

#if defined(LANGUAGE_PACKS_DEFINITION)
const LanguagePack * const languagePacks[] = {
  // alphabetical order
  &cnLanguagePack,
  &czLanguagePack,
  &daLanguagePack,
  &deLanguagePack,
  &enLanguagePack,
  &esLanguagePack,
  &frLanguagePack,
  &heLanguagePack,
  &huLanguagePack,
  &itLanguagePack,
  &jpLanguagePack,
  &koLanguagePack,
  &nlLanguagePack,
  &plLanguagePack,
  &ptLanguagePack,
  &ruLanguagePack,
  &seLanguagePack,
  &skLanguagePack,
  &uaLanguagePack,
  NULL
};
#endif

#if defined(SIMU)
#define LANGUAGE_PACK_DECLARE(lng, name)                                  \
  const LanguagePack lng##LanguagePack = {#lng, name, lng##_##playNumber, \
                                          lng##_##playDuration}
#else
#define LANGUAGE_PACK_DECLARE(lng, name)          \
  extern const LanguagePack lng##LanguagePack = { \
      #lng, name, lng##_##playNumber, lng##_##playDuration}
#endif

#define LANGUAGE_PACK_DECLARE_DEFAULT(lng, name)                \
  LANGUAGE_PACK_DECLARE(lng, name);                             \
  const LanguagePack* currentLanguagePack = &lng##LanguagePack; \
  uint8_t currentLanguagePackIdx

#define PLAY_FUNCTION(x, ...)    void x(__VA_ARGS__, uint8_t id, int8_t fragmentVolume = USE_SETTINGS_VOLUME)

inline PLAY_FUNCTION(playNumber, getvalue_t number, uint8_t unit, uint8_t flags) {
  currentLanguagePack->playNumber(number, unit, flags, id, fragmentVolume);
}

inline PLAY_FUNCTION(playDuration, int seconds, uint8_t flags) {
   currentLanguagePack->playDuration(seconds, flags, id, fragmentVolume);
}

// Static string
#define STR(x) extern const char STR_##x[];
// Static string array
#define STRARRAY(x) extern const char* const STR_##x[];

#include "string_list.h"

STR(SF_SET_SCREEN);
STR(RX);

#if defined(AUDIO) && defined(BUZZER)
STR(SPEAKER);
STR(BUZZER);
#else
#define STR_SPEAKER STR_MODE
#define STR_BUZZER  STR_MODE
#endif

#undef STR
#undef STRARRAY
