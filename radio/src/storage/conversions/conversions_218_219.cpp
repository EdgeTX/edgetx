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
#include "datastructs_218.h"

/*
 * 60 (Horus / X9) / 40 (others) telemetry sensors instead of 32
 * ALL: ReceiverData array added
 * ALL: registrationId added
 * ALL: failsafeChannels moved from ModuleData to ModelData
 * ALL: ModuleData / TrainerModuleData modified
 * PCBX9 : 6 chars for expos / mixes names instead of 8
 */

typedef ModelData ModelData_v219;

int convertSource_218_to_219(int source)
{
  // on X7: 2 additional switches
  // on X9D / X9D+: 1 additional switch
  // on xlite : 2 more storage switches

#if defined(PCBXLITE)
  if (source >= MIXSRC_SE)
    source += 2;
#endif

#if defined(PCBX7)
  if (source >= MIXSRC_SI)
    source += 2;
#endif

#if defined(PCBX9D) || defined(PCBX9DP)
  if (source >= MIXSRC_SI)
    source += 1;
#endif

#if defined(PCBHORUS)
  if (source >= MIXSRC_SI)
    source += 2;
#endif

#if defined(PCBX10)
  if (source == MIXSRC_EXT1 || source == MIXSRC_EXT2)
    source += 2;
#endif

  return source;
}

int convertSwitch_218_to_219(int swtch)
{
  // on X7: 2 additional switches
  // on X9D / X9D+: 1 additional switch
  // on XLite: 2 additional storage switches
  // on X10: 2 additional pots => 12 multipos switches
#if defined(PCBX7) || defined(PCBHORUS) || defined(PCBX9D) || defined(PCBX9DP) || defined(PCBXLITE)
  if (swtch < 0)
    return -convertSwitch_218_to_219(-swtch);
#endif

#if defined(PCBXLITE)
  if (swtch >= SWSRC_SE0)
    swtch += 2 * 3;
#endif

#if defined(PCBX7)
  if (swtch >= SWSRC_SI0)
    swtch += 2 * 3;
#endif

#if defined(PCBX9D) || defined(PCBX9DP)
  if (swtch >= SWSRC_SI0)
    swtch += 3;
#endif

#if defined(PCBHORUS)
  if (swtch >= SWSRC_SI0)
    swtch += 2 * 3;
#endif

#if defined(PCBX10)
  if (swtch >= SWSRC_FIRST_MULTIPOS_SWITCH + 3 * XPOTS_MULTIPOS_COUNT)
    swtch += 2 * XPOTS_MULTIPOS_COUNT;
#endif

  return swtch;
}

