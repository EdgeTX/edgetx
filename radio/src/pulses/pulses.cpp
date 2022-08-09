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
#include "mixer_scheduler.h"
#include "heartbeat_driver.h"

#include "pulses/pxx2.h"
#include "pulses/flysky.h"

#if defined(INTMODULE_USART)
#include "intmodule_serial_driver.h"
#else
#include "intmodule_pulses_driver.h"
#endif

#include "extmodule_serial_driver.h"
#include "extmodule_driver.h"

#if defined(CROSSFIRE)
#include "pulses/crossfire.h"
#endif

#if defined(GHOST)
#include "telemetry/ghost.h"
#endif

#if defined(MULTIMODULE)
#include "io/multi_protolist.h"
#include "pulses/multi.h"
#endif

#if defined(AFHDS2)
#include "pulses/afhds2.h"
#endif

#if defined(AFHDS3)
#include "pulses/afhds3.h"
#endif

uint8_t s_pulses_paused = 0;
ModuleState moduleState[NUM_MODULES];
InternalModulePulsesData intmodulePulsesData __DMA;
ExternalModulePulsesData extmodulePulsesData __DMA;
TrainerPulsesData trainerPulsesData __DMA;

void startPulses()
{
  telemetryStart();
  s_pulses_paused = false;

#if defined(HARDWARE_INTERNAL_MODULE)
  setupPulsesInternalModule();
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  setupPulsesExternalModule();
#endif

#if defined(HARDWARE_EXTRA_MODULE)
  extramodulePpmStart();
#endif
}

void stopPulses()
{
  telemetryStop();
  s_pulses_paused = true;

  for (uint8_t i = 0; i < NUM_MODULES; i++)
    moduleState[i].protocol = PROTOCOL_CHANNELS_UNINITIALIZED;
}

void restartModule(uint8_t idx)
{
  if (idx == INTERNAL_MODULE) {
    if (!IS_INTERNAL_MODULE_ON()) return;
  } else if (idx == EXTERNAL_MODULE){
    if (!IS_EXTERNAL_MODULE_ON()) return;
  } else {
    return;
  }

  pauseMixerCalculations();
  pausePulses();

  if (idx == INTERNAL_MODULE) stopPulsesInternalModule();
#if defined(HARDWARE_EXTERNAL_MODULE)
  else stopPulsesExternalModule();
#endif

  RTOS_WAIT_MS(200); // 20ms so that the pulses interrupt will reinit the frame rate

  resumePulses();
  resumeMixerCalculations();
}

// use only for PXX
void ModuleState::startBind(BindInformation* destination,
                            ModuleCallback bindCallback)
{
  bindInformation = destination;
  callback = bindCallback;
  mode = MODULE_MODE_BIND;
#if defined(SIMU)
  bindInformation->candidateReceiversCount = 2;
  strcpy(bindInformation->candidateReceiversNames[0], "SimuRX1");
  strcpy(bindInformation->candidateReceiversNames[1], "SimuRX2");
#endif
}

void ModuleState::readModuleInformation(ModuleInformation* destination,
                                        int8_t first, int8_t last)
{
  moduleInformation = destination;
  moduleInformation->current = first;
  moduleInformation->maximum = last;
  mode = MODULE_MODE_GET_HARDWARE_INFO;
}

void ModuleState::readModuleSettings(ModuleSettings* destination)
{
  moduleSettings = destination;
  moduleSettings->state = PXX2_SETTINGS_READ;
  mode = MODULE_MODE_MODULE_SETTINGS;
}

void ModuleState::writeModuleSettings(ModuleSettings* source)
{
  moduleSettings = source;
  moduleSettings->state = PXX2_SETTINGS_WRITE;
  moduleSettings->timeout = 0;
  mode = MODULE_MODE_MODULE_SETTINGS;
}

