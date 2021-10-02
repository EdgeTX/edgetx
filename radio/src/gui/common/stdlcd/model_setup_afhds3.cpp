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


void onBindMenuAfhds3(const char * result)
{
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);

  if (result == STR_AFHDS3_ONE_TO_ONE_TELEMETRY) {
    g_model.moduleData[EXTERNAL_MODULE].afhds3.telemetry = 1;
  }
  else if (result == STR_AFHDS3_ONE_TO_MANY) {
    g_model.moduleData[EXTERNAL_MODULE].afhds3.telemetry = 0;
  }
  else {
    return;
  }

  moduleState[moduleIdx].mode = MODULE_MODE_BIND;
}

void startBindMenuAfhds3(uint8_t moduleIdx)
{
  uint8_t selection = 0;
#if defined(AFHDS3)
  POPUP_MENU_ADD_ITEM(STR_AFHDS3_ONE_TO_ONE_TELEMETRY);
  POPUP_MENU_ADD_ITEM(STR_AFHDS3_ONE_TO_MANY);
#endif
  POPUP_MENU_SELECT_ITEM(selection);
  POPUP_MENU_START(onBindMenuAfhds3);
}