void convertModelData_218_to_219(ModelData &model)
{
  static_assert(sizeof(ModelData_v218) <= sizeof(ModelData), "ModelData size has been reduced");

#if defined(STM32)
  ModelData_v218 * oldModelAllocated = (ModelData_v218 *)malloc(sizeof(ModelData_v218));
  ModelData_v218 &oldModel = *oldModelAllocated;
#else
  ModelData_v218 oldModel;
#endif

  memcpy(&oldModel, &model, sizeof(ModelData_v218));
  ModelData_v219 & newModel = (ModelData_v219 &) model;

#if defined(PCBHORUS)
  // 4 bytes more for the ModelHeader::bitmap
  memclear(&newModel.header.bitmap[10], 4);
  memcpy(newModel.timers, oldModel.timers, offsetof(ModelData_v218, mixData) - offsetof(ModelData_v218, timers));

#if defined(BLUETOOTH)
  // trainer battery compartment removed
  if (newModel.trainerData.mode >= TRAINER_MODE_MASTER_BLUETOOTH)
    newModel.trainerData.mode -= 1;
#endif
#endif

  memclear(newModel.mixData, sizeof(ModelData_v219) - offsetof(ModelData_v219, mixData));

  char name[LEN_MODEL_NAME+1];
  zchar2str(name, oldModel.header.name, LEN_MODEL_NAME);
  TRACE("Model %s conversion from v218 to v219", name);

  for (uint8_t i=0; i<MAX_TIMERS_218; i++) {
    TimerData & timer = newModel.timers[i];
    if (timer.mode >= TMRMODE_COUNT)
      timer.mode = TMRMODE_COUNT + convertSwitch_218_to_219(oldModel.timers[i].mode - TMRMODE_COUNT + 1) - 1;
    else if (timer.mode < 0)
      timer.mode = convertSwitch_218_to_219(oldModel.timers[i].mode);
  }

  for (uint8_t i=0; i<MAX_MIXERS_218; i++) {
    memmove(&newModel.mixData[i], &oldModel.mixData[i], sizeof(MixData_v218));
    newModel.mixData[i].srcRaw = convertSource_218_to_219(newModel.mixData[i].srcRaw); // from newModel to avoid overwrite
    newModel.mixData[i].swtch = convertSwitch_218_to_219(newModel.mixData[i].swtch); // from newModel to avoid overwrite
  }

  for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS_218; i++) {
    memmove(&newModel.limitData[i], &oldModel.limitData[i], sizeof(LimitData));
  }

  for (uint8_t i=0; i<MAX_EXPOS_218; i++) {
    memmove(&newModel.expoData[i], &oldModel.expoData[i], sizeof(ExpoData_v218));
    newModel.expoData[i].srcRaw = convertSource_218_to_219(newModel.expoData[i].srcRaw); // from newModel to avoid overwrite
    newModel.expoData[i].swtch = convertSwitch_218_to_219(newModel.expoData[i].swtch); // from newModel to avoid overwrite
#if LCD_W == 212
    newModel.expoData[i].offset = oldModel.expoData[i].offset; // 212x64: expo name has been reduced to 6 chars instead of 8
    newModel.expoData[i].curve = oldModel.expoData[i].curve; // 212x64: expo name has been reduced to 6 chars instead of 8
#endif
  }

  for (uint8_t i=0; i<MAX_CURVES_218; i++) {
    memmove(&newModel.curves[i], &oldModel.curves[i], sizeof(CurveHeader_v218));
  }

  for (uint32_t i=0; i<MAX_CURVE_POINTS_218; i++) {
    newModel.points[i] = oldModel.points[i];
  }

  for (uint8_t i=0; i<MAX_LOGICAL_SWITCHES_218; i++) {
    memmove(&newModel.logicalSw[i], &oldModel.logicalSw[i], sizeof(LogicalSwitchData_v218));
    LogicalSwitchData & sw = newModel.logicalSw[i];
    uint8_t cstate = lswFamily(sw.func);
    if (cstate == LS_FAMILY_OFS || cstate == LS_FAMILY_COMP || cstate == LS_FAMILY_DIFF) {
      LogicalSwitchData & sw = newModel.logicalSw[i];
      sw.v1 = convertSource_218_to_219(sw.v1);
      if (cstate == LS_FAMILY_COMP) {
        sw.v2 = convertSource_218_to_219(sw.v2);
      }
    }
    else if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
      sw.v1 = convertSwitch_218_to_219(sw.v1);
      sw.v2 = convertSwitch_218_to_219(sw.v2);
    }
    else if (cstate == LS_FAMILY_EDGE) {
      sw.v1 = convertSwitch_218_to_219(sw.v1);
    }
    sw.andsw = convertSwitch_218_to_219(sw.andsw);
  }

  for (uint8_t i=0; i<MAX_SPECIAL_FUNCTIONS_218; i++) {
    memmove(&newModel.customFn[i], &oldModel.customFn[i], sizeof(CustomFunctionData_v218));
    CustomFunctionData & cf = newModel.customFn[i];
    cf.swtch = convertSwitch_218_to_219(cf.swtch);
    if (cf.func == FUNC_PLAY_VALUE || cf.func == FUNC_VOLUME || (IS_ADJUST_GV_FUNC(cf.func) && cf.all.mode == FUNC_ADJUST_GVAR_SOURCE)) {
      cf.all.val = convertSource_218_to_219(cf.all.val);
    }
  }

  newModel.swashR = oldModel.swashR;

  for (uint8_t i=0; i<MAX_FLIGHT_MODES_218; i++) {
    memmove(&newModel.flightModeData[i], &oldModel.flightModeData[i], sizeof(FlightModeData_v218));
    FlightModeData & sw = newModel.flightModeData[i];
    sw.swtch = convertSwitch_218_to_219(sw.swtch);
  }

  newModel.thrTraceSrc = oldModel.thrTraceSrc;
#if defined(PCBX10)
  if (newModel.thrTraceSrc > 3) // 0=Thr, 1/2/3=Old 3 Pots, then Sliders
    newModel.thrTraceSrc += 2;
#endif
  newModel.switchWarningState = oldModel.switchWarningState;
