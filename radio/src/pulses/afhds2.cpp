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
#include "afhds2.h"
#include "flysky.h"

#include "intmodule_serial_driver.h"
#include "mixer_scheduler.h"

#include "pulses/pulses.h"
#include "telemetry/telemetry.h"




const etx_serial_init afhds2SerialInitParams = {
    .baudrate = INTMODULE_USART_AFHDS2_BAUDRATE,
    .parity = ETX_Parity_None,
    .stop_bits = ETX_StopBits_One,
    .word_length = ETX_WordLength_8,
    .rx_enable = true,
};

static void* afhds2Init(uint8_t module)
{
  (void)module;

  // serial port setup
  resetPulsesAFHDS2();
  void* ctx = IntmoduleSerialDriver.init(&afhds2SerialInitParams);

  // mixer setup
  mixerSchedulerSetPeriod(INTERNAL_MODULE, AFHDS2_PERIOD);
  INTERNAL_MODULE_ON();

  return ctx;
}

static void afhds2DeInit(void* context)
{
  INTERNAL_MODULE_OFF();
  IntmoduleSerialDriver.deinit(context);

  // mixer setup
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
}

static void afhds2SetupPulses(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;

  // TODO:
  (void)channels;
  (void)nChannels;

  ModuleSyncStatus& status = getModuleSyncStatus(INTERNAL_MODULE);
  mixerSchedulerSetPeriod(INTERNAL_MODULE, status.isValid()
                                               ? status.getAdjustedRefreshRate()
                                               : AFHDS2_PERIOD);
  status.invalidate();
  setupPulsesAFHDS2();
}

static void afhds2SendPulses(void* context)
{
  uint8_t* data = (uint8_t*)intmodulePulsesData.flysky.pulses;
  uint16_t size = intmodulePulsesData.flysky.ptr - data;
  IntmoduleSerialDriver.sendBuffer(context, data, size);
}

static int afhds2GetByte(void* context, uint8_t* data)
{
  return IntmoduleSerialDriver.getByte(context, data);
}

static void afhds2ProcessData(void*, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  processInternalFlySkyTelemetryData(data, buffer, len);
}

const etx_module_driver_t Afhds2InternalDriver = {
  .protocol = PROTOCOL_CHANNELS_AFHDS2A,
  .init = afhds2Init,
  .deinit = afhds2DeInit,
  .setupPulses = afhds2SetupPulses,
  .sendPulses = afhds2SendPulses,
  .getByte = afhds2GetByte,
  .processData = afhds2ProcessData,
};
