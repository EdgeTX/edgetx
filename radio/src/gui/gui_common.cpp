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

#include "opentx.h"

#if defined(PCBFRSKY) || defined(PCBFLYSKY)
uint8_t switchToMix(uint8_t source)
{
  div_t qr = div(source-1, 3);
  return qr.quot+MIXSRC_FIRST_SWITCH;
}
#else
uint8_t switchToMix(uint8_t source)
{
  if (source <= 3)
    return MIXSRC_3POS;
  else
    return MIXSRC_FIRST_SWITCH - 3 + source;
}
#endif

int circularIncDec(int current, int inc, int min, int max, IsValueAvailable isValueAvailable)
{
  do {
    current += inc;
    if (current < min)
      current = max;
    else if (current > max)
      current = min;
    if (!isValueAvailable || isValueAvailable(current))
      return current;
  } while(1);
  return 0;
}

bool isInputAvailable(int input)
{
  for (int i=0; i<MAX_EXPOS; i++) {
    ExpoData * expo = expoAddress(i);
    if (!EXPO_VALID(expo))
      break;
    if (expo->chn == input)
      return true;
  }
  return false;
}

bool isRssiSensorAvailable(int sensor)
{
  if (sensor == 0)
    return true;
  else {
    TelemetrySensor &telemSensor = g_model.telemetrySensors[abs(sensor) - 1];
    return (telemSensor.isAvailable() && telemSensor.id == RSSI_ID);
  }
}

bool isSensorAvailable(int sensor)
{
  if (sensor == 0)
    return true;
  else
    return isTelemetryFieldAvailable(abs(sensor) - 1);
}

bool isSensorUnit(int sensor, uint8_t unit)
{
  if (sensor <= 0 || sensor > MAX_TELEMETRY_SENSORS ) {
    return true;
  }
  else {
    return g_model.telemetrySensors[sensor-1].unit == unit;
  }
}

bool isCellsSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_CELLS);
}

bool isGPSSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_GPS);
}

bool isAltSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_DIST) || isSensorUnit(sensor, UNIT_FEET);
}

bool isVoltsSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_VOLTS) || isSensorUnit(sensor, UNIT_CELLS);
}

bool isCurrentSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_AMPS);
}

bool isTelemetryFieldAvailable(int index)
{
  TelemetrySensor & sensor = g_model.telemetrySensors[index];
  return sensor.isAvailable();
}

uint8_t getTelemetrySensorsCount()
{
  uint8_t count = 0;
  for (auto telemetrySensor : g_model.telemetrySensors) {
    if (telemetrySensor.isAvailable()) {
      ++count;
    }
  }
  return count;
}

bool isTelemetryFieldComparisonAvailable(int index)
{
  if (!isTelemetryFieldAvailable(index))
    return false;

  TelemetrySensor & sensor = g_model.telemetrySensors[index];
  if (sensor.unit >= UNIT_DATETIME)
    return false;
  return true;
}

bool isChannelUsed(int channel)
{
  for (int i=0; i<MAX_MIXERS; ++i) {
    MixData *md = mixAddress(i);
    if (md->srcRaw == 0) return false;
    if (md->destCh == channel) return true;
    if (md->destCh > channel) return false;
  }
  return false;
}

int getChannelsUsed()
{
  int result = 0;
  int lastCh = -1;
  for (int i=0; i<MAX_MIXERS; ++i) {
    MixData *md = mixAddress(i);
    if (md->srcRaw == 0) return result;
    if (md->destCh != lastCh) { ++result; lastCh = md->destCh; }
  }
  return result;
}