void ModuleState::readReceiverSettings(ReceiverSettings* destination)
{
  receiverSettings = destination;
  receiverSettings->state = PXX2_SETTINGS_READ;
  mode = MODULE_MODE_RECEIVER_SETTINGS;
}

void ModuleState::writeReceiverSettings(ReceiverSettings* source)
{
  receiverSettings = source;
  receiverSettings->state = PXX2_SETTINGS_WRITE;
  receiverSettings->timeout = 0;
  mode = MODULE_MODE_RECEIVER_SETTINGS;
}

void getModuleStatusString(uint8_t moduleIdx, char * statusText)
{
  *statusText = 0;
#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    //change it
    getMultiModuleStatus(moduleIdx).getStatusString(statusText);
  }
#endif
#if defined(AFHDS3)
  if (isModuleAFHDS3(moduleIdx)) {
    afhds3::getStatusString(moduleIdx, statusText);
  }
#endif
}

void getModuleSyncStatusString(uint8_t moduleIdx, char * statusText)
{
  *statusText = 0;
#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    getModuleSyncStatus(moduleIdx).getRefreshString(statusText);
  }
#endif
}

ModuleSettingsMode getModuleMode(int moduleIndex)
{
  return (ModuleSettingsMode)moduleState[moduleIndex].mode;
}

void setModuleMode(int moduleIndex, ModuleSettingsMode mode)
{
  moduleState[moduleIndex].mode = mode;
}

uint8_t getModuleType(uint8_t module)
{
  uint8_t type = g_model.moduleData[module].type;

#if defined(HARDWARE_INTERNAL_MODULE)
  if (module == INTERNAL_MODULE && isInternalModuleAvailable(type)) {
    return type;
  }
#endif

  if (module == EXTERNAL_MODULE && isExternalModuleAvailable(type)) {
    return type;
  }

  return MODULE_TYPE_NONE;
}

uint8_t getRequiredProtocol(uint8_t module)
{
  uint8_t protocol = PROTOCOL_CHANNELS_UNINITIALIZED;

  switch (getModuleType(module)) {
    case MODULE_TYPE_PPM:
      protocol = PROTOCOL_CHANNELS_PPM;
      break;

    case MODULE_TYPE_XJT_PXX1:
#if defined(INTMODULE_USART)
      if (module == INTERNAL_MODULE) {
        protocol = PROTOCOL_CHANNELS_PXX1_SERIAL;
        break;
      }
#endif
      protocol = PROTOCOL_CHANNELS_PXX1_PULSES;
      break;

    case MODULE_TYPE_R9M_PXX1:
      protocol = PROTOCOL_CHANNELS_PXX1_PULSES;
      break;

#if defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case MODULE_TYPE_R9M_LITE_PXX1:
      protocol = PROTOCOL_CHANNELS_PXX1_SERIAL;
      break;

    case MODULE_TYPE_R9M_LITE_PXX2:
      protocol = PROTOCOL_CHANNELS_PXX2_LOWSPEED;
      break;
#endif

    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_PXX2:
#if defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case MODULE_TYPE_XJT_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
#endif
      protocol = PROTOCOL_CHANNELS_PXX2_HIGHSPEED;
      break;

    case MODULE_TYPE_SBUS:
      protocol = PROTOCOL_CHANNELS_SBUS;
      break;

#if defined(MULTIMODULE)
    case MODULE_TYPE_MULTIMODULE:
      protocol = PROTOCOL_CHANNELS_MULTIMODULE;
      break;
#endif

#if defined(DSM2)
    case MODULE_TYPE_DSM2:
      protocol = limit<uint8_t>(
          PROTOCOL_CHANNELS_DSM2_LP45,
          PROTOCOL_CHANNELS_DSM2_LP45 + g_model.moduleData[module].subType,
          PROTOCOL_CHANNELS_DSM2_DSMX);
      // The module is set to OFF during one second before BIND start
      {
        static tmr10ms_t bindStartTime = 0;
        if (moduleState[module].mode == MODULE_MODE_BIND) {
          if (bindStartTime == 0) bindStartTime = get_tmr10ms();
          if ((tmr10ms_t)(get_tmr10ms() - bindStartTime) < 100) {
            protocol = PROTOCOL_CHANNELS_NONE;
            break;
          }
        }
        else {
          bindStartTime = 0;
        }
      }
      break;
#endif

#if defined(CROSSFIRE)
    case MODULE_TYPE_CROSSFIRE:
      protocol = PROTOCOL_CHANNELS_CROSSFIRE;
      break;
#endif

#if defined(AFHDS3) || defined(AFHDS2)
    case MODULE_TYPE_FLYSKY:
      if (isModuleAFHDS3(module)) {
        protocol = PROTOCOL_CHANNELS_AFHDS3;
      } else if (isModuleAFHDS2A(module)) {
        protocol = PROTOCOL_CHANNELS_AFHDS2A;
      }
      break;
#endif

#if defined(GHOST)
    case MODULE_TYPE_GHOST:
      protocol = PROTOCOL_CHANNELS_GHOST;
      break;
#endif

    case MODULE_TYPE_LEMON_DSMP:
      protocol = PROTOCOL_CHANNELS_DSMP;
      break;
      
    default:
      protocol = PROTOCOL_CHANNELS_NONE;
      break;
  }

  if (s_pulses_paused) {
    protocol = PROTOCOL_CHANNELS_NONE;
  }

#if 0
  // will need an EEPROM conversion
  if (moduleState[module].mode == MODULE_OFF) {
    protocol = PROTOCOL_CHANNELS_NONE;
  }
#endif

  return protocol;
}

