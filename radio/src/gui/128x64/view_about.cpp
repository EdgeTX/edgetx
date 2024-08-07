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
#include "stamp.h"

#define ABOUT_INDENT 4

#if defined(VERSION_TAG)
const char ABOUT_VERSION_1[] = "EdgeTX " "(" VERSION_TAG ")" "\036\"" CODENAME "\"";
#else
const char ABOUT_VERSION_1[] = "EdgeTX " "(" VERSION "-" VERSION_SUFFIX ")";
#endif
const char ABOUT_VERSION_2[] = "Copyright (C) " BUILD_YEAR " EdgeTX";
const char ABOUT_VERSION_3[] = "https://edgetx.org";

void menuAboutView(event_t event)
{
  switch(event)
  {
    case EVT_KEY_BREAK(KEY_EXIT):
    case EVT_KEY_BREAK(KEY_ENTER):
      chainMenu(menuMainView);
      break;
  }

  lcdDrawText(1, 0, STR_ABOUTUS, DBLSIZE|INVERS);

  lcdDrawText(ABOUT_INDENT, 22, ABOUT_VERSION_1, SMLSIZE);
  lcdDrawText(ABOUT_INDENT, 38, ABOUT_VERSION_2, SMLSIZE);
  lcdDrawText(ABOUT_INDENT, 46, ABOUT_VERSION_3, SMLSIZE);
}
