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

int16_t ppmInput[MAX_TRAINER_CHANNELS];
uint8_t ppmInputValidityTimer;
uint8_t currentTrainerMode = 0xff;

void checkTrainerSignalWarning()
{
  enum {
    PPM_IN_IS_NOT_USED = 0,
    PPM_IN_IS_VALID,
    PPM_IN_INVALID
  };

  static uint8_t ppmInputValidState = PPM_IN_IS_NOT_USED;

  if (ppmInputValidityTimer && (ppmInputValidState == PPM_IN_IS_NOT_USED)) {
    ppmInputValidState = PPM_IN_IS_VALID;
  }
  else if (!ppmInputValidityTimer && (ppmInputValidState == PPM_IN_IS_VALID)) {
    ppmInputValidState = PPM_IN_INVALID;
    AUDIO_TRAINER_LOST();
  }
  else if (ppmInputValidityTimer && (ppmInputValidState == PPM_IN_INVALID)) {
    ppmInputValidState = PPM_IN_IS_VALID;
    AUDIO_TRAINER_BACK();
  }
}

#if defined(PCBSKY9X)
void stopTrainer()
{
  stop_trainer_capture();
}

void checkTrainerSettings()
{
  uint8_t requiredTrainerMode = SLAVE_MODE();

  if (requiredTrainerMode != currentTrainerMode) {
    currentTrainerMode = requiredTrainerMode;
    if (requiredTrainerMode)
      stopTrainer();
    else
      init_trainer_capture();
  }
}
#else
void stopTrainer()
{
  switch (currentTrainerMode) {
    case TRAINER_MODE_MASTER_TRAINER_JACK:
      stop_trainer_capture();
      break;

    case TRAINER_MODE_SLAVE:
      stop_trainer_ppm();
      break;

#if defined(TRAINER_MODULE_CPPM)
    case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
      stop_trainer_module_cppm();
      break;
#endif

#if defined(TRAINER_MODULE_SBUS)
    case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
      stop_trainer_module_sbus();
      break;
#endif

#if defined(TRAINER_BATTERY_COMPARTMENT)
    case TRAINER_MODE_MASTER_BATTERY_COMPARTMENT:
#if defined(AUX_SERIAL)
      if (g_eeGeneral.auxSerialMode == UART_MODE_SBUS_TRAINER)
        auxSerialStop();
#endif
#if defined(AUX2_SERIAL)
      if (g_eeGeneral.aux2SerialMode == UART_MODE_SBUS_TRAINER)
        aux2SerialStop();
#endif
      break;
#endif
  }

  currentTrainerMode = 0xFF;
}

void checkTrainerSettings()
{
  uint8_t requiredTrainerMode = g_model.trainerData.mode;

  if (requiredTrainerMode != currentTrainerMode) {
    if (currentTrainerMode != 0xFF) {
      stopTrainer();
    }

    currentTrainerMode = requiredTrainerMode;

    switch (requiredTrainerMode) {
      case TRAINER_MODE_SLAVE:
        init_trainer_ppm();
        break;

#if defined(TRAINER_MODULE_CPPM)
      case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
        init_trainer_module_cppm();
        break;
#endif

#if defined(TRAINER_MODULE_SBUS)
      case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
        init_trainer_module_sbus();
        break;
#endif

#if defined(TRAINER_BATTERY_COMPARTMENT)
      case TRAINER_MODE_MASTER_BATTERY_COMPARTMENT:
#if defined(AUX_SERIAL)
        if (g_eeGeneral.auxSerialMode == UART_MODE_SBUS_TRAINER) {
          auxSerialSbusInit();
          break;
        }
#endif

#if defined(AUX2_SERIAL)
        if (g_eeGeneral.aux2SerialMode == UART_MODE_SBUS_TRAINER) {
          aux2SerialSbusInit();
          break;
        }
#endif

        // no break
#endif

      case TRAINER_MODE_MASTER_TRAINER_JACK:
        init_trainer_capture();
        break;
    }

#if defined(TRAINER_MODULE_CPPM) || defined(TRAINER_MODULE_SBUS)
    if (requiredTrainerMode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE || requiredTrainerMode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE)
      stop_intmodule_heartbeat();
    else
      init_intmodule_heartbeat();
#else
#if defined(HARDWARE_EXTERNAL_ACCESS_MOD)
    if (g_model.moduleData[EXTERNAL_MODULE].type != MODULE_TYPE_R9M_PXX2) // externalaccessmod 'bridges' HB and Ext module RX pins
      init_intmodule_heartbeat();
#elif defined(INTMODULE_HEARTBEAT_GPIO)
    init_intmodule_heartbeat();
#endif
#endif
  }
}
#endif
