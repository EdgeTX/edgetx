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

#include "function_switches.h"
#include "opentx.h"

#include "strhelpers.h"

#if defined(FUNCTION_SWITCHES)

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc1[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_col_dsc2[] = {LV_GRID_FR(10), LV_GRID_FR(10), LV_GRID_FR(10), LV_GRID_FR(12), LV_GRID_FR(8),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_col_dsc3[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
                                          
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static const char* _fct_sw_start[] = { STR_CHAR_UP, STR_CHAR_DOWN, STR_LAST };

const std::string edgetx_fs_manual_url = "https://edgetx.gitbook.io/edgetx-user-manual/b-and-w-radios/model-select/setup#function-switches";

ModelFunctionSwitches::ModelFunctionSwitches() : Page(ICON_MODEL_SETUP)
{
  header->setTitle(STR_MENU_MODEL_SETUP);
  header->setTitle2(STR_MENU_FSWITCH);

  body->padAll(PAD_SMALL);
  lv_obj_set_scrollbar_mode(body->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

  auto form = new Window(body, rect_t{});
  form->setFlexLayout();
  form->padAll(PAD_ZERO);

  FlexGridLayout grid1(line_col_dsc1, line_row_dsc, PAD_TINY);
  FlexGridLayout grid2(line_col_dsc2, line_row_dsc, PAD_TINY);
  FlexGridLayout grid3(line_col_dsc3, line_row_dsc, PAD_TINY);

  auto line = form->newLine(grid2);
  new StaticText(line, rect_t{}, STR_SWITCHES);
  new StaticText(line, rect_t{}, STR_NAME, FONT(XS));
  new StaticText(line, rect_t{}, STR_SWITCH_TYPE, FONT(XS));
  new StaticText(line, rect_t{}, STR_SWITCH_GROUP, FONT(XS));
  new StaticText(line, rect_t{}, STR_SWITCH_STARTUP, FONT(XS));

  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
    line = form->newLine(grid2);

    std::string s(STR_CHAR_SWITCH);
    s += switchGetName(i+switchGetMaxSwitches());

    (new StaticText(line, rect_t{}, s))->padLeft(8);

    auto nameEdit = new ModelTextEdit(line, rect_t(), g_model.switchNames[i], LEN_SWITCH_NAME);
    nameEdit->setWidth(80);

    new Choice(line, rect_t{}, STR_SWTYPES, SWITCH_NONE, SWITCH_2POS,
               [=]() {
                 return FSWITCH_CONFIG(i);
               },
               [=](int val) {
                 swconfig_t mask = (swconfig_t)0x03 << (2 * i);
                 g_model.functionSwitchConfig = (g_model.functionSwitchConfig & ~mask) | ((swconfig_t(val) & 0x03) << (2 * i));
                 SET_DIRTY();
               });

    new Choice(line, rect_t{}, STR_FUNCTION_SWITCH_GROUPS, 0, 3,
               [=]() {
                 return FSWITCH_GROUP(i);
               },
               [=](int val) {
                 swconfig_t mask = (swconfig_t)0x03 << (2 * i);
                 g_model.functionSwitchGroup = (g_model.functionSwitchGroup & ~mask) | ((swconfig_t(val) & 0x03) << (2 * i));
                 SET_DIRTY();
               });

    new Choice(line, rect_t{}, _fct_sw_start, 0, 2,
               [=]() {
                 return (g_model.functionSwitchStartConfig >> (2 * i)) & 0x03;
               },
               [=](int val) {
                 swconfig_t mask = (swconfig_t)0x03 << (2 * i);
                 g_model.functionSwitchStartConfig = (g_model.functionSwitchStartConfig & ~mask) | ((swconfig_t(val) & 0x03) << (2 * i));
                 SET_DIRTY();
               });
  }

  line = form->newLine(grid1);
  line->padTop(10);

  new StaticText(line, rect_t{}, STR_GROUPS);

  line = form->newLine(grid3);
  line->padLeft(50);
  line->padBottom(10);

  for (uint8_t i = 1; i <= 3; i += 1) {
    auto btn = new TextButton(line, rect_t{}, STR_FUNCTION_SWITCH_GROUPS[i], [=]() -> int8_t {
      swconfig_t mask = (swconfig_t) 0x01 << (2 * NUM_FUNCTIONS_SWITCHES + i);
      g_model.functionSwitchGroup ^= mask;
      SET_DIRTY();
      return IS_FSWITCH_GROUP_ON(i);
    });
    btn->check(IS_FSWITCH_GROUP_ON(i));
  }

  line = form->newLine(grid1);
  line->padBottom(10);

  new StaticText(line, rect_t{}, STR_MORE_INFO);

  line = form->newLine(grid1);
  line->padBottom(10);
  line->padLeft((width() - 150) / 2);

  auto qr = lv_qrcode_create(line->getLvObj(), 150, makeLvColor(COLOR_THEME_SECONDARY1), makeLvColor(COLOR_THEME_SECONDARY3));
  lv_qrcode_update(qr, edgetx_fs_manual_url.c_str(), edgetx_fs_manual_url.length());
}

#endif
