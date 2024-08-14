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

#include "edgetx.h"
#include "afhds2.h"
#include "flysky.h"

#include "mixer_scheduler.h"
#include "hal/module_port.h"

#include "pulses/pulses.h"
#include "telemetry/telemetry.h"


const etx_serial_init afhds2SerialInitParams = {
    .baudrate = INTMODULE_USART_AFHDS2_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
};

static void* afhds2Init(uint8_t module)
{
  (void)module;

  // serial port setup
  resetPulsesAFHDS2();
  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART,
                                     &afhds2SerialInitParams, false);

  // mixer setup
  mixerSchedulerSetPeriod(module, AFHDS2_PERIOD);

  return (void*)mod_st;
}

static void afhds2DeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void afhds2SendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  // TODO:
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  ModuleSyncStatus& status = getModuleSyncStatus(module);
  mixerSchedulerSetPeriod(module, status.isValid()
                                      ? status.getAdjustedRefreshRate()
                                      : AFHDS2_PERIOD);
  status.invalidate();

  auto p_data = buffer;
  setupPulsesAFHDS2(p_data);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);

  drv->sendBuffer(drv_ctx, buffer, p_data - buffer);
}

static void afhds2ProcessData(void*, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  processInternalFlySkyTelemetryData(data, buffer, len);
}

const etx_proto_driver_t Afhds2InternalDriver = {
  .protocol = PROTOCOL_CHANNELS_AFHDS2A,
  .init = afhds2Init,
  .deinit = afhds2DeInit,
  .sendPulses = afhds2SendPulses,
  .processData = afhds2ProcessData,
  .processFrame = nullptr,
  .onConfigChange = nullptr,
};
