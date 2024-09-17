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

#include "trainer_bluetooth.h"
#include "edgetx.h"


#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class BTDiscoverMenu : public Menu
{
  uint8_t devCount = 0;
  
  void checkEvents() override;
  void selectAddr(const char* addr);

public:
  BTDiscoverMenu();
};

BTDiscoverMenu::BTDiscoverMenu() :
  Menu()
{
  setTitle(STR_BT_SELECT_DEVICE);
  // TODO: set minimum height

}

void BTDiscoverMenu::checkEvents()
{
  if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_START ||
      bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
    int cnt = min<uint8_t>(reusableBuffer.moduleSetup.bt.devicesCount,
                           MAX_BLUETOOTH_DISTANT_ADDR);

    if (devCount < cnt) {
      for (int i = 0; i < cnt - devCount; i++) {
        int index = devCount + i;
        const char* item = reusableBuffer.moduleSetup.bt.devices[index];
        addLine(item, [=]() { selectAddr(item); });
      }
      devCount = cnt;
    }
  }
}

void BTDiscoverMenu::selectAddr(const char* addr)
{
  strncpy(bluetooth.distantAddr, addr, LEN_BLUETOOTH_ADDR);
  bluetooth.state = BLUETOOTH_STATE_BIND_REQUESTED;
  SET_DIRTY();  
}

BluetoothTrainerWindow::BluetoothTrainerWindow(Window* parent) :
    Window(parent, rect_t{})
{
  setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  auto line = newLine(grid);
  state = new StaticText(line, rect_t{}, "");
  r_addr = new StaticText(line, rect_t{}, "");

  auto lclline = newLine(grid);
  new StaticText(lclline, rect_t{}, STR_BLUETOOTH_LOCAL_ADDR);
  new StaticText(lclline, rect_t{}, bluetooth.localAddr);

  btn_line = newLine(grid);
  grid.nextCell();

  btn = new TextButton(btn_line, rect_t{}, "");
}

void BluetoothTrainerWindow::setMaster(bool master)
{
  btn_line->show(master);
  is_master = master;
}

static const char _empty_addr[] = "---";

void BluetoothTrainerWindow::checkEvents()
{
  if(bluetooth.state != lastbtstate ||
     reusableBuffer.moduleSetup.bt.devicesCount != devcount)
    refresh();
  lastbtstate = bluetooth.state;
  devcount = reusableBuffer.moduleSetup.bt.devicesCount;
  Window::checkEvents();
}

void BluetoothTrainerWindow::refresh()
{
  if (is_master) {
    if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_SENT) {
      btn->setText(STR_BLUETOOTH_SCANNING);
      btn->setPressHandler([=]() {
        startScan(); // Allow restart scan if stuck here
        return 0;});
    } else if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_START) {
      if(reusableBuffer.moduleSetup.bt.devicesCount && !menuopened) { // On first item found, open menu
        auto btdm = new BTDiscoverMenu();
        btdm->setCloseHandler([=]() {
          menuopened = false;
          if(bluetooth.state != BLUETOOTH_STATE_BIND_REQUESTED)
            bluetooth.state = BLUETOOTH_STATE_IDLE;
        });
        menuopened = true;
      }
    } else if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
      if(reusableBuffer.moduleSetup.bt.devicesCount == 0) {
        new MessageDialog(STR_BLUETOOTH, STR_BLUETOOTH_NODEVICES);
        bluetooth.state = BLUETOOTH_STATE_OFF;
      }
    } else if (bluetooth.distantAddr[0]) {
      r_addr->setText(bluetooth.distantAddr);
      btn->setText(STR_CLEAR);
      btn->setPressHandler([]() {
        bluetooth.state = BLUETOOTH_STATE_CLEAR_REQUESTED;
        memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
        return 0;
      });
    } else if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
      r_addr->setText(_empty_addr);
      btn->setText(STR_BLUETOOTH_INIT);
      btn->setPressHandler([]() {
        bluetooth.state = BLUETOOTH_STATE_OFF;
        return 0;
      });
    } else {
      r_addr->setText(_empty_addr);
      btn->setText(STR_DISCOVER);
      btn->setPressHandler([=]() {
        startScan();
        return 0;
      });
    }
  }

  if (bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
    state->setText(STR_CONNECTED);
    if (!is_master) r_addr->setText(bluetooth.distantAddr);
  } else if (bluetooth.state != BLUETOOTH_STATE_DISCOVER_REQUESTED ||
             bluetooth.state != BLUETOOTH_STATE_DISCOVER_SENT ||
             !is_master) {
    state->setText(STR_NOT_CONNECTED);
    if (!is_master) r_addr->setText(_empty_addr);
  }
}

void BluetoothTrainerWindow::startScan()
{
  reusableBuffer.moduleSetup.bt.devicesCount = 0;
  devcount = 0;
  bluetooth.state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
}
