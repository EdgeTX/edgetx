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

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

InternalModuleWindow::InternalModuleWindow(Window *parent) :
    FormGroup::Line(parent),
    lastModule(g_eeGeneral.internalModule)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  setLayout(&grid);

  new StaticText(this, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(this, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  auto internalModule = new Choice(
      box, rect_t{}, STR_INTERNAL_MODULE_PROTOCOLS, MODULE_TYPE_NONE,
      MODULE_TYPE_COUNT - 1, GET_DEFAULT(g_eeGeneral.internalModule),
      [=](int type) { return setModuleType(type); });

  internalModule->setAvailableHandler(
      [](int module) { return isInternalModuleSupported(module); });

#if defined(CROSSFIRE)
  box = new FormGroup(box, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));

  br_box = box->getLvObj();
  lv_obj_set_width(br_box, LV_SIZE_CONTENT); 
  lv_obj_set_style_flex_cross_place(br_box, LV_FLEX_ALIGN_CENTER, 0);

  new StaticText(box, rect_t{}, STR_BAUDRATE, 0, COLOR_THEME_PRIMARY1);
  new Choice(box, rect_t{}, STR_CRSF_BAUDRATE, 0,
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

#if defined(CROSSFIRE)
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
#endif

void InternalModuleWindow::updateBaudrateLine()
{
#if defined(CROSSFIRE)
  if (isInternalModuleCrossfire()) {
    lv_obj_clear_flag(br_box, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
  }
#endif
}