#if defined(HARDWARE_INTERNAL_MODULE)
static void* internalModuleContext = nullptr;
static const etx_module_driver_t* internalModuleDriver = nullptr;

const etx_module_driver_t* getIntModuleDriver()
{
  return internalModuleDriver;
}

void* getIntModuleCtx()
{
  return internalModuleContext;
}

static void enablePulsesInternalModule(uint8_t protocol)
{
  // start new protocol hardware here
  if (internalModuleDriver) {
    internalModuleDriver->deinit(internalModuleContext);
    internalModuleContext = nullptr;
    internalModuleDriver = nullptr;
  } else {
    intmoduleStop();
  }

  switch (protocol) {
#if defined(INTERNAL_MODULE_PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmodulePxx1PulsesStart();
#if defined(INTMODULE_HEARTBEAT)
      init_intmodule_heartbeat();
#endif
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
      break;
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      internalModuleContext = Pxx1InternalSerialDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &Pxx1InternalSerialDriver;
      break;
#endif

#if defined(INTERNAL_MODULE_PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
      internalModuleContext = Pxx2InternalDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &Pxx2InternalDriver;
      break;
#endif

#if defined(INTERNAL_MODULE_MULTI) && defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      internalModuleContext = MultiInternalDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &MultiInternalDriver;
      break;
#endif

#if defined(INTERNAL_MODULE_CRSF) && defined(CROSSFIRE)
    case PROTOCOL_CHANNELS_CROSSFIRE:
      internalModuleContext = CrossfireInternalDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &CrossfireInternalDriver;
      break;
#endif

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      intmodulePpmStart();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PPM_PERIOD(INTERNAL_MODULE));
      break;
#endif

#if defined(INTERNAL_MODULE_AFHDS2A) && defined(AFHDS2)
    case PROTOCOL_CHANNELS_AFHDS2A:
      internalModuleContext = Afhds2InternalDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &Afhds2InternalDriver;
      break;
#endif

#if defined(INTERNAL_MODULE_AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      internalModuleContext = afhds3::internalDriver.init(INTERNAL_MODULE);
      internalModuleDriver = &afhds3::internalDriver;
      break;
#endif

    default:
      // internal module stopped, use default mixer period
      mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
      break;
  }
}

