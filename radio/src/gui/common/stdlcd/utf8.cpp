/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx -   https://github.com/opentx/opentx
 *   th9x -     http://code.google.com/p/th9x
 *   er9x -     http://code.google.com/p/er9x
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
/* -*- coding: utf-8 -*- */

#include <stdint.h>
#include "fonts.h"
#include "definitions.h"
#include "translations/translations.h"

#define FR_LUT      L'é', L'è', L'à', L'î', L'ç'
#define FR_SUB_LUT  L'Â', L'A', L'À', L'A', L'Ê', L'E', L'É', L'E', L'È', L'E'
#define DA_LUT      L'å', L'æ', L'ø', L'Å', L'Æ', L'Ø'
#define DE_LUT      L'Ä', L'ä', L'Ö', L'ö', L'Ü', L'ü', L'ß'
#define CZ_LUT      L'á', L'č', L'é', L'ě', L'í', L'ó', L'ř', L'š', L'ú', L'ů', L'ý', L'Á', L'Í', L'Ř', L'Ý', L'ž', L'É', L'ň'
#define CZ_SUB_LUT  L'Ě', L'ě', L'Š', L'š', L'Č', L'č', L'Ž', L'ž', L'Ú', L'ú', L'Ů', L'ů', L'Ó', L'ó', L'Ň', L'ň'
#define ES_LUT      L'Ñ', L'ñ', L'Á', L'á', L'É', L'é', L'Í', L'í', L'Ó', L'ó', L'ú'
#define PT_LUT      L'Á', L'á', L'Â', L'â', L'Ã', L'ã', L'À', L'à', L'Ç', L'ç', L'É', L'é', L'Ê', L'ê', L'Í', L'í', L'Ó', L'ó', L'Ô', L'ô', L'Õ', L'õ', L'Ú', L'ú'
#define FI_LUT      L'å', L'ä', L'ö', L'Å', L'Ä', L'Ö'
#define IT_LUT      L'à', L'ù'
#define PL_LUT      L'ą', L'ć', L'ę', L'ł', L'ń', L'ó', L'ś', L'ż', L'ź', L'Ą', L'Ć', L'Ę', L'Ł', L'Ń', L'Ó', L'Ś', L'Ż', L'Ź'
#define RU_LUT      L'А', L'Б', L'В', L'Г', L'Д', L'Е', L'Ж', L'З', L'И', L'Й', L'К', L'Л', L'М', L'Н', \
                    L'О', L'П', L'Р', L'С', L'Т', L'У', L'Ф', L'Х', L'Ц', L'Ч', L'Ш', L'Щ', L'Ъ', L'Ы', \
                    L'Ь', L'Э', L'Ю', L'Я', L'а', L'б', L'в', L'г', L'д', L'е', L'ж', L'з', L'и', L'й', \
                    L'к', L'л', L'м', L'н', L'о', L'п', L'р', L'с', L'т', L'у', L'ф', L'х', L'ц', L'ч', \
                    L'ш', L'щ', L'ъ', L'ы', L'ь', L'э', L'ю', L'я'
#define UA_LUT      L'Б', L'Г', L'Ґ', L'Д', L'Є', L'Ж', L'З', L'И', L'Ї', L'Й', L'Л', L'П', L'У', L'Ф', \
                    L'Ц', L'Ч', L'Ш', L'Щ', L'Ь', L'Ю', L'Я', L'б', L'в', L'г', L'ґ', L'д', L'є', L'ж', \
                    L'з', L'и', L'ї', L'й', L'к', L'л', L'м', L'н', L'п', L'т', L'ф', L'ц', L'ч', L'ш', \
                    L'щ', L'ь', L'ю', L'я'
