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

#include "afhds2.h"
#include "flysky.h"

#include "intmodule_serial_driver.h"
#include "mixer_scheduler.h"

// solely for 'intmoduleFifo'
// TODO: remove this non-sense
#include "opentx.h"
#include "pulses/pxx2.h"

afhds2::afhds2() {
  // TODO Auto-generated constructor stub

}

afhds2::~afhds2() {
  // TODO Auto-generated destructor stub
}

const etx_serial_init afhds2SerialInitParams = {
    .baudrate = INTMODULE_USART_AFHDS2_BAUDRATE,
    .parity = ETX_Parity_None,
    .stop_bits = ETX_StopBits_One,
    .word_length = ETX_WordLength_8,
    .rx_enable = true,
    .rx_dma_buf = nullptr,
    .rx_dma_buf_len = 0,
    .on_receive = intmoduleFifoReceive,
    .on_error = intmoduleFifoError,
};

static void* afhds2Init(uint8_t module)
{
  (void)module;

  // serial port setup
  resetPulsesAFHDS2();
  intmoduleFifo.clear();
  IntmoduleSerialDriver.init(&afhds2SerialInitParams);

  // mixer setup
  mixerSchedulerSetPeriod(INTERNAL_MODULE, AFHDS2_PERIOD);
  INTERNAL_MODULE_ON();

  return nullptr;
}

static void afhds2DeInit(void* context)
{
  (void)context;

  INTERNAL_MODULE_OFF();
  intmoduleFifo.clear();
  IntmoduleSerialDriver.deinit();

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
  (void)context;

  uint8_t* data = (uint8_t*)intmodulePulsesData.flysky.pulses;
  uint16_t size = intmodulePulsesData.flysky.ptr - data;
  IntmoduleSerialDriver.sendBuffer(data, size);
}

const etx_module_driver_t Afhds2InternalDriver = {
  .protocol = PROTOCOL_CHANNELS_AFHDS2A,
  .init = afhds2Init,
  .deinit = afhds2DeInit,
  .setupPulses = afhds2SetupPulses,
  .sendPulses = afhds2SendPulses,    
};
