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

#include "hw_serial.h"
#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

SerialConfigWindow::SerialConfigWindow(Window *parent, const rect_t &rect) :
    FormGroup(parent, rect, FORWARD_SCROLL)
{
  setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, 2);

  bool display_ttl_warning = false;
  for (uint8_t port_nr = 0; port_nr < MAX_SERIAL_PORTS; port_nr++) {
    auto port = serialGetPort(port_nr);
    if (!port || !port->name) continue;

    if (port_nr != SP_VCP) display_ttl_warning = true;

    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, port->name, 0, COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
    lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
    
    auto aux = new Choice(
        box, rect_t{}, STR_AUX_SERIAL_MODES, 0, UART_MODE_MAX,
        [=]() { return serialGetMode(port_nr); },
        [=](int value) {
          serialSetMode(port_nr, value);
          serialInit(port_nr, value);
          SET_DIRTY();
        });
    aux->setAvailableHandler(
        [=](int value) { return isSerialModeAvailable(port_nr, value); });

    if (port->set_pwr != nullptr) {
      new StaticText(box, rect_t{}, STR_AUX_SERIAL_PORT_POWER, 0,
                     COLOR_THEME_PRIMARY1);
      new CheckBox(
          box, rect_t{}, [=] { return serialGetPower(port_nr); },
          [=](int8_t newValue) {
            serialSetPower(port_nr, (bool)newValue);
            SET_DIRTY();
          });
    }
  }

  if (display_ttl_warning) {
    grid.setColSpan(2);
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_TTL_WARNING, 0, COLOR_THEME_WARNING);
  }
}
