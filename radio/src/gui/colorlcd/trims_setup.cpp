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

#include "trims_setup.h"
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static void resetTrims()
{
  for (auto &fm : g_model.flightModeData) memclear(&fm.trim, sizeof(fm.trim));
  SET_DIRTY();
  AUDIO_WARNING1();
}

TrimsSetup::TrimsSetup() : Page(ICON_MODEL_SETUP)
{
  new StaticText(&header,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_TRIMS, 0, COLOR_THEME_PRIMARY2);

  body.setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Trim step
  auto line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_TRIMINC, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VTRIMINC, -2, 2,
             GET_SET_DEFAULT(g_model.trimInc));

  // Extended trims
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_ETRIMS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_model.extendedTrims));

  // Reset trims
  line = body.newLine(&grid);
  new TextButton(line, rect_t{}, STR_RESET_BTN, []() -> uint8_t {
    resetTrims();
    return 0;
  });

  // Display trims
  // TODO: move to "Screen setup" ?
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_DISPLAY_TRIMS, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VDISPLAYTRIMS, 0, 2,
             GET_SET_DEFAULT(g_model.displayTrims));
}
