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

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

ExternalModuleWindow::ExternalModuleWindow(Window *parent) :
    FormGroup::Line(parent), lastModule(g_eeGeneral.internalModule)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  setLayout(&grid);

  new StaticText(this, rect_t{}, STR_SAMPLE_MODE, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(this, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  new Choice(box, rect_t{}, STR_SAMPLE_MODES, 0, UART_SAMPLE_MODE_MAX,
             getSampleMode, setSampleMode);
}

int ExternalModuleWindow::getSampleMode() { return g_eeGeneral.uartSampleMode; }

void ExternalModuleWindow::setSampleMode(int modeValue)
{
  g_eeGeneral.uartSampleMode = modeValue;
  SET_DIRTY();
  restartModule(EXTERNAL_MODULE);
}
