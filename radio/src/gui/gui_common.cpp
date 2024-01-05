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

#include "hal/module_port.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "hal/trainer_driver.h"

#include "opentx.h"
#include "switches.h"
#include "mixes.h"

#undef CPN
#include "MultiSubtypeDefs.h"

uint8_t switchToMix(uint8_t source)
{
  div_t qr = div(source-1, 3);
  return qr.quot + MIXSRC_FIRST_SWITCH;
}

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
    if (modelCustomScriptsEnabled()) {
      div_t qr = div(source - MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
      return (qr.rem < scriptInputsOutputs[qr.quot].outputsCount);
    } else {
      return false;
    }
  }
#elif defined(LUA_INPUTS)
  if (source >= MIXSRC_FIRST_LUA && source <= MIXSRC_LAST_LUA)
    return false;
#endif

  if (source >= MIXSRC_FIRST_STICK && source <= MIXSRC_LAST_STICK) {
    auto idx = source - MIXSRC_FIRST_STICK;
    return idx < adcGetMaxInputs(ADC_INPUT_MAIN);
  }

  if (source >= MIXSRC_FIRST_POT && source <= MIXSRC_LAST_POT) {
    return IS_POT_SLIDER_AVAILABLE(source - MIXSRC_FIRST_POT);
  }

#if defined(PCBHORUS) && !defined(SPACEMOUSE)
  if (source >= MIXSRC_FIRST_SPACEMOUSE && source <= MIXSRC_LAST_SPACEMOUSE)
    return false;
#elif defined(PCBHORUS) && defined(SPACEMOUSE)
  if ((serialGetModePort(UART_MODE_SPACEMOUSE) < 0) &&
      (source >= MIXSRC_FIRST_SPACEMOUSE && source <= MIXSRC_LAST_SPACEMOUSE))
    return false;
#endif

  if (source >= MIXSRC_FIRST_SWITCH && source <= MIXSRC_LAST_SWITCH) {
    return SWITCH_EXISTS(source - MIXSRC_FIRST_SWITCH);
  }

#if !defined(HELI)
  if (source >= MIXSRC_FIRST_HELI && source <= MIXSRC_LAST_HELI)
    return false;
#else
  if (!modelHeliEnabled() && source >= MIXSRC_FIRST_HELI && source <= MIXSRC_LAST_HELI)
    return false;
#endif

  if (source >= MIXSRC_FIRST_TRIM && source <= MIXSRC_LAST_TRIM) {
    return (source - MIXSRC_FIRST_TRIM) < keysGetMaxTrims();
  }

  if (source >= MIXSRC_FIRST_LOGICAL_SWITCH && source <= MIXSRC_LAST_LOGICAL_SWITCH) {
    LogicalSwitchData * cs = lswAddress(source - MIXSRC_FIRST_LOGICAL_SWITCH);
    return (cs->func != LS_FUNC_NONE);
  }

  if (source >= MIXSRC_FIRST_TRAINER && source <= MIXSRC_LAST_TRAINER)
    return g_model.trainerData.mode > 0;

  if (source >= MIXSRC_FIRST_CH && source <= MIXSRC_LAST_CH) {
    return isChannelUsed(source - MIXSRC_FIRST_CH);
  }

#if !defined(GVARS)
  if (source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR)
    return false;
#else
  if (!modelGVEnabled() && source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR)
    return false;
#endif

  // TX VOLTAGE, TIME and GPS are always true

  if (source >= MIXSRC_FIRST_TIMER && source <= MIXSRC_LAST_TIMER) {
    TimerData *timer = &g_model.timers[source - MIXSRC_FIRST_TIMER];
    return timer->mode != 0;
  }

  if (source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    if (!modelTelemetryEnabled())
      return false;
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

  if (result && modelTelemetryEnabled() && source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    div_t qr = div(source - MIXSRC_FIRST_TELEM, 3);
    result = isTelemetryFieldComparisonAvailable(qr.quot);
  }

  return result;
}

