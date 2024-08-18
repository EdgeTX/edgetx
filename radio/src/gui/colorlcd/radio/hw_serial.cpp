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
#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

SerialConfigWindow::SerialConfigWindow(Window *parent, FlexGridLayout& grid)
{
  for (uint8_t port_nr = 0; port_nr < MAX_SERIAL_PORTS; port_nr++) {
    auto port = serialGetPort(port_nr);
    if (!port || !port->name) continue;

    auto line = parent->newLine(grid);
    (new StaticText(line, rect_t{}, port->name))->padLeft(PAD_SMALL);

    auto box = new Window(line, rect_t{});
    box->padAll(PAD_TINY);
    box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM);
    lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
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
      new StaticText(box, rect_t{}, STR_AUX_SERIAL_PORT_POWER);
      new ToggleSwitch(
          box, rect_t{}, [=] { return serialGetPower(port_nr); },
          [=](int8_t newValue) {
            serialSetPower(port_nr, (bool)newValue);
            SET_DIRTY();
          });
    }
      
    if (port_nr != SP_VCP) {
        grid.setColSpan(2);
        auto line = parent->newLine(grid);
        line->padLeft(WARN_PADL);
        line->padBottom(PAD_MEDIUM);
        new StaticText(line, rect_t{}, STR_TTL_WARNING, COLOR_THEME_WARNING_INDEX);
        grid.setColSpan(1);
    }
  }
}
