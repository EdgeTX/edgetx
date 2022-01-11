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

#include "io/frsky_pxx2.h"
#include "io/multi_protolist.h"
#include "pulses/pxx2.h"
#include "pulses/flysky.h"

#if defined(HARDWARE_INTERNAL_MODULE)
  #if defined(INTMODULE_USART)
    #include "intmodule_serial_driver.h"
  #else
    #include "intmodule_pulses_driver.h"
  #endif
#endif

uint8_t s_pulses_paused = 0;
ModuleState moduleState[NUM_MODULES];
InternalModulePulsesData intmodulePulsesData __DMA;
ExternalModulePulsesData extmodulePulsesData __DMA;
TrainerPulsesData trainerPulsesData __DMA;

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
  if (moduleIdx == EXTERNAL_MODULE && isModuleAFHDS3(moduleIdx)) {
    extmodulePulsesData.afhds3.getStatusString(statusText);
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
#if defined(AFHDS3)
  if (moduleIdx == EXTERNAL_MODULE && isModuleAFHDS3(moduleIdx)) {
    extmodulePulsesData.afhds3.getPowerStatus(statusText);
  }
#endif
}

#if defined(AFHDS3)
uint8_t actualAfhdsRunPower(int moduleIndex)
{
  if (moduleIndex == EXTERNAL_MODULE && isModuleAFHDS3(moduleIndex)) {
    return (uint8_t)extmodulePulsesData.afhds3.actualRunPower();
  }
  return 0;
}
#endif

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
      protocol = limit<uint8_t>(PROTOCOL_CHANNELS_DSM2_LP45, PROTOCOL_CHANNELS_DSM2_LP45+g_model.moduleData[module].rfProtocol, PROTOCOL_CHANNELS_DSM2_DSMX);
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

#if defined(HARDWARE_EXTERNAL_MODULE)
void enablePulsesExternalModule(uint8_t protocol)
{
  // start new protocol hardware here

  switch (protocol) {
#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmodulePxx1PulsesStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX_PULSES_PERIOD);
      break;
#endif

#if defined(PXX1) && defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      extmodulePxx1SerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, EXTMODULE_PXX1_SERIAL_PERIOD);
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
      EXTERNAL_MODULE_ON();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, CROSSFIRE_PERIOD);
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
      extmoduleInvertedSerialStart(PXX2_HIGHSPEED_BAUDRATE);
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_NO_HEARTBEAT_PERIOD);
      break;

    case PROTOCOL_CHANNELS_PXX2_LOWSPEED:
      extmoduleInvertedSerialStart(PXX2_LOWSPEED_BAUDRATE);
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_NO_HEARTBEAT_PERIOD);
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
      extmodulePpmStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PPM_PERIOD(EXTERNAL_MODULE));
      break;
#endif

#if defined(AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      extmodulePulsesData.afhds3.init(EXTERNAL_MODULE);
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, AFHDS3_COMMAND_TIMEOUT * 1000 /* us */);
      break;
#endif

    default:
      // external module stopped, use default mixer period
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
      break;
  }
}

bool setupPulsesExternalModule(uint8_t protocol)
{
  switch (protocol) {

#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmodulePulsesData.pxx.setupFrame(EXTERNAL_MODULE);
      return true;
#endif

#if defined(PXX1) && defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      extmodulePulsesData.pxx_uart.setupFrame(EXTERNAL_MODULE);
      return true;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
    case PROTOCOL_CHANNELS_PXX2_LOWSPEED:
      extmodulePulsesData.pxx2.setupFrame(EXTERNAL_MODULE);
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

#if defined(CROSSFIRE)
    case PROTOCOL_CHANNELS_CROSSFIRE:
    {
      ModuleSyncStatus& status = getModuleSyncStatus(EXTERNAL_MODULE);
      if (status.isValid())
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, status.getAdjustedRefreshRate());
      else
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, CROSSFIRE_PERIOD);
      setupPulsesCrossfire(EXTERNAL_MODULE);
      return true;
    }
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
    {
      ModuleSyncStatus& status = getModuleSyncStatus(EXTERNAL_MODULE);
      if (status.isValid())
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, status.getAdjustedRefreshRate());
      else
        mixerSchedulerSetPeriod(EXTERNAL_MODULE, MULTIMODULE_PERIOD);
      setupPulsesMultiExternalModule();
      return true;
    }
#endif

#if defined(PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMExternalModule();
      return true;
#endif

#if defined(AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      extmodulePulsesData.afhds3.setupFrame();
      return true;
