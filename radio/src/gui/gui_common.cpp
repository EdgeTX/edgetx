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

#include "edgetx.h"
#include "switches.h"
#include "mixes.h"

#undef CPN
#include "MultiSubtypeDefs.h"

uint8_t switchToMix(uint8_t source)
{
  div_t qr = div(source-1, 3);
  return qr.quot + MIXSRC_FIRST_SWITCH;
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

bool isVarioSensorAvailable(int sensor)
{
  if (sensor == 0)
    return true;
  else {
    return (isSensorAvailable(sensor) && (isSensorUnit(sensor, UNIT_METERS_PER_SECOND) || isSensorUnit(sensor, UNIT_FEET_PER_SECOND)));
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

static bool sourceIsAvailable(int source) { return true; }

static bool isSourceLuaAvailable(int source) {
#if defined(LUA_MODEL_SCRIPTS)
  if (modelCustomScriptsEnabled()) {
    div_t qr = div(source, MAX_SCRIPT_OUTPUTS);
    return (qr.rem < scriptInputsOutputs[qr.quot].outputsCount);
  }
#endif
  return false;
}

static bool isSourceStickAvailable(int source) {
  return source < adcGetMaxInputs(ADC_INPUT_MAIN);
}

static bool isSourcePotAvailable(int source) {
  return IS_POT_SLIDER_AVAILABLE(source);
}

#if defined(PCBHORUS)
static bool isSourceSpacemouseAvailable(int source) {
#if defined(SPACEMOUSE)
  return serialGetModePort(UART_MODE_SPACEMOUSE) >= 0;
#else
  return false;
#endif
}
#endif

static bool isSourceHeliAvailable(int source) {
#if defined(HELI)
  return modelHeliEnabled();
#else
  return false;
#endif
}

static bool isSourceTrimAvailable(int source) {
  return source < keysGetMaxTrims();
}

static bool isSourceSwitchAvailable(int source) {
  return SWITCH_EXISTS(source);
}

#if defined(FUNCTION_SWITCHES)
static bool isSourceFuncSwitchAvailable(int source) {
  return getSwitchCountInFSGroup(source + 1) > 0;
}
#endif

static bool isSourceLSAvailable(int source) {
  LogicalSwitchData * cs = lswAddress(source);
  return (cs->func != LS_FUNC_NONE);
}

static bool isSourceTrainerAvailable(int source) {
  return g_model.trainerData.mode > 0;
}

static bool isSourceGvarAvailable(int source) {
#if defined(GVARS)
  return modelGVEnabled();
#else
  return false;
#endif
}

static bool isSourceTimerAvailable(int source) {
  TimerData *timer = &g_model.timers[source];
  return timer->mode != 0;
}

static bool isSourceTelemAvailable(int source) {
  if (!modelTelemetryEnabled())
    return false;
  div_t qr = div(source, 3);
  if (qr.rem == 0)
    return isTelemetryFieldAvailable(qr.quot);
  else
    return isTelemetryFieldComparisonAvailable(qr.quot);
}

static bool isSourceTelemCompAvailable(int source) {
  if (!modelTelemetryEnabled())
    return false;
  div_t qr = div(source, 3);
  return isTelemetryFieldComparisonAvailable(qr.quot);
}

enum SrcTypes {
  SRC_INPUT = 1 << 0,
  SRC_LUA = 1 << 1,
  SRC_STICK = 1 << 2,
  SRC_POT = 1 << 3,
  SRC_TILT = 1 << 4,
  SRC_SPACEMOUSE = 1 << 5,
  SRC_MINMAX = 1 << 6,
  SRC_HELI = 1 << 7,
  SRC_TRIM = 1 << 8,
  SRC_SWITCH = 1 << 9,
  SRC_FUNC_SWITCH = 1 << 10,
  SRC_LOGICAL_SWITCH = 1 << 11,
  SRC_TRAINER = 1 << 12,
  SRC_CHANNEL = 1 << 13,
  SRC_CHANNEL_ALL = 1 << 14,
  SRC_GVAR = 1 << 15,
  SRC_TX = 1 << 16,
  SRC_TIMER = 1 << 17,
  SRC_TELEM = 1 << 18,
  SRC_TELEM_COMP = 1 << 19,
};

struct sourceAvailableCheck {
  uint16_t first;
  uint16_t last;
  SrcTypes type;
  bool (*check)(int);
};

static struct sourceAvailableCheck sourceChecks[] = {
  { MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, SRC_INPUT, isInputAvailable },
  { MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, SRC_LUA, isSourceLuaAvailable },
  { MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK, SRC_STICK, isSourceStickAvailable },
  { MIXSRC_FIRST_POT, MIXSRC_LAST_POT, SRC_POT, isSourcePotAvailable },
#if defined(IMU)
  { MIXSRC_TILT_X, MIXSRC_TILT_Y, SRC_TILT, sourceIsAvailable },
#endif
#if defined(PCBHORUS)
  { MIXSRC_FIRST_SPACEMOUSE, MIXSRC_LAST_SPACEMOUSE, SRC_SPACEMOUSE, isSourceSpacemouseAvailable },
#endif
  { MIXSRC_MIN, MIXSRC_MAX, SRC_MINMAX, sourceIsAvailable },
  { MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, SRC_HELI, isSourceHeliAvailable },
  { MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM, SRC_TRIM, isSourceTrimAvailable },
  { MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH, SRC_SWITCH, isSourceSwitchAvailable },
#if defined(FUNCTION_SWITCHES)
  { MIXSRC_FIRST_CUSTOMSWITCH_GROUP, MIXSRC_LAST_CUSTOMSWITCH_GROUP, SRC_FUNC_SWITCH, isSourceFuncSwitchAvailable },
#endif
  { MIXSRC_FIRST_LOGICAL_SWITCH, MIXSRC_LAST_LOGICAL_SWITCH, SRC_LOGICAL_SWITCH, isSourceLSAvailable },
  { MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER, SRC_TRAINER, isSourceTrainerAvailable },
  { MIXSRC_FIRST_CH, MIXSRC_LAST_CH, SRC_CHANNEL, isChannelUsed },
  { MIXSRC_FIRST_CH, MIXSRC_LAST_CH, SRC_CHANNEL_ALL, sourceIsAvailable },
  { MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, SRC_GVAR, isSourceGvarAvailable },
  { MIXSRC_TX_VOLTAGE, MIXSRC_TX_GPS, SRC_TX, sourceIsAvailable },
  { MIXSRC_FIRST_TIMER, MIXSRC_LAST_TIMER, SRC_TIMER, isSourceTimerAvailable },
  { MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM, SRC_TELEM, isSourceTelemAvailable },
  { MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM, SRC_TELEM_COMP, isSourceTelemCompAvailable },
};

bool checkSourceAvailable(int source, uint32_t sourceTypes)
{
  if (source < 0)
    source = -source;

  for (size_t i = 0 ; i < DIM(sourceChecks); i += 1) {
    if (sourceChecks[i].type & sourceTypes && source >= sourceChecks[i].first && source <= sourceChecks[i].last) {
      return sourceChecks[i].check(source - sourceChecks[i].first);
    }
  }

  return false;
}

#define SRC_COMMON \
            SRC_STICK | SRC_POT | SRC_TILT | SRC_SPACEMOUSE | SRC_MINMAX | SRC_TRIM | \
            SRC_SWITCH | SRC_FUNC_SWITCH | SRC_LOGICAL_SWITCH | SRC_TRAINER | SRC_GVAR

bool isSourceAvailable(int source)
{
  return checkSourceAvailable(source,
            SRC_COMMON | SRC_INPUT | SRC_LUA | SRC_HELI | SRC_CHANNEL | SRC_TX | SRC_TIMER | SRC_TELEM
            );
}

// Used only in B&W radios for Global Functions when funcion is FUNC_PLAY_VALUE
bool isSourceAvailableInGlobalFunctions(int source)
{
  return checkSourceAvailable(source,
            SRC_COMMON | SRC_INPUT | SRC_LUA | SRC_HELI | SRC_CHANNEL | SRC_TX | SRC_TIMER
            );
}

// Used only in B&W radios with wide screen LCD (212x64) for logical switches
// V1 parameter when LS function is LS_FAMILY_OFS or LS_FAMILY_DIFF
bool isSourceAvailableInCustomSwitches(int source)
{
  return checkSourceAvailable(source,
            SRC_COMMON | SRC_INPUT | SRC_LUA | SRC_HELI | SRC_CHANNEL | SRC_TX | SRC_TIMER | SRC_TELEM_COMP
            );
}

// Only used for B&W radios for Input source (color radios use isSourceAvailable)
bool isSourceAvailableInInputs(int source)
{
  return checkSourceAvailable(source,
            SRC_COMMON | SRC_CHANNEL_ALL | SRC_TELEM_COMP
            );
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
  if (mode == UART_MODE_CLI) return false;
#else
  // CLI is only supported on VCP
  if (port_nr != SP_VCP && mode == UART_MODE_CLI) return false;
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

bool isSwitchAvailableForArming(int swtch)
{
  return isSwitchAvailable(swtch, ModelCustomFunctionsContext);
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

bool isAssignableFunctionAvailable(int function, bool modelFunctions)
{
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
#if !defined(DEBUG)
    case FUNC_TEST:
      return false;
#endif
    default:
      return true;
  }
}

#if !defined(COLORLCD)
bool isAssignableFunctionAvailable(int function)
{
  return isAssignableFunctionAvailable(function, menuHandlers[menuLevel] == menuModelSpecialFunctions);
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
  AntennaSelectionMenu(bool& done) : Menu(), done(done) {
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
      POPUP_MENU_START(onAntennaSelection, 2, STR_USE_INTERNAL_ANTENNA, STR_USE_EXTERNAL_ANTENNA);
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

#if defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML) and !defined(EXTMODULE_USART)
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
    return serialGetModePort(UART_MODE_SBUS_TRAINER) >= 0;
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
      const etx_module_port_t *port = nullptr;

      // check if UART with inverter on heartbeat pin is available
      port = modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
                            ETX_MOD_PORT_UART, ETX_Pol_Normal,
                            ETX_MOD_DIR_RX);
      if (!port) {
        // otherwise fall back to S.Port pin
        port =
            modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
                           ETX_MOD_PORT_SPORT, ETX_Pol_Normal, ETX_MOD_DIR_RX);
      }

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
  char filename[sizeof(MODELS_PATH)+1+LEN_MODEL_NAME*3+sizeof(TEXT_EXT)] = MODELS_PATH "/";
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

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
uint8_t MENU_FIRST_LINE_EDIT(const uint8_t * horTab, uint8_t horTabMax)
{
  if (horTab) {
    uint8_t result = 0;
    while (result < horTabMax && horTab[result] >= HIDDEN_ROW)
      ++result;
    return result;
  }
  else {
    return 0;
  }
}
#endif

uint8_t MODULE_BIND_ROWS(int moduleIdx)
{
  if (isModuleELRS(moduleIdx) && (crossfireModuleStatus[moduleIdx].major >= 4 || (crossfireModuleStatus[moduleIdx].major == 3 && crossfireModuleStatus[moduleIdx].minor >= 4)))
    return 1;

  if (isModuleCrossfire(moduleIdx))
    return 0;

  if (isModuleMultimodule(moduleIdx)) {
    if (IS_RX_MULTI(moduleIdx))
      return 1;
    else
      return 2;
  }
  else if (isModuleXJTD8(moduleIdx) || isModuleSBUS(moduleIdx) || isModuleAFHDS3(moduleIdx) || isModuleDSMP(moduleIdx)) {
    return 1;
  }
  else if (isModulePPM(moduleIdx) || isModulePXX1(moduleIdx) || isModulePXX2(moduleIdx) || isModuleDSM2(moduleIdx)) {
    return 2;
  }
  else {
    return HIDDEN_ROW;
  }
}

uint8_t MODULE_CHANNELS_ROWS(int moduleIdx)
{
  if (!IS_MODULE_ENABLED(moduleIdx)) {
    return HIDDEN_ROW;
  }
#if defined(MULTIMODULE)
  else if (isModuleMultimodule(moduleIdx)) {
    if (IS_RX_MULTI(moduleIdx))
      return HIDDEN_ROW;
    else if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
      return 1;
    else
      return 0;
  }
#endif
  else if (isModuleDSM2(moduleIdx) || isModuleCrossfire(moduleIdx) ||
             isModuleGhost(moduleIdx) || isModuleSBUS(moduleIdx) ||
             isModuleDSMP(moduleIdx)) {
    // fixed number of channels
    return 0;
  } else {
    return 1;
  }
}

#if defined(MULTIMODULE)
uint8_t MULTI_DISABLE_CHAN_MAP_ROW_STATIC(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return HIDDEN_ROW;

  uint8_t protocol = g_model.moduleData[moduleIdx].multi.rfProtocol;
  if (protocol < MODULE_SUBTYPE_MULTI_LAST) {
    const mm_protocol_definition * pdef = getMultiProtocolDefinition(protocol);
    if (pdef->disable_ch_mapping)
      return 0;
  }

  return HIDDEN_ROW;
}

uint8_t MULTI_DISABLE_CHAN_MAP_ROW(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return HIDDEN_ROW;

  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  if (status.isValid()) {
    return status.supportsDisableMapping() == true ? 0 : HIDDEN_ROW;
  }

  return MULTI_DISABLE_CHAN_MAP_ROW_STATIC(moduleIdx);
}

bool MULTIMODULE_PROTOCOL_KNOWN(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx)) {
    return false;
  }

  if (g_model.moduleData[moduleIdx].multi.rfProtocol < MODULE_SUBTYPE_MULTI_LAST) {
    return true;
  }

  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  if (status.isValid()) {
    return status.protocolValid();
  }

  return false;
}

bool MULTIMODULE_HAS_SUBTYPE(uint8_t moduleIdx)
{
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  int proto = g_model.moduleData[moduleIdx].multi.rfProtocol;

  if (status.isValid()) {
    TRACE("(%d) status.protocolSubNbr = %d", proto, status.protocolSubNbr);
    return status.protocolSubNbr > 0;
  }
  else
  {
    if (proto > MODULE_SUBTYPE_MULTI_LAST) {
      return true;
    }
    else {
      auto subProto = getMultiProtocolDefinition(proto);
      return subProto->subTypeString != nullptr;
    }
  }
}

uint8_t MULTIMODULE_RFPROTO_COLUMNS(uint8_t moduleIdx)
{
#if LCD_W < 212
  if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
    return (MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t) 1 : HIDDEN_ROW);
  else
    return (MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW);
#else
  return (MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? (uint8_t) 1 : 0);
#endif
}

uint8_t MULTIMODULE_HASOPTIONS(uint8_t moduleIdx)
{
  if (!isModuleMultimodule(moduleIdx))
    return false;

  uint8_t protocol = g_model.moduleData[moduleIdx].multi.rfProtocol;
  MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);

  if (status.isValid())
    return status.optionDisp;

  if (protocol < MODULE_SUBTYPE_MULTI_LAST)
    return getMultiProtocolDefinition(protocol)->optionsstr != nullptr;

  return false;
}
#endif

uint8_t MODULE_OPTION_ROW(uint8_t moduleIdx)
{
  if(isModuleR9MNonAccess(moduleIdx) || isModuleSBUS(moduleIdx))
    return TITLE_ROW;
  if(isModuleAFHDS3(moduleIdx))
    return HIDDEN_ROW;
  if(isModuleGhost(moduleIdx))
    return 0;
  return MULTIMODULE_OPTIONS_ROW(moduleIdx);
}