#define UA_SUB_LUT  L'А', L'A', L'а', L'a', L'В', L'B', L'С', L'C', L'с', L'c', L'Е', L'E', L'е', L'e', L'Н', L'H', \
                    L'К', L'K', L'М', L'M', L'І', L'I', L'і', L'i', L'О', L'O', L'о', L'o', L'о', L'o', L'Р', L'P', \
                    L'р', L'p', L'Т', L'T', L'Х', L'X', L'х', L'x', L'у', L'y'
#define SE_LUT      L'å', L'ä', L'ö', L'Å', L'Ä', L'Ö'

#if !defined(ALL_LANGS) || defined(BOOT)

#if defined(TRANSLATIONS_FR)
static const uint16_t _utf8_lut[] = { FR_LUT };
#define UTF8_SUBS_LUT
static const uint16_t _utf8_substitution_lut[] = { FR_SUB_LUT };
#elif defined(TRANSLATIONS_DA)
static const uint16_t _utf8_lut[] = { DA_LUT };
#elif defined(TRANSLATIONS_DE)
static const uint16_t _utf8_lut[] = { DE_LUT };
#elif defined(TRANSLATIONS_CZ)
static const uint16_t _utf8_lut[] = { CZ_LUT };
#define UTF8_SUBS_LUT
static const uint16_t _utf8_substitution_lut[] = { CZ_SUB_LUT };
#elif defined(TRANSLATIONS_ES) && !defined(BOOT)
static const uint16_t _utf8_lut[] = { ES_LUT };
#elif defined(TRANSLATIONS_PT)
static const uint16_t _utf8_lut[] = { PT_LUT };
#elif defined(TRANSLATIONS_FI) && !defined(BOOT)
static const uint16_t _utf8_lut[] = { FI_LUT };
#elif defined(TRANSLATIONS_IT)
static const uint16_t _utf8_lut[] = { IT_LUT };
#elif defined(TRANSLATIONS_PL)
static const uint16_t _utf8_lut[] = { PL_LUT };
#elif defined(TRANSLATIONS_RU) && !defined(BOOT)
static const uint16_t _utf8_lut[] = { RU_LUT };
#elif defined(TRANSLATIONS_UA) && !defined(BOOT)
static const uint16_t _utf8_lut[] = { UA_LUT };
#define UTF8_SUBS_LUT
static const uint16_t _utf8_substitution_lut[] = { UA_SUB_LUT };
#elif defined(TRANSLATIONS_SE)
static const uint16_t _utf8_lut[] = { SE_LUT};
#else
  #define NO_UTF8_LUT
#endif

#else

static const uint16_t fr_utf8_lut[] = { FR_LUT, 0 };
static const uint16_t fr_utf8_substitution_lut[] = { FR_SUB_LUT, 0 };
static const uint16_t da_utf8_lut[] = { DA_LUT, 0 };
static const uint16_t de_utf8_lut[] = { DE_LUT, 0 };
static const uint16_t cz_utf8_lut[] = { CZ_LUT, 0 };
static const uint16_t cz_utf8_substitution_lut[] = { CZ_SUB_LUT, 0 };
static const uint16_t es_utf8_lut[] = { ES_LUT, 0 };
static const uint16_t pt_utf8_lut[] = { PT_LUT, 0 };
static const uint16_t fi_utf8_lut[] = { FI_LUT, 0 };
static const uint16_t it_utf8_lut[] = { IT_LUT, 0 };
static const uint16_t pl_utf8_lut[] = { PL_LUT, 0 };
static const uint16_t ru_utf8_lut[] = { RU_LUT, 0 };
static const uint16_t ua_utf8_lut[] = { UA_LUT, 0 };
static const uint16_t ua_utf8_substitution_lut[] = { UA_SUB_LUT, 0 };
static const uint16_t se_utf8_lut[] = { SE_LUT, 0 };

struct LangUtf {
  const uint16_t* lut;
  const uint16_t* substitution_lut;
};