bool isSourceAvailableInInputs(int source)
{
  if (source >= MIXSRC_FIRST_STICK && source <= MIXSRC_LAST_STICK) {
    auto idx = source - MIXSRC_FIRST_STICK;
    return idx < adcGetMaxInputs(ADC_INPUT_MAIN);
  }

  if (source >= MIXSRC_FIRST_POT && source <= MIXSRC_LAST_POT)
    return IS_POT_SLIDER_AVAILABLE(source - MIXSRC_FIRST_POT);

#if defined(IMU)
  if (source == MIXSRC_TILT_X || source == MIXSRC_TILT_Y)
    return true;
#endif

#if defined(PCBHORUS) && defined(SPACEMOUSE)
  if (source >= MIXSRC_FIRST_SPACEMOUSE && source <= MIXSRC_LAST_SPACEMOUSE)
    return true;
#endif
  if (source == MIXSRC_MIN || source == MIXSRC_MAX)
    return true;

  if (source >= MIXSRC_FIRST_TRIM && source <= MIXSRC_LAST_TRIM) {
    auto idx = source - MIXSRC_FIRST_TRIM;
    return idx < keysGetMaxTrims();
  }

  if (source >= MIXSRC_FIRST_SWITCH && source <= MIXSRC_LAST_SWITCH)
    return SWITCH_EXISTS(source - MIXSRC_FIRST_SWITCH);

  if (source >= MIXSRC_FIRST_CH && source <= MIXSRC_LAST_CH)
    return true;

  if (source >= MIXSRC_FIRST_LOGICAL_SWITCH && source <= MIXSRC_LAST_LOGICAL_SWITCH) {
    LogicalSwitchData * cs = lswAddress(source - MIXSRC_FIRST_LOGICAL_SWITCH);
    return (cs->func != LS_FUNC_NONE);
  }

  if (source >= MIXSRC_FIRST_TRAINER && source <= MIXSRC_LAST_TRAINER)
    return g_model.trainerData.mode > 0;

  if (modelTelemetryEnabled() && source >= MIXSRC_FIRST_TELEM && source <= MIXSRC_LAST_TELEM) {
    div_t qr = div(source - MIXSRC_FIRST_TELEM, 3);
    return isTelemetryFieldAvailable(qr.quot) && isTelemetryFieldComparisonAvailable(qr.quot);
  }

  if (modelGVEnabled() && source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR)
    return true;

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
    if (swinfo.quot >= switchGetMaxSwitches() + switchGetMaxFctSwitches()) {
      return false;
    }

    if (!SWITCH_EXISTS(swinfo.quot)) {
      return false;
    }

    if (IS_SWITCH_FS(swinfo.quot) && context == GeneralCustomFunctionsContext) {
      return false;   // FS are defined at model level, and cannot be in global functions
    }

    if (!IS_CONFIG_3POS(swinfo.quot)) {
      if (swinfo.rem == 1) {
        // mid position not available for 2POS switches
        return false;
      }
    }
    return true;
  }

  if (swtch >= SWSRC_FIRST_MULTIPOS_SWITCH && swtch <= SWSRC_LAST_MULTIPOS_SWITCH) {
    int index = (swtch - SWSRC_FIRST_MULTIPOS_SWITCH) / XPOTS_MULTIPOS_COUNT;
    return (index < adcGetMaxInputs(ADC_INPUT_FLEX)) ? IS_POT_MULTIPOS(index) : false;
  }

  if (swtch >= SWSRC_FIRST_TRIM && swtch <= SWSRC_LAST_TRIM) {
    int index = (swtch - SWSRC_FIRST_TRIM) / 2;
    return index < keysGetMaxTrims();
  }
  
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

