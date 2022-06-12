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

#include "hw_intmodule.h"
#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

InternalModuleWindow::InternalModuleWindow(Window *parent, const rect_t &rect) :
    FormGroup(parent, rect, FORWARD_SCROLL),
    lastModule(g_eeGeneral.internalModule)
{
  setFlexLayout();
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  lv_obj_set_style_pad_left(lvobj, lv_dpx(8), 0);

  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, TR_INTERNAL_MODULE, 0, COLOR_THEME_PRIMARY1);
  auto internalModule = new Choice(
      line, rect_t{}, STR_INTERNAL_MODULE_PROTOCOLS, MODULE_TYPE_NONE,
      MODULE_TYPE_COUNT - 1, GET_DEFAULT(g_eeGeneral.internalModule),
      [=](int type) { return setModuleType(type); });

  internalModule->setAvailableHandler(
      [](int module) { return isInternalModuleSupported(module); });

#if defined(CROSSFIRE)
  line = newLine(&grid);
  br_line = line->getLvObj();

  new StaticText(line, rect_t{}, STR_BAUDRATE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_CRSF_BAUDRATE, 0,
             CROSSFIRE_MAX_INTERNAL_BAUDRATE, getBaudrate, setBaudrate);

  updateBaudrateLine();
#endif
}

void InternalModuleWindow::setModuleType(int moduleType)
{
  if (g_model.moduleData[INTERNAL_MODULE].type != moduleType) {
    memclear(&g_model.moduleData[INTERNAL_MODULE], sizeof(ModuleData));
    storageDirty(EE_MODEL);
  }
  g_eeGeneral.internalModule = moduleType;
  updateBaudrateLine();
  SET_DIRTY();
}

int InternalModuleWindow::getBaudrate()
{
  return CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate);
}

void InternalModuleWindow::setBaudrate(int val)
{
  g_eeGeneral.internalModuleBaudrate = CROSSFIRE_INDEX_TO_STORE(val);
  restartModule(INTERNAL_MODULE);
  SET_DIRTY();
}

void InternalModuleWindow::updateBaudrateLine()
{
#if defined(CROSSFIRE)
  if (isInternalModuleCrossfire()) {
    lv_obj_clear_flag(br_line, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(br_line, LV_OBJ_FLAG_HIDDEN);
  }
#endif
}
