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

#pragma once

// these are shared by Firmware and Boards
// this allows Firmware::getCapability to call Boards::getCapability and vice versa
// Note: Firmware::getCapability falls thru to Boards::getCapability
// Caution: need to watch out for cyclic references triggering an infinite loop

enum Capability {
  ChannelsName,
  CustomFunctions,
  DangerousFunctions,
  ExtendedTrimsRange,
  FlightModes,
  FlightModesName,
  GlobalFunctions,
  Gvars,
  GvarsName,
  Haptic,
  HasExpoNames,
  HasFailsafe,
  HasFlySkyGimbals,
  HasIntModuleCRSF,
  HasIntModuleELRS,
  HasIntModuleFlySky,
  HasIntModuleMulti,
  HasMixerNames,
  HasModelImage,
  HasModelLabels,
  HasModelsList,
  HasModuleR9MFlex,
  HasModuleR9MMini,
  HasSportConnector,
  HasTopLcd,
  HasVario,
  HasVarioSink,
  Heli,
  Inputs,
  InputsName,
  InputsLength,
  IsLandscape,
  IsNarrowLayout,
  IsPortrait,
  IsWideLayout,
  FailsafeChannels,
  KeyShortcuts,
  LogicalSwitches,
  LuaInputsPerScript,
  LuaOutputsPerScript,
  LuaScripts,
  MavlinkTelemetry,
  Mixes,
  ModelImageFilters,
  ModelImageKeepExtn,
  ModelImageNameLen,
  ModelName,
  Models,
  ModelTrainerEnable,
  Modes,
  ModesName,
  NumCurvePoints,
  NumCurves,
  NumFirstUsableModule,
  NumModules,
  OffsetWeight,
  Outputs,
  PPMCenter,
  PPMFrameLength,
  QMFavourites,
  SafetyChannelCustomFunction,
  Sensors,
  SlowRange,
  SlowScale,
  TelemetryCustomScreens,
  TelemetryCustomScreensBars,
  TelemetryCustomScreensFieldsPerLine,
  TelemetryCustomScreensLines,
  Timers,
  TimersName,
  TopBarZones,
  TrainerInputs,
  TrimsRange,
  VirtualInputs,
  VoicesMaxLength,
};