bool isSerialModeAvailable(uint8_t port_nr, int mode)
{
#if defined(USB_SERIAL)
  // Do not list OFF on VCP if internal RF module is set to CROSSFIRE to allow pass-through flashing
  if (port_nr == SP_VCP && mode == UART_MODE_NONE && isInternalModuleCrossfire())
    return false;
#endif

  if (mode == UART_MODE_NONE)
    return true;

#if !defined(DEBUG)
  if (mode == UART_MODE_DEBUG)
    return false;
#endif

#if !defined(CLI)
  if (mode == UART_MODE_CLI)
    return false;
#endif

#if !defined(INTERNAL_GPS)
  if (mode == UART_MODE_GPS)
    return false;
#elif defined(USB_SERIAL)
  // GPS is not supported on VCP
  if (port_nr == SP_VCP && mode == UART_MODE_GPS)
    return false;
#endif

#if !defined(SPACEMOUSE)
  if (mode == UART_MODE_SPACEMOUSE)
    return false;
#elif defined(USB_SERIAL)
  // SPACEMOUSE is not supported on VCP
  if (port_nr == SP_VCP && mode == UART_MODE_SPACEMOUSE)
    return false;
#endif

#if !defined(AUX_SERIAL_DMA_TX) || defined(EXTMODULE_USART)
  if (mode == UART_MODE_EXT_MODULE)
    return false;
#else // defined(AUX_SERIAL_DMA_TX) && !defined(EXTMODULE_USART)
  // UART_MODE_EXT_MODULE is only supported on AUX1, as AUX2 has no TX DMA
  if (mode == UART_MODE_EXT_MODULE && port_nr != SP_AUX1)
    return false;
#endif

#if !defined(LUA)
  if (mode == UART_MODE_LUA)
    return false;
#endif

#if defined(USB_SERIAL)
  // Telemetry input & SBUS trainer on VCP is not yet supported
  if (port_nr == SP_VCP &&
      (mode == UART_MODE_TELEMETRY || mode == UART_MODE_SBUS_TRAINER))
    return false;
#endif

  auto p = serialGetModePort(mode);
  if (p >= 0 && p != port_nr) return false;
  return true;
}

