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

#include "edgetx.h"
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
#include "special_functions.h"

PageDef modelMenuItems[] = {
  { ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, QuickMenu::MODEL_SETUP, [](PageDef& pageDef) { return new ModelSetupPage(pageDef); }},
#if defined(HELI)
  { ICON_MODEL_HELI, STR_MENUHELISETUP, QuickMenu::MODEL_HELI, [](PageDef& pageDef) { return new ModelHeliPage(pageDef); }, modelHeliEnabled},
#endif
#if defined(FLIGHT_MODES)
  { ICON_MODEL_FLIGHT_MODES, STR_MENUFLIGHTMODES, QuickMenu::MODEL_FLIGHTMODES, [](PageDef& pageDef) { return new ModelFlightModesPage(pageDef); }, modelFMEnabled},
#endif
  { ICON_MODEL_INPUTS, STR_MENUINPUTS, QuickMenu::MODEL_INPUTS, [](PageDef& pageDef) { return new ModelInputsPage(pageDef); }},
  { ICON_MODEL_MIXER, STR_MIXES, QuickMenu::MODEL_MIXES, [](PageDef& pageDef) { return new ModelMixesPage(pageDef); }},
  { ICON_MODEL_OUTPUTS, STR_MENULIMITS, QuickMenu::MODEL_OUTPUTS, [](PageDef& pageDef) { return new ModelOutputsPage(pageDef); }},
  { ICON_MODEL_CURVES, STR_MENUCURVES, QuickMenu::MODEL_CURVES, [](PageDef& pageDef) { return new ModelCurvesPage(pageDef); }, modelCurvesEnabled},
#if defined(GVARS)
  { ICON_MODEL_GVARS, STR_MENU_GLOBAL_VARS, QuickMenu::MODEL_GVARS, [](PageDef& pageDef) { return new ModelGVarsPage(pageDef); }, modelGVEnabled},
#endif
  { ICON_MODEL_LOGICAL_SWITCHES, STR_MENULOGICALSWITCHES, QuickMenu::MODEL_LS, [](PageDef& pageDef) { return new ModelLogicalSwitchesPage(pageDef); }, modelSFEnabled},
  { ICON_MODEL_SPECIAL_FUNCTIONS, STR_MENUCUSTOMFUNC, QuickMenu::MODEL_SF, [](PageDef& pageDef) { return new SpecialFunctionsPage(pageDef); }, modelSFEnabled},
#if defined(LUA_MODEL_SCRIPTS)
  { ICON_MODEL_LUA_SCRIPTS, STR_MENUCUSTOMSCRIPTS, QuickMenu::MODEL_SCRIPTS, [](PageDef& pageDef) { return new ModelMixerScriptsPage(pageDef); }, modelCustomScriptsEnabled},
#endif
  { ICON_MODEL_TELEMETRY, STR_MENUTELEMETRY, QuickMenu::MODEL_TELEMETRY, [](PageDef& pageDef) { return new ModelTelemetryPage(pageDef); }, modelTelemetryEnabled},
  { EDGETX_ICONS_COUNT }
};

ModelMenu::ModelMenu() : PageGroup(ICON_MODEL, modelMenuItems)
{
}
