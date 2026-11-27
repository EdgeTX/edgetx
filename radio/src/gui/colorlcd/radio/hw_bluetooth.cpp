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

#include "choice.h"
#include "dialog.h"
#include "edgetx.h"
#include "getset_helpers.h"
#include "menu.h"
#include "static.h"
#include "textedit.h"

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

#if defined(BLUETOOTH_A2DP)
class BTAudioDiscoverMenu : public Menu
{
 public:
  BTAudioDiscoverMenu() : Menu() { setTitle(STR_BT_SELECT_DEVICE); }

  void checkEvents() override
  {
    for (; devCount < bluetooth.audioDevicesCount; devCount++) {
      const char* addr = bluetooth.audioDevices[devCount];
      addLine(addr, [=]() { bluetooth.audioSelectDevice(addr); });
    }
    Menu::checkEvents();
  }

 protected:
  uint8_t devCount = 0;
};

// Sink selection line: address, scan/init button and clear button
class BTAudioBox : public Window
{
 public:
  BTAudioBox(Window* parent) : Window(parent, rect_t{})
  {
    setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_SMALL);
    lv_obj_set_style_flex_cross_place(getLvObj(), LV_FLEX_ALIGN_START, 0);

    addr = new StaticText(this, rect_t{}, "");
    auto actions = new Window(this, rect_t{});
    actions->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
    scanBtn = new TextButton(actions, rect_t{}, "", []() -> uint8_t {
      if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
        bluetooth.state = BLUETOOTH_STATE_OFF;
      } else if (bluetooth.state >= BLUETOOTH_STATE_IDLE &&
                 bluetooth.state != BLUETOOTH_STATE_DISCOVER_REQUESTED &&
                 bluetooth.state != BLUETOOTH_STATE_DISCOVER_SENT &&
                 bluetooth.state != BLUETOOTH_STATE_DISCOVER_START) {
        bluetooth.audioStartScan();
      }
      return 0;
    });
    clearBtn = new TextButton(actions, rect_t{}, STR_CLEAR, []() -> uint8_t {
      bluetooth.audioForgetDevice();
      return 0;
    });
  }

  void checkEvents() override
  {
    const uint8_t st = bluetooth.state;
    const bool scanning = st == BLUETOOTH_STATE_DISCOVER_REQUESTED ||
                          st == BLUETOOTH_STATE_DISCOVER_SENT ||
                          st == BLUETOOTH_STATE_DISCOVER_START;

    if ((st == BLUETOOTH_STATE_DISCOVER_START || st == BLUETOOTH_STATE_DISCOVER_END) &&
        bluetooth.audioDevicesCount > 0 && !menuOpen) {
      menuOpen = true;
      auto menu = new BTAudioDiscoverMenu();
      menu->onClosing([=]() {
        menuOpen = false;
        if (bluetooth.state != BLUETOOTH_STATE_BIND_REQUESTED)
          bluetooth.state = BLUETOOTH_STATE_IDLE;
      });
    } else if (st == BLUETOOTH_STATE_DISCOVER_END && !menuOpen &&
               bluetooth.audioDevicesCount == 0) {
      new MessageDialog(STR_BLUETOOTH, STR_BLUETOOTH_NODEVICES);
      bluetooth.state = BLUETOOTH_STATE_IDLE;
    }

    std::string a = bluetooth.distantAddr[0] ? bluetooth.distantAddr : "---";
    if (st == BLUETOOTH_STATE_CONNECTED) a += std::string(" ") + STR_CONNECTED;
    if (a != lastAddr) {
      lastAddr = a;
      addr->setText(a);
    }

    const char* label = st < BLUETOOTH_STATE_IDLE ? STR_BLUETOOTH_INIT
                        : scanning                ? STR_BLUETOOTH_SCANNING
                                                  : STR_DISCOVER;
    if (label != lastLabel) {
      lastLabel = label;
      scanBtn->setText(label);
    }

    clearBtn->show(bluetooth.distantAddr[0] != 0);
    Window::checkEvents();
  }

 protected:
  StaticText* addr = nullptr;
  TextButton* scanBtn = nullptr;
  TextButton* clearBtn = nullptr;
  std::string lastAddr;
  const char* lastLabel = nullptr;
  bool menuOpen = false;
};
#endif

BluetoothConfigWindow::BluetoothConfigWindow(Window* parent, FlexGridLayout& grid)
{
  auto line = parent->newLine(grid);
  line->padLeft(PAD_SMALL);
  new StaticText(line, rect_t{}, STR_MODE);

  auto box = new Window(line, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_SMALL);
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);

  new Choice(
      box, rect_t{}, STR_BLUETOOTH_MODES, BLUETOOTH_OFF, BLUETOOTH_MAX,
      GET_DEFAULT(g_eeGeneral.bluetoothMode), [=](int value) {
        g_eeGeneral.bluetoothMode = value;
        settingsBtn->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);
        nameEdit->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);
#if defined(BLUETOOTH_A2DP)
        audioLine->show(g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO);
#endif
      });

  settingsBtn =
      new TextButton(box, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new BTDetailsDialog();
        return 0;
      });
  settingsBtn->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);

  // BT radio name
  nameEdit = parent->newLine(grid);
  nameEdit->padLeft(PAD_SMALL);
  new StaticText(nameEdit, rect_t{}, STR_NAME);

  box = new Window(nameEdit, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_SMALL);
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
  new RadioTextEdit(box, rect_t{}, g_eeGeneral.bluetoothName,
                    LEN_BLUETOOTH_NAME);
  nameEdit->show(g_eeGeneral.bluetoothMode != BLUETOOTH_OFF);

#if defined(BLUETOOTH_A2DP)
  audioLine = parent->newLine(grid);
  audioLine->padLeft(PAD_SMALL);
  new StaticText(audioLine, rect_t{}, STR_BLUETOOTH_DIST_ADDR);
  new BTAudioBox(audioLine);
  audioLine->show(g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO);
#endif
}