bool hasSportPower() {
  auto mod_desc = modulePortGetModuleDescription(SPORT_MODULE);
  if (mod_desc && mod_desc->set_pwr) {
    return true;
  }
  return false;
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

bool isThrottleSourceAvailable(int src)
{
#if !defined(LIBOPENUI)
  src = throttleSource2Source(src);
#endif
  return isSourceAvailable(src) &&
    ((src == MIXSRC_FIRST_STICK + inputMappingGetThrottle()) ||
     ((src >= MIXSRC_FIRST_POT) && (src <= MIXSRC_LAST_POT)) ||
     ((src >= MIXSRC_FIRST_CH) && (src <= MIXSRC_LAST_CH)));
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
      return false;
#endif
#if !defined(DANGEROUS_MODULE_FUNCTIONS)
    case FUNC_RANGECHECK:
    case FUNC_BIND:
      return false;
#endif
#if !defined(LUA)
    case FUNC_PLAY_SCRIPT:
      return false;
#endif
#if !defined(AUDIO_MUTE_GPIO)
    case FUNC_DISABLE_AUDIO_AMP:
      return false;
#endif
#if !defined(LED_STRIP_GPIO)
    case FUNC_RGB_LED:
      return false;
#endif
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

bool isTimerSourceAvailable(int index)
{
  TimerData *timer = &g_model.timers[index];
  return timer->mode != 0;
}

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
  else if (index <= FUNC_RESET_TIMER3) {
    if (index > (TIMERS - 1))
      return false;
    else {
      TimerData *timer = &g_model.timers[index];
      return timer->mode != 0;
    }
  }
  else {
    return true;
  }
}

#if defined(EXTERNAL_ANTENNA) && defined(INTERNAL_MODULE_PXX1)

#if defined(COLORLCD)

class AntennaSelectionMenu : public Menu
{
  bool& done;

public:
  AntennaSelectionMenu(bool& done) : Menu(MainWindow::instance()), done(done) {
    setTitle(STR_ANTENNA);
    addLine(STR_USE_INTERNAL_ANTENNA,
            [] { globalData.externalAntennaEnabled = false; });
    addLine(STR_USE_EXTERNAL_ANTENNA,
            [] { globalData.externalAntennaEnabled = true; });
    setCloseHandler([=]() { this->done = true; });
    setCloseWhenClickOutside(false);
  }
protected:
  void onCancel() override {}
};

static void runAntennaSelectionMenu()
{
  bool finished = false;
  new AntennaSelectionMenu(finished);

  while (!finished) {
    WDG_RESET();
    MainWindow::instance()->run();
    LvglWrapper::runNested();
    RTOS_WAIT_MS(20);
  }
}
#else
void onAntennaSelection(const char* result)
{
  if (result == STR_USE_INTERNAL_ANTENNA) {
    globalData.externalAntennaEnabled = false;
  } else if (result == STR_USE_EXTERNAL_ANTENNA) {
    globalData.externalAntennaEnabled = true;
  } else {
    checkExternalAntenna();
  }
}

void onAntennaSwitchConfirm(const char * result)
{
  if (result == STR_OK) {
    // Switch to external antenna confirmation
    globalData.externalAntennaEnabled = true;
  }
}
#endif

void checkExternalAntenna()
{
  if (isModuleXJT(INTERNAL_MODULE)) {
    if (g_eeGeneral.antennaMode == ANTENNA_MODE_EXTERNAL) {
      // TRACE("checkExternalAntenna(): External");
      globalData.externalAntennaEnabled = true;
    } else if (g_eeGeneral.antennaMode == ANTENNA_MODE_PER_MODEL &&
               g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode ==
                   ANTENNA_MODE_EXTERNAL) {
      // TRACE("checkExternalAntenna(): Per Model, External");
      if (!globalData.externalAntennaEnabled) {
#if defined(COLORLCD)
        if (confirmationDialog(STR_ANTENNACONFIRM1, STR_ANTENNACONFIRM2)) {
          globalData.externalAntennaEnabled = true;
        }
#else
        POPUP_CONFIRMATION(STR_ANTENNACONFIRM1, onAntennaSwitchConfirm);
        SET_WARNING_INFO(STR_ANTENNACONFIRM2, sizeof(TR_ANTENNACONFIRM2), 0);
#endif
      }
    } else if (g_eeGeneral.antennaMode == ANTENNA_MODE_ASK ||
               (g_eeGeneral.antennaMode == ANTENNA_MODE_PER_MODEL &&
                g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode ==
                    ANTENNA_MODE_ASK)) {

      // TRACE("checkExternalAntenna(): Ask");
      globalData.externalAntennaEnabled = false;

#if defined(COLORLCD)
      runAntennaSelectionMenu();
#else
      POPUP_MENU_ADD_ITEM(STR_USE_INTERNAL_ANTENNA);
      POPUP_MENU_ADD_ITEM(STR_USE_EXTERNAL_ANTENNA);
      POPUP_MENU_START(onAntennaSelection);
#endif
    } else {
      globalData.externalAntennaEnabled = false;
    }
  } else {
    globalData.externalAntennaEnabled = false;
  }
}
#endif

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
  if (g_model.trainerData.mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE ||
      g_model.trainerData.mode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) {
    return true;
  }

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
    case MODULE_TYPE_FLYSKY_AFHDS2A:
    case MODULE_TYPE_FLYSKY_AFHDS3:
      return false;

    case MODULE_TYPE_XJT_PXX1:
      // External XJT has a physical switch to disable S.PORT
    case MODULE_TYPE_R9M_PXX1:
      // R9M telemetry is disabled by pulses (pxx1.cpp)
#if defined(HARDWARE_EXTERNAL_MODULE)
      if (moduleBay == EXTERNAL_MODULE)
        return false;
#endif

    case MODULE_TYPE_CROSSFIRE:
#if defined(HARDWARE_INTERNAL_MODULE)
      if (moduleBay == INTERNAL_MODULE)
        return false;
#endif

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
#if defined(INTERNAL_MODULE_AFHDS2A)
  case MODULE_TYPE_FLYSKY_AFHDS2A: return true;
#endif
#if defined(INTERNAL_MODULE_AFHDS3)
  case MODULE_TYPE_FLYSKY_AFHDS3: return true;
#endif
  }
  return false;
}

