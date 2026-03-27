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
#include "translations/tts/tts.h"

#if !defined(ALL_LANGS) || defined(BOOT)

// Static string
#define STR(x) extern const char STR_##x[];
// Static string array
#define STRARRAY(x) extern const char* const STR_##x[];

#include "string_list.h"
#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

#else

bool isTextLangAvail(int lang);

// Static string
#define STR(x) extern const char STR_##x[];
// Static string array
#define STRARRAY(x) extern const char* const STR_##x[];

#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

// Static string
#define STR(x) const char* STR_##x;
// Static string array
#define STRARRAY(x) const char* const* STR_##x;

struct LangStrings {
#include "string_list.h"
};

#undef STR
#undef STRARRAY

extern const LangStrings cnLangStrings;
extern const LangStrings czLangStrings;
extern const LangStrings daLangStrings;
extern const LangStrings deLangStrings;
extern const LangStrings enLangStrings;
extern const LangStrings esLangStrings;
extern const LangStrings fiLangStrings;
extern const LangStrings frLangStrings;
extern const LangStrings heLangStrings;
extern const LangStrings huLangStrings;
extern const LangStrings itLangStrings;
extern const LangStrings koLangStrings;
extern const LangStrings jpLangStrings;
extern const LangStrings nlLangStrings;
extern const LangStrings plLangStrings;
extern const LangStrings ptLangStrings;
extern const LangStrings ruLangStrings;
extern const LangStrings seLangStrings;
extern const LangStrings twLangStrings;
extern const LangStrings uaLangStrings;

extern const LangStrings* const langStrings[];
extern const LangStrings* currentLangStrings;

#include "sim_string_list.h"

#undef STR
#undef STRARRAY

// Static string
#define STR(x) extern const char* STR_##x##_FN();
// Static string array
#define STRARRAY(x) extern const char* const* STR_##x##_FN();

#include "string_list.h"
#include "string_list_notrans.h"

#undef STR
#undef STRARRAY

#endif