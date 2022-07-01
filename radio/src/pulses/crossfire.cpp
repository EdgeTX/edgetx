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

#include "telemetry/crossfire.h"

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

static bool _checkFrameCRC(uint8_t* rxBuffer)
{
  uint8_t len = rxBuffer[1];
  uint8_t crc = crc8(&rxBuffer[2], len-1);
  return (crc == rxBuffer[len+1]);
}

struct CrossfireState {
  uint8_t                    module;
  CrossfirePulsesData*       data;
  const etx_serial_driver_t* uart_drv;
  void*                      uart_ctx;
  uint8_t                    endpoint;

  void init(uint8_t _module, CrossfirePulsesData* _data,
            const etx_serial_driver_t* _drv, void* _ctx, uint8_t _endpoint)
  {
    module = _module;
    data = _data;
    uart_drv = _drv;
    uart_ctx = _ctx;
    endpoint = _endpoint;
  }
  
  void deinit() { uart_drv->deinit(uart_ctx); }
};

static CrossfireState crossfireState[NUM_MODULES];

static void crossfireSetupPulses(void* context, int16_t* channels, uint8_t nChannels)
{
  auto state = (CrossfireState*)context;
  crossfireSetupMixerScheduler(state->module);
  setupPulsesCrossfire(state->module, state->data, state->endpoint, channels, nChannels);
}

static void crossfireSendPulses(void* context)
{
  auto state = (CrossfireState*)context;
  auto data = state->data;

  if (state->uart_drv) {
    auto drv = state->uart_drv;
    auto ctx = state->uart_ctx;
    drv->sendBuffer(ctx, data->pulses, data->length);
  } else {
    sportSendBuffer(data->pulses, data->length);
  }
}

static int crossfireGetByte(void* context, uint8_t* data)
{
  auto state = (CrossfireState*)context;
  if (state->uart_drv) {
    auto drv = state->uart_drv;
    auto ctx = state->uart_ctx;
    return drv->getByte(ctx, data);
  } else {
    return sportGetByte(data);
  }
}

static bool _lenIsSane(uint8_t len)
{
  // packet len must be at least 3 bytes (type+payload+crc) and 2 bytes < MAX (hdr+len)
  return (len > 2 && len < TELEMETRY_RX_PACKET_SIZE-1);
}

static void _seekStart(uint8_t* buffer, uint8_t* len)
{
  // Bad telemetry packets frequently are just truncated packets, with the start
  // of a new packet contained in the data. This causes multiple packet drops as
  // the parser tries to resync.
  // Search through the rxBuffer for a sync byte, shift the contents if found
  // and reduce rxBufferCount
  for (uint8_t idx = 1; idx < *len; ++idx) {
    uint8_t data = buffer[idx];
    if (data == RADIO_ADDRESS || data == UART_SYNC) {
      uint8_t remain = *len - idx;
      // If there's at least 2 bytes, check the length for validity too
      if (remain > 1 && !_lenIsSane(buffer[idx + 1])) continue;

      // TRACE("Found 0x%02x with %u remain", data, remain);
      // copy the data to the front of the buffer
      for (uint8_t src = idx; src < *len; ++src) {
        buffer[src - idx] = buffer[src];
      }

      *len = remain;
      return;
    }  // if found sync
  }

  // Not found, clear the buffer
  *len = 0;
}

static void crossfireProcessData(void* context, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  if (*len == 0 && data != RADIO_ADDRESS && data != UART_SYNC) {
    TRACE("[XF] address 0x%02X error", data);
    return;
  }

  if (*len == 1 && !_lenIsSane(data)) {
    TRACE("[XF] length 0x%02X error", data);
    *len = 0;
    return;
  }

  if (*len < TELEMETRY_RX_PACKET_SIZE) {
    buffer[(*len)++] = data;
  }
  else {
    TRACE("[XF] array size %d error", *len);
    *len = 0;
  }

  // rxBuffer[1] holds the packet length-2, check if the whole packet was received
  while (*len > 4 && (buffer[1]+2) == *len) {
    // TODO: module in context?
    if (_checkFrameCRC(buffer)) {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, *len);
      }
#endif
      auto state = (CrossfireState*)context;
      processCrossfireTelemetryFrame(state->module);
      *len = 0;
    }
    else {
      TRACE("[XF] CRC error ");
      _seekStart(buffer, len); // adjusts len
    }
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
  // serial port setup
  etx_serial_init params(intmoduleCrossfireInitParams);
  params.baudrate = INT_CROSSFIRE_BAUDRATE;
  INTERNAL_MODULE_ON();

  auto state = &crossfireState[module];
  state->init(module, &intmodulePulsesData.crossfire,
              &IntmoduleSerialDriver, IntmoduleSerialDriver.init(&params), 0);

  return state;
}

static void crossfireDeInitInternal(void* context)
{
  INTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);

  auto state = (CrossfireState*)context;
  state->deinit();
}

etx_module_driver_t CrossfireInternalDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInitInternal,
  .deinit = crossfireDeInitInternal,
  .setupPulses = crossfireSetupPulses,
  .sendPulses = crossfireSendPulses,
  .getByte = crossfireGetByte,
  .processData = crossfireProcessData,
};
#endif

static void* crossfireInitExternal(uint8_t module)
{
  // telemetryInit(PROTOCOL_TELEMETRY_CROSSFIRE);
  telemetryProtocol = PROTOCOL_TELEMETRY_CROSSFIRE;
  telemetryPortInit(EXT_CROSSFIRE_BAUDRATE, TELEMETRY_SERIAL_DEFAULT);
#if defined(LUA)
  outputTelemetryBuffer.reset();
#endif
  telemetryPortSetDirectionOutput();

  mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD);
  EXTERNAL_MODULE_ON();
  
  auto state = &crossfireState[module];
  state->init(module, &extmodulePulsesData.crossfire, nullptr, nullptr,
              TELEMETRY_ENDPOINT_SPORT);
  
  return state;
}

static void crossfireDeInitExternal(void* context)
{
  (void)context;
  telemetryProtocol = 0xFF;

  EXTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
}

etx_module_driver_t CrossfireExternalDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInitExternal,
  .deinit = crossfireDeInitExternal,
  .setupPulses = crossfireSetupPulses,
  .sendPulses = crossfireSendPulses,
  .getByte = crossfireGetByte,
  .processData = crossfireProcessData,
};