bool isInternalModuleAvailable(int moduleType)
{
#if defined(MUTUALLY_EXCLUSIVE_MODULES)
  if (!isModuleNone(EXTERNAL_MODULE))
    return false;
#endif

  if (moduleType == MODULE_TYPE_NONE)
    return true;

  if (g_eeGeneral.internalModule != moduleType)
    return false;

#if defined(INTERNAL_MODULE_PXX1) && defined(HARDWARE_EXTERNAL_MODULE)
  if ((moduleType == MODULE_TYPE_XJT_PXX1) &&
      isModuleUsingSport(EXTERNAL_MODULE,
                         g_model.moduleData[EXTERNAL_MODULE].type)) {
    return false;
  }
#endif

#if defined(INTERNAL_MODULE_PXX2) && defined(HARDWARE_EXTERNAL_MODULE)
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

#if defined(HARDWARE_EXTERNAL_MODULE)
bool isExternalModuleAvailable(int moduleType)
{

#if defined(MUTUALLY_EXCLUSIVE_MODULES)
  if (!isModuleNone(INTERNAL_MODULE))
    return false;
#endif

#if !defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
  if (isModuleTypeR9MLite(moduleType) || moduleType == MODULE_TYPE_XJT_LITE_PXX2)
    return false;
#endif

#if !defined(PXX1)
  if (isModuleTypePXX1(moduleType))
    return false;
#endif

#if defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML) and !defined(EXTMODULE_USART_GPIO)
  if (moduleType == MODULE_TYPE_XJT_LITE_PXX2 ||
      moduleType == MODULE_TYPE_R9M_PXX2)
    return false;
#endif

#if !defined(HARDWARE_EXTERNAL_MODULE_SIZE_STD)
  if (moduleType == MODULE_TYPE_R9M_PXX1 ||
      moduleType == MODULE_TYPE_R9M_PXX2 ||
      moduleType == MODULE_TYPE_XJT_PXX1 ||
      moduleType == MODULE_TYPE_DSM2 ||
      moduleType == MODULE_TYPE_LEMON_DSMP )
    return false;
#endif

  if (moduleType == MODULE_TYPE_ISRM_PXX2)
    return false; // doesn't exist for now


  if (moduleType == MODULE_TYPE_XJT_LITE_PXX2 ||
      moduleType == MODULE_TYPE_R9M_PXX2 ||
      moduleType == MODULE_TYPE_R9M_LITE_PXX2 ||
      moduleType == MODULE_TYPE_R9M_LITE_PRO_PXX2) {

#if defined(PXX2)
    return modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
                          ETX_MOD_PORT_UART, ETX_Pol_Normal,
                          ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX);
#else
    return false;
#endif
  }

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
  if (moduleType == MODULE_TYPE_FLYSKY_AFHDS3)
    return false;
#endif

#if !defined(AFHDS2)
  if (moduleType == MODULE_TYPE_FLYSKY_AFHDS2A)
    return false;
#endif
  
#if !defined(AFHDS3)
  if (moduleType == MODULE_TYPE_FLYSKY_AFHDS3)
    return false;
#endif

  return true;
}

#else // !defined(HARDWARE_EXTERNAL_MODULE)

bool isExternalModuleAvailable(int moduleType)
{
  return false;
}
#endif

bool isRfProtocolAvailable(int protocol)
{
#if defined(CROSSFIRE) && defined(HARDWARE_EXTERNAL_MODULE)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE) {
    return false;
  }
#endif

#if defined(GHOST) && defined(HARDWARE_EXTERNAL_MODULE)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_GHOST) {
    return false;
  }
#endif

#if !defined(MODULE_PROTOCOL_D8)
  if (protocol == MODULE_SUBTYPE_PXX1_ACCST_D8) {
    return false;
  }
#endif

#if (defined(PCBTARANIS) || defined(PCBHORUS)) && defined(HARDWARE_EXTERNAL_MODULE)
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_PXX1) {
    return false;
  }
  if (protocol != MODULE_SUBTYPE_PXX1_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_R9M_PXX2) {
    return false;
  }
#endif

  return true;
}

