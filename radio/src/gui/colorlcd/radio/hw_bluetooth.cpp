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

#include "hw_bluetooth.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class BTDetailsDialog : public BaseDialog
{
 public:
  BTDetailsDialog() : BaseDialog(STR_BLUETOOTH, true)
  {
    FlexGridLayout grid(col_dsc, row_dsc);

    if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY) {
      auto line = form->newLine(grid);
      new StaticText(line, rect_t{}, STR_BLUETOOTH_PIN_CODE);
      new StaticText(line, rect_t{}, "000000");
    }

    // Local MAC
    auto line = form->newLine(grid);
    new StaticText(line, rect_t{}, STR_BLUETOOTH_LOCAL_ADDR);
    new DynamicText(
        line, rect_t{},
        [=]() {
          return std::string(bluetooth.localAddr[0] ? bluetooth.localAddr
                                                    : "---");
        });

    // Remote MAC
    line = form->newLine(grid);
    new StaticText(line, rect_t{}, STR_BLUETOOTH_DIST_ADDR);
    new DynamicText(
        line, rect_t{},
        [=]() {
          return std::string(bluetooth.distantAddr[0] ? bluetooth.distantAddr
                                                      : "---");
        });
  }
};

BluetoothConfigWindow::BluetoothConfigWindow(Window* parent, FlexGridLayout& grid)
{
  auto line = parent->newLine(grid);
  new StaticText(line, rect_t{}, STR_MODE);

  auto box = new Window(line, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_SMALL);
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);

  new Choice(
      box, rect_t{}, STR_BLUETOOTH_MODES, BLUETOOTH_OFF, BLUETOOTH_TRAINER,
      GET_DEFAULT(g_eeGeneral.bluetoothMode), [=](int value) {
        g_eeGeneral.bluetoothMode = value;
        settingsBtn->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);
        nameEdit->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);
      });

  settingsBtn =
      new TextButton(box, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new BTDetailsDialog();
        return 0;
      });
  settingsBtn->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);

  // BT radio name
  nameEdit = parent->newLine(grid);
  new StaticText(nameEdit, rect_t{}, STR_NAME);

  box = new Window(nameEdit, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_SMALL);
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
  new RadioTextEdit(box, rect_t{}, g_eeGeneral.bluetoothName,
                    LEN_BLUETOOTH_NAME);
  nameEdit->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);
}
