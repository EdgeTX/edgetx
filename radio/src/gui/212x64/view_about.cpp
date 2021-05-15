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

const unsigned char about_bmp[]  = {
#include "about.lbm"
};

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
#define ABOUT_INDENT 6

void menuAboutView(event_t event)
{
  static uint8_t screenIndex;
  static uint8_t greyIndex;

  switch(event)
  {
    case EVT_ENTRY:
      screenIndex = 0;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_DOWN):
      screenIndex < ABOUT_PARENTS ? screenIndex++ : screenIndex = ABOUT_OPENTX;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_UP):
      screenIndex > ABOUT_OPENTX ? screenIndex-- : screenIndex = ABOUT_PARENTS;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  lcdDrawBitmap(0, 0, about_bmp);
  lcdDrawText(64, 0, STR_ABOUTUS, DBLSIZE);
  lcdDrawSolidHorizontalLine(ABOUT_X, 18, 120);
  lcdDrawSolidHorizontalLine(ABOUT_X, 19, 130, GREY_DEFAULT);
  LcdFlags att = GREY(max(0, 15-greyIndex/2));

  uint8_t screenDuration = 150;

  switch (screenIndex) {
    case ABOUT_OPENTX:
    case ABOUT_END:
      lcdDrawText(ABOUT_X, 22, STR_ABOUT_OPENTX_1, SMLSIZE);
      lcdDrawText(ABOUT_X, 30, STR_ABOUT_OPENTX_2, SMLSIZE);
      lcdDrawText(ABOUT_X, 38, STR_ABOUT_OPENTX_3, SMLSIZE);
      lcdDrawText(ABOUT_X, 46, STR_ABOUT_OPENTX_4, SMLSIZE);
      lcdDrawText(ABOUT_X, 54, STR_ABOUT_OPENTX_5, SMLSIZE);
      screenDuration = 255;
      break;

    case ABOUT_BERTRAND:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_BERTRAND_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_BERTRAND_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_BERTRAND_3, att|SMLSIZE);
      break;

    case ABOUT_ANDRE:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_ANDRE_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ANDRE_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_ANDRE_3, att|SMLSIZE);
      break;

    case ABOUT_MIKE:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_MIKE_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_MIKE_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_MIKE_3, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_MIKE_4, att|SMLSIZE);
      break;

    case ABOUT_KJELL:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_KJELL_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_KJELL_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_KJELL_3, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_KJELL_4, att|SMLSIZE);
      break;

    case ABOUT_MARTIN:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_MARTIN_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_MARTIN_2, att|SMLSIZE);
      break;

    case ABOUT_ROMOLO:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_ROMOLO_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ROMOLO_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_ROMOLO_3, att|SMLSIZE);
      break;

    case ABOUT_ROB:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_ROB_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ROB_2, att|SMLSIZE);
      break;

    case ABOUT_HARDWARE:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_HARDWARE_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_HARDWARE_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_HARDWARE_3, att|SMLSIZE);
      break;

    case ABOUT_PARENTS:
      lcdDrawText(ABOUT_X, 24, STR_ABOUT_PARENTS_1, att);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_PARENTS_2, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_PARENTS_3, att|SMLSIZE);
      lcdDrawText(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_PARENTS_4, att|SMLSIZE);
      screenDuration = 255;
      break;
  }

  if (++greyIndex == screenDuration) {
    greyIndex = 0;
    if (++screenIndex == ABOUT_COUNT) {
      chainMenu(menuMainView);
    }
  }
}
