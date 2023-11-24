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
  header.setTitle(STR_MENU_MODEL_SETUP);
  header.setTitle2(STR_TRIMS);

  body.setFlexLayout();
  body.padAll(8);
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 4);

  // Reset trims
  auto line = body.newLine();
  line->padBottom(4);
  auto btn = new TextButton(line, rect_t{}, STR_RESET_BTN, []() -> uint8_t {
    resetTrims();
    return 0;
  });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));

#if defined(USE_HATS_AS_KEYS)
  // Hats mode for NV14/EL18
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_HATSMODE, 0, COLOR_THEME_PRIMARY1);
  auto box = new FormWindow(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 4);
  new Choice(box, rect_t{}, STR_HATSOPT, HATSMODE_TRIMS_ONLY, HATSMODE_GLOBAL, 
             GET_SET_DEFAULT(g_model.hatsMode));
  new TextButton(box, rect_t{}, "?", [=]() {
    new HelpDialog(this, {50, 100, LCD_W - 100, LCD_H - 200},
                   STR_HATSMODE_KEYS, STR_HATSMODE_KEYS_HELP, LEFT);
    return 0;
  });
#endif

  // Trim step
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_TRIMINC, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VTRIMINC, -2, 2,
             GET_SET_DEFAULT(g_model.trimInc));

  // Extended trims
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_ETRIMS, 0, COLOR_THEME_PRIMARY1);
  new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_model.extendedTrims));

  // Display trims
  // TODO: move to "Screen setup" ?
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_DISPLAY_TRIMS, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VDISPLAYTRIMS, 0, 2,
             GET_SET_DEFAULT(g_model.displayTrims));
}
