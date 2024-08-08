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

#include "definitions.h"
#include "dataconstants.h"
#include "pulses_common.h"
#include "modules_helpers.h"
#include "hal/module_driver.h"

#if defined(PXX2)
#include "pxx2.h"
#include "pxx2_ota.h"
#endif

#if defined(DSM2)
  #define IS_DSM2_PROTOCOL(protocol)         (protocol>=PROTOCOL_CHANNELS_DSM2_LP45 && protocol<=PROTOCOL_CHANNELS_DSM2_DSMX)
#else
  #define IS_DSM2_PROTOCOL(protocol)         (0)
#endif

#if defined(DSM2_SERIAL)
  #define IS_DSM2_SERIAL_PROTOCOL(protocol)  (IS_DSM2_PROTOCOL(protocol))
#else
  #define IS_DSM2_SERIAL_PROTOCOL(protocol)  (0)
#endif

#if defined(MULTIMODULE)
  #define IS_MULTIMODULE_PROTOCOL(protocol)  (protocol==PROTOCOL_CHANNELS_MULTIMODULE)
  #if !defined(DSM2)
     #error You need to enable DSM2 = PPM for MULTIMODULE support
  #endif
#else
  #define IS_MULTIMODULE_PROTOCOL(protocol)  (0)
#endif

#if defined(AFHDS3)
#define IS_AFHDS3_PROTOCOL(protocol)         (protocol == PROTOCOL_CHANNELS_AFHDS3)
#else
#define IS_AFHDS3_PROTOCOL(protocol)         (0)
#endif

typedef void (* ModuleCallback)();

PACK(struct ModuleState {
  uint8_t protocol;
  uint8_t mode:4;
  uint8_t forced_off:1;
  uint8_t settings_updated:1;
  uint8_t spare:2;
  uint16_t counter;

#if defined(PXX2)
  // PXX specific items
  union
  {
    ModuleInformation * moduleInformation;
    ModuleSettings * moduleSettings;
    ReceiverSettings * receiverSettings;
    BindInformation * bindInformation;
    OtaUpdateInformation * otaUpdateInformation;
  };
  ModuleCallback callback;

  void startBind(BindInformation * destination, ModuleCallback bindCallback = nullptr);
  void readModuleInformation(ModuleInformation * destination, int8_t first, int8_t last);
  void readModuleSettings(ModuleSettings * destination);
  void writeModuleSettings(ModuleSettings * source);
  void readReceiverSettings(ReceiverSettings * destination);
  void writeReceiverSettings(ReceiverSettings * source);
#endif
});

extern ModuleState moduleState[NUM_MODULES];

inline bool isModuleBeeping(uint8_t moduleIndex)
{
#if defined(MULTIMODULE)
  if (getMultiBindStatus(moduleIndex) != MULTI_BIND_NONE)
    return true;
#endif

  return moduleState[moduleIndex].mode >= MODULE_MODE_BEEP_FIRST;
}

template<class T> struct PpmPulsesData {
  T pulses[20];
  T * ptr;
};

#define MAX_PULSES_TRANSITIONS 300
PACK(struct Dsm2TimerPulsesData {
  pulse_duration_t pulses[MAX_PULSES_TRANSITIONS];
  pulse_duration_t * ptr;
  uint8_t index;
});
typedef Dsm2TimerPulsesData Dsm2PulsesData;

