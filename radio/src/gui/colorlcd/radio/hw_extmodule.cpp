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

#include "hw_extmodule.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

ExternalModuleWindow::ExternalModuleWindow(Window *parent, FlexGridLayout& grid)
{
  auto line = parent->newLine(grid);

  new StaticText(line, rect_t{}, STR_SAMPLE_MODE);

  new Choice(line, rect_t{}, STR_SAMPLE_MODES, 0, UART_SAMPLE_MODE_MAX,
             GET_DEFAULT(g_eeGeneral.uartSampleMode), [=](int modeValue) {
               g_eeGeneral.uartSampleMode = modeValue;
               SET_DIRTY();
               restartModule(EXTERNAL_MODULE);
             });
}
