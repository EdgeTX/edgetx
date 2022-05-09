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

#include "trainer_setup.h"
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

#include "form.h"
#include "menu.h"
#include "static.h"
#include "button.h"
#include "numberedit.h"

#if defined(BLUETOOTH)
#include "bluetooth.h"
#endif

class TrChoice;

class TrainerModuleWindow : public FormGroup
{
 public:
  TrainerModuleWindow(FormWindow *parent, const rect_t &rect);

  void checkEvents() override;
  void update();

 protected:
  TrChoice *trainerChoice = nullptr;
  NumberEdit *channelStart = nullptr;
  NumberEdit *channelEnd = nullptr;
  bool trChoiceOpen = false;
#if defined(BLUETOOTH)
  StaticText *btChannelEnd = nullptr;
  StaticText *btDistAddress = nullptr;
  TextButton *btMasterButton = nullptr;
  Menu *btPopUpMenu = nullptr;
  bool btCanceled = false;

 private:
  bool popupopen = false;
  int devicecount = 0;
  uint8_t lastbluetoothstate = BLUETOOTH_STATE_OFF;

  void btDiscoverMenuItemChosen();
  void btDiscoverMenuAddItem(const char *itm);

#endif
};

class TrChoice : public Choice
{
 public:
  TrChoice(FormGroup *parent, const rect_t &rect, const char **values, int vmin,
           int vmax, std::function<int()> getValue,
           std::function<void(int)> setValue = nullptr,
           bool *menuOpen = nullptr) :
      Choice(parent, rect, values, vmin, vmax, getValue, setValue),
      menuOpen(menuOpen)
  {
  }

 protected:
  void openMenu()
  {
    if (menuOpen) *menuOpen = true;
    Choice::openMenu();
  }

 private:
  bool *menuOpen;
};

TrainerModuleWindow::TrainerModuleWindow(FormWindow *parent,
                                         const rect_t &rect) :
    FormGroup(parent, rect, FORWARD_SCROLL | NO_FOCUS | FORM_FORWARD_FOCUS)
{
  update();
}

void TrainerModuleWindow::checkEvents()
{
#if defined(BLUETOOTH)
  if (popupopen) {
    if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_START ||
        bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
      int cnt = min<uint8_t>(reusableBuffer.moduleSetup.bt.devicesCount,
                             MAX_BLUETOOTH_DISTANT_ADDR);
      if (devicecount < cnt) {
        for (int i = 0; i < cnt - devicecount; i++) {
          int index = devicecount + i;
          btDiscoverMenuAddItem(reusableBuffer.moduleSetup.bt.devices[index]);
        }
        devicecount = cnt;
      }
    }
  }
  if (bluetooth.state != lastbluetoothstate) {
    if (!popupopen && !trChoiceOpen) update();
    lastbluetoothstate = bluetooth.state;
  }
#endif
  FormGroup::checkEvents();
}

