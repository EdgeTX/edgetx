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
#include "view_text.h"

PageDef modelMenuItems[] = {
  { ICON_MODEL_SETUP, STR_DEF(STR_QM_GEN_SETTINGS), STR_DEF(STR_MAIN_MENU_SETTINGS), PAGE_CREATE, QM_MODEL_SETUP, [](PageDef& pageDef) { return new ModelSetupPage(pageDef); }},
#if defined(FLIGHT_MODES)
  { ICON_MODEL_FLIGHT_MODES, STR_DEF(STR_QM_FLIGHT_MODES), STR_DEF(STR_MENUFLIGHTMODES), PAGE_CREATE, QM_MODEL_FLIGHTMODES, [](PageDef& pageDef) { return new ModelFlightModesPage(pageDef); }, modelFMEnabled},
#endif
  { ICON_MODEL_INPUTS, STR_DEF(STR_QM_INPUTS), STR_DEF(STR_MENUINPUTS), PAGE_CREATE, QM_MODEL_INPUTS, [](PageDef& pageDef) { return new ModelInputsPage(pageDef); }},
  { ICON_MODEL_MIXER, STR_DEF(STR_QM_MIXES), STR_DEF(STR_MIXES), PAGE_CREATE, QM_MODEL_MIXES, [](PageDef& pageDef) { return new ModelMixesPage(pageDef); }},
  { ICON_MODEL_OUTPUTS, STR_DEF(STR_QM_OUTPUTS), STR_DEF(STR_MENULIMITS), PAGE_CREATE, QM_MODEL_OUTPUTS, [](PageDef& pageDef) { return new ModelOutputsPage(pageDef); }},
  { ICON_MODEL_CURVES, STR_DEF(STR_QM_CURVES), STR_DEF(STR_MENUCURVES), PAGE_CREATE, QM_MODEL_CURVES, [](PageDef& pageDef) { return new ModelCurvesPage(pageDef); }, modelCurvesEnabled},
#if defined(GVARS)
  { ICON_MODEL_GVARS, STR_DEF(STR_QM_GLOBAL_VARS), STR_DEF(STR_MENU_GLOBAL_VARS), PAGE_CREATE, QM_MODEL_GVARS, [](PageDef& pageDef) { return new ModelGVarsPage(pageDef); }, modelGVEnabled},
#endif
  { ICON_MODEL_LOGICAL_SWITCHES, STR_DEF(STR_QM_LOGICAL_SW), STR_DEF(STR_MENULOGICALSWITCHES), PAGE_CREATE, QM_MODEL_LS, [](PageDef& pageDef) { return new ModelLogicalSwitchesPage(pageDef); }, modelLSEnabled},
  { ICON_MODEL_SPECIAL_FUNCTIONS, STR_DEF(STR_QM_SPEC_FUNC), STR_DEF(STR_MENUCUSTOMFUNC), PAGE_CREATE, QM_MODEL_SF, [](PageDef& pageDef) { return new SpecialFunctionsPage(pageDef); }, modelSFEnabled},
#if defined(LUA_MODEL_SCRIPTS)
  { ICON_MODEL_LUA_SCRIPTS, STR_DEF(STR_QM_CUSTOM_LUA), STR_DEF(STR_MENUCUSTOMSCRIPTS), PAGE_CREATE, QM_MODEL_SCRIPTS, [](PageDef& pageDef) { return new ModelMixerScriptsPage(pageDef); }, modelCustomScriptsEnabled},
#endif
  { ICON_MODEL_TELEMETRY, STR_DEF(STR_QM_TELEM), STR_DEF(STR_MENUTELEMETRY), PAGE_CREATE, QM_MODEL_TELEMETRY, [](PageDef& pageDef) { return new ModelTelemetryPage(pageDef); }, modelTelemetryEnabled},
  { ICON_MODEL_NOTES, STR_DEF(STR_MAIN_MENU_MODEL_NOTES), STR_DEF(STR_MAIN_MENU_MODEL_NOTES), PAGE_CREATE, QM_MODEL_NOTES, [](PageDef& pageDef) { return new ModelNotesPage(pageDef); }, modelHasNotes},
  { EDGETX_ICONS_COUNT }
};
