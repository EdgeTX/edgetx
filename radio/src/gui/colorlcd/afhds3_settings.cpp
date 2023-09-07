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

#include "afhds3_settings.h"
#include "afhds3_options.h"
#include "opentx.h"

static const char* _afhds3_region[] = { "CE", "FCC" };

static const char* _afhds3_phy_mode[] = {
  // V0
  "Classic 18ch",
  "C-Fast 10ch",
  // V1
  "Routine 18ch",
  "Fast 8ch",
  "Lora 12ch",
};

#include "pulses/flysky.h"
#include "pulses/afhds3.h"
#include "pulses/afhds3_config.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

AFHDS3Settings::AFHDS3Settings(Window* parent, const FlexGridLayout& g,
                               uint8_t moduleIdx) :
    FormWindow(parent, rect_t{}),
    moduleIdx(moduleIdx),
    md(&g_model.moduleData[moduleIdx]),
    grid(g)
{
  setFlexLayout();

  FormWindow::Line* line;

  // Status
  line = newLine(&grid);
  afhds3StatusLabel = new StaticText(line, rect_t{}, STR_MODULE_STATUS, 0, COLOR_THEME_PRIMARY1);
  afhds3StatusText = new DynamicText(line, rect_t{}, [=] {
                                       char msg[64] = "";
                                       getModuleStatusString(moduleIdx, msg);
                                       return std::string(msg);
                                     });

  // TYPE
  line = newLine(&grid);
  afhds3TypeLabel = new StaticText(line, rect_t{}, STR_TYPE, 0, COLOR_THEME_PRIMARY1);

  afhds3TypeForm = new FormWindow(line, rect_t{});
  afhds3TypeForm->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_grid_cell_x_align(afhds3TypeForm->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  afhds3PhyMode = new Choice(afhds3TypeForm, rect_t{}, _afhds3_phy_mode, 0, afhds3::PHYMODE_MAX,
             GET_SET_DEFAULT(md->afhds3.phyMode));
  lv_obj_set_style_bg_color(afhds3PhyMode->getLvObj(), makeLvColor(COLOR_THEME_DISABLED), LV_PART_MAIN | LV_STATE_DISABLED);

  afhds3Emi = new Choice(afhds3TypeForm, rect_t{}, _afhds3_region,
             afhds3::LNK_ES_CE, afhds3::LNK_ES_FCC,
             GET_SET_DEFAULT(md->afhds3.emi));
  lv_obj_set_style_bg_color(afhds3Emi->getLvObj(), makeLvColor(COLOR_THEME_DISABLED), LV_PART_MAIN | LV_STATE_DISABLED);

  new TextButton(afhds3TypeForm, rect_t{}, STR_MODULE_OPTIONS,
                 [=]() {
                     afhds3::applyModelConfig(moduleIdx);
                     new AFHDS3_Options(moduleIdx);
                     return 0;
                 });

  if (moduleIdx == EXTERNAL_MODULE) {
    line = newLine(&grid);
    auto cfg = afhds3::getConfig(moduleIdx);
    new StaticText(line, rect_t{}, STR_MULTI_RFPOWER);
    afhds3RfPower = new Choice(line, rect_t{}, STR_AFHDS3_POWERS, 0, AFHDS3_FRM303_POWER_MAX,
                             GET_DEFAULT(md->afhds3.rfPower),
                             [=](int32_t newValue) { md->afhds3.rfPower = newValue;
                                                     cfg->others.dirtyFlag |= (uint32_t) 1 << afhds3::DirtyConfig::DC_RX_CMD_TX_PWR;
                                                     SET_DIRTY();});
  }

  hideAFHDS3Options();
}

void AFHDS3Settings::hideAFHDS3Options()
{
  lv_obj_add_flag(afhds3StatusLabel->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(afhds3StatusText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(afhds3TypeLabel->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(afhds3TypeForm->getLvObj(), LV_OBJ_FLAG_HIDDEN);
}

void AFHDS3Settings::showAFHDS3Options()
{
  lv_obj_clear_flag(afhds3StatusLabel->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(afhds3StatusText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(afhds3TypeLabel->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(afhds3TypeForm->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_event_send(afhds3StatusText->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  lv_event_send(afhds3PhyMode->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  lv_event_send(afhds3Emi->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  if (moduleIdx == EXTERNAL_MODULE) {
    lv_event_send(afhds3RfPower->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);  
  }
  if (afhds3::getConfig(moduleIdx)->others.isConnected)
  {
    lv_obj_add_state(afhds3PhyMode->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(afhds3Emi->getLvObj(), LV_STATE_DISABLED);
  }
  else
  {
    lv_obj_clear_state(afhds3PhyMode->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(afhds3Emi->getLvObj(), LV_STATE_DISABLED);
  }
}

void AFHDS3Settings::checkEvents() {
  if (afhds3::getConfig(moduleIdx)->others.lastUpdated > lastRefresh) {
    update();
  }
  FormWindow::checkEvents();
}

void AFHDS3Settings::update()
{
  lastRefresh = get_tmr10ms();

  if (isModuleAFHDS3(moduleIdx)) {
    showAFHDS3Options();
  } else {
    hideAFHDS3Options();
  }
}
