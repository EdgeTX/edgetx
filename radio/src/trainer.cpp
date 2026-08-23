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

#include "hal/trainer_driver.h"

#include "edgetx.h"
#include "serial.h"

// Timer gets decremented in per10ms()
#define TRAINER_IN_VALID_TIMEOUT 100 // 1s

int16_t trainerInput[MAX_TRAINER_CHANNELS];
uint8_t trainerInputValidityTimer;
uint8_t currentTrainerMode = 0xff;

void (*_on_change_cb)(uint8_t, uint8_t) = nullptr;

// Active signal received
bool isTrainerValid() { return trainerInputValidityTimer != 0; }

void trainerResetTimer()
{
  trainerInputValidityTimer = TRAINER_IN_VALID_TIMEOUT;
}

void trainerDecTimer()
{
  if (trainerInputValidityTimer) trainerInputValidityTimer--;
}

void trainerSetTimer(uint16_t t)
{
  trainerInputValidityTimer = t;
}

enum {
  TRAINER_NOT_CONNECTED = 0,
  TRAINER_CONNECTED,
  TRAINER_DISCONNECTED,
  TRAINER_RECONNECTED
};
uint8_t trainerStatus = TRAINER_NOT_CONNECTED;

bool isTrainerConnected()
{
  return trainerStatus == TRAINER_CONNECTED || trainerStatus == TRAINER_RECONNECTED;
}

void checkTrainerSignalWarning()
{
  enum {
    TRAINER_IN_IS_NOT_USED = 0,
    TRAINER_IN_IS_VALID,
    TRAINER_IN_IS_INVALID
  };

  static uint8_t trainerInputValidState = TRAINER_IN_IS_NOT_USED;

  if (trainerInputValidityTimer && (trainerInputValidState == TRAINER_IN_IS_NOT_USED)) {
    trainerInputValidState = TRAINER_IN_IS_VALID;
    trainerStatus = TRAINER_CONNECTED;
    AUDIO_TRAINER_CONNECTED();
  }
  else if (!trainerInputValidityTimer && (trainerInputValidState == TRAINER_IN_IS_VALID)) {
    trainerInputValidState = TRAINER_IN_IS_INVALID;
    trainerStatus = TRAINER_DISCONNECTED;
    AUDIO_TRAINER_LOST();
  }
  else if (trainerInputValidityTimer && (trainerInputValidState == TRAINER_IN_IS_INVALID)) {
    trainerInputValidState = TRAINER_IN_IS_VALID;
    trainerStatus = TRAINER_RECONNECTED;
    AUDIO_TRAINER_BACK();
  }
}

static bool trainer_init_aux_sbus();
static void trainer_init_module_sbus();
static void trainer_stop_module_sbus();

void stopTrainer()
{
  switch (currentTrainerMode) {
    case TRAINER_MODE_MASTER_TRAINER_JACK:
    case TRAINER_MODE_SLAVE:
      trainer_stop_dsc();
      break;

    case TRAINER_MODE_MASTER_SERIAL:
      sbusTrainerRelease();
      break;

    case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
      trainer_stop_module_cppm();
      break;

    case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
      trainer_stop_module_sbus();
      break;
  }

  if (_on_change_cb) {
    _on_change_cb(currentTrainerMode, 0xFF);
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

    switch (requiredTrainerMode) {
      case TRAINER_MODE_MASTER_TRAINER_JACK:
        trainer_init_dsc_in();
        break;

      case TRAINER_MODE_SLAVE:
        trainer_init_dsc_out();
        break;

      case TRAINER_MODE_MASTER_SERIAL:
        trainer_init_aux_sbus();
        break;

      case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
        trainer_init_module_cppm();
        break;

      case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
        trainer_init_module_sbus();
        break;
    }

    if (_on_change_cb) {
      _on_change_cb(currentTrainerMode, requiredTrainerMode);
    }
    currentTrainerMode = requiredTrainerMode;

  } else if (currentTrainerMode == TRAINER_MODE_MASTER_SERIAL &&
             !sbusTrainerActive()) {
    // The serial port backing this mode can be (re-)configured at any time
    // from the radio settings, which drops us as its user. Pick it up again
    // as soon as it is usable, rather than relying on every caller of
    // serialInit() to notify us.
    trainer_init_aux_sbus();
  }
}

void trainerSetChangeCb(void (*changeCb)(uint8_t, uint8_t))
{
  _on_change_cb = changeCb;
}


static const etx_serial_init sbusTrainerParams = {
    .baudrate = SBUS_BAUDRATE,
    .encoding = ETX_Encoding_8E2,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Normal,
};

static etx_module_state_t* sbus_trainer_mod_st = nullptr;

// SBUS trainer input from an AUX serial port
static bool trainer_init_aux_sbus()
{
  int port_nr = serialGetSbusTrainerPort();
  if (port_nr < 0) return false;

  void* ctx;
  const etx_serial_driver_t* drv;
  if (!serialGetPortCtx(port_nr, &ctx, &drv)) return false;

  return sbusTrainerAcquire(ctx, drv);
}

// SBUS trainer input from the external module bay
static void trainer_init_module_sbus()
{
  if (sbus_trainer_mod_st) return;

  // check if UART with inverter on heartbeat pin is available
  sbus_trainer_mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_UART,
                                             &sbusTrainerParams, false);
  if(sbus_trainer_mod_st == nullptr) {
    // otherwise fall back to S.Port pin
    sbus_trainer_mod_st = modulePortInitSerial(
        EXTERNAL_MODULE, ETX_MOD_PORT_SPORT, &sbusTrainerParams, false);
  }

  if (!sbus_trainer_mod_st) return;

  auto drv = modulePortGetSerialDrv(sbus_trainer_mod_st->rx);
  auto ctx = modulePortGetCtx(sbus_trainer_mod_st->rx);

  if (!sbusTrainerAcquire(ctx, drv)) {
    modulePortDeInit(sbus_trainer_mod_st);
    sbus_trainer_mod_st = nullptr;
    return;
  }

  // switch ON
  modulePortSetPower(EXTERNAL_MODULE, true);
}

static void trainer_stop_module_sbus()
{
  if (!sbus_trainer_mod_st) return;

  // release before modulePortDeInit() invalidates the driver context
  sbusTrainerRelease();

  modulePortDeInit(sbus_trainer_mod_st);
  modulePortSetPower(EXTERNAL_MODULE,false);
  sbus_trainer_mod_st = nullptr;
}
