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

#include "dsm2.h"
#include "hal/module_port.h"
#include "mixer_scheduler.h"

#include "edgetx.h"

#define DSM2_SEND_BIND                     (1 << 7)
#define DSM2_SEND_RANGECHECK               (1 << 5)


// DSM2 control bits
#define DSM2_CHANS           6
#define FRANCE_BIT           0x10
#define DSMX_BIT             0x08
#define BAD_DATA             0x47

#define DSM2_BITRATE         125000
#define DSM2_FRAME_SIZE      14

static bool _bind_restart = false;

const etx_serial_init dsmUartParams = {
    .baudrate = 0,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX,
    .polarity = ETX_Pol_Inverted,
};

etx_module_state_t* dsmInit(uint8_t module, uint32_t baudrate,
                            uint16_t period, bool telemetry)
{
  // only external module supported
  if (module == INTERNAL_MODULE) return nullptr;

  etx_serial_init params(dsmUartParams);
  params.baudrate = baudrate;
  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, true);
  if (!mod_st) return nullptr;

  if (telemetry) {
    params.direction = ETX_Dir_RX;
    modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &params, true);
  }

  mixerSchedulerSetPeriod(module, period);
  return mod_st;
}

void dsmDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void* dsm2Init(uint8_t module)
{
  auto mod_st = dsmInit(module, DSM2_BITRATE, DSM2_PERIOD, false);
  mod_st->user_data = (void*)(uintptr_t)g_model.moduleData[module].subType;
  return (void*)mod_st;
}

static inline void sendByte(uint8_t*& p_buf, uint8_t b) { *p_buf++ = b; }

static void setupPulsesDSM2(uint8_t module, uint8_t type, uint8_t*& p_buf)
{
  uint8_t dsmDat[DSM2_FRAME_SIZE];

  switch (type) {
    case DSM2_PROTO_LP45:
      dsmDat[0] = 0x00;
      break;
    case DSM2_PROTO_DSM2:
      dsmDat[0] = 0x10;
      break;
    default: // DSM2_PROTO_DSMX
      dsmDat[0] = 0x10 | DSMX_BIT;
      break;
  }

  if (moduleState[module].mode == MODULE_MODE_BIND) {

    // The module is set to OFF during one second,
    // before BIND starts. It will start with bind packets
    // directly after restarts.
    if (!_bind_restart) {
      _bind_restart = true;
      // approx 1.1s @ 22ms cycle
      restartModuleAsync(module, 50);
    }
    
    dsmDat[0] |= DSM2_SEND_BIND;
  } else {
    _bind_restart = false;
  }

  if (moduleState[module].mode == MODULE_MODE_RANGECHECK) {
    dsmDat[0] |= DSM2_SEND_RANGECHECK;
  }

  dsmDat[1] = g_model.header.modelId[module]; // DSM2 Header second byte for model match

  for (int i = 0; i < DSM2_CHANS; i++) {
    int channel = g_model.moduleData[module].channelsStart + i;
    int value =
        channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;

    uint16_t pulse = limit(0, ((value * 13) >> 5) + 512, 1023);
    dsmDat[2 + 2 * i] = (i << 2) | ((pulse >> 8) & 0x03);
    dsmDat[3 + 2 * i] = pulse & 0xff;
  }

  for (int i = 0; i < DSM2_FRAME_SIZE; i++) {
    sendByte(p_buf, dsmDat[i]);
  }
}


static void dsm2SendPulses(void* ctx, uint8_t* buffer, int16_t* channels,
                           uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto drv    = modulePortGetSerialDrv(mod_st->tx);
  auto drvCtx = modulePortGetCtx(mod_st->tx);

  auto type = (uint8_t)(uintptr_t)mod_st->user_data;

  auto p_data = buffer;
  setupPulsesDSM2(module, type, p_data);
  drv->sendBuffer(drvCtx, buffer, p_data - buffer);
}

static void dsm2ConfigChange(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto type = (uint8_t)(uintptr_t)mod_st->user_data;

  if (type != g_model.moduleData[module].subType) {
    // restart during next mixer cycle
    restartModuleAsync(module, 0);    
  }
}

// No telemetry
const etx_proto_driver_t DSM2Driver = {
  .protocol = PROTOCOL_CHANNELS_DSM2,
  .init = dsm2Init,
  .deinit = dsmDeInit,
  .sendPulses = dsm2SendPulses,
  .processData = nullptr,
  .processFrame = nullptr,
  .onConfigChange = dsm2ConfigChange,
  .txCompleted = modulePortSerialTxCompleted,
};