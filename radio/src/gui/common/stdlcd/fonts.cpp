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

#include "fonts.h"
#include "debug.h"

/*
  Strings in EdgeTX are encoded using UTF8. Latin characters are single byte in the range 32 - 127.
  Special characters for language translations and special symbols are encoded as 2-byte or 3-byte
  sequences.
  The UTF8 encoded characters are mapped to single byte characters before display.
  The single byte characters are mapped to the various font sizes as per the table below:

  Char        3x5         4x6         5x7         5x7 BOLD      8x10        10x14       22x38
  Range       TINSIZE     SMLSIZE     Standard    BOLD          MDLSIZE     DBLSIZE     XXLSIZE
  -----       -------     -------     --------    --------      -------     -------     -------
  0 - 31      not used    not used    not used    not used      not used    not used    not used
  31 - 127    Latin(1)    Latin       Latin       Latin(2)      Latin       Latin(2)    Latin(3)
  128 - 148   not used    extra(4)    extra(5)    extra(6)      not used    extra(4)    not used
  149 - 255   not used    Language    Language    Language(7)   Language    Language    not used

  Notes:
  (1) - reduced character set, contains only space, upper case letters, numbers, '-' and '.'.
  (2) - reduced character set, contains only space, alphanumeric, ',', '.', '-', ':' and '_'.
  (3) - reduced character set, contains only space, numbers, ',', '.', '-', ':' and '_'.
        Note: to display ' ' use '+', to display '_' use '/'.
  (4) - arrows
  (5) - arrows and symbols (e.g. delta, stick, switch, etc)
  (6) - uses the 5x7 arrow and symbol font characters (not bold)
  (7) - uses the 5x7 Language font characters (not bold)
*/

#if !defined(BOOT)

const uint8_t font_3x5[] = {
#include "font_03x05.lbm"
};

const uint8_t font_5x7_B[] = {
#include "font_05x07_B_compressed.lbm"
};

const uint8_t font_22x38_num[] = {
#include "font_22x38_num.lbm"
};

#endif

#if !defined(ALL_LANGS) || defined(BOOT)

const uint8_t font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_05x07.lbm"
#elif defined(TRANSLATIONS_DA)
#include "font_da_05x07.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_05x07.lbm"
#elif defined(TRANSLATIONS_ES) && !defined(BOOT)
#include "font_es_05x07.lbm"
#elif defined(TRANSLATIONS_FI) && !defined(BOOT)
#include "font_fi_05x07.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_05x07.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_05x07.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_05x07.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_05x07.lbm"
#elif defined(TRANSLATIONS_RU) && !defined(BOOT)
#include "font_ru_05x07.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_05x07.lbm"
#elif defined(TRANSLATIONS_UA) && !defined(BOOT)
#include "font_ua_05x07.lbm"
#endif
};

#if !defined(BOOT)

const uint8_t font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_10x14.lbm"
#elif defined(TRANSLATIONS_DA)
#include "font_da_10x14.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_10x14.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_10x14.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_10x14.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_10x14.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_10x14.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_10x14.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_10x14.lbm"
#elif defined(TRANSLATIONS_RU)
#include "font_ru_10x14.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_10x14.lbm"
#elif defined(TRANSLATIONS_UA)
#include "font_ua_10x14.lbm"
#endif
};

const uint8_t font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_04x06.lbm"
#elif defined(TRANSLATIONS_DA)
#include "font_da_04x06.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_04x06.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_04x06.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_04x06.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_04x06.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_04x06.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_04x06.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_04x06.lbm"
#elif defined(TRANSLATIONS_RU)
#include "font_ru_04x06.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_04x06.lbm"
#elif defined(TRANSLATIONS_UA)
#include "font_ua_04x06.lbm"
#endif
};

const uint8_t font_8x10[] = {
#include "font_08x10.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_08x10.lbm"
#elif defined(TRANSLATIONS_DA)
#include "font_da_08x10.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_08x10.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_08x10.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_08x10.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_08x10.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_08x10.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_08x10.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_08x10.lbm"
#elif defined(TRANSLATIONS_RU)
#include "font_ru_08x10.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_08x10.lbm"
#elif defined(TRANSLATIONS_UA)
#include "font_ua_08x10.lbm"
#endif
};

#endif // !defined(BOOT)

#else

const uint8_t en_font_4x6[]  = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
};
const uint8_t en_font_5x7[]  = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
};
const uint8_t en_font_8x10[]  = {
#include "font_08x10.lbm"
};
const uint8_t en_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
};

const uint8_t cz_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_cz_04x06.lbm"
};
const uint8_t cz_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_cz_05x07.lbm"
};
const uint8_t cz_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_cz_08x10.lbm"
};
const uint8_t cz_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_cz_10x14.lbm"
};

const uint8_t da_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_da_04x06.lbm"
};
const uint8_t da_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_da_05x07.lbm"
};
const uint8_t da_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_da_08x10.lbm"
};
const uint8_t da_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_da_10x14.lbm"
};

const uint8_t de_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_de_04x06.lbm"
};
const uint8_t de_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_de_05x07.lbm"
};
const uint8_t de_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_de_08x10.lbm"
};
const uint8_t de_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_de_10x14.lbm"
};

