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
#include "edgetx.h"

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

void pulsesRestartModuleUnsafe(uint8_t module)
{
  if (module >= MAX_MODULES)
    return;
  
  auto mod_drv = pulsesGetModuleDriver(module);
  if (!mod_drv->drv) return;
  
  auto drv = mod_drv->drv;
  drv->deinit(mod_drv->ctx);
  mod_drv->ctx = drv->init(module);
}

#if !defined(SIMU)
#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

static void _setup_async_module_restart(void* p1, uint32_t p2)
{
  if (!mixerTaskTryLock()) {
    // In case the mixer cannot be locked, try again later
    // and make the same function pending again.
    PendedFunction_t cb = _setup_async_module_restart;
    xTimerPendFunctionCall(cb, p1, p2, 0/* do not wait */);
    return;
  }

  uint8_t module = (uint8_t)(uintptr_t)p1;
  moduleState[module].forced_off = 1;

  uint32_t timeout = p2;
  moduleState[module].counter = timeout;

  mixerTaskUnlock();
}
#endif

// return true if the request could be posted to the timer queue
bool restartModuleAsync(uint8_t module, uint8_t cnt_delay)
{
#if !defined(SIMU)
  PendedFunction_t cb = _setup_async_module_restart;
  return xTimerPendFunctionCall(cb, (void*)(uintptr_t)module, cnt_delay,
                                0/* do not wait */) == pdPASS;
#else
  return true;
#endif
}

void pulsesModuleSettingsUpdate(uint8_t module)
{
  moduleState[module].settings_updated = 1;
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
      protocol = PROTOCOL_CHANNELS_DSM2;
      break;
#endif

#if defined(CROSSFIRE)
    case MODULE_TYPE_CROSSFIRE:
      protocol = PROTOCOL_CHANNELS_CROSSFIRE;
      break;
#endif

#if defined(AFHDS2)
    case MODULE_TYPE_FLYSKY_AFHDS2A:
      protocol = PROTOCOL_CHANNELS_AFHDS2A;
      break;
#endif

#if defined(AFHDS3)
    case MODULE_TYPE_FLYSKY_AFHDS3:
      protocol = PROTOCOL_CHANNELS_AFHDS3;
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
  if (!ctx) {
    TRACE("Module #%d init failed", module);
    return;
  }

  mod->drv = drv;
  mod->ctx = ctx;

  // board specific hook
  if (_on_module_init)
    _on_module_init(module, drv);
  
  // power ON
  modulePortSetPower(module, true);
  TRACE("Module #%d init succeeded", module);
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
  TRACE("Module #%d de-init succeeded", module);
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
    case PROTOCOL_CHANNELS_DSM2:
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

#if defined(INTERNAL_MODULE_AFHDS3) || defined(AFHDS3)
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

// TODO: declare a function in telemetry
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

static bool _handle_async_restart(uint8_t module)
{
  auto& state = moduleState[module];
  if (state.forced_off) {
    if (state.counter > 0) {
      _deinit_module(module);
      state.protocol = PROTOCOL_CHANNELS_NONE;
      --state.counter;
      return true;
    } else {
      state.forced_off = 0;
    }
  }
  return false;
}

void pulsesSendNextFrame(uint8_t module)
{
  if (module >= MAX_MODULES) return;

  uint8_t protocol = getRequiredProtocol(module);

  auto& state = moduleState[module];
  if (state.protocol != protocol || state.forced_off) {

    if (_telemetryIsPolling) {
      // In case the telemetry timer is currently polling the port,
      // we just yield in the hope it will be different next time.
      return;
    }

    if (_handle_async_restart(module))
      return;
    
    pulsesEnableModule(module, protocol);
    moduleState[module].protocol = protocol;
    return;
  }

  auto mod = &(_module_drivers[module]);
  if (mod->drv) {
    auto drv = mod->drv;
    auto ctx = mod->ctx;

    if (state.settings_updated) {
      if (drv->onConfigChange) drv->onConfigChange(ctx);
      state.settings_updated = 0;
    }

    uint8_t channelStart = g_model.moduleData[module].channelsStart;
    int16_t* channels = &channelOutputs[channelStart];
    uint8_t nChannels = 16;  // TODO: MAX_CHANNELS - channelsStart

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
