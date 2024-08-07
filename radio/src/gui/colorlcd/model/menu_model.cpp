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

#include "menu_model.h"

#include "menu_radio.h"
#include "menu_screen.h"
#include "model_curves.h"
#include "model_flightmodes.h"
#include "model_gvars.h"
#include "model_heli.h"
#include "model_inputs.h"
#include "model_logical_switches.h"
#include "model_mixer_scripts.h"
#include "model_mixes.h"
#include "model_outputs.h"
#include "model_select.h"
#include "model_setup.h"
#include "model_telemetry.h"
#include "edgetx.h"
#include "special_functions.h"
#include "translations.h"
#include "view_channels.h"

ModelMenu::ModelMenu() : TabsGroup(ICON_MODEL) { build(); }

void ModelMenu::build()
{
  addTab(new ModelSetupPage());
#if defined(HELI)
  addTab(new ModelHeliPage());
#endif
#if defined(FLIGHT_MODES)
  addTab(new ModelFlightModesPage());
#endif
  addTab(new ModelInputsPage());
  addTab(new ModelMixesPage());
  addTab(new ModelOutputsPage());
  addTab(new ModelCurvesPage());
#if defined(GVARS)
  addTab(new ModelGVarsPage());
#endif
  addTab(new ModelLogicalSwitchesPage());
  addTab(new SpecialFunctionsPage());
#if defined(LUA_MODEL_SCRIPTS)
  addTab(new ModelMixerScriptsPage());
#endif
  addTab(new ModelTelemetryPage());

#if defined(PCBNV14) || defined(PCBPL18)
  new TextButton(
      this,
      {LCD_W / 2 + 6, MENU_TITLE_TOP, LCD_W / 2 - 8, MENU_TITLE_HEIGHT},
      STR_OPEN_CHANNEL_MONITORS, [=]() {
        new ChannelsViewMenu(this);
        return 0;
      });
#endif
}

#if defined(HARDWARE_KEYS)
void ModelMenu::onPressSYS()
{
  onCancel();
  new RadioMenu();
}
void ModelMenu::onLongPressSYS()
{
  onCancel();
  // Radio setup
  (new RadioMenu())->setCurrentTab(2);
}
void ModelMenu::onPressMDL() { new ChannelsViewMenu(this); }
void ModelMenu::onLongPressMDL()
{
  onCancel();
  new ModelLabelsWindow();
}
void ModelMenu::onPressTELE()
{
  onCancel();
  new ScreenMenu();
}
void ModelMenu::onLongPressTELE() { new ChannelsViewMenu(this); }
#endif