bool setupPulsesInternalModule(uint8_t protocol)
{
  uint8_t channelStart = g_model.moduleData[INTERNAL_MODULE].channelsStart;
  int16_t* channels = &channelOutputs[channelStart];
  uint8_t nChannels = 16;  // TODO: MAX_CHANNELS - channelsStart

  if (internalModuleDriver) {
    internalModuleDriver->setupPulses(internalModuleContext,
                                      channels, nChannels);
    return true;
  }

  switch (protocol) {
#if defined(HARDWARE_INTERNAL_MODULE) && defined(PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmodulePulsesData.pxx.setupFrame(INTERNAL_MODULE);
      return true;
#endif

#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMInternalModule();
      return true;
#endif

    default:
      return false;
  }
}

void stopPulsesInternalModule()
{
  if (moduleState[INTERNAL_MODULE].protocol != PROTOCOL_CHANNELS_UNINITIALIZED) {
    if (internalModuleDriver) {
      internalModuleDriver->deinit(internalModuleContext);
      internalModuleDriver = nullptr;
      internalModuleContext = nullptr;
    } else {
      mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
      intmoduleStop();
    }
    moduleState[INTERNAL_MODULE].protocol = PROTOCOL_CHANNELS_NONE;
  }
}

void intmoduleSendNextFrame()
{
  if (internalModuleDriver) {
    internalModuleDriver->sendPulses(internalModuleContext);
    return;
  }

  switch (moduleState[INTERNAL_MODULE].protocol) {

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      intmoduleSendNextFramePPM(
          intmodulePulsesData.ppm.pulses,
          intmodulePulsesData.ppm.ptr - intmodulePulsesData.ppm.pulses);
      break;
#endif

#if defined(PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmoduleSendNextFramePxx1(intmodulePulsesData.pxx.getData(),
                                 intmodulePulsesData.pxx.getSize());
      break;
#endif
  }
}

bool setupPulsesInternalModule()
{
  uint8_t protocol = getRequiredProtocol(INTERNAL_MODULE);

  heartbeat |= (HEART_TIMER_PULSES << INTERNAL_MODULE);

  if (moduleState[INTERNAL_MODULE].protocol != protocol) {
    enablePulsesInternalModule(protocol);
    moduleState[INTERNAL_MODULE].protocol = protocol;
    return false;
  }
  else {
    return setupPulsesInternalModule(protocol);
  }
}
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
static void* externalModuleContext = nullptr;
static const etx_module_driver_t* externalModuleDriver = nullptr;

const etx_module_driver_t* getExtModuleDriver()
{
  return externalModuleDriver;
}

void* getExtModuleCtx()
{
  return externalModuleContext;
}

void enablePulsesExternalModule(uint8_t protocol)
{
  // start new protocol hardware here
  if (externalModuleDriver) {
    externalModuleDriver->deinit(externalModuleContext);
    externalModuleDriver = nullptr;
    externalModuleContext = nullptr;
  } else {
    extmoduleStop();
  }

  switch (protocol) {
#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmodulePxx1PulsesStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX_PULSES_PERIOD);
      break;
#endif

#if defined(PXX1) && defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      externalModuleContext = Pxx1ExternalSerialDriver.init(EXTERNAL_MODULE);
      externalModuleDriver = &Pxx1ExternalSerialDriver;
      break;
#endif

#if defined(DSM2)
    case PROTOCOL_CHANNELS_DSM2_LP45:
    case PROTOCOL_CHANNELS_DSM2_DSM2:
    case PROTOCOL_CHANNELS_DSM2_DSMX:
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, DSM2_PERIOD);
      break;
#endif

#if defined(CROSSFIRE)
    case PROTOCOL_CHANNELS_CROSSFIRE:
      externalModuleContext = CrossfireExternalDriver.init(EXTERNAL_MODULE);
      externalModuleDriver = &CrossfireExternalDriver;
      break;
#endif

#if defined(GHOST)
    case PROTOCOL_CHANNELS_GHOST:
      EXTERNAL_MODULE_ON();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, GHOST_PERIOD);
      break;