bool isSourceAvailable(int source)
{
  if (source < 0)
    return false;

  if (source >= MIXSRC_FIRST_INPUT && source <= MIXSRC_LAST_INPUT) {
    return isInputAvailable(source - MIXSRC_FIRST_INPUT);
  }

#if defined(LUA_MODEL_SCRIPTS)
  if (source >= MIXSRC_FIRST_LUA && source <= MIXSRC_LAST_LUA) {
    div_t qr = div(source - MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
    return (qr.rem<scriptInputsOutputs[qr.quot].outputsCount);
  }
#elif defined(LUA_INPUTS)
  if (source >= MIXSRC_FIRST_LUA && source <= MIXSRC_LAST_LUA)
    return false;
#endif

  if (source >= MIXSRC_FIRST_POT && source <= MIXSRC_LAST_POT) {
    return IS_POT_SLIDER_AVAILABLE(POT1+source - MIXSRC_FIRST_POT);
  }

#if defined(PCBX10)
  if (source >= MIXSRC_MOUSE1 && source <= MIXSRC_MOUSE2)
    return false;
#endif

  if (source >= MIXSRC_FIRST_SWITCH && source <= MIXSRC_LAST_SWITCH) {
    return SWITCH_EXISTS(source - MIXSRC_FIRST_SWITCH);
  }

#if !defined(HELI)
  if (source >= MIXSRC_CYC1 && source <= MIXSRC_CYC3)
    return false;
#endif

  if (source >= MIXSRC_FIRST_CH && source <= MIXSRC_LAST_CH) {
    return isChannelUsed(source - MIXSRC_FIRST_CH);
  }

  if (source >= MIXSRC_FIRST_LOGICAL_SWITCH && source <= MIXSRC_LAST_LOGICAL_SWITCH) {
    LogicalSwitchData * cs = lswAddress(source - MIXSRC_FIRST_LOGICAL_SWITCH);
    return (cs->func != LS_FUNC_NONE);
  }

#if !defined(GVARS)
  if (source >= MIXSRC_GVAR1 && source <= MIXSRC_LAST_GVAR)
    return false;
#endif

  if (source >= MIXSRC_FIRST_RESERVE && source <= MIXSRC_LAST_RESERVE)
    return false;

  if (source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    div_t qr = div(source - MIXSRC_FIRST_TELEM, 3);
    if (qr.rem == 0)
      return isTelemetryFieldAvailable(qr.quot);
    else
      return isTelemetryFieldComparisonAvailable(qr.quot);
  }

  return true;
}

bool isSourceAvailableInGlobalFunctions(int source)
{
  if (source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    return false;
  }
  return isSourceAvailable(source);
}

bool isSourceAvailableInCustomSwitches(int source)
{
  bool result = isSourceAvailable(source);

  if (result && source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    div_t qr = div(source - MIXSRC_FIRST_TELEM, 3);
    result = isTelemetryFieldComparisonAvailable(qr.quot);
  }

  return result;
}

bool isSourceAvailableInInputs(int source)
{
  if (source >= MIXSRC_FIRST_POT && source <= MIXSRC_LAST_POT)
    return IS_POT_SLIDER_AVAILABLE(POT1+source - MIXSRC_FIRST_POT);

#if defined(PCBX10)
  if (source >= MIXSRC_MOUSE1 && source <= MIXSRC_MOUSE2)
    return false;
#endif

  if (source >= MIXSRC_Rud && source <= MIXSRC_MAX)
    return true;

  if (source >= MIXSRC_FIRST_TRIM && source <= MIXSRC_LAST_TRIM)
    return true;

  if (source >= MIXSRC_FIRST_SWITCH && source <= MIXSRC_LAST_SWITCH)
    return SWITCH_EXISTS(source - MIXSRC_FIRST_SWITCH);

  if (source >= MIXSRC_FIRST_CH && source <= MIXSRC_LAST_CH)
    return true;

  if (source >= MIXSRC_FIRST_LOGICAL_SWITCH && source <= MIXSRC_LAST_LOGICAL_SWITCH) {
    LogicalSwitchData * cs = lswAddress(source - MIXSRC_SW1);
    return (cs->func != LS_FUNC_NONE);
  }

  if (source >= MIXSRC_FIRST_TRAINER && source <= MIXSRC_LAST_TRAINER)
    return true;

  if (source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    div_t qr = div(source - MIXSRC_FIRST_TELEM, 3);
    return isTelemetryFieldAvailable(qr.quot) && isTelemetryFieldComparisonAvailable(qr.quot);
  }

  return false;
}

bool isLogicalSwitchAvailable(int index)
{
  LogicalSwitchData * lsw = lswAddress(index);
  return (lsw->func != LS_FUNC_NONE);
}

bool isSwitchAvailable(int swtch, SwitchContext context)
{
  bool negative = false;
  (void)negative;

  if (swtch < 0) {
    if (swtch == -SWSRC_ON || swtch == -SWSRC_ONE) {
      return false;
    }
    negative = true;
    swtch = -swtch;
  }

  if (swtch >= SWSRC_FIRST_SWITCH && swtch <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(swtch);
    if (!SWITCH_EXISTS(swinfo.quot)) {
      return false;
    }
    if (!IS_CONFIG_3POS(swinfo.quot)) {
      if (swinfo.rem == 1) {
        // mid position not available for 2POS switches
        return false;
      }
    }
    return true;
  }

#if NUM_XPOTS > 0
  if (swtch >= SWSRC_FIRST_MULTIPOS_SWITCH && swtch <= SWSRC_LAST_MULTIPOS_SWITCH) {
    int index = (swtch - SWSRC_FIRST_MULTIPOS_SWITCH) / XPOTS_MULTIPOS_COUNT;
    return IS_POT_MULTIPOS(POT1+index);
  }
#endif

  if (swtch >= SWSRC_FIRST_LOGICAL_SWITCH && swtch <= SWSRC_LAST_LOGICAL_SWITCH) {
    if (context == GeneralCustomFunctionsContext) {
      return false;
    }
    else if (context != LogicalSwitchesContext) {
      return isLogicalSwitchAvailable(swtch - SWSRC_FIRST_LOGICAL_SWITCH);
    }
  }

  if (context != ModelCustomFunctionsContext && context != GeneralCustomFunctionsContext && (swtch == SWSRC_ON || swtch == SWSRC_ONE)) {
    return false;
  }

  if (swtch >= SWSRC_FIRST_FLIGHT_MODE && swtch <= SWSRC_LAST_FLIGHT_MODE) {
    if (context == MixesContext || context == GeneralCustomFunctionsContext) {
      return false;
    }
    else {
      swtch -= SWSRC_FIRST_FLIGHT_MODE;
      if (swtch == 0) {
        return true;
      }
      FlightModeData * fm = flightModeAddress(swtch);
      return (fm->swtch != SWSRC_NONE);
    }
  }

  if (swtch >= SWSRC_FIRST_SENSOR && swtch <= SWSRC_LAST_SENSOR) {
    if (context == GeneralCustomFunctionsContext)
      return false;
    else
      return isTelemetryFieldAvailable(swtch - SWSRC_FIRST_SENSOR);
  }

  return true;
}

bool isAuxModeAvailable(int mode)
{
#if defined(AUX2_SERIAL)
  if (mode == UART_MODE_SBUS_TRAINER)
    return g_eeGeneral.aux2SerialMode != UART_MODE_SBUS_TRAINER;
#if defined(RADIO_TX16S)
  else
    return (g_model.trainerData.mode != TRAINER_MODE_MASTER_BATTERY_COMPARTMENT || g_eeGeneral.aux2SerialMode == UART_MODE_SBUS_TRAINER);
#endif
#endif
  return true;
}

bool isAux2ModeAvailable(int mode)
{
#if defined(AUX_SERIAL)
  if (mode == UART_MODE_SBUS_TRAINER)
    return g_eeGeneral.auxSerialMode != UART_MODE_SBUS_TRAINER;
#if defined(RADIO_TX16S)
  else
    return (g_model.trainerData.mode != TRAINER_MODE_MASTER_BATTERY_COMPARTMENT || g_eeGeneral.auxSerialMode == UART_MODE_SBUS_TRAINER);
#endif
#endif
  return true;
}

bool isSwitchAvailableInLogicalSwitches(int swtch)
{
  return isSwitchAvailable(swtch, LogicalSwitchesContext);
}

bool isSwitchAvailableInMixes(int swtch)
{
  return isSwitchAvailable(swtch, MixesContext);
}

#if defined(COLORLCD)
bool isSwitch2POSWarningStateAvailable(int state)
{
  return (state != 2); // two pos switch - middle state not available
}
#endif // #if defined(COLORLCD)

//bool isSwitchAvailableInTimers(int swtch)
//{
//  if (swtch >= 0) {
//    if (swtch < TMRMODE_COUNT)
//      return true;
//    else
//      swtch -= TMRMODE_COUNT-1;
//  }
//  else {
//    if (swtch > -TMRMODE_COUNT)
//      return false;
//    else
//      swtch += TMRMODE_COUNT-1;
//  }
//
//  return isSwitchAvailable(swtch, TimersContext);
//}

bool isThrottleSourceAvailable(int src)
{
#if !defined(LIBOPENUI)
  src = throttleSource2Source(src);
#endif
  return isSourceAvailable(src) &&
    ((src == MIXSRC_Thr) ||
     ((src >= MIXSRC_FIRST_POT) && (src <= MIXSRC_LAST_POT)) ||
     ((src >= MIXSRC_FIRST_CH) && (src <= MIXSRC_LAST_CH)));
}

bool isLogicalSwitchFunctionAvailable(int function)
{
  return function != LS_FUNC_RANGE;
}

bool isAssignableFunctionAvailable(int function, CustomFunctionData * functions)
{
#if defined(OVERRIDE_CHANNEL_FUNCTION) || defined(GVARS)
  bool modelFunctions = (functions == g_model.customFn);
#endif

  switch (function) {
    case FUNC_OVERRIDE_CHANNEL:
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      return modelFunctions;
#else
      return false;
#endif
    case FUNC_ADJUST_GVAR:
#if defined(GVARS)
      return modelFunctions;
#else
      return false;
#endif
#if !defined(HAPTIC)
      case FUNC_HAPTIC:
#endif
    case FUNC_RESERVE4:
#if !defined(DANGEROUS_MODULE_FUNCTIONS)
    case FUNC_RANGECHECK:
    case FUNC_BIND:
#endif
#if !defined(LUA)
    case FUNC_PLAY_SCRIPT:
#endif
    case FUNC_RESERVE5:
      return false;

    default:
      return true;
  }
}

#if !defined(COLORLCD)
bool isAssignableFunctionAvailable(int function)
{
  return isAssignableFunctionAvailable(function, menuHandlers[menuLevel] == menuModelSpecialFunctions ? g_model.customFn : g_eeGeneral.customFn);
}
#endif

bool isSourceAvailableInGlobalResetSpecialFunction(int index)
{
  if (index >= FUNC_RESET_PARAM_FIRST_TELEM)
    return false;
  else
    return isSourceAvailableInResetSpecialFunction(index);
}

bool isSourceAvailableInResetSpecialFunction(int index)
{
  if (index >= FUNC_RESET_PARAM_FIRST_TELEM) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index-FUNC_RESET_PARAM_FIRST_TELEM];
    return telemetrySensor.isAvailable();
  }
