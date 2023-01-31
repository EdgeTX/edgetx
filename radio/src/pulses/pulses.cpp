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

// #include "hal.h"
#include "opentx.h"

#include "mixer_scheduler.h"
#include "heartbeat_driver.h"
#include "hal/module_port.h"
#include "tasks/mixer_task.h"

#include "pulses/pxx2.h"
#include "pulses/flysky.h"
#include "pulses/dsm2.h"

#if defined(PPM)
#include "pulses/ppm.h"
#endif

#if defined(PXX1)
#include "pulses/pxx1.h"
#endif

#if defined(SBUS)
#include "pulses/sbus.h"
#endif

#if defined(CROSSFIRE)
#include "pulses/crossfire.h"
#endif

#if defined(GHOST)
#include "pulses/ghost.h"
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

static module_pulse_driver _module_drivers[MAX_MODULES];
static module_pulse_buffer _module_buffers[MAX_MODULES] __DMA;

void pulsesInit()
{
  memset(_module_drivers, 0, sizeof(_module_drivers));
}

module_pulse_driver* pulsesGetModuleDriver(uint8_t module)
{
  return &(_module_drivers[module]);
}

uint8_t* pulsesGetModuleBuffer(uint8_t module)
{
  return _module_buffers[module]._buffer;
}

ModuleState moduleState[NUM_MODULES];
TrainerPulsesData trainerPulsesData __DMA;

void pulsesStart()
{
  telemetryStart();
  mixerTaskStart();
}

void pulsesStop()
{
  telemetryStop();
  mixerTaskStop();

  for (uint8_t i = 0; i < MAX_MODULES; i++)
    pulsesStopModule(i);
}

void restartModule(uint8_t module)
{
  mixerTaskStop();

  // wait for the power output to be drained
  pulsesStopModule(module);
  RTOS_WAIT_MS(200);

  mixerTaskStart();
}

// TODO: this should be moved to PXX2 territory!
#if defined(PXX2)
// use only for PXX
void ModuleState::startBind(BindInformation* destination,
                            ModuleCallback bindCallback)
{
  bindInformation = destination;
  callback = bindCallback;
  mode = MODULE_MODE_BIND;
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
#endif

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

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE && isExternalModuleAvailable(type)) {
    return type;
  }
#endif

  return MODULE_TYPE_NONE;
}