#endif

#if defined(PXX2) && defined(EXTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
      externalModuleContext = Pxx2ExternalDriver.init(EXTERNAL_MODULE);
      externalModuleDriver = &Pxx2ExternalDriver;
      break;

    case PROTOCOL_CHANNELS_PXX2_LOWSPEED:
      externalModuleContext = Pxx2LowSpeedExternalDriver.init(EXTERNAL_MODULE);
      externalModuleDriver = &Pxx2LowSpeedExternalDriver;
      break;
#endif

#if defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, MULTIMODULE_PERIOD);
      getMultiModuleStatus(EXTERNAL_MODULE).failsafeChecked = false;
      getMultiModuleStatus(EXTERNAL_MODULE).flags = 0;
#if defined(MULTI_PROTOLIST)
      MultiRfProtocols::instance(EXTERNAL_MODULE)->triggerScan();
#endif
      break;
#endif

#if defined(SBUS)
    case PROTOCOL_CHANNELS_SBUS:
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, SBUS_PERIOD);
      break;
#endif

#if defined(PPM)
    case PROTOCOL_CHANNELS_PPM:
      extmodulePpmStart(
          GET_MODULE_PPM_DELAY(EXTERNAL_MODULE),
          GET_MODULE_PPM_POLARITY(EXTERNAL_MODULE));
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PPM_PERIOD(EXTERNAL_MODULE));
      break;
#endif

#if defined(AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      externalModuleContext = afhds3::externalDriver.init(EXTERNAL_MODULE);
      externalModuleDriver = &afhds3::externalDriver;
      break;
#endif

    case PROTOCOL_CHANNELS_DSMP:
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, 11 * 1000 /* 11ms in us */);
      break;
      
    default:
      // external module stopped, use default mixer period
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
      break;
  }
}

bool setupPulsesExternalModule(uint8_t protocol)
{
  uint8_t channelStart = g_model.moduleData[EXTERNAL_MODULE].channelsStart;
  int16_t* channels = &channelOutputs[channelStart];
  uint8_t nChannels = 16;  // TODO: MAX_CHANNELS - channelsStart

  if (externalModuleDriver) {
    externalModuleDriver->setupPulses(externalModuleContext,
                                      channels, nChannels);
    return true;
  }
  
  switch (protocol) {

#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmodulePulsesData.pxx.setupFrame(EXTERNAL_MODULE);
      return true;
#endif

#if defined(SBUS)
    case PROTOCOL_CHANNELS_SBUS:
      setupPulsesSbus();
      // SBUS_PERIOD is not a constant! It can be set from UI
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, SBUS_PERIOD);
      return true;
#endif

#if defined(DSM2)
    case PROTOCOL_CHANNELS_DSM2_LP45:
    case PROTOCOL_CHANNELS_DSM2_DSM2:
    case PROTOCOL_CHANNELS_DSM2_DSMX:
      setupPulsesDSM2();
      return true;
#endif

#if defined(GHOST)
    case PROTOCOL_CHANNELS_GHOST:
    {
      ModuleSyncStatus& status = getModuleSyncStatus(EXTERNAL_MODULE);
      if (status.isValid())
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, status.getAdjustedRefreshRate());
      else
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, GHOST_PERIOD);
      setupPulsesGhost();
      return true;
    }
#endif

#if defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      setupPulsesMultiExternalModule();
      return true;
#endif

#if defined(PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMExternalModule();
      return true;
#endif

    case PROTOCOL_CHANNELS_DSMP:
      setupPulsesLemonDSMP();
      return true;

    default:
      return false;
  }
}