#endif

    default:
      return false;
  }
}
#endif

#if defined(HARDWARE_INTERNAL_MODULE)

#if defined(INTERNAL_MODULE_CRSF)
static void intmoduleCRSF_rx(uint8_t data)
{
  intmoduleFifo.push(data);

#if !defined(SIMU)
  // wakeup mixer when rx buffer is quarter full (16 bytes)
  if (intmoduleFifo.size() >= 16) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(mixerTaskId.rtos_handle, 0, eNoAction,
                       &xHigherPriorityTaskWoken);

    // might effect a context switch on ISR exit
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}
#endif

static void enablePulsesInternalModule(uint8_t protocol)
{
  // start new protocol hardware here

  switch (protocol) {
#if defined(PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmodulePxx1PulsesStart();
#if defined(INTMODULE_HEARTBEAT)
      init_intmodule_heartbeat();
#endif
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
      break;
#endif

#if defined(PXX1) && defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      intmodulePxx1SerialStart();
#if defined(INTMODULE_HEARTBEAT)
      init_intmodule_heartbeat();
#endif
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
      break;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED: {
      etx_serial_init params;
      params.baudrate = PXX2_HIGHSPEED_BAUDRATE;
      params.rx_enable = true;

      intmoduleFifo.clear();
      intmoduleSerialStart(&params);
      resetAccessAuthenticationCount();

#if defined(INTMODULE_HEARTBEAT)
      // use backup trigger (1 ms later)
      init_intmodule_heartbeat();
#endif
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
    } break;
#endif

#if defined(INTERNAL_MODULE_MULTI)
    case PROTOCOL_CHANNELS_MULTIMODULE: {

      // serial port setup
      etx_serial_init params;
      params.baudrate    = MULTIMODULE_BAUDRATE;
      params.rx_enable   = true;
      params.parity      = USART_Parity_Even;
      params.stop_bits   = USART_StopBits_2;
      params.word_length = USART_WordLength_9b;

      intmodulePulsesData.multi.initFrame();
      intmoduleFifo.clear();
      intmoduleSerialStart(&params);

      // mixer setup
      mixerSchedulerSetPeriod(INTERNAL_MODULE, MULTIMODULE_PERIOD);

      // reset status
      getMultiModuleStatus(INTERNAL_MODULE).failsafeChecked = false;
      getMultiModuleStatus(INTERNAL_MODULE).flags = 0;

#if defined(MULTI_PROTOLIST)
      TRACE("enablePulsesInternalModule(): trigger scan");
      MultiRfProtocols::instance(INTERNAL_MODULE)->triggerScan();
      TRACE("counter = %d", moduleState[INTERNAL_MODULE].counter);
#endif
    } break;
#endif

#if defined(INTERNAL_MODULE_CRSF)
    case PROTOCOL_CHANNELS_CROSSFIRE: {

      // serial port setup
      etx_serial_init params;
      params.baudrate  = CROSSFIRE_BAUDRATE;
      params.rx_enable = true;

      // wakeup mixer when rx buffer is quarter full (16 bytes)
      params.on_receive = intmoduleCRSF_rx;

      intmoduleFifo.clear();
      intmoduleSerialStart(&params);

      // mixer setup
      mixerSchedulerSetPeriod(INTERNAL_MODULE, CROSSFIRE_PERIOD);
    } break;
#endif

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      intmodulePpmStart();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PPM_PERIOD(INTERNAL_MODULE));
      break;
#endif

#if defined(AFHDS2)
    case PROTOCOL_CHANNELS_AFHDS2A: {

      // serial port setup
      etx_serial_init params;
      params.baudrate  = INTMODULE_USART_AFHDS2_BAUDRATE;
      params.rx_enable = true;

      resetPulsesAFHDS2();
      intmoduleFifo.clear();
      intmoduleSerialStart(&params);

      // mixer setup
      mixerSchedulerSetPeriod(INTERNAL_MODULE, AFHDS2_PERIOD);
    } break;
#endif

    default:
      // internal module stopped, use default mixer period
      mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
      break;
  }
}

bool setupPulsesInternalModule(uint8_t protocol)
{
  switch (protocol) {
#if defined(HARDWARE_INTERNAL_MODULE) && defined(PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmodulePulsesData.pxx.setupFrame(INTERNAL_MODULE);
      return true;
#endif

#if defined(PXX1) && defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      intmodulePulsesData.pxx_uart.setupFrame(INTERNAL_MODULE);
      return true;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
    {
      bool result = intmodulePulsesData.pxx2.setupFrame(INTERNAL_MODULE);
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_SPECTRUM_ANALYSER || moduleState[INTERNAL_MODULE].mode == MODULE_MODE_POWER_METER) {
        mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_TOOLS_PERIOD);
      }
      else {
        mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
      }
      return result;
    }