const uint8_t es_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_es_04x06.lbm"
};
const uint8_t es_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_es_05x07.lbm"
};
const uint8_t es_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_es_08x10.lbm"
};
const uint8_t es_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_es_10x14.lbm"
};

const uint8_t fi_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_fi_04x06.lbm"
};
const uint8_t fi_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_fi_05x07.lbm"
};
const uint8_t fi_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_fi_08x10.lbm"
};
const uint8_t fi_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_fi_10x14.lbm"
};

const uint8_t fr_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_fr_04x06.lbm"
};
const uint8_t fr_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_fr_05x07.lbm"
};
const uint8_t fr_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_fr_08x10.lbm"
};
const uint8_t fr_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_fr_10x14.lbm"
};

const uint8_t it_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_it_04x06.lbm"
};
const uint8_t it_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_it_05x07.lbm"
};
const uint8_t it_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_it_08x10.lbm"
};
const uint8_t it_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_it_10x14.lbm"
};

const uint8_t pl_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_pl_04x06.lbm"
};
const uint8_t pl_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_pl_05x07.lbm"
};
const uint8_t pl_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_pl_08x10.lbm"
};
const uint8_t pl_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_pl_10x14.lbm"
};

const uint8_t pt_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_pt_04x06.lbm"
};
const uint8_t pt_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_pt_05x07.lbm"
};
const uint8_t pt_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_pt_08x10.lbm"
};
const uint8_t pt_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_pt_10x14.lbm"
};

const uint8_t ru_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_ru_04x06.lbm"
};
const uint8_t ru_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_ru_05x07.lbm"
};
const uint8_t ru_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_ru_08x10.lbm"
};
const uint8_t ru_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_ru_10x14.lbm"
};

const uint8_t se_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_se_04x06.lbm"
};
const uint8_t se_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_se_05x07.lbm"
};
const uint8_t se_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_se_08x10.lbm"
};
const uint8_t se_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_se_10x14.lbm"
};

const uint8_t ua_font_4x6[] = {
#include "font_04x06.lbm"
#include "font_04x06_extra.lbm"
#include "font_ua_04x06.lbm"
};
const uint8_t ua_font_5x7[] = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#include "font_ua_05x07.lbm"
};
const uint8_t ua_font_8x10[] = {
#include "font_08x10.lbm"
#include "font_ua_08x10.lbm"
};
const uint8_t ua_font_10x14[] = {
#include "font_10x14_compressed.lbm"
#include "font_10x14_extra.lbm"
#include "font_ua_10x14.lbm"
};

const uint8_t* const font4x6table[] = {
  en_font_4x6,
  cz_font_4x6,
  da_font_4x6,
  de_font_4x6,
  en_font_4x6,
  es_font_4x6,
  fi_font_4x6,
  fr_font_4x6,
  en_font_4x6,
  en_font_4x6,
  it_font_4x6,
  en_font_4x6,
  en_font_4x6,
  en_font_4x6,
  pl_font_4x6,
  pt_font_4x6,
  ru_font_4x6,
  se_font_4x6,
  en_font_4x6,
  en_font_4x6,
  ua_font_4x6,
};

const uint8_t* const font5x7table[] = {
  en_font_5x7,
  cz_font_5x7,
  da_font_5x7,
  de_font_5x7,
  en_font_5x7,
  es_font_5x7,
  fi_font_5x7,
  fr_font_5x7,
  en_font_5x7,
  en_font_5x7,
  it_font_5x7,
  en_font_5x7,
  en_font_5x7,
  en_font_5x7,
  pl_font_5x7,
  pt_font_5x7,
  ru_font_5x7,
  se_font_5x7,
  en_font_5x7,
  en_font_5x7,
  ua_font_5x7,
};

const uint8_t* const font8x10table[] = {
  en_font_8x10,
  cz_font_8x10,
  da_font_8x10,
  de_font_8x10,
  en_font_8x10,
  es_font_8x10,
  fi_font_8x10,
  fr_font_8x10,
  en_font_8x10,
  en_font_8x10,
  it_font_8x10,
  en_font_8x10,
  en_font_8x10,
  en_font_8x10,
  pl_font_8x10,
  pt_font_8x10,
  ru_font_8x10,
  se_font_8x10,
  en_font_8x10,
  en_font_8x10,
  ua_font_8x10,
};

const uint8_t* const font10x14table[] = {
  en_font_10x14,
  cz_font_10x14,
  da_font_10x14,
  de_font_10x14,
  en_font_10x14,
  es_font_10x14,
  fi_font_10x14,
  fr_font_10x14,
  en_font_10x14,
  en_font_10x14,
  it_font_10x14,
  en_font_10x14,
  en_font_10x14,
  en_font_10x14,
  pl_font_10x14,
  pt_font_10x14,
  ru_font_10x14,
  se_font_10x14,
  en_font_10x14,
  en_font_10x14,
  ua_font_10x14,
};

const uint8_t* font_4x6 = en_font_4x6;
const uint8_t* font_5x7 = en_font_5x7;
const uint8_t* font_8x10 = en_font_8x10;
const uint8_t* font_10x14 = en_font_10x14;

void setLanguageFont(int n)
{
  font_4x6 = font4x6table[n];
  font_5x7 = font5x7table[n];
  font_8x10 = font8x10table[n];
  font_10x14 = font10x14table[n];
  extern void setLanguageUTF(int n);
  setLanguageUTF(n);
}

#endif