// TODO: replace with some mapping between
//       module type and protocol driver.
//
uint8_t getRequiredProtocol(uint8_t module)
{
  uint8_t protocol = PROTOCOL_CHANNELS_UNINITIALIZED;

  switch (getModuleType(module)) {
    case MODULE_TYPE_PPM:
      protocol = PROTOCOL_CHANNELS_PPM;
      break;

    case MODULE_TYPE_XJT_PXX1:
    case MODULE_TYPE_R9M_PXX1:
    case MODULE_TYPE_R9M_LITE_PXX1:
      protocol = PROTOCOL_CHANNELS_PXX1;
      break;

    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_PXX2:
    case MODULE_TYPE_R9M_LITE_PXX2:
    case MODULE_TYPE_XJT_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
      protocol = PROTOCOL_CHANNELS_PXX2;
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
      // TODO: move this to DSM2 driver...
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

  return protocol;
}

static module_init_cb_t _on_module_init = nullptr;
static module_deinit_cb_t _on_module_deinit = nullptr;

void pulsesSetModuleInitCb(module_init_cb_t cb)
{
  _on_module_init = cb;
}

void pulsesSetModuleDeInitCb(module_deinit_cb_t cb)
{
  _on_module_deinit = cb;
}

static void _init_module(uint8_t module, const etx_proto_driver_t* drv)
{
  auto mod = &(_module_drivers[module]);
  void* ctx = drv->init(module);

  // TODO: module init failed somehow, we should handle this better...
  if (!ctx) return;

  mod->drv = drv;
  mod->ctx = ctx;

  // board specific hook
  if (_on_module_init)
    _on_module_init(module, drv);
  
  // power ON
  modulePortSetPower(module, true);
}

static void _deinit_module(uint8_t module)
{
  auto mod = &(_module_drivers[module]);
  if (!mod->drv) return;

  // scheduling OFF
  mixerSchedulerSetPeriod(module, 0);

  // board specific hook
  auto drv = mod->drv;
  if (_on_module_deinit)
    _on_module_deinit(module, drv);
  
  // de-init
  auto ctx = mod->ctx;
  drv->deinit(ctx);

  // power OFF
  modulePortSetPower(module, false);

  // clear
  memset(mod, 0, sizeof(module_pulse_driver));
}

static void pulsesEnableModule(uint8_t module, uint8_t protocol)
{
  _deinit_module(module);

  switch (protocol) {
#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1:
      _init_module(module, &Pxx1Driver);
      break;
#endif

#if defined(DSM2)
    case PROTOCOL_CHANNELS_DSM2_LP45:
    case PROTOCOL_CHANNELS_DSM2_DSM2:
    case PROTOCOL_CHANNELS_DSM2_DSMX:
      _init_module(module, &DSM2Driver);
      break;
#endif

#if defined(SBUS)
    case PROTOCOL_CHANNELS_SBUS:
      _init_module(module, &SBusDriver);
      break;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2:
      _init_module(module, &Pxx2Driver);
      break;
#endif

#if defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      _init_module(module, &MultiDriver);
      break;
#endif

#if defined(CROSSFIRE)
    case PROTOCOL_CHANNELS_CROSSFIRE:
      _init_module(module, &CrossfireDriver);
      break;
#endif

#if defined(GHOST)
    case PROTOCOL_CHANNELS_GHOST:
      _init_module(module, &GhostDriver);
      break;
#endif

#if defined(PPM)
  case PROTOCOL_CHANNELS_PPM:
      _init_module(module, &PpmDriver);
      break;
#endif

#if defined(INTERNAL_MODULE_AFHDS2A) && defined(AFHDS2)
    case PROTOCOL_CHANNELS_AFHDS2A:
      _init_module(module, &Afhds2InternalDriver);
      break;
#endif

#if defined(INTERNAL_MODULE_AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      _init_module(module, &afhds3::ProtoDriver);
      break;
#endif

#if defined(DSM2)
    case PROTOCOL_CHANNELS_DSMP:
      _init_module(module, &DSMPDriver);
      break;
#endif

    default:
      break;
  }
}

extern volatile uint8_t _telemetryIsPolling;

void pulsesStopModule(uint8_t module)
{
  if (module >= MAX_MODULES) return;

  while(_telemetryIsPolling) {
    // In case the telemetry timer is currently polling the port,
    // we give the timer task a chance to run and finish the polling.
    RTOS_WAIT_MS(1);
  }
  _deinit_module(module);

  auto& proto = moduleState[module].protocol;
  proto = PROTOCOL_CHANNELS_NONE;
}

void pulsesSendNextFrame(uint8_t module)
{
  if (module >= MAX_MODULES) return;
  uint8_t protocol = getRequiredProtocol(module);

  if (moduleState[module].protocol != protocol) {
    // TODO: error checking!

    if (_telemetryIsPolling) {
      // In case the telemetry timer is currently polling the port,
      // we just yield in the hope it will be different next time.
      return;
    }

    pulsesEnableModule(module, protocol);
    moduleState[module].protocol = protocol;
    return; // really??? why not start right now?
  }

  auto mod = &(_module_drivers[module]);
  if (mod->drv) {
    uint8_t channelStart = g_model.moduleData[module].channelsStart;
    int16_t* channels = &channelOutputs[channelStart];
    uint8_t nChannels = 16; // TODO: MAX_CHANNELS - channelsStart

    auto drv = mod->drv;
    auto ctx = mod->ctx;
    auto buffer = _module_buffers[module]._buffer;
    drv->sendPulses(ctx, buffer, channels, nChannels);
  }
}

void pulsesSendChannels()
{
  for (uint8_t i = 0; i < MAX_MODULES; i++) {
    pulsesSendNextFrame(i);
  }
}

// set the failsafe channel values to the current output values
void setCustomFailsafe(uint8_t moduleIndex)
{
  if (moduleIndex < NUM_MODULES) {
    for (int ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
      if (ch < g_model.moduleData[moduleIndex].channelsStart ||
          ch >= sentModuleChannels(moduleIndex) +
                    g_model.moduleData[moduleIndex].channelsStart) {
        g_model.failsafeChannels[ch] = 0;
      } else if (g_model.failsafeChannels[ch] < FAILSAFE_CHANNEL_HOLD) {
        g_model.failsafeChannels[ch] = channelOutputs[ch];
      }
    }
    storageDirty(EE_MODEL);
  }
}

int32_t getChannelValue(uint8_t channel)
{
  return channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;
}