#if !defined(COLORLCD)
  newModel.switchWarningEnable = oldModel.switchWarningEnable;
#endif

  for (uint8_t i=0; i<MAX_GVARS_218; i++) {
    memmove(&newModel.gvars[i], &oldModel.gvars[i], sizeof(GVarData_v218));
  }

  newModel.varioData.source = oldModel.frsky.varioSource;
  newModel.varioData.centerSilent = oldModel.frsky.varioCenterSilent;
  newModel.varioData.centerMax = oldModel.frsky.varioCenterMax;
  newModel.varioData.centerMin = oldModel.frsky.varioCenterMin;
  newModel.varioData.min = oldModel.frsky.varioMin;
  newModel.varioData.max = oldModel.frsky.varioMax;

#if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E)
  newModel.voltsSource = oldModel.frsky.voltsSource;
  newModel.altitudeSource = oldModel.frsky.altitudeSource;
#endif

  newModel.rssiAlarms = oldModel.rssiAlarms;
  newModel.potsWarnMode = oldModel.potsWarnMode;

  for (int i=0; i<NUM_MODULES; i++) {
    memcpy(&newModel.moduleData[i], &oldModel.moduleData[i], 4);
    memcpy(((uint8_t *)&newModel.moduleData[i]) + 4, ((uint8_t *)&oldModel.moduleData[i]) + 64 + 4, 2);
    if (newModel.moduleData[i].type >= MODULE_TYPE_ISRM_PXX2)
      newModel.moduleData[i].type += 1;
    if (newModel.moduleData[i].type >= MODULE_TYPE_R9M_PXX2)
      newModel.moduleData[i].type += 4;
    if (newModel.moduleData[i].type == MODULE_TYPE_XJT_PXX1) {
      newModel.moduleData[i].subType = newModel.moduleData[i].rfProtocol;
#if defined(RADIO_X9DP2019)
      if (i == INTERNAL_MODULE) {
        newModel.moduleData[i].type = MODULE_TYPE_ISRM_PXX2;
        newModel.moduleData[i].subType = MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16;
      }
#endif
    }
  }

#if defined(RADIO_T12)
  newModel.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_NONE; // Early t12 firmware had unused INT settings that need to be cleared
#endif

  for (uint8_t module=0; module<2; module++) {
    if (oldModel.moduleData[module].failsafeMode == FAILSAFE_CUSTOM) {
      memcpy(newModel.failsafeChannels, oldModel.moduleData[module].failsafeChannels, sizeof(newModel.failsafeChannels));
    }
  }

#if !defined(PCBSKY9X)
  newModel.trainerData.mode = oldModel.trainerMode;
#endif
  newModel.trainerData.channelsStart = oldModel.moduleData[NUM_MODULES].channelsStart;
  newModel.trainerData.channelsCount = oldModel.moduleData[NUM_MODULES].channelsCount;
  newModel.trainerData.frameLength = oldModel.moduleData[NUM_MODULES].ppm.frameLength;
  newModel.trainerData.delay = oldModel.moduleData[NUM_MODULES].ppm.delay;
  newModel.trainerData.pulsePol = oldModel.moduleData[NUM_MODULES].ppm.pulsePol;

#if defined(PCBHORUS) || defined(PCBTARANIS)
  memmove(newModel.scriptsData, oldModel.scriptsData,
         sizeof(newModel.scriptsData) +
         sizeof(newModel.inputNames) +
         sizeof(newModel.potsWarnEnabled) +
         sizeof(newModel.potsWarnPosition));
#else
  memmove(newModel.inputNames, oldModel.inputNames,
         sizeof(newModel.inputNames) +
         sizeof(newModel.potsWarnEnabled) +
         sizeof(newModel.potsWarnPosition));
