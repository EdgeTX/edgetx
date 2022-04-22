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

#include "model_heli.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

ModelHeliPage::ModelHeliPage():
  PageTab(STR_MENUHELISETUP, ICON_MODEL_HELI)
{
}

void ModelHeliPage::build(FormWindow* form)
{
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);
  form->setFlexLayout();

  // Swash type
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWASHTYPE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VSWASHTYPE, 0, SWASH_TYPE_MAX,
             GET_SET_DEFAULT(g_model.swashR.type));

  // Swash ring
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWASHRING, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, 0, 100, GET_SET_DEFAULT(g_model.swashR.value));

  // Elevator source
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_ELEVATOR, 0, COLOR_THEME_PRIMARY1);
  new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_CH,
                   GET_SET_DEFAULT(g_model.swashR.elevatorSource));

  // Elevator weight
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, -100, 100,
                 GET_SET_DEFAULT(g_model.swashR.elevatorWeight));

  // Aileron source
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_AILERON, 0, COLOR_THEME_PRIMARY1);
  new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_CH,
                   GET_SET_DEFAULT(g_model.swashR.aileronSource));

  // Aileron weight
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, -100, 100,
                 GET_SET_DEFAULT(g_model.swashR.aileronWeight));

  // Collective source
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_COLLECTIVE, 0, COLOR_THEME_PRIMARY1);
  new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_CH,
                   GET_SET_DEFAULT(g_model.swashR.collectiveSource));

  // Collective weight
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, -100, 100,
                 GET_SET_DEFAULT(g_model.swashR.collectiveWeight));


  auto obj = form->getLvObj();
  lv_obj_update_layout(obj);
  lv_coord_t h =
    lv_obj_get_height(obj)
    + lv_obj_get_scroll_top(obj)
    + lv_obj_get_scroll_bottom(obj);

  form->setInnerHeight(h);
}