void TrainerModuleWindow::update()
{
  FormGridLayout grid;
  clear();

  new StaticText(this, grid.getLabelSlot(true), STR_MODE, 0,
                 COLOR_THEME_PRIMARY1);
  trainerChoice = new TrChoice(
      this, grid.getFieldSlot(), STR_VTRAINERMODES, 0, TRAINER_MODE_MAX(),
      GET_DEFAULT(g_model.trainerData.mode),
      [=](int32_t newValue) {
#if defined(BLUETOOTH)
        memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
        bluetooth.state = BLUETOOTH_STATE_OFF;
#endif
        g_model.trainerData.mode = newValue;
        SET_DIRTY();
        update();
        trainerChoice->setFocus(SET_FOCUS_DEFAULT);
        trChoiceOpen = false;
      },
      &trChoiceOpen);
  trainerChoice->setAvailableHandler(isTrainerModeAvailable);

  grid.nextLine();

  if (g_model.isTrainerTraineeEnable()) {
#if defined(BLUETOOTH)
    if (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH) {
      btDistAddress = new StaticText(this, grid.getFieldSlot(true), "---", 0,
                                     COLOR_THEME_PRIMARY1);
      if (bluetooth.state == BLUETOOTH_STATE_CONNECTED)
        new StaticText(this, grid.getLabelSlot(true), STR_CONNECTED, 0,
                       COLOR_THEME_PRIMARY1);
      else
        new StaticText(this, grid.getLabelSlot(true), STR_NOT_CONNECTED, 0,
                       COLOR_THEME_PRIMARY1);

      grid.nextLine();
      btMasterButton =
          new TextButton(this, grid.getFieldSlot(), "", [=]() -> uint8_t {
            if (bluetooth.distantAddr[0]) {
              bluetooth.state = BLUETOOTH_STATE_CLEAR_REQUESTED;
              memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
            } else if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
              bluetooth.state = BLUETOOTH_STATE_OFF;
            } else {
              reusableBuffer.moduleSetup.bt.devicesCount = 0;
              devicecount = 0;
              bluetooth.state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
              btPopUpMenu = new Menu(parent);
              btPopUpMenu->setTitle(STR_BT_SELECT_DEVICE);
              popupopen = true;
              btPopUpMenu->setCloseHandler([=]() { popupopen = false; });
            }
            return 0;
          });

      if (bluetooth.distantAddr[0]) {
        btDistAddress->setText(bluetooth.distantAddr);
        btMasterButton->setText(STR_CLEAR);
      } else if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
        btMasterButton->setText(STR_BLUETOOTH_INIT);
      } else {
        btMasterButton->setText(STR_DISCOVER);
      }

      grid.nextLine();

    } else if (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH) {
      if (bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        new StaticText(this, grid.getLabelSlot(true), STR_CONNECTED, 0,
                       COLOR_THEME_PRIMARY1);
        new StaticText(this, grid.getFieldSlot(), bluetooth.distantAddr, 0,
                       COLOR_THEME_PRIMARY1);
      } else
        new StaticText(this, grid.getLabelSlot(true), STR_NOT_CONNECTED, 0,
                       COLOR_THEME_PRIMARY1);

      grid.nextLine();

      new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE, 0,
                     COLOR_THEME_PRIMARY1);
      channelStart =
          new NumberEdit(this, grid.getFieldSlot(2, 0), 1,
                         MAX_OUTPUT_CHANNELS - BLUETOOTH_TRAINER_CHANNELS + 1,
                         GET_DEFAULT(1 + g_model.trainerData.channelsStart));
      char chend[6];
      snprintf(chend, sizeof(chend), "%s%d", STR_CH,
               g_model.trainerData.channelsStart + BLUETOOTH_TRAINER_CHANNELS);

      btChannelEnd = new StaticText(this, grid.getFieldSlot(2, 1), chend, 0,
                                    COLOR_THEME_PRIMARY1);
      channelStart->setPrefix(STR_CH);
      channelStart->setSetValueHandler([=](int32_t newValue) {
        g_model.trainerData.channelsStart = newValue - 1;
        g_model.trainerData.channelsCount =
            BLUETOOTH_TRAINER_CHANNELS - DEF_TRAINER_CHANNELS;
        char chend[6];
        snprintf(
            chend, sizeof(chend), "%s%d", STR_CH,
            g_model.trainerData.channelsStart + BLUETOOTH_TRAINER_CHANNELS);
        SET_DIRTY();
        btChannelEnd->setText(chend);
      });

      grid.nextLine();
    } else
