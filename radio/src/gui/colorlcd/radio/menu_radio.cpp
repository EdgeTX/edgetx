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

#include "menu_radio.h"

#include "menu_model.h"
#include "menu_screen.h"
#include "model_select.h"
#include "myeeprom.h"
#include "edgetx.h"
#include "radio_calibration.h"
#include "radio_hardware.h"
#include "radio_sdmanager.h"
#include "radio_setup.h"
#include "radio_theme.h"
#include "radio_tools.h"
#include "radio_trainer.h"
#include "radio_version.h"
#include "special_functions.h"
#include "view_channels.h"

RadioMenu::RadioMenu() : TabsGroup(ICON_RADIO) { build(); }

RadioMenu::~RadioMenu() { storageCheck(true); }

void RadioMenu::build()
{
  addTab(new RadioToolsPage());
  addTab(new RadioSdManagerPage());
  addTab(new RadioSetupPage());
  addTab(new ThemeSetupPage(this));
  addTab(new GlobalFunctionsPage());
  addTab(new RadioTrainerPage());
  addTab(new RadioHardwarePage());
  addTab(new RadioVersionPage());
}

#if defined(HARDWARE_KEYS)
void RadioMenu::onPressSYS() { setCurrentTab(0); }
void RadioMenu::onLongPressSYS() { setCurrentTab(2); }
void RadioMenu::onPressMDL()
{
  onCancel();
  new ModelMenu();
}
void RadioMenu::onLongPressMDL()
{
  onCancel();
  new ModelLabelsWindow();
}
void RadioMenu::onPressTELE()
{
  onCancel();
  new ScreenMenu();
}
void RadioMenu::onLongPressTELE()
{
  onCancel();
  new ChannelsViewMenu();
}
#endif
