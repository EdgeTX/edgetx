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
#include "hal/serial_driver.h"
#include "mixer_scheduler.h"

#include "edgetx.h"

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

typedef void (*ppm_telemetry_fct_t)(uint8_t module, uint8_t data, uint8_t* buffer, uint8_t* len);
static ppm_telemetry_fct_t _processTelemetryData;

const etx_serial_init sbusUartParams = {
    .baudrate = SBUS_BAUDRATE,
    .encoding = ETX_Encoding_8E2,
    .direction = ETX_Dir_TX,
    .polarity = ETX_Pol_Inverted,
};

static const etx_serial_init sbusSportSerialParams = {
    .baudrate = FRSKY_SPORT_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Normal,
};

static bool sbusInitSPortTelemetry(uint8_t module)
{
  // Try S.PORT hardware USART (requires HW inverters)
  if (modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &sbusSportSerialParams, false) != nullptr) {
    return true;
  }

  return false;
}

static void _init_telemetry(uint8_t module, uint8_t telemetry_type)
{
  switch (telemetry_type) {
    case SBUS_PROTO_TLM_SPORT:
      if (sbusInitSPortTelemetry(module)) {
        _processTelemetryData = processFrskySportTelemetryData;
      }
      break;

    default:
      _processTelemetryData = nullptr;
      break;
  }
}

static void* sbusInit(uint8_t module)
{
#if defined(HARDWARE_INTERNAL_MODULE)
  // only external module supported
  if (module == INTERNAL_MODULE) return nullptr;
#endif

  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &sbusUartParams, true);
  if (!mod_st) return nullptr;

  mixerSchedulerSetPeriod(module, SBUS_PERIOD(module));

  uint8_t telemetry_type = g_model.moduleData[module].subType;
  mod_st->user_data = (void*)(uintptr_t)telemetry_type;

  _init_telemetry(module, telemetry_type);
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

  auto p_data = buffer;
  setupPulsesSbus(module, p_data);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);

  auto port = mod_st->tx.port;
  if (port && port->set_inverted) {
    port->set_inverted(GET_SBUS_POLARITY(module));
  } else if (drv->setPolarity) {
    drv->setPolarity(drv_ctx, GET_SBUS_POLARITY(module));
  }
    
  drv->sendBuffer(drv_ctx, buffer, p_data - buffer);

  // SBUS_PERIOD is not a constant! It can be set from UI
  mixerSchedulerSetPeriod(module, SBUS_PERIOD(module));
}

static void sbusProcessTelemetryData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len) {
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  if (_processTelemetryData) {
    _processTelemetryData(module, data, buffer, len);
  }
}

static void sbusOnConfigChange(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  uint8_t telemetry_type = (uint8_t)(uintptr_t)mod_st->user_data;
  if (telemetry_type != g_model.moduleData[module].subType) {
    // restart during next mixer cycle
    restartModuleAsync(module, 0);
  }
}

const etx_proto_driver_t SBusDriver = {
  .protocol = PROTOCOL_CHANNELS_SBUS,
  .init = sbusInit,
  .deinit = sbusDeInit,
  .sendPulses = sbusSendPulses,
  .processData = sbusProcessTelemetryData,
  .processFrame = nullptr,
  .onConfigChange = sbusOnConfigChange,
};
