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

#include "model_heli.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

ModelHeliPage::ModelHeliPage():
  PageTab(STR_MENUHELISETUP, ICON_MODEL_HELI)
{
}

void ModelHeliPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  // Swash type
  new StaticText(window, grid.getLabelSlot(), STR_SWASHTYPE);
  new Choice(window, grid.getFieldSlot(), STR_VSWASHTYPE, 0, SWASH_TYPE_MAX, GET_SET_DEFAULT(g_model.swashR.type));
  grid.nextLine();

  // Swash ring
  new StaticText(window, grid.getLabelSlot(), STR_SWASHRING);
  new NumberEdit(window, grid.getFieldSlot(), 0, 100, GET_SET_DEFAULT(g_model.swashR.value));
  grid.nextLine();

  // Elevator source
  new StaticText(window, grid.getLabelSlot(), STR_ELEVATOR);
  new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH, GET_SET_DEFAULT(g_model.swashR.elevatorSource));
  grid.nextLine();

  // Elevator weight
  new StaticText(window, grid.getLabelSlot(), STR_WEIGHT);
  new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(g_model.swashR.elevatorWeight));
  grid.nextLine();

  // Aileron source
  new StaticText(window, grid.getLabelSlot(), STR_AILERON);
  new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH, GET_SET_DEFAULT(g_model.swashR.aileronSource));
  grid.nextLine();

  // Aileron weight
  new StaticText(window, grid.getLabelSlot(), STR_WEIGHT);
  new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(g_model.swashR.aileronWeight));
  grid.nextLine();

  // Collective source
  new StaticText(window, grid.getLabelSlot(), STR_COLLECTIVE);
  new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH, GET_SET_DEFAULT(g_model.swashR.collectiveSource));
  grid.nextLine();

  // Collective weight
  new StaticText(window, grid.getLabelSlot(), STR_WEIGHT);
  new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(g_model.swashR.collectiveWeight));

  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}
