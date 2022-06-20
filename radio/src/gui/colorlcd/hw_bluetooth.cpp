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
#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class BTDetailsDialog : public Dialog
{
  StaticText* l_addr;
  StaticText* r_addr;

  void setAddr(StaticText* label, const char* addr)
  {
    if (addr[0] == '\0') {
      label->setText("---");
    } else {
      label->setText(addr);
    }
  }
  
public:
  BTDetailsDialog(Window *parent) :
    Dialog(parent, STR_BLUETOOTH, rect_t{})
  {
    setCloseWhenClickOutside(true);

    auto form = &content->form;
    form->setFlexLayout();
    
    FlexGridLayout grid(col_dsc, row_dsc);

    if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY) {
      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_BLUETOOTH_PIN_CODE, 0,
                     COLOR_THEME_PRIMARY1);
      new StaticText(line, rect_t{}, "000000", 0, COLOR_THEME_PRIMARY1);
    }

    // Local MAC
    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_BLUETOOTH_LOCAL_ADDR, 0,
                   COLOR_THEME_PRIMARY1);
    l_addr = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

    // Remote MAC
    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_BLUETOOTH_DIST_ADDR, 0,
                   COLOR_THEME_PRIMARY1);
    r_addr = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

    content->setWidth(LCD_W * 0.8);
    content->updateSize();
  }

  void checkEvents() override
  {
    setAddr(l_addr, bluetooth.localAddr);
    setAddr(r_addr, bluetooth.distantAddr);
  }
};

static void bt_mode_changed(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);
  auto choice = (Choice*)lv_obj_get_user_data(target);
  if (!obj || !choice) return;

  if (choice->getIntValue() != BLUETOOTH_OFF) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }
}

BluetoothConfigWindow::BluetoothConfigWindow(Window *parent) :
    FormGroup(parent, rect_t{})
{
  setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, 2);

  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);

  auto mode = new Choice(box, rect_t{}, STR_BLUETOOTH_MODES, BLUETOOTH_OFF,
                         BLUETOOTH_TRAINER, GET_SET_DEFAULT(g_eeGeneral.bluetoothMode));

  auto btn =
      new TextButton(box, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new BTDetailsDialog(Layer::back());
        return 0;
      });
  btn->padAll(lv_dpx(4));
  btn->setWidth(LV_DPI_DEF / 2);

  lv_obj_add_event_cb(mode->getLvObj(), bt_mode_changed,
                      LV_EVENT_VALUE_CHANGED, btn->getLvObj());
  
  // BT radio name
  line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new RadioTextEdit(line, rect_t{}, g_eeGeneral.bluetoothName, LEN_BLUETOOTH_NAME);

  lv_obj_add_event_cb(mode->getLvObj(), bt_mode_changed,
                      LV_EVENT_VALUE_CHANGED, line->getLvObj());
}
