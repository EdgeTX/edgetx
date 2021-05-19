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

#include "opentx.h"
#include "io/frsky_pxx2.h"
#include "pulses/pxx2.h"
#include "mixer_scheduler.h"

uint8_t s_pulses_paused = 0;
ModuleState moduleState[NUM_MODULES];
InternalModulePulsesData intmodulePulsesData __DMA;
ExternalModulePulsesData extmodulePulsesData __DMA;
TrainerPulsesData trainerPulsesData __DMA;

//use only for PXX
void ModuleState::startBind(BindInformation * destination, ModuleCallback bindCallback)
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
  uint8_t protocol;

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

#if defined(AFHDS3)
    case MODULE_TYPE_AFHDS3:
      protocol = PROTOCOL_CHANNELS_AFHDS3;
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
#if defined(PCBSKY9X)
      extmoduleSerialStart(DSM2_BAUDRATE, DSM2_PERIOD * 2000, false);
#else
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, DSM2_PERIOD);
#endif
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
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_PERIOD);
      break;

    case PROTOCOL_CHANNELS_PXX2_LOWSPEED:
      extmoduleInvertedSerialStart(PXX2_LOWSPEED_BAUDRATE);
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_PERIOD);
      break;
#endif

#if defined(MULTIMODULE)
    case PROTOCOL_CHANNELS_MULTIMODULE:
#if defined(PCBSKY9X)
      extmoduleSerialStart(MULTIMODULE_BAUDRATE, MULTIMODULE_PERIOD * 2000, true);
#else
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, MULTIMODULE_PERIOD);
#endif
      break;
#endif

#if defined(SBUS)
    case PROTOCOL_CHANNELS_SBUS:
#if defined(PCBSKY9X)
      extmoduleSerialStart(SBUS_BAUDRATE, SBUS_PERIOD_HALF_US, false);
#else
      extmoduleSerialStart();
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, SBUS_PERIOD);
#endif
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
      extmoduleSerialStart(/*AFHDS3_BAUDRATE, AFHDS3_COMMAND_TIMEOUT * 2000, false*/);
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, AFHDS3_COMMAND_TIMEOUT * 1000 /* us */);
      break;
#endif

    default:
      // external module stopped, set period to 50ms (necessary for USB Joystick, for instance)
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, 50000/*us*/);
      break;
  }
}

bool setupPulsesExternalModule(uint8_t protocol)
{
  switch (protocol) {
#if defined(PXX1)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      extmodulePulsesData.pxx.setupFrame(EXTERNAL_MODULE);
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, PXX_PULSES_PERIOD);
#endif
      return true;
#endif

#if defined(PXX1) && defined(HARDWARE_EXTERNAL_MODULE_SIZE_SML)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      extmodulePulsesData.pxx_uart.setupFrame(EXTERNAL_MODULE);
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, EXTMODULE_PXX1_SERIAL_PERIOD);
#endif
      return true;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
    case PROTOCOL_CHANNELS_PXX2_LOWSPEED:
      extmodulePulsesData.pxx2.setupFrame(EXTERNAL_MODULE);
#if defined(PCBSKY9X)
      sheduleNextMixerCalculation(EXTERNAL_MODULE, PXX2_PERIOD);
#endif
      return true;
#endif

#if defined(SBUS)
    case PROTOCOL_CHANNELS_SBUS:
      setupPulsesSbus();
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, SBUS_PERIOD);
#else
      // SBUS_PERIOD is not a constant! It can be set from UI
      mixerSchedulerSetPeriod(EXTERNAL_MODULE, SBUS_PERIOD);
      return true;
#endif
#endif

#if defined(DSM2)
    case PROTOCOL_CHANNELS_DSM2_LP45:
    case PROTOCOL_CHANNELS_DSM2_DSM2:
    case PROTOCOL_CHANNELS_DSM2_DSMX:
      setupPulsesDSM2();
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, DSM2_PERIOD);
#endif
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
      setupPulsesCrossfire();
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, CROSSFIRE_PERIOD);
#endif
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
#if defined(PCBSKPCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, GHOST_PERIOD);
#endif
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
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, MULTIMODULE_PERIOD);
#endif
      return true;
    }
#endif

