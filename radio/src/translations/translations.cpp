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
#include "translations/translation_def.h"

#if defined(TRANSLATIONS_FR)
#include "translations/i18n/fr.h"
#elif defined(TRANSLATIONS_IT)
#include "translations/i18n/it.h"
#elif defined(TRANSLATIONS_DA)
#include "translations/i18n/da.h"
#elif defined(TRANSLATIONS_SE)
#include "translations/i18n/se.h"
#elif defined(TRANSLATIONS_FI)
#include "translations/i18n/fi.h"
#elif defined(TRANSLATIONS_DE)
#include "translations/i18n/de.h"
#elif defined(TRANSLATIONS_CZ)
#include "translations/i18n/cz.h"
#elif defined(TRANSLATIONS_ES)
#include "translations/i18n/es.h"
#elif defined(TRANSLATIONS_PL)
#include "translations/i18n/pl.h"
#elif defined(TRANSLATIONS_PT)
#include "translations/i18n/pt.h"
#elif defined(TRANSLATIONS_NL)
#include "translations/i18n/nl.h"
#elif defined(TRANSLATIONS_CN)
#include "translations/i18n/cn.h"
#elif defined(TRANSLATIONS_TW)
#include "translations/i18n/tw.h"
#elif defined(TRANSLATIONS_JP)
#include "translations/i18n/jp.h"
#elif defined(TRANSLATIONS_RU)
#include "translations/i18n/ru.h"
#elif defined(TRANSLATIONS_HE)
#include "translations/i18n/he.h"
#elif defined(TRANSLATIONS_KO)
#include "translations/i18n/ko.h"
#elif defined(TRANSLATIONS_UA)
#include "translations/i18n/ua.h"
#else
#include "translations/i18n/en.h"
#endif

const char CHR_HOUR = TR_CHR_HOUR;
const char CHR_INPUT = TR_CHR_INPUT;

#if !defined(ALL_LANGS) || defined(BOOT)

// Static string
#define STR(x) const char STR_##x[] = TR_##x;
// Static string array
#define STRARRAY(x) const char* const STR_##x[] = { TR_##x };

#include "string_list.h"
#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

#else

// Order must match languagePack[]
#if defined(COLORLCD)
const LangStrings* const langStrings[] = {
  &cnLangStrings,
  &czLangStrings,
  &daLangStrings,
  &deLangStrings,
  &enLangStrings,
  &esLangStrings,
  &fiLangStrings,
  &frLangStrings,
  &heLangStrings,
  &enLangStrings,
  &itLangStrings,
  &jpLangStrings,
  &koLangStrings,
  &nlLangStrings,
  &plLangStrings,
  &ptLangStrings,
  &ruLangStrings,
  &seLangStrings,
  &enLangStrings,
  &twLangStrings,
  &uaLangStrings,
};
#else
const LangStrings* const langStrings[] = {
  &enLangStrings,
  &czLangStrings,
  &daLangStrings,
  &deLangStrings,
  &enLangStrings,
  &esLangStrings,
  &fiLangStrings,
  &frLangStrings,
  &enLangStrings,
  &enLangStrings,
  &itLangStrings,
  &enLangStrings,
  &enLangStrings,
  &enLangStrings,
  &plLangStrings,
  &ptLangStrings,
  &ruLangStrings,
  &seLangStrings,
  &enLangStrings,
  &enLangStrings,
  &uaLangStrings,
};
#endif
const LangStrings* currentLangStrings = &enLangStrings;

// Static string
#define STR(x) const char STR_##x[] = TR_##x;
// Static string array
#define STRARRAY(x) const char* const STR_##x[] = { TR_##x };

#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

// Static string
#define STR(x) const char* STR_##x##_FN() { return STR_##x; }
// Static string array
#define STRARRAY(x) const char* const* STR_##x##_FN() { return STR_##x; }

#include "string_list.h"
#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

#endif
