/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

enum AboutScreens {
  ABOUT_OPENTX,
  ABOUT_HARDWARE,
  ABOUT_BERTRAND,
  ABOUT_ANDRE,
  ABOUT_MIKE,
  ABOUT_KJELL,
  ABOUT_MARTIN,
  ABOUT_ROMOLO,
  ABOUT_ROB,
  ABOUT_PARENTS,
  ABOUT_END,
  ABOUT_COUNT,
};

#define ABOUT_X      62
#define ABOUT_INDENT 16

bool menuAboutView(event_t event)
{
  static uint8_t screenIndex = 0;
  static uint8_t greyIndex = 0;

  switch (event) {
    case EVT_ENTRY:
      screenIndex = 0;
      greyIndex = 0;
      break;
#if defined(PCBX12S)
    case EVT_KEY_FIRST(KEY_PGDN):
#elif defined(PCBX10)
    case EVT_KEY_BREAK(KEY_PGDN):
#endif
      screenIndex < ABOUT_PARENTS ? screenIndex++ : screenIndex = ABOUT_OPENTX;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_PGUP):
#if defined(PCBX10)
    case EVT_KEY_LONG(KEY_PGDN):
#endif
      screenIndex > ABOUT_OPENTX ? screenIndex-- : screenIndex = ABOUT_PARENTS;
      greyIndex = 0;
      killEvents(event);
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  theme->drawBackground();
  theme->drawTopbarBackground(0);

  uint8_t screenDuration = 150;

  switch (screenIndex) {
    case ABOUT_OPENTX:
    case ABOUT_END:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_OPENTX_1);
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP + FH, STR_ABOUT_OPENTX_2);
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_OPENTX_3);
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP + 3*FH, STR_ABOUT_OPENTX_4);
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP + 4*FH, STR_ABOUT_OPENTX_5);
      screenDuration = 255;
      break;

    case ABOUT_BERTRAND:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_BERTRAND_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_BERTRAND_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_BERTRAND_3);
      break;

    case ABOUT_ANDRE:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_ANDRE_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_ANDRE_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_ANDRE_3);
      break;

    case ABOUT_MIKE:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_MIKE_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_MIKE_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_MIKE_3);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 3*FH, STR_ABOUT_MIKE_4);
      break;

    case ABOUT_KJELL:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_KJELL_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_KJELL_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_KJELL_3);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 3*FH, STR_ABOUT_KJELL_4);
      break;

    case ABOUT_MARTIN:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_MARTIN_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_MARTIN_2);
      break;

    case ABOUT_ROMOLO:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_ROMOLO_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_ROMOLO_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_ROMOLO_3);
      break;

    case ABOUT_ROB:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_ROB_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_ROB_2);
      break;

    case ABOUT_HARDWARE:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_HARDWARE_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_HARDWARE_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_HARDWARE_3);
      break;

    case ABOUT_PARENTS:
      lcdDrawText(ABOUT_X, MENU_CONTENT_TOP, STR_ABOUT_PARENTS_1, INVERS);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + FH, STR_ABOUT_PARENTS_2);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 2*FH, STR_ABOUT_PARENTS_3);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, MENU_CONTENT_TOP + 3*FH, STR_ABOUT_PARENTS_4);
      screenDuration = 255;
      break;
  }

  if (++greyIndex == screenDuration) {
    greyIndex = 0;
    if (++screenIndex == ABOUT_COUNT) {
      chainMenu(menuMainView);
    }
  }

  return true;
}