void extmoduleSendNextFrame()
{
  if (externalModuleDriver) {
    externalModuleDriver->sendPulses(externalModuleContext);
    return;
  }

  switch (moduleState[EXTERNAL_MODULE].protocol) {

    case PROTOCOL_CHANNELS_PPM:
      extmoduleSendNextFramePpm(
          extmodulePulsesData.ppm.pulses,
          extmodulePulsesData.ppm.ptr - extmodulePulsesData.ppm.pulses,
          GET_MODULE_PPM_DELAY(EXTERNAL_MODULE),
          GET_MODULE_PPM_POLARITY(EXTERNAL_MODULE));
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PPM_PERIOD(EXTERNAL_MODULE));
      break;

#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmoduleSendNextFramePxx1(extmodulePulsesData.pxx.getData(),
                                 extmodulePulsesData.pxx.getSize());
      break;
#endif

#if defined(SBUS) || defined(DSM2) || defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_SBUS:
      extmoduleSendNextFrameSoftSerial(
          extmodulePulsesData.dsm2.pulses,
          extmodulePulsesData.dsm2.ptr - extmodulePulsesData.dsm2.pulses,
          GET_SBUS_POLARITY(EXTERNAL_MODULE));
      break;

    case PROTOCOL_CHANNELS_DSM2_LP45:
    case PROTOCOL_CHANNELS_DSM2_DSM2:
    case PROTOCOL_CHANNELS_DSM2_DSMX:
    case PROTOCOL_CHANNELS_MULTIMODULE:
      extmoduleSendNextFrameSoftSerial(
          extmodulePulsesData.dsm2.pulses,
          extmodulePulsesData.dsm2.ptr - extmodulePulsesData.dsm2.pulses);
      break;
#endif
    case PROTOCOL_CHANNELS_DSMP:
      extmoduleSendNextFrameSoftSerial(
          extmodulePulsesData.dsm2.pulses,
          extmodulePulsesData.dsm2.ptr - extmodulePulsesData.dsm2.pulses,
          true);
      break;

#if defined(GHOST)
    case PROTOCOL_CHANNELS_GHOST:
      sportSendBuffer(extmodulePulsesData.ghost.pulses,
                      extmodulePulsesData.ghost.length);
      break;
#endif
  }
}

void stopPulsesExternalModule()
{
  if (moduleState[EXTERNAL_MODULE].protocol !=
      PROTOCOL_CHANNELS_UNINITIALIZED) {
    if (externalModuleDriver) {
      externalModuleDriver->deinit(externalModuleContext);
      externalModuleDriver = nullptr;
      externalModuleContext = nullptr;
    } else {
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
      extmoduleStop();
    }
    moduleState[EXTERNAL_MODULE].protocol = PROTOCOL_CHANNELS_NONE;
  }
}

bool setupPulsesExternalModule()
{
  uint8_t protocol = getRequiredProtocol(EXTERNAL_MODULE);

  heartbeat |= (HEART_TIMER_PULSES << EXTERNAL_MODULE);

  if (moduleState[EXTERNAL_MODULE].protocol != protocol) {
    enablePulsesExternalModule(protocol);
    moduleState[EXTERNAL_MODULE].protocol = protocol;
    return false;
  }
  else {
    return setupPulsesExternalModule(protocol);
  }
}
#endif

void setCustomFailsafe(uint8_t moduleIndex)
{
  if (moduleIndex < NUM_MODULES) {
    for (int ch=0; ch<MAX_OUTPUT_CHANNELS; ch++) {
      if (ch < g_model.moduleData[moduleIndex].channelsStart || ch >= sentModuleChannels(moduleIndex) + g_model.moduleData[moduleIndex].channelsStart) {
        g_model.failsafeChannels[ch] = 0;
      }
      else if (g_model.failsafeChannels[ch] < FAILSAFE_CHANNEL_HOLD) {
        g_model.failsafeChannels[ch] = channelOutputs[ch];
      }
    }
    storageDirty(EE_MODEL);
  }
}

int32_t getChannelValue(uint8_t channel)
{
  return channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
}