#endif

  for (uint8_t i=0; i<MAX_TELEMETRY_SENSORS_218; i++) {
    newModel.telemetrySensors[i].id = oldModel.telemetrySensors[i].id;
    if (oldModel.telemetrySensors[i].type == 0 && ZLEN(oldModel.telemetrySensors[i].label) > 0 && (isModuleTypePXX1(oldModel.moduleData[0].type) || isModuleTypePXX1(oldModel.moduleData[1].type)))
      newModel.telemetrySensors[i].instance = 0xE0 + (oldModel.telemetrySensors[i].instance & 0x1F) - 1;
    else
      newModel.telemetrySensors[i].instance = oldModel.telemetrySensors[i].instance;
    memcpy(newModel.telemetrySensors[i].label, oldModel.telemetrySensors[i].label, TELEM_LABEL_LEN); // id + instance + label
    newModel.telemetrySensors[i].subId = oldModel.telemetrySensors[i].subId;
    newModel.telemetrySensors[i].type = oldModel.telemetrySensors[i].type;
    newModel.telemetrySensors[i].unit = oldModel.telemetrySensors[i].unit;
    if (newModel.telemetrySensors[i].unit >= UNIT_MILLILITERS_PER_MINUTE)
      newModel.telemetrySensors[i].unit += 11;
    newModel.telemetrySensors[i].prec = oldModel.telemetrySensors[i].prec;
    newModel.telemetrySensors[i].autoOffset = oldModel.telemetrySensors[i].autoOffset;
    newModel.telemetrySensors[i].filter = oldModel.telemetrySensors[i].filter;
    newModel.telemetrySensors[i].logs = oldModel.telemetrySensors[i].logs;
    newModel.telemetrySensors[i].persistent = oldModel.telemetrySensors[i].persistent;
    newModel.telemetrySensors[i].onlyPositive = oldModel.telemetrySensors[i].onlyPositive;
    memcpy(((uint8_t *)&newModel.telemetrySensors[i]) + 10, ((uint8_t *)&oldModel.telemetrySensors[i]) + 9, 4);
  }

#if defined(PCBX9E)
  newModel.toplcdTimer = oldModel.toplcdTimer;
#endif

#if defined(PCBHORUS)
#if defined(FIXME) //TODO
  memcpy(newModel.screenData, oldModel.screenData,
         sizeof(newModel.screenData) +
         sizeof(newModel.topbarData));

  for (int screen=0; screen<MAX_CUSTOM_SCREENS; screen++) {
    CustomScreenData& screenData = g_model.screenData[screen];
    if (screenData.LayoutId[0] == '\0')
      continue;
    for (int zone=0; zone<MAX_LAYOUT_ZONES; zone++) {
      Layout::ZonePersistentData * zoneData = &screenData.layoutData.zones[zone];
      if (strcmp("Value", zoneData->widgetName))
        continue;

      ZoneOptionValue& option = zoneData->widgetData.options[0];
      option.unsignedValue = convertSource_218_to_219(option.unsignedValue);
    }
  }

  for (int zone=0; zone<MAX_LAYOUT_ZONES; zone++) {
    TopBar::ZonePersistentData * zoneData = &g_model.topbarData.zones[zone];
    if (strcmp("Value", zoneData->widgetName))
      continue;

    ZoneOptionValue & option = zoneData->widgetData.options[0];
    option.unsignedValue = convertSource_218_to_219(option.unsignedValue);
  }
#endif
#else
  newModel.screensType = oldModel.frsky.screensType;
  memmove(&newModel.screens, &oldModel.frsky.screens, sizeof(newModel.screens));
  for (int i=0; i<MAX_TELEMETRY_SCREENS; i++) {
    uint8_t screenType = (newModel.screensType >> (2*i)) & 0x03;
    if (screenType == TELEMETRY_SCREEN_TYPE_VALUES) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < NUM_LINE_ITEMS; k++) {
          newModel.screens[i].lines[j].sources[k] = convertSource_218_to_219(oldModel.frsky.screens[i].lines[j].sources[k]);
        }
      }
    }
    else if (screenType == TELEMETRY_SCREEN_TYPE_BARS) {
      for (int j = 0; j < 4; j++) {
        newModel.screens[i].bars[j].source = convertSource_218_to_219(oldModel.frsky.screens[i].bars[j].source);
      }
    }
  }
#endif

#if defined(STM32)
  free(oldModelAllocated);
#endif
}