#if TIMERS < 3
    else if (index == FUNC_RESET_TIMER3) {
    return false;
  }
#endif
#if TIMERS < 2
    else if (index == FUNC_RESET_TIMER2) {
    return false;
  }
#endif
  else {
    return true;
  }
}


#if defined(PXX2)
bool isPxx2IsrmChannelsCountAllowed(int channels)
{
  if (g_model.moduleData[INTERNAL_MODULE].subType == MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16 && channels > 8)
    return false;
  return (channels % 8 == 0);
}
#else
bool isPxx2IsrmChannelsCountAllowed(int channels)
{
  return true;
}
#endif

bool isTrainerUsingModuleBay()
{
#if defined(PCBTARANIS)
  if (TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE <= g_model.trainerData.mode && g_model.trainerData.mode <= TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE)
    return true;
#endif
  return false;
}

bool isModuleUsingSport(uint8_t moduleBay, uint8_t moduleType)
{
  switch (moduleType) {
    case MODULE_TYPE_NONE:
    case MODULE_TYPE_SBUS:
    case MODULE_TYPE_PPM:
    case MODULE_TYPE_DSM2:
    case MODULE_TYPE_MULTIMODULE:
    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
    case MODULE_TYPE_FLYSKY:
      return false;

    case MODULE_TYPE_XJT_PXX1:
      // External XJT has a physical switch to disable S.PORT
    case MODULE_TYPE_R9M_PXX1:
      // R9M telemetry is disabled by pulses (pxx1.cpp)
      if (moduleBay == EXTERNAL_MODULE)
        return false;

    case MODULE_TYPE_CROSSFIRE:
      if (moduleBay == INTERNAL_MODULE)
        return false;
      
    default:
      return true;
  }
}

bool areModulesConflicting(int intModuleType, int extModuleType)
{
  if (intModuleType == MODULE_TYPE_ISRM_PXX2)
    return (extModuleType == MODULE_TYPE_GHOST);

  return false;
}

