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

const MenuHandlerFunc menuTabGeneral[MENU_RADIO_PAGES_COUNT] = {
#if defined(RADIO_TOOLS)
  menuRadioTools,
#endif
  menuRadioSdManager,
  menuRadioSetup,
  menuRadioSpecialFunctions,
  menuRadioTrainer,
  menuRadioHardware,
  menuRadioVersion
};

void menuRadioSpecialFunctions(event_t event)
{
  MENU(STR_MENUSPECIALFUNCS, menuTabGeneral, MENU_RADIO_SPECIAL_FUNCTIONS, MAX_SPECIAL_FUNCTIONS, { NAVIGATION_LINE_BY_LINE|4/*repeated*/ });
  return menuSpecialFunctions(event, g_eeGeneral.customFn, &globalFunctionsContext);
}
