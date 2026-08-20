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
// TODO add infinite loop test and exit

enum Capability {
  // firmware
  ChannelsName,
  SpecialFunctions,
  DangerousFunctions,
  ExtendedTrimsRange,
  FlightModes,
  FlightModesName,
  GlobalFunctions,
  Gvars,
  GvarsName,
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
  InputsCnt, // renamed to Inputs to InputsCnt as duplicate in Boards keep until move to std::vector
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
  // new
  // Fatfish F16
  HasLCDtoVideo,
  // IS_JUMPER_T18
  HasBacklightKeys,
  // IS_TARANIS
  HasPowerOnDelay,
  // TODO boards start here but need to sort
  Air,
  BacklightLevelMin,
  BluetoothName,
  Contrast,
  BacklightBright,
  BacklightDelay,
  BacklightMode,
  BacklightOffBright,
  BackgroundVolume,
  SpeakerVolume,
  WavVolume,
  HatsMode,
  InactivityTimer,
  StickDeadZone,
  CPU,
  CPUType,
  EepromSize,
  DefaultInternalModule,
  ExternalModuleSize,
  FlashSize,
  FlexInputs,
  FlexSwitches,
  FourCC,
  FunctionSwitchColors,
  FunctionSwitches,
  FunctionSwitchGroups,
  GyroAxes,
  Gyros,
  Haptic,
  HasAudioMuteGPIO,
  HasAuxSerialMode,
  HasAux2SerialMode,
  HasBacklightColor,
  HasBlingLEDS,
  HasBluetooth,
  HasColorLcd,
  HasExternalAntenna,
  HasExternalModuleSupport,
  HasHardwareAntennaSwitch,
  HasIMU,
  HasInternalGPS,
  HasInternalModuleSupport,
  HasIntModuleHeartbeatGPIO,
  HasRTC,
  HasSDCard,
  HasSoftwareSerialPower,
  HasSwitchableJack,
  HasTrainerModuleCPPM,
  HasTrainerModuleSBUS,
  HasVBat,
  HasVCPSerialMode,
  Inputs,
  InputSwitches,
  HasCPUF4,
  HasCPUH5,
  HasCPUH7,
  JoystickAxes,
  Joysticks,
  Keys,
  LcdDepth,
  LcdHeight,
  LcdOLED,
  LcdWidth,
  Manufacturer,
  MaxContrast,
  MaxVolume,
  MinContrast,
  MultiposPots,
  MultiposPotsPositions,
  NumFunctionSwitchesPositions,
  NumTrims,
  NumTrimSwitches,
  Pots,
  PwrButtonPress,
  RotaryEncoderNavigation,
  Sliders,
  SportMaxBaudRate,
  StandardSwitches,
  Sticks,
  Surface,
  Switches,
  SwitchesPositions,
  // NEW
  // "Charge while radio on" is only available on RadioMaster radios
  // that expose the charger-enable pin (rm-h750 based targets).
  //if (IS_RADIOMASTER_TX16SMK3(board) || IS_RADIOMASTER_TX15(board) ||
  //    IS_RADIOMASTER_GX15(board)) {
  HasChargeWhileOn,
  //if (IS_FLYSKY_EL18(board) || IS_FLYSKY_NV14(board) || IS_FAMILY_PL18(board)) {
  HasHats,
  HasKeyLockCombo,
  // (IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board) || IS_FAMILY_PL18(board))
  HasStickDeadZone,
  // IS_HELLORADIOSKY_V16
  HasVoiceControl,
  Count,
};