#if defined(HARDWARE_INTERNAL_MODULE)
bool isInternalModuleSupported(int moduleType)
{
  switch(moduleType) {
  case MODULE_TYPE_NONE: return true;
#if defined(INTERNAL_MODULE_MULTI)
  case MODULE_TYPE_MULTIMODULE: return true;
#endif
#if defined(INTERNAL_MODULE_CRSF)
  case MODULE_TYPE_CROSSFIRE: return true;
#endif
#if defined(INTERNAL_MODULE_PXX1)
  case MODULE_TYPE_XJT_PXX1: return true;
#endif
#if defined(INTERNAL_MODULE_PXX2)
  case MODULE_TYPE_ISRM_PXX2: return true;
#endif
#if defined(INTERNAL_MODULE_PPM)
  case MODULE_TYPE_PPM: return true;
#endif
#if defined(INTERNAL_MODULE_AFHDS2A) || defined(INTERNAL_MODULE_AFHDS3)
  case MODULE_TYPE_FLYSKY: return true;
#endif
  }
  return false;
}

bool isInternalModuleAvailable(int moduleType)
{
  if (moduleType == MODULE_TYPE_NONE)
    return true;

  if (g_eeGeneral.internalModule != moduleType)
    return false;

#if defined(INTERNAL_MODULE_PXX1)
  if ((moduleType == MODULE_TYPE_XJT_PXX1) &&
      isModuleUsingSport(EXTERNAL_MODULE,
                         g_model.moduleData[EXTERNAL_MODULE].type)) {
    return false;
  }
#endif

#if defined(INTERNAL_MODULE_PXX2)
  if ((moduleType == MODULE_TYPE_ISRM_PXX2) &&
      areModulesConflicting(moduleType,
                            g_model.moduleData[EXTERNAL_MODULE].type)) {
    return false;
  }
#endif

  return true;
}
#else
bool isInternalModuleSupported(int moduleType)
{
  return false;
}

bool isInternalModuleAvailable(int moduleType)
{
  return false;
}
#endif

bool isExternalModuleAvailable(int moduleType)
{
#if !defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
  if (isModuleTypeR9MLite(moduleType) || moduleType == MODULE_TYPE_XJT_LITE_PXX2)
    return false;
#endif

#if !defined(PXX1)
  if (isModuleTypePXX1(moduleType))
    return false;
#endif

#if !defined(XJT)
  if (moduleType == MODULE_TYPE_XJT_PXX1)
    return false;
#endif

#if !defined(HARDWARE_EXTERNAL_MODULE_SIZE_STD)
  if (moduleType == MODULE_TYPE_R9M_PXX1 || moduleType == MODULE_TYPE_R9M_PXX2)
    return false;
#endif

  if (moduleType == MODULE_TYPE_ISRM_PXX2)
    return false; // doesn't exist for now


#if !defined(PXX2) || !defined(EXTMODULE_USART)
  if (moduleType == MODULE_TYPE_XJT_LITE_PXX2 || moduleType == MODULE_TYPE_R9M_PXX2 || moduleType == MODULE_TYPE_R9M_LITE_PXX2 || moduleType == MODULE_TYPE_R9M_LITE_PRO_PXX2) {
    return false;
  }
#endif

#if !defined(CROSSFIRE)
  if (moduleType == MODULE_TYPE_CROSSFIRE)
    return false;
#endif

#if !defined(GHOST)
  if (moduleType == MODULE_TYPE_GHOST)
    return false;
#endif

#if !defined(DSM2)
  if (moduleType == MODULE_TYPE_DSM2)
     return false;
#endif

#if !defined(SBUS)
  if (moduleType == MODULE_TYPE_SBUS)
    return false;
#endif

#if !defined(MULTIMODULE)
  if (moduleType == MODULE_TYPE_MULTIMODULE)
    return false;
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
  if (areModulesConflicting(g_model.moduleData[INTERNAL_MODULE].type, moduleType))
    return false;

  if (isTrainerUsingModuleBay() || (isModuleUsingSport(EXTERNAL_MODULE, moduleType) && isModuleUsingSport(INTERNAL_MODULE, g_model.moduleData[INTERNAL_MODULE].type)))
    return false;
#endif

#if !defined(PPM)
  if (moduleType == MODULE_TYPE_PPM)
    return false;
#endif

#if !defined(AFHDS3)
  if (moduleType == MODULE_TYPE_FLYSKY)
    return false;
#endif

  return true;
}

bool isRfProtocolAvailable(int protocol)
{
#if defined(CROSSFIRE)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE) {
    return false;
  }
#endif
#if defined(GHOST)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_GHOST) {
    return false;
  }
#endif
#if !defined(MODULE_PROTOCOL_D8)
  if (protocol == MODULE_SUBTYPE_PXX1_ACCST_D8) {
    return false;
  }
#endif
#if defined(PCBTARANIS) || defined(PCBHORUS)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_PXX1) {
    return false;
  }
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_PXX2) {
    return false;
  }
#endif

  return true;
}

bool isTelemetryProtocolAvailable(int protocol)
{
#if defined(PCBTARANIS)
  if (protocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY && g_eeGeneral.auxSerialMode != UART_MODE_TELEMETRY) {
    return false;
  }
#endif

  if (protocol== PROTOCOL_TELEMETRY_CROSSFIRE) {
    return false;
  }

  if ( protocol== PROTOCOL_TELEMETRY_GHOST) {
    return false;
  }

#if !defined(MULTIMODULE)
  if (protocol == PROTOCOL_TELEMETRY_SPEKTRUM || protocol == PROTOCOL_TELEMETRY_FLYSKY_IBUS || protocol == PROTOCOL_TELEMETRY_MULTIMODULE) {
    return false;
  }
#endif

#if !defined(AFHDS3)
  if (protocol == PROTOCOL_TELEMETRY_AFHDS3) {
    return false;
  }
#endif

#if defined(PCBHORUS)
  if (protocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
    return false;
  }
#endif

  return true;
}