#endif
        if (g_model.trainerData.mode == TRAINER_MODE_SLAVE) {
#define PPMCH_START_MIN 1
#define PPMCH_START_MAX                                          \
  (MAX_OUTPUT_CHANNELS - g_model.trainerData.channelsCount + 1 - \
   DEF_TRAINER_CHANNELS)
#define PPMCH_END_MIN (g_model.trainerData.channelsStart + MIN_TRAINER_CHANNELS)
#define PPMCH_END_MAX                                                    \
  min<uint8_t>(g_model.trainerData.channelsStart + MAX_TRAINER_CHANNELS, \
               MAX_OUTPUT_CHANNELS)

      new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE, 0,
                     COLOR_THEME_PRIMARY1);
      channelStart = new NumberEdit(
          this, grid.getFieldSlot(2, 0), PPMCH_START_MIN, PPMCH_START_MAX,
          GET_DEFAULT(1 + g_model.trainerData.channelsStart));
      channelEnd = new NumberEdit(
          this, grid.getFieldSlot(2, 1), PPMCH_END_MIN, PPMCH_END_MAX,
          GET_DEFAULT((g_model.trainerData.channelsStart +
                       DEF_TRAINER_CHANNELS +
                       g_model.trainerData.channelsCount)));
      channelStart->setPrefix(STR_CH);
      channelEnd->setPrefix(STR_CH);
      channelStart->setSetValueHandler([=](int32_t newValue) {
        g_model.trainerData.channelsStart = newValue - 1;
        SET_DIRTY();
        channelEnd->setMin(PPMCH_END_MIN);
        channelEnd->setMax(PPMCH_END_MAX);
        channelEnd->invalidate();
      });
      channelEnd->setSetValueHandler([=](int32_t newValue) {
        g_model.trainerData.channelsCount =
            newValue - g_model.trainerData.channelsStart - DEF_TRAINER_CHANNELS;
        SET_DIRTY();
        channelStart->setMax(PPMCH_START_MAX);
      });

      grid.nextLine();

      // PPM frame
      new StaticText(this, grid.getLabelSlot(true), STR_PPMFRAME, 0,
                     COLOR_THEME_PRIMARY1);

      // PPM frame length
      auto edit = new NumberEdit(
          this, grid.getFieldSlot(3, 0), 125, 35 * 5 + 225,
          GET_DEFAULT(g_model.trainerData.frameLength * 5 + 225),
          SET_VALUE(g_model.trainerData.frameLength, (newValue - 225) / 5), 0,
          PREC1);
      edit->setStep(5);
      edit->setSuffix(STR_MS);

      // PPM frame delay
      edit = new NumberEdit(
          this, grid.getFieldSlot(3, 1), 100, 800,
          GET_DEFAULT(g_model.trainerData.delay * 50 + 300),
          SET_VALUE(g_model.trainerData.delay, (newValue - 300) / 50));
      edit->setStep(50);
      edit->setSuffix(STR_US);

      // PPM Polarity
      new Choice(this, grid.getFieldSlot(3, 2), STR_PPM_POL, 0, 1,
                 GET_SET_DEFAULT(g_model.trainerData.pulsePol));
      grid.nextLine();
    }
  }
#if defined(HARDWARE_TOUCH)
  new StaticText(this, grid.getLabelSlot(true));
#endif
  auto par = getParent();
  par->moveWindowsTop(top() + 1, adjustHeight());
  par->adjustInnerHeight();
}

#if defined(BLUETOOTH)
void TrainerModuleWindow::btDiscoverMenuItemChosen()
{
  if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_SENT ||
      bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
    int index = btPopUpMenu->selection();
    if (index >= 0 && index < reusableBuffer.moduleSetup.bt.devicesCount) {
      strncpy(bluetooth.distantAddr,
              reusableBuffer.moduleSetup.bt.devices[index], LEN_BLUETOOTH_ADDR);
      bluetooth.state = BLUETOOTH_STATE_BIND_REQUESTED;
      SET_DIRTY();
    }
  }
}

void TrainerModuleWindow::btDiscoverMenuAddItem(const char *itm)
{
  if (btPopUpMenu != nullptr && !btPopUpMenu->deleted())
    btPopUpMenu->addLine(
        itm, std::bind(&TrainerModuleWindow::btDiscoverMenuItemChosen, this));
}
#endif

TrainerPage::TrainerPage() : Page(ICON_MODEL_SETUP)
{
  new StaticText(&header,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_TRAINER, 0, COLOR_THEME_PRIMARY2);

  new TrainerModuleWindow(&body, {0, 0, width(), 0});
}
