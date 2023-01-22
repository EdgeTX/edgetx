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

#include "sbus.h"
#include "hal/module_port.h"
#include "mixer_scheduler.h"

#include "opentx.h"

#define SBUS_NORMAL_CHANS 16
#define SBUS_CHAN_BITS    11

/* Definitions from CleanFlight/BetaFlight */

#define SBUS_FLAG_CHANNEL_17        (1 << 0)
#define SBUS_FLAG_CHANNEL_18        (1 << 1)
#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3)
#define SBUS_FRAME_BEGIN_BYTE       0x0F

#define SBUS_CHAN_CENTER            992

inline int getChannelValue(uint8_t port, int channel)
{
  int ch = g_model.moduleData[port].channelsStart + channel;
  // We will ignore 17 and 18th if that brings us over the limit
  if (ch > 31) return 0;
  return channelOutputs[ch] + 2 * PPM_CH_CENTER(ch) - 2 * PPM_CENTER;
}

void setupPulsesSbus(UartMultiPulses* uart)
{
  // extmodulePulsesData.dsm2.index = 0;
  // extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;

  // Sync Byte
  uart->sendByte(SBUS_FRAME_BEGIN_BYTE);

  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  // byte 1-22, channels 0..2047, limits not really clear (B
  for (int i=0; i<SBUS_NORMAL_CHANS; i++) {
    int value = getChannelValue(EXTERNAL_MODULE, i);

    value =  value*8/10 + SBUS_CHAN_CENTER;
    bits |= limit(0, value, 2047) << bitsavailable;
    bitsavailable += SBUS_CHAN_BITS;
    while (bitsavailable >= 8) {
      uart->sendByte((uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }

  // flags
  uint8_t flags=0;
  if (getChannelValue(EXTERNAL_MODULE, 16) > 0)
    flags |= SBUS_FLAG_CHANNEL_17;
  if (getChannelValue(EXTERNAL_MODULE, 17) > 0)
    flags |= SBUS_FLAG_CHANNEL_18;

  uart->sendByte(flags);

  // last byte, always 0x0
  uart->sendByte(0x00);
}


#define SBUS_BAUDRATE 100000

etx_serial_init sbusUartParams = {
    .baudrate = SBUS_BAUDRATE,
    .parity = ETX_Parity_Even,
    .stop_bits = ETX_StopBits_Two,
    .word_length = ETX_WordLength_9,
    .rx_enable = false,
};

static void* sbusInit(uint8_t module)
{
  // only external module supported
  if (module == INTERNAL_MODULE) return nullptr;

  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_EXTERNAL_SOFT_INV,
                                     ETX_MOD_DIR_TX, &sbusUartParams);

  extmodulePulsesData.multi.initFrame();
  mod_st->user_data = (void*)&extmodulePulsesData.multi;

  EXTERNAL_MODULE_ON();
  mixerSchedulerSetPeriod(module, SBUS_PERIOD);

  return (void*)mod_st;
}

static void sbusDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  EXTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(module, 0);
  modulePortDeInit(mod_st);
}

static void sbusSetupPulses(void* ctx, int16_t* channels, uint8_t nChannels)
{
  // TODO:
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;

  auto pulses = (UartMultiPulses*)mod_st->user_data;
  pulses->initFrame();

  setupPulsesSbus(pulses);
}

static void sbusSendPulses(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  auto drv = mod_st->tx.port->drv.serial;
  auto drv_ctx = mod_st->tx.ctx;

  // TODO: set polarity for next packet sent (can be changed from UI)
  // GET_SBUS_POLARITY(EXTERNAL_MODULE)

  auto pulses = (UartMultiPulses*)mod_st->user_data;
  drv->sendBuffer(drv_ctx, pulses->getData(), pulses->getSize());

  // SBUS_PERIOD is not a constant! It can be set from UI
  mixerSchedulerSetPeriod(module, SBUS_PERIOD);
}

const etx_proto_driver_t SBusDriver = {
  .protocol = PROTOCOL_CHANNELS_SBUS,
  .init = sbusInit,
  .deinit = sbusDeInit,
  .setupPulses = sbusSetupPulses,
  .sendPulses = sbusSendPulses,
  .getByte = nullptr,
  .processData = nullptr,
};