static const LangUtf langUtf[] = {
  { nullptr, nullptr },
  { cz_utf8_lut, cz_utf8_substitution_lut },
  { da_utf8_lut, nullptr },
  { de_utf8_lut, nullptr },
  { nullptr, nullptr },
  { es_utf8_lut, nullptr },
  { fi_utf8_lut, nullptr },
  { fr_utf8_lut, fr_utf8_substitution_lut },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { it_utf8_lut, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { pl_utf8_lut, nullptr },
  { pt_utf8_lut, nullptr },
  { ru_utf8_lut, nullptr },
  { se_utf8_lut, nullptr },
  { nullptr, nullptr },
  { nullptr, nullptr },
  { ua_utf8_lut, ua_utf8_substitution_lut },
};

static const uint16_t* _utf8_lut = nullptr;
static const uint16_t* _utf8_substitution_lut = nullptr;

void setLanguageUTF(int n)
{
  _utf8_lut = langUtf[n].lut;
  _utf8_substitution_lut = langUtf[n].substitution_lut;
}
#define UTF8_SUBS_LUT

#endif

#if !defined(NO_UTF8_LUT)
#if !defined(ALL_LANGS) || defined(BOOT)
#define MAX_TRANSLATED_CHARS    107
static_assert(sizeof(_utf8_lut) / sizeof(_utf8_lut[0]) <= MAX_TRANSLATED_CHARS, "Number of translated chars exceeds the limit");
#endif

static unsigned char lookup_utf8_mapping(wchar_t w)
{
#if !defined(ALL_LANGS) || defined(BOOT)
  for (int i=0; i < DIM(_utf8_lut); i++) {
    if (w == _utf8_lut[i])
      return FONT_LANG_START + (uint8_t)i;
  }
#else
  if (_utf8_lut == nullptr) return w;
  for (int i=0; _utf8_lut[i]; i++) {
    if (w == _utf8_lut[i])
      return FONT_LANG_START + (uint8_t)i;
  }
#endif
  return 0x20; // return 'space' for unknown chars
}
#endif

#if defined(UTF8_SUBS_LUT)
static uint16_t lookup_utf8_substitution(wchar_t w)
{
#if !defined(ALL_LANGS) || defined(BOOT)
  for (int i=0; i < DIM(_utf8_substitution_lut); i+=2) {
    if (w == _utf8_substitution_lut[i])
      return _utf8_substitution_lut[i+1];
  }
#else
  if (!_utf8_substitution_lut) return w;
  for (int i=0; _utf8_substitution_lut[i]; i+=2) {
    if (w == _utf8_substitution_lut[i])
      return _utf8_substitution_lut[i+1];
  }
#endif
  return w;
}
#endif

unsigned char map_utf8_char(const char*& s, uint8_t& len)
{
  uint8_t c = *s;
  if (((c & 0xE0) == 0xC0) || ((c & 0xF0) == 0xE0)) {
    if (!len) return 0;
    wchar_t w = 0;
    if((c & 0xE0) == 0xC0) // two byte sequence
    {
      w = (c & 0x1F) << 6;
      len--; s++; c = *s;
      w |= c & 0x3F;
    } else if ((c & 0xF0) == 0xE0) { // three byte sequence
      w = (c & 0x0F) << 12;
      len--; s++; c = *s;
      if (!len) return 0;
      w |= (c & 0x3F) << 6;
      len--; s++; c = *s;
      w |= c & 0x3F;
    }
    if (w >= FONT_SYMS_START && w < FONT_LANG_START) { // extra chars
      return (unsigned char)w;
    }
    if(w == L'≥')
      return CHAR_BW_GREATEREQUAL;
    if(w == L'°')
      return CHAR_BW_DEGREE;
#if defined(UTF8_SUBS_LUT)
    auto w_map = lookup_utf8_substitution(w);
    if (w_map> FONT_LANG_START)
      w_map = lookup_utf8_mapping(w_map);
    return w_map;
#elif !defined(NO_UTF8_LUT)
    return lookup_utf8_mapping(w);
#else
    return 0x20;
#endif
  }

  return c;
}

