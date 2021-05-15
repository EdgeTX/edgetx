/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "menu_model.h"
#include "model_setup.h"
#include "model_heli.h"
#include "model_flightmodes.h"
#include "model_mixes.h"
#include "model_inputs.h"
#include "model_outputs.h"
#include "model_gvars.h"
#include "model_curves.h"
#include "model_logical_switches.h"
#include "special_functions.h"
#include "model_telemetry.h"

ModelMenu::ModelMenu():
  TabsGroup(ICON_MODEL)
{
  addTab(new ModelSetupPage());
#if defined(HELI)
  addTab(new ModelHeliPage());
#endif
  addTab(new ModelFlightModesPage());
  addTab(new ModelInputsPage());
  addTab(new ModelMixesPage());
  addTab(new ModelOutputsPage());
  addTab(new ModelCurvesPage());
  addTab(new ModelGVarsPage());
  addTab(new ModelLogicalSwitchesPage());
  addTab(new SpecialFunctionsPage(g_model.customFn));
  addTab(new ModelTelemetryPage());
}
