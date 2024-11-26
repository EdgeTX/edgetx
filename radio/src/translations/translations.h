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

// Static string
#define STR(x) extern const char STR_##x[];
// Static string array
#define STRARRAY(x) extern const char* const STR_##x[];

#include "translations/string_list.h"

#if !(defined(AUDIO) && defined(BUZZER))
#define STR_SPEAKER STR_MODE
#define STR_BUZZER  STR_MODE
#endif

#undef STR
#undef STRARRAY
