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

#if defined(ALL_LANGS)
#include <functional>
#endif

#include "audio.h"

#if !defined(ALL_LANGS)
struct LanguagePack {
  const char* id;
  const char* name;
  void (*playNumber)(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id, int8_t fragmentVolume);
  void (*playDuration)(int seconds, uint8_t flags, uint8_t id, int8_t fragmentVolume);
};
#else
struct LanguagePack {
  const char* id;
  const char* (*name)();
  void (*playNumber)(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id, int8_t fragmentVolume);
  void (*playDuration)(int seconds, uint8_t flags, uint8_t id, int8_t fragmentVolume);
};
#endif

extern const LanguagePack * currentLanguagePack;
extern uint8_t currentLanguagePackIdx;

enum RadioLanguage {
  LANG_CN,
  LANG_CZ,
  LANG_DA,
  LANG_DE,
  LANG_EN,
  LANG_ES,
  LANG_FI,
  LANG_FR,
  LANG_HE,
  LANG_HU,
  LANG_IT,
  LANG_JP,
  LANG_KO,
  LANG_NL,
  LANG_PL,
  LANG_PT,
  LANG_RU,
  LANG_SE,
  LANG_SK,
  LANG_TW,
  LANG_UA,
  LANG_COUNT
};

extern const LanguagePack cnLanguagePack;
extern const LanguagePack czLanguagePack;
extern const LanguagePack daLanguagePack;
extern const LanguagePack deLanguagePack;
extern const LanguagePack enLanguagePack;
extern const LanguagePack esLanguagePack;
extern const LanguagePack fiLanguagePack;
extern const LanguagePack frLanguagePack;
extern const LanguagePack heLanguagePack;
extern const LanguagePack huLanguagePack;
extern const LanguagePack itLanguagePack;
extern const LanguagePack jpLanguagePack;
extern const LanguagePack koLanguagePack;
extern const LanguagePack nlLanguagePack;
extern const LanguagePack plLanguagePack;
extern const LanguagePack ptLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack seLanguagePack;
extern const LanguagePack skLanguagePack;
extern const LanguagePack twLanguagePack;
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
  &fiLanguagePack,
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
  &twLanguagePack,
  &uaLanguagePack,
  NULL
};
#endif

#if !defined(ALL_LANGS)
#define LANGUAGE_PACK_DECLARE(lng, name) \
  const LanguagePack lng##LanguagePack = {#lng, name, lng##_playNumber, lng##_playDuration}
#else
#define LANGUAGE_PACK_DECLARE(lng, name) \
  const char* lng##_nameFunc() { return name; }; \
  const LanguagePack lng##LanguagePack = {#lng, lng##_nameFunc, lng##_playNumber, \
                                          lng##_playDuration}
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

#define I18N_PLAY_FUNCTION(lng, x, ...) void lng ## _ ## x(__VA_ARGS__, uint8_t id, int8_t fragmentVolume = USE_SETTINGS_VOLUME)
