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

extern void cn_playNumber(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id, int8_t fragmentVolume);
extern void cn_playDuration(int seconds, uint8_t flags, uint8_t id, int8_t fragmentVolume);

#if !defined(ALL_LANGS)
const LanguagePack twLanguagePack = {"tw", STR_VOICE_TAIWANESE, cn_playNumber, cn_playDuration};
#else
const LanguagePack twLanguagePack = {"tw", []() { return STR_VOICE_TAIWANESE; }, cn_playNumber, cn_playDuration};
#endif