bool isTrainerModeAvailable(int mode)
{
#if defined(PCBTARANIS)
  if (IS_EXTERNAL_MODULE_ENABLED() && (mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE || mode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE))
    return false;
#endif

#if defined(PCBX9E)
  if (g_eeGeneral.bluetoothMode && mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE) {
    // bluetooth uses the same USART than SBUS
    return false;
  }
#endif

#if defined(RADIO_TX16S) && defined(TRAINER_BATTERY_COMPARTMENT)
  if (mode == TRAINER_MODE_MASTER_BATTERY_COMPARTMENT)
    return (g_eeGeneral.auxSerialMode == UART_MODE_SBUS_TRAINER || g_eeGeneral.aux2SerialMode == UART_MODE_SBUS_TRAINER);
#elif defined(PCBTARANIS) && !defined(TRAINER_BATTERY_COMPARTMENT)
  if (mode == TRAINER_MODE_MASTER_BATTERY_COMPARTMENT)
    return false;
#elif defined(PCBTARANIS)
  if (mode == TRAINER_MODE_MASTER_BATTERY_COMPARTMENT)
    return g_eeGeneral.auxSerialMode == UART_MODE_SBUS_TRAINER;
#endif

#if defined(BLUETOOTH) && !defined(PCBX9E)
  if (g_eeGeneral.bluetoothMode != BLUETOOTH_TRAINER && (mode == TRAINER_MODE_MASTER_BLUETOOTH || mode == TRAINER_MODE_SLAVE_BLUETOOTH))
    return false;
#else
  if (mode == TRAINER_MODE_MASTER_BLUETOOTH || mode == TRAINER_MODE_SLAVE_BLUETOOTH)
    return false;
#endif

#if defined(PCBXLITE) && !defined(PCBXLITES)
  if (mode == TRAINER_MODE_MASTER_TRAINER_JACK || mode == TRAINER_MODE_SLAVE)
    return false;
#endif

#if defined(PCBTARANIS) || defined(PCBNV14) || defined(PCBPL18)
  #if !defined(TRAINER_MODULE_CPPM)
  if (mode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE)
    return false;
  #endif
  #if !defined(TRAINER_MODULE_SBUS)
  if (mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE)
    return false;
  #endif
#endif
  
  return true;
}

bool modelHasNotes()
{
  char filename[sizeof(MODELS_PATH)+1+sizeof(g_model.header.name)+sizeof(TEXT_EXT)] = MODELS_PATH "/";
  char *buf = strcat_currentmodelname(&filename[sizeof(MODELS_PATH)], 0);
  strcpy(buf, TEXT_EXT);
  if (isFileAvailable(filename)) {
    return true;
  }

  buf = strcat_currentmodelname(&filename[sizeof(MODELS_PATH)], ' ');
  strcpy(buf, TEXT_EXT);
  if (isFileAvailable(filename)) {
    return true;
  }

#if defined(STORAGE_MODELSLIST)
  buf = strAppendFilename(&filename[sizeof(MODELS_PATH)],
                          g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
  strcpy(buf, TEXT_EXT);
  if (isFileAvailable(filename)) {
    return true;
  }
#endif

  return false;
}

bool confirmModelChange()
{
  if (TELEMETRY_STREAMING()) {
    RAISE_ALERT(STR_MODEL, STR_MODEL_STILL_POWERED, STR_PRESS_ENTER_TO_CONFIRM, AU_MODEL_STILL_POWERED);
    while (TELEMETRY_STREAMING()) {
      RTOS_WAIT_MS(20);
      if (readKeys() == (1 << KEY_ENTER)) {
        killEvents(KEY_ENTER);
        return true;
      }
      else if (readKeys() == (1 << KEY_EXIT)) {
        killEvents(KEY_EXIT);
        return false;
      }
    }
  }
  return true;
}

int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable)
{
  int retval = 0;
  for (int i = min; i <= max; i++) {
    if (isValueAvailable(i)) {
      retval = i;
      break;
    }
  }
  return retval;
}
#if defined(MULTIMODULE)

// This maps OpenTX multi type with Pascal's Multi type
uint8_t multiConvertOtxToMulti(uint8_t moduleIdx, uint8_t type)
{

  // 15  for Multimodule is FrskyX or D16 which we map as a subprotocol of 3 (FrSky)
  // all protos > frskyx are therefore also off by one
  if (type >= 15)
    type = type + 1;

  // 25 is again a FrSky protocol (FrskyV) so shift again
  if (type >= 25)
    type = type + 1;

  if (type == MODULE_SUBTYPE_MULTI_FRSKY) {
    int subtype = g_model.moduleData[moduleIdx].subType;
    if (subtype == MM_RF_FRSKY_SUBTYPE_D8 || subtype == MM_RF_FRSKY_SUBTYPE_D8_CLONED) {
      //D8
      type = 3;
    }
    else if (subtype == MM_RF_FRSKY_SUBTYPE_V8) {
      //V8
      type = 25;
    }
    else {
      type = 15;
    }
  }
  return type;
}

