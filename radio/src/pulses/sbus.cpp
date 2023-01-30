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

static inline void sendByte(uint8_t*& p_buf, uint8_t b)
{
  *p_buf++ = b;
}

static inline int getChannelValue(uint8_t port, int channel)
{
  int ch = g_model.moduleData[port].channelsStart + channel;
  // We will ignore 17 and 18th if that brings us over the limit
  if (ch > 31) return 0;
  return channelOutputs[ch] + 2 * PPM_CH_CENTER(ch) - 2 * PPM_CENTER;
}

static void setupPulsesSbus(uint8_t module, uint8_t*& p_buf)
{
  // extmodulePulsesData.dsm2.index = 0;
  // extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;

  // Sync Byte
  sendByte(p_buf, SBUS_FRAME_BEGIN_BYTE);

  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  // byte 1-22, channels 0..2047, limits not really clear (B
  for (int i=0; i<SBUS_NORMAL_CHANS; i++) {
    int value = getChannelValue(module, i);

    value =  value*8/10 + SBUS_CHAN_CENTER;
    bits |= limit(0, value, 2047) << bitsavailable;
    bitsavailable += SBUS_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendByte(p_buf, (uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }

  // flags
  uint8_t flags=0;
  if (getChannelValue(module, 16) > 0)
    flags |= SBUS_FLAG_CHANNEL_17;
  if (getChannelValue(module, 17) > 0)
    flags |= SBUS_FLAG_CHANNEL_18;

  sendByte(p_buf, flags);

  // last byte, always 0x0
  sendByte(p_buf, 0x00);
}


#define SBUS_BAUDRATE 100000

const etx_serial_init sbusUartParams = {
    .baudrate = SBUS_BAUDRATE,
    .encoding = ETX_Encoding_8E2,
    .direction = ETX_Dir_TX,
    .polarity = ETX_Pol_Inverted,
};

static void* sbusInit(uint8_t module)
{
#if defined(HARDWARE_INTERNAL_MODULE)
  // only external module supported
  if (module == INTERNAL_MODULE) return nullptr;
#endif

  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &sbusUartParams);
  if (!mod_st) {
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SOFT_INV, &sbusUartParams);
    if (!mod_st) return nullptr;
  }

  mixerSchedulerSetPeriod(module, SBUS_PERIOD(module));
  return (void*)mod_st;
}

static void sbusDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void sbusSendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  // TODO:
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  // TODO: set polarity for next packet sent (can be changed from UI)
  // GET_SBUS_POLARITY(EXTERNAL_MODULE)

  auto p_data = buffer;
  setupPulsesSbus(module, p_data);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(drv_ctx, buffer, p_data - buffer);

  // SBUS_PERIOD is not a constant! It can be set from UI
  mixerSchedulerSetPeriod(module, SBUS_PERIOD(module));
}

const etx_proto_driver_t SBusDriver = {
  .protocol = PROTOCOL_CHANNELS_SBUS,
  .init = sbusInit,
  .deinit = sbusDeInit,
  .sendPulses = sbusSendPulses,
  .processData = nullptr,
};