#endif

#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMInternalModule();
      return true;
#endif

#if defined(INTERNAL_MODULE_CRSF)
    case PROTOCOL_CHANNELS_CROSSFIRE:
    {
      ModuleSyncStatus& status = getModuleSyncStatus(INTERNAL_MODULE);
      if (status.isValid())
        mixerSchedulerSetPeriod(INTERNAL_MODULE, status.getAdjustedRefreshRate());
      else
        mixerSchedulerSetPeriod(INTERNAL_MODULE, CROSSFIRE_PERIOD);
      setupPulsesCrossfire(INTERNAL_MODULE);
      return true;
    }
#endif

#if defined(INTERNAL_MODULE_MULTI)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      setupPulsesMultiInternalModule();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, MULTIMODULE_PERIOD);
      return true;
#endif

#if defined(AFHDS2)
    case PROTOCOL_CHANNELS_AFHDS2A:
    { 
      ModuleSyncStatus& status = getModuleSyncStatus(INTERNAL_MODULE);
      mixerSchedulerSetPeriod(
          INTERNAL_MODULE,
          status.isValid() ? status.getAdjustedRefreshRate() : AFHDS2_PERIOD);
      status.invalidate();
      setupPulsesAFHDS2();
      return true;
    }
#endif

    default:
      return false;
  }
}

void stopPulsesInternalModule()
{
  if (moduleState[INTERNAL_MODULE].protocol != PROTOCOL_CHANNELS_UNINITIALIZED) {
    mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
    intmoduleStop();
    moduleState[INTERNAL_MODULE].protocol = PROTOCOL_CHANNELS_NONE;
  }
}

void intmoduleSendNextFrame()
{
  switch (moduleState[INTERNAL_MODULE].protocol) {
#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
      intmoduleSendBuffer(intmodulePulsesData.pxx2.getData(),
                          intmodulePulsesData.pxx2.getSize());
      break;
#endif

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      intmoduleSendNextFramePPM(
          intmodulePulsesData.ppm.pulses,
          intmodulePulsesData.ppm.ptr - intmodulePulsesData.ppm.pulses);
      break;
#endif

#if defined(PXX1)
#if defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      intmoduleSendBuffer(intmodulePulsesData.pxx_uart.getData(),
                          intmodulePulsesData.pxx_uart.getSize());
      break;
#else
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmoduleSendNextFramePxx1(intmodulePulsesData.pxx.getData(),
                                 intmodulePulsesData.pxx.getSize());
      break;
#endif
#endif

#if defined(INTERNAL_MODULE_MULTI)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      intmoduleSendBuffer(intmodulePulsesData.multi.getData(),
                          intmodulePulsesData.multi.getSize());
      break;
#endif

#if defined(INTERNAL_MODULE_CRSF)
    case PROTOCOL_CHANNELS_CROSSFIRE:
      intmoduleSendBuffer(intmodulePulsesData.crossfire.pulses,
                          intmodulePulsesData.crossfire.length);
      break;
#endif

#if defined(AFHDS2)
  case PROTOCOL_CHANNELS_AFHDS2A: {
    uint8_t* data = (uint8_t*)intmodulePulsesData.flysky.pulses;
    uint16_t size = intmodulePulsesData.flysky.ptr - data;
    intmoduleSendBuffer(data, size);
  } break;
#endif
  }
}

bool setupPulsesInternalModule()
{
  uint8_t protocol = getRequiredProtocol(INTERNAL_MODULE);

  heartbeat |= (HEART_TIMER_PULSES << INTERNAL_MODULE);

  if (moduleState[INTERNAL_MODULE].protocol != protocol) {
    intmoduleStop();
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
void stopPulsesExternalModule()
{
  if (moduleState[EXTERNAL_MODULE].protocol != PROTOCOL_CHANNELS_UNINITIALIZED) {
    mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
    extmoduleStop();
    moduleState[EXTERNAL_MODULE].protocol = PROTOCOL_CHANNELS_NONE;
  }
}

bool setupPulsesExternalModule()
{
  uint8_t protocol = getRequiredProtocol(EXTERNAL_MODULE);

  heartbeat |= (HEART_TIMER_PULSES << EXTERNAL_MODULE);

  if (moduleState[EXTERNAL_MODULE].protocol != protocol) {
    extmoduleStop();
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