#define PPM_DEF_PERIOD               225 /* 22.5ms */
#define PPM_STEP_SIZE                5 /*0.5ms*/
#define PPM_PERIOD_FL_TO_HALF_US(fl) (((fl)*PPM_STEP_SIZE+PPM_DEF_PERIOD)*200) /* half us*/
#define PPM_PERIOD_HALF_US(module)   PPM_PERIOD_FL_TO_HALF_US(g_model.moduleData[module].ppm.frameLength) /*half us*/
#define PPM_TRAINER_PERIOD_HALF_US() PPM_PERIOD_FL_TO_HALF_US(g_model.trainerData.frameLength) /*half us*/
#define PPM_PERIOD(module)           (PPM_PERIOD_HALF_US(module) / 2) /*us*/
#define DSM2_BAUDRATE                125000
#define DSM2_PERIOD                  22000 /*us*/
#define SBUS_BAUDRATE                100000
#define SBUS_MIN_PERIOD              60  /*6.0ms 1/10ms*/
#define SBUS_MAX_PERIOD              325 /*Overflows uint16_t if set higher*/
#define SBUS_DEF_PERIOD              225
#define SBUS_STEPSIZE                5   /* SBUS Step Size 0.5ms */
#define SBUS_PERIOD_HALF_US(m)       ((g_model.moduleData[m].sbus.refreshRate * SBUS_STEPSIZE + SBUS_DEF_PERIOD) * 200) /*half us*/
#define SBUS_PERIOD(m)               (SBUS_PERIOD_HALF_US(m) / 2) /*us*/
#define MULTIMODULE_BAUDRATE         100000
#define MULTIMODULE_PERIOD           7000 /*us*/

#define CROSSFIRE_FRAME_MAXLEN         64
#define GHOST_FRAME_MAXLEN             16

union TrainerPulsesData {
  PpmPulsesData<trainer_pulse_duration_t> ppm;
};

extern TrainerPulsesData trainerPulsesData;

#if !defined(AFHDS3)
  #define MODULE_BUFFER_SIZE 64
#else
  #define MODULE_BUFFER_SIZE 128
#endif

struct module_pulse_buffer {
  uint8_t _buffer[MODULE_BUFFER_SIZE];
};

struct module_pulse_driver {
  module_pulse_buffer buffer;
  const etx_proto_driver_t* drv;
  void* ctx;
};

module_pulse_driver* pulsesGetModuleDriver(uint8_t module);
uint8_t* pulsesGetModuleBuffer(uint8_t module);

void pulsesStopModule(uint8_t module);
void pulsesSendNextFrame(uint8_t module);
void pulsesSendChannels();

typedef void (*module_init_cb_t)(uint8_t, const etx_proto_driver_t*);
typedef void (*module_deinit_cb_t)(uint8_t, const etx_proto_driver_t*);

void pulsesSetModuleInitCb(module_init_cb_t cb);
void pulsesSetModuleDeInitCb(module_deinit_cb_t cb);

void restartModule(uint8_t module);
bool restartModuleAsync(uint8_t module, uint8_t cnt_delay);

// Re-Init module
// 
// Note: this can only be used from within
//       module init.
void pulsesRestartModuleUnsafe(uint8_t module);

void pulsesModuleSettingsUpdate(uint8_t module);

void setupPulsesPPMTrainer();

void getModuleStatusString(uint8_t moduleIdx, char * statusText);
void getModuleSyncStatusString(uint8_t moduleIdx, char * statusText);

#if defined(AFHDS3)
uint8_t actualAfhdsRunPower(int moduleIndex);
#endif

void pulsesInit();
void pulsesStart();
void pulsesStop();

inline void SEND_FAILSAFE_NOW(uint8_t idx)
{
  moduleState[idx].counter = 1;
}

inline void SEND_FAILSAFE_1S()
{
  for (uint8_t i=0; i<NUM_MODULES; i++) {
    moduleState[i].counter = 100;
  }
}

// Assign failsafe values using the current channel outputs
// for channels not set previously to HOLD or NOPULSE
void setCustomFailsafe(uint8_t moduleIndex);

inline bool isModuleInRangeCheckMode()
{
  if (moduleState[0].mode == MODULE_MODE_RANGECHECK)
    return true;

#if NUM_MODULES > 1
  if (moduleState[1].mode == MODULE_MODE_RANGECHECK)
    return true;
#endif

  return false;
}

inline bool isModuleInBeepMode()
{
  if (moduleState[0].mode >= MODULE_MODE_BEEP_FIRST)
    return true;

#if NUM_MODULES > 1
  if (moduleState[1].mode >= MODULE_MODE_BEEP_FIRST)
    return true;
#endif

  return false;
}
