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
#include "edgetx.h"

static const char* const _afhds3_region[] = {"CE", "FCC"};

static const char* const _afhds3_phy_mode[] = {
    // V0
    "Classic 18ch",
    "C-Fast 10ch",
    // V1
    "Routine 18ch",
    "Fast 8ch",
    "Lora 12ch",
};

#include "pulses/afhds3.h"
#include "pulses/afhds3_config.h"
#include "pulses/flysky.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

AFHDS3Settings::AFHDS3Settings(Window* parent, const FlexGridLayout& g,
                               uint8_t moduleIdx) :
    Window(parent, rect_t{}),
    moduleIdx(moduleIdx),
    md(&g_model.moduleData[moduleIdx]),
    grid(g)
{
  setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  FormLine* line;

  // Status
  line = newLine(grid);
  afhds3StatusLabel = new StaticText(line, rect_t{}, STR_MODULE_STATUS);
  afhds3StatusText = new DynamicText(line, rect_t{}, [=] {
    char msg[64] = "";
    getModuleStatusString(moduleIdx, msg);
    return std::string(msg);
  });

  // TYPE
  line = newLine(grid);
  afhds3TypeLabel =
      new StaticText(line, rect_t{}, STR_TYPE);

  afhds3TypeForm = new Window(line, rect_t{});
  afhds3TypeForm->padAll(PAD_TINY);
  afhds3TypeForm->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_TINY);
  lv_obj_set_style_grid_cell_x_align(afhds3TypeForm->getLvObj(),
                                     LV_GRID_ALIGN_STRETCH, 0);

  afhds3PhyMode =
      new Choice(afhds3TypeForm, rect_t{}, _afhds3_phy_mode, 0,
                 afhds3::PHYMODE_MAX, GET_SET_DEFAULT(md->afhds3.phyMode));

  afhds3Emi =
      new Choice(afhds3TypeForm, rect_t{}, _afhds3_region, afhds3::LNK_ES_CE,
                 afhds3::LNK_ES_FCC, GET_SET_DEFAULT(md->afhds3.emi));

  new TextButton(afhds3TypeForm, rect_t{}, STR_MODULE_OPTIONS, [=]() {
    afhds3::applyModelConfig(moduleIdx);
    new AFHDS3_Options(moduleIdx);
    return 0;
  });

  if (moduleIdx == EXTERNAL_MODULE) {
    line = newLine(grid);
    auto cfg = afhds3::getConfig(moduleIdx);
    new StaticText(line, rect_t{}, STR_MULTI_RFPOWER);
    afhds3RfPower = new Choice(
        line, rect_t{}, STR_AFHDS3_POWERS, 0, AFHDS3_FRM303_POWER_MAX,
        GET_DEFAULT(md->afhds3.rfPower), [=](int32_t newValue) {
          md->afhds3.rfPower = newValue;
          cfg->others.dirtyFlag |= (uint32_t)1
                                   << afhds3::DirtyConfig::DC_RX_CMD_TX_PWR;
          SET_DIRTY();
        });
  }

  hideAFHDS3Options();
}

void AFHDS3Settings::hideAFHDS3Options()
{
  afhds3StatusLabel->hide();
  afhds3StatusText->hide();
  afhds3TypeLabel->hide();
  afhds3TypeForm->hide();
}

void AFHDS3Settings::showAFHDS3Options()
{
  afhds3StatusLabel->show();
  afhds3StatusText->show();
  afhds3TypeLabel->show();
  afhds3TypeForm->show();
  afhds3PhyMode->update();
  afhds3Emi->update();
  if (moduleIdx == EXTERNAL_MODULE) {
    afhds3RfPower->update();
  }
  if (afhds3::getConfig(moduleIdx)->others.isConnected) {
    afhds3PhyMode->disable();
    afhds3Emi->disable();
  } else {
    afhds3PhyMode->enable();
    afhds3Emi->enable();
  }
}

void AFHDS3Settings::checkEvents()
{
  if (afhds3::getConfig(moduleIdx)->others.lastUpdated > lastRefresh) {
    update();
  }
  Window::checkEvents();
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