void convertRadioData_218_to_219(RadioData & settings)
{
  TRACE("Radio conversion from v218 to v219");

  settings.version = 219;
  settings.variant = EEPROM_VARIANT;

#if defined(STM32)
  RadioData_v218 * oldSettingsAllocated = (RadioData_v218 *)malloc(sizeof(RadioData_v218));
  RadioData_v218 & oldSettings = *oldSettingsAllocated;
  memcpy(&oldSettings, &settings, sizeof(RadioData_v218));
#endif

#if defined(PCBX9D) || defined(PCBX9DP)
  // no bluetooth before PCBREV 2019
  settings.auxSerialMode = oldSettings.auxSerialMode;
  settings.slidersConfig = oldSettings.slidersConfig;
  settings.potsConfig = oldSettings.potsConfig;
  settings.backlightColor = oldSettings.backlightColor;
  settings.switchUnlockStates = oldSettings.switchUnlockStates;
  settings.switchConfig = oldSettings.switchConfig;
#if defined(RADIO_X9DP2019)
  settings.switchConfig |= SWITCH_TOGGLE << 16;
#endif
  memcpy(&settings.switchNames[0], &oldSettings.switchNames[0], 8 * LEN_SWITCH_NAME);
  memclear(&settings.switchNames[8], LEN_SWITCH_NAME);
  memcpy(&settings.anaNames[0], &oldSettings.anaNames[0], (NUM_STICKS+NUM_POTS+NUM_SLIDERS) * LEN_ANA_NAME);
#endif

#if defined(PCBHORUS)
  // 2 new pots from X10:
  //  - copy btw. 'chkSum' and 'auxSerialMode' (excl.)
  memcpy(&settings.chkSum, &oldSettings.chkSum, offsetof(RadioData, switchConfig) - sizeof(uint8_t) - offsetof(RadioData, chkSum));
  //  - move calibration data
  memcpy(&settings.calib[NUM_STICKS + 5], &oldSettings.calib[NUM_STICKS + 3], sizeof(CalibData) * (STORAGE_NUM_SLIDERS + STORAGE_NUM_MOUSE_ANALOGS));
  memclear(&settings.calib[NUM_STICKS + 3], sizeof(CalibData) * 2);

  // move fields after custom functions
  settings.auxSerialMode = oldSettings.auxSerialMode;
  settings.switchConfig = oldSettings.switchConfig;
  settings.potsConfig = oldSettings.potsConfig;
  settings.slidersConfig = oldSettings.slidersConfig;

  // 2 new switches
  memcpy(&settings.switchNames[0], &oldSettings.switchNames[0], 8 * LEN_SWITCH_NAME);
  memclear(&settings.switchNames[8], 2 * LEN_SWITCH_NAME);

  // 2 new pots for X10
  //  - split 'anaNames' (sticks + 3 old pots, 2 new pots, other old analogs)
  memcpy(&settings.anaNames[0], &oldSettings.anaNames[0], (NUM_STICKS + 3) * LEN_ANA_NAME);
  memclear(&settings.anaNames[NUM_STICKS + 3], 2 * LEN_SWITCH_NAME);
  memcpy(&settings.anaNames[NUM_STICKS + 5], &oldSettings.anaNames[NUM_STICKS + 3], STORAGE_NUM_SLIDERS * LEN_ANA_NAME);

  //  - copy rest of RadioData struct
  memcpy(&settings.currModelFilename[0], &oldSettings.currModelFilename[0], sizeof(RadioData_v218) - offsetof(RadioData_v218, currModelFilename[0]));
#endif

#if defined(RADIO_T12)
  g_eeGeneral.switchConfig = bfSet<uint32_t>(g_eeGeneral.switchConfig, SWITCH_2POS, 10, 2);  // T12 comes with wrongly defined pot2
  g_eeGeneral.potsConfig = bfSet<uint32_t>(g_eeGeneral.potsConfig, POT_WITHOUT_DETENT, 2, 2);  // T12 comes with wrongly defined pot2
#endif

#if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX7) || defined(PCBXLITE) || defined(PCBHORUS)
  for (uint8_t i=0; i<MAX_SPECIAL_FUNCTIONS_218; i++) {
    CustomFunctionData & cf = settings.customFn[i];
    cf.swtch = convertSwitch_218_to_219(cf.swtch);
    if (cf.func == FUNC_PLAY_VALUE || cf.func == FUNC_VOLUME || (IS_ADJUST_GV_FUNC(cf.func) && cf.all.mode == FUNC_ADJUST_GVAR_SOURCE)) {
      cf.all.val = convertSource_218_to_219(cf.all.val);
    }
  }
#endif

#if defined(RADIO_X9DP2019)
  // force re-calibration
  settings.chkSum = 0xFFFF;
  setDefaultOwnerId();
#endif

  settings.pwrOnSpeed = 0;
  settings.pwrOffSpeed = 0;

#if defined(STM32)
  free(oldSettingsAllocated);
#endif
}