#if defined(PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMExternalModule();
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, PPM_PERIOD(EXTERNAL_MODULE));
#endif
      return true;
#endif

#if defined(AFHDS3)
    case PROTOCOL_CHANNELS_AFHDS3:
      extmodulePulsesData.afhds3.setupFrame();
#if defined(PCBSKY9X)
      scheduleNextMixerCalculation(EXTERNAL_MODULE, AFHDS3_COMMAND_TIMEOUT);
#endif
      return true;
#endif

    default:
      return false;
  }
}
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
static void enablePulsesInternalModule(uint8_t protocol)
{
  // start new protocol hardware here

  switch (protocol) {
#if defined(PXX1) && !defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_PULSES:
      intmodulePxx1PulsesStart();
#if defined(INTMODULE_HEARTBEAT)
      // use backup trigger (1 ms later)
      init_intmodule_heartbeat();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD + 1000/*us*/);
#else
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
#endif
      break;
#endif

#if defined(PXX1) && defined(INTMODULE_USART)
    case PROTOCOL_CHANNELS_PXX1_SERIAL:
      intmodulePxx1SerialStart();
#if defined(INTMODULE_HEARTBEAT)
      // use backup trigger (1 ms later)
      init_intmodule_heartbeat();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD + 1000/*us*/);
#else
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
#endif
      break;
#endif

#if defined(PXX2)
    case PROTOCOL_CHANNELS_PXX2_HIGHSPEED:
      intmoduleSerialStart(PXX2_HIGHSPEED_BAUDRATE, true, USART_Parity_No, USART_StopBits_1, USART_WordLength_8b);
      resetAccessAuthenticationCount();

#if defined(INTMODULE_HEARTBEAT)
      // use backup trigger (1 ms later)
      init_intmodule_heartbeat();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD + 1000/*us*/);
#else
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
#endif
      break;
#endif

#if defined(INTERNAL_MODULE_MULTI)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      intmodulePulsesData.multi.initFrame();
      intmoduleSerialStart(MULTIMODULE_BAUDRATE, true, USART_Parity_Even, USART_StopBits_2, USART_WordLength_9b);
      mixerSchedulerSetPeriod(INTERNAL_MODULE, MULTIMODULE_PERIOD);
      break;
#endif

#if defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      intmodulePpmStart();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PPM_PERIOD(INTERNAL_MODULE));
      break;
#endif

    default:
      // internal module stopped, set internal period to 0 and start the scheduler
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
#if defined(INTMODULE_HEARTBEAT)
      mixerSchedulerResetTimer();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD + 1000 /* backup */);
#else
      mixerSchedulerSetPeriod(INTERNAL_MODULE, INTMODULE_PXX1_SERIAL_PERIOD);
#endif
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
#if defined(INTMODULE_HEARTBEAT)
        mixerSchedulerResetTimer();
        mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD + 2000 /* backup */);
#else
        mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
#endif
      }
      return result;
    }
#endif

#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
    case PROTOCOL_CHANNELS_PPM:
      setupPulsesPPMInternalModule();
      // probably useless, as the interval did not change since "enable" function
      mixerSchedulerSetPeriod(INTERNAL_MODULE, PPM_PERIOD(INTERNAL_MODULE));
      return true;
#endif

#if defined(INTERNAL_MODULE_MULTI)
    case PROTOCOL_CHANNELS_MULTIMODULE:
      setupPulsesMultiInternalModule();
      mixerSchedulerSetPeriod(INTERNAL_MODULE, MULTIMODULE_PERIOD);
      return true;
#endif

    default:
      //mixerSchedulerSetPeriod(INTERNAL_MODULE, 10000 /*us*/); // used for USB sim for example
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

bool setupPulsesInternalModule()
{
  uint8_t protocol = getRequiredProtocol(INTERNAL_MODULE);

  heartbeat |= (HEART_TIMER_PULSES << INTERNAL_MODULE);

  if (moduleState[INTERNAL_MODULE].protocol != protocol) {
    intmoduleStop();
    moduleState[INTERNAL_MODULE].protocol = protocol;
    enablePulsesInternalModule(protocol);
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
    moduleState[EXTERNAL_MODULE].protocol = protocol;
    enablePulsesExternalModule(protocol);
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