bool isTrainerModeAvailable(int mode)
{
#if defined(PCBX9E)
  if (g_eeGeneral.bluetoothMode &&
      mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE) {
    // bluetooth uses the same USART than SBUS
    return false;
  }
#endif

  if (mode == TRAINER_MODE_MASTER_SERIAL) {
#if defined(SBUS_TRAINER)
    return serialGetModePort(UART_MODE_SBUS_TRAINER) >= 0;
#else
    return false;
#endif
  }

  if ((mode == TRAINER_MODE_MASTER_BLUETOOTH ||
       mode == TRAINER_MODE_SLAVE_BLUETOOTH)
#if defined(BLUETOOTH) && !defined(PCBX9E)
      && g_eeGeneral.bluetoothMode != BLUETOOTH_TRAINER
#endif
  )
    return false;

  if ((mode == TRAINER_MODE_MASTER_TRAINER_JACK ||
       mode == TRAINER_MODE_SLAVE) &&
      !trainer_dsc_available())
    return false;

  if (mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE ||
      mode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) {

    // no external module or is enabled
    if (!modulePortGetModuleDescription(EXTERNAL_MODULE) ||
        IS_EXTERNAL_MODULE_ENABLED()) {
      return false;
    }

    if (mode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) {
      auto port =  modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_TIMER,
                                  ETX_MOD_PORT_TIMER, ETX_Pol_Normal,
                                  ETX_MOD_DIR_RX);
      return port != nullptr;      
    }
    
    if (mode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE) {
      auto port =  modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
                                  ETX_MOD_PORT_UART, ETX_Pol_Normal,
                                  ETX_MOD_DIR_RX);
      return port != nullptr;
    }
  }

  if (mode == TRAINER_MODE_MULTI) {

#if !defined(MULTIMODULE)
    return false;
#else
  if ((!IS_INTERNAL_MODULE_ENABLED() && !IS_EXTERNAL_MODULE_ENABLED()) ||
       (!isModuleMultimodule(INTERNAL_MODULE) &&
        !isModuleMultimodule(EXTERNAL_MODULE)))
    return false;
#endif
  }

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

const uint8_t getMaxMultiOptions()
{
  return DIM(mm_options_strings::options);
}

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

const char * getMultiOptionTitleStatic(uint8_t moduleIdx)
{
  const uint8_t multi_proto = g_model.moduleData[moduleIdx].multi.rfProtocol;
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

#if !defined(COLORLCD)
uint8_t expandableSection(coord_t y, const char* title, uint8_t value, uint8_t attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, title);
  lcdDrawText(LCD_W == 128 ? 120 : 200, y, value ? STR_CHAR_UP : STR_CHAR_DOWN, attr);
  if (attr && (event == EVT_KEY_BREAK(KEY_ENTER))) {
    value = !value;
    s_editMode = 0;
  }
  return value;
}
#endif

bool isPotTypeAvailable(uint8_t type)
{
  if (type == FLEX_SWITCH) {
    if (MAX_FLEX_SWITCHES == 0)
      return false;

    auto availableFlexSwitch = MAX_FLEX_SWITCHES;
    for (uint8_t i = 0; i < adcGetMaxInputs(ADC_INPUT_FLEX); i++) {
      if (POT_CONFIG(i) == FLEX_SWITCH) availableFlexSwitch--;
      if (availableFlexSwitch == 0) return false;
    }
  }

  return true;
}

bool isFlexSwitchSourceValid(int source)
{
  if (MAX_FLEX_SWITCHES == 0) return false;

  // Allow NONE
  if (source < 0) return true;

  // already assigned ?
  for (int i=0; i < MAX_FLEX_SWITCHES;i++) {
    if (source == switchGetFlexConfig_raw(i))
      return false;
  }

  if (POT_CONFIG(source) != FLEX_SWITCH) return false;

  return true;
}

bool getPotInversion(int index)
{
  return bfGet<potconfig_t>(g_eeGeneral.potsConfig, (POT_CFG_BITS * index) + POT_CFG_TYPE_BITS, POT_CFG_INV_BITS);
}

void setPotInversion(int index, bool value)
{
  g_eeGeneral.potsConfig = bfSet<potconfig_t>(g_eeGeneral.potsConfig, value, (POT_CFG_BITS * index) + POT_CFG_TYPE_BITS, POT_CFG_INV_BITS);
}

uint8_t getPotType(int index)
{
  return bfGet<potconfig_t>(g_eeGeneral.potsConfig, POT_CFG_BITS * index, POT_CFG_TYPE_BITS);
}

void setPotType(int index, int value)
{
  g_eeGeneral.potsConfig = bfSet<potconfig_t>(g_eeGeneral.potsConfig, value, (POT_CFG_BITS * index), POT_CFG_TYPE_BITS);
}