// This maps multi type to OpenTX number, type ONLY (no subtype)
int convertMultiToOtx(int type)
{
  if  (type == 3) //FrSkyD
    return MODULE_SUBTYPE_MULTI_FRSKY;

  if  (type == 15) //FrSkyX
    return MODULE_SUBTYPE_MULTI_FRSKY;

  if  (type == 25) //FrSkyV
    return MODULE_SUBTYPE_MULTI_FRSKY;

  if (type > 25)
    type = type - 1;

  if (type > 15)
    type = type - 1;

  return type - 1; //Multi list starts at 1
}

// Third row is number of subtypes -1 (max valid subtype)
#define NO_SUBTYPE  nullptr

// Table is designed to be shared with companion multi.cpp

// The subtype and options strings are only referenced here, so
// define them here to avoid duplication in all language files
// Also since these strings are ARM only and likely stay ARM only
// we don't need the special eeprom/flash string handling, just define them as
// local strings

const char STR_SUBTYPE_FLYSKY[] =     "\004""Std\0""V9x9""V6x6""V912""CX20";
const char STR_SUBTYPE_HUBSAN[] =     "\004""H107""H301""H501";
const char STR_SUBTYPE_FRSKY[] =      "\011""D16\0     ""D8\0      ""D16 8ch\0 ""V8\0      ""LBT(EU)\0 ""LBT 8ch\0 ""D8Cloned\0""D16Cloned";
const char STR_SUBTYPE_HISKY[] =      "\005""Std\0 ""HK310";
const char STR_SUBTYPE_V2X2[] =       "\006""Std\0  ""JXD506""MR101\0";
const char STR_SUBTYPE_DSM[] =        "\004""2 1F""2 2F""X 1F""X 2F""Auto""R 1F";
const char STR_SUBTYPE_DEVO[] =       "\004""8ch\0""10ch""12ch""6ch\0""7ch\0";
const char STR_SUBTYPE_YD717[] =      "\007""Std\0   ""SkyWlkr""Syma X4""XINXUN\0""NIHUI\0 ";
const char STR_SUBTYPE_KN[] =         "\006""WLtoys""FeiLun";
const char STR_SUBTYPE_SYMAX[] =      "\003""Std""X5C";
const char STR_SUBTYPE_SLT[] =        "\006""V1_6ch""V2_8ch""Q100\0 ""Q200\0 ""MR100\0";
const char STR_SUBTYPE_CX10[] =       "\007""Green\0 ""Blue\0  ""DM007\0 ""-\0     ""JC3015a""JC3015b""MK33041";
const char STR_SUBTYPE_CG023[] =      "\005""Std\0 ""YD829";
const char STR_SUBTYPE_BAYANG[] =     "\007""Std\0   ""H8S3D\0 ""X16 AH\0""IRDrone""DHD D4\0""QX100\0 ";
const char STR_SUBTYPE_ESky[] =       "\003""Std""ET4";
const char STR_SUBTYPE_MT99[] =       "\006""MT99\0 ""H7\0   ""YZ\0   ""LS\0   ""FY805";
const char STR_SUBTYPE_MJXQ[] =       "\007""WLH08\0 ""X600\0  ""X800\0  ""H26D\0  ""E010\0  ""H26WH\0 ""Phoenix";
const char STR_SUBTYPE_FY326[] =      "\005""Std\0 ""FY319";
const char STR_SUBTYPE_FUTABA[] =     "\005""SFHSS";
const char STR_SUBTYPE_HONTAI[] =     "\007""Std\0   ""JJRC X1""X5C1\0  ""FQ_951";
const char STR_SUBTYPE_AFHDS2A[] =    "\010""PWM,IBUS""PPM,IBUS""PWM,SBUS""PPM,SBUS""PWM,IB16""PPM,IB16";
const char STR_SUBTYPE_Q2X2[] =       "\004""Q222""Q242""Q282";
const char STR_SUBTYPE_WK2x01[] =     "\006""WK2801""WK2401""W6_5_1""W6_6_1""W6_HeL""W6_HeI";
const char STR_SUBTYPE_Q303[] =       "\006""Std\0  ""CX35\0 ""CX10D\0""CX10WD";
const char STR_SUBTYPE_CABELL[] =     "\007""V3\0    ""V3 Telm""-\0     ""-\0     ""-\0     ""-\0     ""F-Safe\0""Unbind\0";
const char STR_SUBTYPE_ESKY150[] =    "\003""4ch""7ch";
const char STR_SUBTYPE_H83D[] =       "\007""Std\0   ""H20H\0  ""H20Mini""H30Mini";
const char STR_SUBTYPE_CORONA[] =     "\005""V1\0  ""V2\0  ""FD V3";
const char STR_SUBTYPE_HITEC[] =      "\007""Optima\0""Opt Hub""Minima\0";
const char STR_SUBTYPE_WFLY[] =       "\005""WFR0x";
const char STR_SUBTYPE_BUGS_MINI[] =  "\006""Std\0  ""Bugs3H";
const char STR_SUBTYPE_TRAXXAS[] =    "\004""6519";
const char STR_SUBTYPE_E01X[] =       "\005""E012\0""E015\0""E016H";
const char STR_SUBTYPE_V911S[] =      "\004""Std\0""E119";
const char STR_SUBTYPE_GD00X[] =      "\005""GD_V1""GD_V2";
const char STR_SUBTYPE_V761[] =       "\003""3ch""4ch";
const char STR_SUBTYPE_REDPINE[] =    "\004""Fast""Slow";
const char STR_SUBTYPE_POTENSIC[] =   "\003""A20";
const char STR_SUBTYPE_ZSX[] =        "\007""280JJRC";
const char STR_SUBTYPE_HEIGHT[] =     "\003""5ch""8ch";
const char STR_SUBTYPE_FRSKYX_RX[] =  "\007""RX\0    ""CloneTX";
const char STR_SUBTYPE_HOTT[] =       "\007""Sync\0  ""No_Sync";
const char STR_SUBTYPE_FX816[] =      "\003""P38";
const char STR_SUBTYPE_PELIKAN[] =    "\004""Pro\0""Lite";
const char STR_SUBTYPE_XK[] =         "\004""X450""X420";
const char STR_SUBTYPE_XN297DUMP[] =  "\004""250K""1M\0 ""2M\0 ""AUTO""NRF\0";
const char STR_SUBTYPE_FRSKYX2[] =    "\010""D16\0    ""D16 8ch\0""LBT(EU)\0""LBT 8ch\0""Cloned\0 ""Clone8ch";
const char STR_SUBTYPE_FRSKYR9[] =    "\007""915MHz\0""868MHz\0""915 8ch""868 8ch""FCC\0   ""---\0   ""FCC 8ch";
const char STR_SUBTYPE_PROPEL[] =     "\004""74-Z";
const char STR_SUBTYPE_FRSKYL[] =     "\010""LR12\0   ""LR12 6ch";
const char STR_SUBTYPE_ESKY150V2[] =  "\006""150 V2";
const char STR_SUBTYPE_JJRC345[] =    "\007""Std\0   ""SkyTmbr";
const char STR_SUBTYPE_KYOSHO[] =     "\004""FHSS""Hype";
const char STR_SUBTYPE_RLINK[] =      "\007""Surface""Air\0   ""DumboRC";
const char STR_SUBTYPE_ELRS[] =       "\007""N/A WIP";
const char STR_SUBTYPE_REALACC[] =    "\003""R11";
const char STR_SUBTYPE_WFLY2[] =      "\005""RF20x";

