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
#include "batsenser.h"
#include "charge_ui.h"
#include "chat_ui.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "measure_trainer_feed.h"
#include "hrs_tools_hub.h"
#include "radio_tools.h"

#if !defined(SIMU)

void customUIActions()
{
#if defined(MODULE_BATTERY_SENSOR)
  hr_exesenserTask();
  v15BatterySensorTask();
  v15BatteryUiTask();
#if defined(USB_CHARGER)
  v15ChargeUiTask();
#endif
#endif
#if defined(MODULE_XIAOZHI_CHAT)
  v15ChatUiTask();
#endif
}

void customMixerActions()
{
  v15ServoTesterMixerHook();
  v15DshotTesterMixerHook();
  v15MeasureTrainerFeedTask();
}

void customShutdownActions()
{
#if defined(MODULE_BATTERY_SENSOR) && defined(USB_CHARGER)
  v15ShutdownWaitIfCharging();
#endif  
}

#endif

static void run_hrs_tools(Window* parent, const std::string&)
{
  (void)parent;
  v15HrsToolsOpen();
}

void customUITools(std::list<ToolEntry>& tools)
{
  tools.emplace_back(ToolEntry{"HRS Tools", "", run_hrs_tools});
}
