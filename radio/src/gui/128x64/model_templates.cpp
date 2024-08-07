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

void menuModelTemplates(event_t event)
{
  SIMPLE_MENU(STR_MENUTEMPLATES, menuTabModel, MENU_MODEL_TEMPLATES, 1+TMPL_COUNT);

  uint8_t sub = menuVerticalPosition - 1;

  if (sub < TMPL_COUNT) {
    if (warningResult) {
      warningResult = 0;
      applyTemplate(sub);
      AUDIO_WARNING2();
    }
    if (event==EVT_KEY_BREAK(KEY_ENTER)) {
      POPUP_CONFIRMATION(STR_VTEMPLATES+1 + (sub * LEN2_VTEMPLATES), nullptr);
      s_editMode = 0;
    }
  }

  coord_t y = MENU_HEADER_HEIGHT + 1;
  uint8_t k = 0;
  for (uint8_t i=0; i<LCD_LINES-1 && k<TMPL_COUNT; i++) {
    k = i+menuVerticalOffset;
    lcdDrawNumber(3*FW, y, k, (sub==k ? INVERS : 0)|LEADING0, 2);
    lcdDrawTextAtIndex(4*FW, y, STR_VTEMPLATES, k, (sub==k ? INVERS  : 0));
    y+=FH;
  }
}
