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
#include "mixer_scheduler.h"
#include "hal/module_driver.h"

#if defined(INTMODULE_USART)
#include "intmodule_serial_driver.h"
#endif

#define CROSSFIRE_CH_BITS           11
#define CROSSFIRE_CENTER            0x3E0
#if defined(PPM_CENTER_ADJUSTABLE)
  #define CROSSFIRE_CENTER_CH_OFFSET(ch)            ((2 * limitAddress(ch)->ppmCenter) + 1)  // + 1 is for rouding
#else
  #define CROSSFIRE_CENTER_CH_OFFSET(ch)            (0)
#endif


uint8_t createCrossfireModelIDFrame(uint8_t moduleIdx, uint8_t * frame)
{
  uint8_t * buf = frame;
  *buf++ = UART_SYNC;                                 /* device address */
  *buf++ = 8;                                         /* frame length */
  *buf++ = COMMAND_ID;                                /* cmd type */
  *buf++ = MODULE_ADDRESS;                            /* Destination Address */
  *buf++ = RADIO_ADDRESS;                             /* Origin Address */
  *buf++ = SUBCOMMAND_CRSF;                           /* sub command */
  *buf++ = COMMAND_MODEL_SELECT_ID;                   /* command of set model/receiver id */
  *buf++ = g_model.header.modelId[moduleIdx];         /* model ID */
  *buf++ = crc8_BA(frame + 2, 6);
  *buf++ = crc8(frame + 2, 7);
  return buf - frame;
}

// Range for pulses (channels output) is [-1024:+1024]
uint8_t createCrossfireChannelsFrame(uint8_t * frame, int16_t * pulses)
{
  uint8_t * buf = frame;
  *buf++ = MODULE_ADDRESS;
  *buf++ = 24; // 1(ID) + 22 + 1(CRC)
  uint8_t * crc_start = buf;
  *buf++ = CHANNELS_ID;
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;
  for (int i=0; i<CROSSFIRE_CHANNELS_COUNT; i++) {
    uint32_t val = limit(0, CROSSFIRE_CENTER + (CROSSFIRE_CENTER_CH_OFFSET(i) * 4) / 5 + (pulses[i] * 4) / 5, 2 * CROSSFIRE_CENTER);
    bits |= val << bitsavailable;
    bitsavailable += CROSSFIRE_CH_BITS;
    while (bitsavailable >= 8) {
      *buf++ = bits;
      bits >>= 8;
      bitsavailable -= 8;
    }
  }
  *buf++ = crc8(crc_start, 23);
  return buf - frame;
}

static void setupPulsesCrossfire(uint8_t idx, CrossfirePulsesData* p_data,
                                 uint8_t endpoint, int16_t* channels,
                                 uint8_t nChannels)
{
#if defined(LUA)
  if (outputTelemetryBuffer.destination == endpoint) {
    memcpy(p_data->pulses, outputTelemetryBuffer.data,
           outputTelemetryBuffer.size);
    p_data->length = outputTelemetryBuffer.size;
    outputTelemetryBuffer.reset();
  } else
#endif
  {
    if (moduleState[idx].counter == CRSF_FRAME_MODELID) {
      p_data->length = createCrossfireModelIDFrame(idx, p_data->pulses);
      moduleState[idx].counter = CRSF_FRAME_MODELID_SENT;
    } else {
      p_data->length = createCrossfireChannelsFrame(
          p_data->pulses,
          channels /*TODO: nChannels*/);
    }
  }
}

static void crossfireSetupMixerScheduler(uint8_t module)
{
  ModuleSyncStatus& status = getModuleSyncStatus(module);
  if (status.isValid()) {
    mixerSchedulerSetPeriod(module, status.getAdjustedRefreshRate());
  } else {
    mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD);
  }
}

#if defined(INTERNAL_MODULE_CRSF)
static const etx_serial_init intmoduleCrossfireInitParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
};

static void* crossfireInitInternal(uint8_t module)
{
  (void)module;

  // serial port setup
  etx_serial_init params(intmoduleCrossfireInitParams);
  params.baudrate = INT_CROSSFIRE_BAUDRATE;

  intmoduleFifo.clear();
  IntmoduleSerialDriver.init(&params);
  INTERNAL_MODULE_ON();

  return nullptr;
}

static void crossfireDeInitInternal(void* context)
{
  (void)context;

  INTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
  intmoduleStop();
  intmoduleFifo.clear();
}

static void crossfireSetupPulsesInternal(void* context, int16_t* channels, uint8_t nChannels)
{
  auto* p_data = &intmodulePulsesData.crossfire;
  crossfireSetupMixerScheduler(INTERNAL_MODULE);
  setupPulsesCrossfire(INTERNAL_MODULE, p_data, 0, channels, nChannels);
}

static void crossfireSendPulsesInternal(void* context)
{
  IntmoduleSerialDriver.sendBuffer(context, intmodulePulsesData.crossfire.pulses,
                                   intmodulePulsesData.crossfire.length);
}

etx_module_driver_t CrossfireInternalDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInitInternal,
  .deinit = crossfireDeInitInternal,
  .setupPulses = crossfireSetupPulsesInternal,
  .sendPulses = crossfireSendPulsesInternal,
};
#endif

static void* crossfireInitExternal(uint8_t module)
{
  telemetryInit(PROTOCOL_TELEMETRY_CROSSFIRE);
  mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD);
  EXTERNAL_MODULE_ON();
  return nullptr;
}

static void crossfireDeInitExternal(void* context)
{
  (void)context;
  telemetryProtocol = 0xFF;

  EXTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
}

static void crossfireSetupPulsesExternal(void* context, int16_t* channels, uint8_t nChannels)
{
  auto* p_data = &extmodulePulsesData.crossfire;
  crossfireSetupMixerScheduler(EXTERNAL_MODULE);
  setupPulsesCrossfire(EXTERNAL_MODULE, p_data, TELEMETRY_ENDPOINT_SPORT,
                       channels, nChannels);
}

static void crossfireSendPulsesExternal(void* context)
{
  (void)context;

  sportSendBuffer(extmodulePulsesData.crossfire.pulses,
                  extmodulePulsesData.crossfire.length);
}

etx_module_driver_t CrossfireExternalDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInitExternal,
  .deinit = crossfireDeInitExternal,
  .setupPulses = crossfireSetupPulsesExternal,
  .sendPulses = crossfireSendPulsesExternal
};