const char* mm_options_strings::options[] = {
  nullptr,
  STR_MULTI_OPTION,
  STR_MULTI_RFTUNE,
  STR_MULTI_VIDFREQ,
  STR_MULTI_FIXEDID,
  STR_MULTI_TELEMETRY,
  STR_MULTI_SERVOFREQ,
  STR_MULTI_MAX_THROW,
  STR_MULTI_RFCHAN,
  STR_MULTI_RFPOWER,
  STR_MULTI_WBUS
};

const uint8_t getMaxMultiOptions()
{
  return DIM(mm_options_strings::options);
}

const mm_protocol_definition multi_protocols[] = {
// Protocol as defined in pulses\modules_constants.h, number of sub_protocols - 1, Failsafe supported, Disable channel mapping supported, Subtype string, Option type
  {MODULE_SUBTYPE_MULTI_FLYSKY,     4, false, true,   STR_SUBTYPE_FLYSKY,    nullptr},
  {MODULE_SUBTYPE_MULTI_HUBSAN,     2, false, false,  STR_SUBTYPE_HUBSAN,    STR_MULTI_VIDFREQ},
  {MODULE_SUBTYPE_MULTI_FRSKY,      7, false, false,  STR_SUBTYPE_FRSKY,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HISKY,      1, false, true,   STR_SUBTYPE_HISKY,     nullptr},
  {MODULE_SUBTYPE_MULTI_V2X2,       2, false, false,  STR_SUBTYPE_V2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_DSM2,       5, false, true,   STR_SUBTYPE_DSM,       STR_MULTI_MAX_THROW},
  {MODULE_SUBTYPE_MULTI_DEVO,       4, false, true,   STR_SUBTYPE_DEVO,      STR_MULTI_FIXEDID},
  {MODULE_SUBTYPE_MULTI_YD717,      4, false, false,  STR_SUBTYPE_YD717,     nullptr},
  {MODULE_SUBTYPE_MULTI_KN,         1, false, false,  STR_SUBTYPE_KN,        nullptr},
  {MODULE_SUBTYPE_MULTI_SYMAX,      1, false, false,  STR_SUBTYPE_SYMAX,     nullptr},
  {MODULE_SUBTYPE_MULTI_SLT,        4, false, true,   STR_SUBTYPE_SLT,       STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_CX10,       6, false, false,  STR_SUBTYPE_CX10,      nullptr},
  {MODULE_SUBTYPE_MULTI_CG023,      1, false, false,  STR_SUBTYPE_CG023,     nullptr},
  {MODULE_SUBTYPE_MULTI_BAYANG,     5, false, false,  STR_SUBTYPE_BAYANG,    STR_MULTI_TELEMETRY},
  {MODULE_SUBTYPE_MULTI_ESky,       1, false, true,   STR_SUBTYPE_ESky,      nullptr},
  {MODULE_SUBTYPE_MULTI_MT99XX,     4, false, false,  STR_SUBTYPE_MT99,      nullptr},
  {MODULE_SUBTYPE_MULTI_MJXQ,       6, false, false,  STR_SUBTYPE_MJXQ,      STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FY326,      1, false, false,  STR_SUBTYPE_FY326,     nullptr},
  {MODULE_SUBTYPE_MULTI_FUTABA,     0, true,  true,   STR_SUBTYPE_FUTABA,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_J6PRO,      0, false, true,   NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_HONTAI,     3, false, false,  STR_SUBTYPE_HONTAI,    nullptr},
  {MODULE_SUBTYPE_MULTI_OLRS,       0, false, false,  NO_SUBTYPE,            STR_RF_POWER},
  {MODULE_SUBTYPE_MULTI_FS_AFHDS2A, 5, true,  true,   STR_SUBTYPE_AFHDS2A,   STR_MULTI_SERVOFREQ},
  {MODULE_SUBTYPE_MULTI_Q2X2,       2, false, false,  STR_SUBTYPE_Q2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_WK_2X01,    5, false, true,   STR_SUBTYPE_WK2x01,    nullptr},
  {MODULE_SUBTYPE_MULTI_Q303,       3, false, false,  STR_SUBTYPE_Q303,      nullptr},
  {MODULE_SUBTYPE_MULTI_CABELL,     7, false, false,  STR_SUBTYPE_CABELL,    STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_ESKY150,    1, false, false,  STR_SUBTYPE_ESKY150,   nullptr},
  {MODULE_SUBTYPE_MULTI_H83D,       3, false, false,  STR_SUBTYPE_H83D,      nullptr},
  {MODULE_SUBTYPE_MULTI_CORONA,     2, false, false,  STR_SUBTYPE_CORONA,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HITEC,      2, false, false,  STR_SUBTYPE_HITEC,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_WFLY,       0, false, false,  STR_SUBTYPE_WFLY,      nullptr},
  {MODULE_SUBTYPE_MULTI_BUGS_MINI,  1, false, false,  STR_SUBTYPE_BUGS_MINI, nullptr},
  {MODULE_SUBTYPE_MULTI_TRAXXAS,    0, false, false,  STR_SUBTYPE_TRAXXAS,   nullptr},
  {MODULE_SUBTYPE_MULTI_E01X,       2, false, false,  STR_SUBTYPE_E01X,      STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_V911S,      1, false, false,  STR_SUBTYPE_V911S,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_GD00X,      1, false, false,  STR_SUBTYPE_GD00X,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_V761,       1, false, false,  STR_SUBTYPE_V761,      nullptr},
  {MODULE_SUBTYPE_MULTI_KF606,      0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_REDPINE,    1, false, false,  STR_SUBTYPE_REDPINE,   STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_POTENSIC,   0, false, false,  STR_SUBTYPE_POTENSIC,  nullptr},
  {MODULE_SUBTYPE_MULTI_ZSX,        0, false, false,  STR_SUBTYPE_ZSX,       nullptr},
  {MODULE_SUBTYPE_MULTI_HEIGHT,     1, false, false,  STR_SUBTYPE_HEIGHT,    nullptr},
  {MODULE_SUBTYPE_MULTI_FRSKYX_RX,  1, false, false,  STR_SUBTYPE_FRSKYX_RX, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HOTT,       1, true,  false,  STR_SUBTYPE_HOTT,      STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FX816,      0, false, false,  STR_SUBTYPE_FX816,     nullptr},
  {MODULE_SUBTYPE_MULTI_PELIKAN,    1, false, true,   STR_SUBTYPE_PELIKAN,   nullptr},
  {MODULE_SUBTYPE_MULTI_XK,         1, false, false,  STR_SUBTYPE_XK,        STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_XN297DUMP,  4, false, false,  STR_SUBTYPE_XN297DUMP, STR_MULTI_RFCHAN},
  {MODULE_SUBTYPE_MULTI_FRSKYX2,    5, true,  false,  STR_SUBTYPE_FRSKYX2,   STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FRSKY_R9,   6, true,  false,  STR_SUBTYPE_FRSKYR9,   nullptr},
  {MODULE_SUBTYPE_MULTI_PROPEL,     0, false, false,  STR_SUBTYPE_PROPEL,    nullptr},
  {MODULE_SUBTYPE_MULTI_FRSKYL,     1, false, false,  STR_SUBTYPE_FRSKYL,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_SKYARTEC,   0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ESKY150V2,  0, false, true,   STR_SUBTYPE_ESKY150V2, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_JJRC345,    1, false, false,  STR_SUBTYPE_JJRC345,   nullptr},
  {MODULE_SUBTYPE_MULTI_Q90C,       0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_KYOSHO,     1, false, true,   STR_SUBTYPE_KYOSHO,    nullptr},
  {MODULE_SUBTYPE_MULTI_RLINK,      2, false, false,  STR_SUBTYPE_RLINK,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ELRS,       0, false, false,  STR_SUBTYPE_ELRS,      nullptr},
  {MODULE_SUBTYPE_MULTI_REALACC,    0, false, false,  STR_SUBTYPE_REALACC,   nullptr},
  {MODULE_SUBTYPE_MULTI_OMP,        0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_WFLY2,      0, false, false,  STR_SUBTYPE_WFLY2,     STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_E016HV2,    0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MM_RF_CUSTOM_SELECTED,           7, true,  true,   NO_SUBTYPE,            STR_MULTI_OPTION},

  // Sentinel and default for protocols not listed above (MM_RF_CUSTOM is 0xff)
  {0xfe,                            0, false, false,   NO_SUBTYPE,            nullptr}
};

#undef NO_SUBTYPE

const mm_protocol_definition *getMultiProtocolDefinition (uint8_t protocol)
{
  const mm_protocol_definition *pdef;
  for (pdef = multi_protocols; pdef->protocol != 0xfe; pdef++) {
    if (pdef->protocol == protocol)
      return pdef;
  }
  // Return the empty last protocol
  return pdef;
}
#endif

#if defined(MULTIMODULE)
const char * getMultiOptionTitleStatic(uint8_t moduleIdx)
{
  const uint8_t multi_proto = g_model.moduleData[moduleIdx].getMultiProtocol();
  const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_proto);
  return pdef->optionsstr;
}

const char * getMultiOptionTitle(uint8_t moduleIdx)
{
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);

  if (status.isValid()) {
    if (status.optionDisp >= getMaxMultiOptions()) {
      status.optionDisp = 1; // Unknown options are defaulted to type 1 (basic option)
    }
    return mm_options_strings::options[status.optionDisp];
  }

  return getMultiOptionTitleStatic(moduleIdx);
}
#endif
