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

const unsigned char font_5x7[]  = {
#include "font_05x07.lbm"
#include "font_05x07_extra.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_05x07.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_05x07.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_05x07.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_05x07.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_05x07.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_05x07.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_05x07.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_05x07.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_05x07.lbm"
#endif
};

#if !defined(BOOT)
const unsigned char font_5x7_B[]  = {
#include "font_05x07_B_compressed.lbm"
};

const unsigned char font_10x14[]  = {
#include "font_10x14_compressed.lbm"
  #if defined(TRANSLATIONS_DE)
  #include "font_de_10x14.lbm"
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
  #elif defined(TRANSLATIONS_SE)
  #include "font_se_10x14.lbm"
  #endif
};

const unsigned char font_3x5[]  = {
#include "font_03x05.lbm"
};

const unsigned char font_4x6[]  = {
#include "font_04x06.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_04x06.lbm"
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
#elif defined(TRANSLATIONS_SE)
#include "font_se_04x06.lbm"
#endif
};

const unsigned char font_8x10[]  = {
#include "font_08x10.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_08x10.lbm"
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
#elif defined(TRANSLATIONS_SE)
#include "font_se_08x10.lbm"
#endif
};

const unsigned char font_22x38_num[]  = {
#include "font_22x38_num.lbm"
};

const unsigned char font_4x6_extra[]  = {
#include "font_04x06_extra.lbm"
};

const unsigned char font_10x14_extra[]  = {
#include "font_10x14_extra.lbm"
};

#endif // !defined(BOOT)
