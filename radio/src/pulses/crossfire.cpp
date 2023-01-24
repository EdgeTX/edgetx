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
#include "hal/module_port.h"

#include "crossfire.h"
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
    mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD(module));
  }
}

static bool _checkFrameCRC(uint8_t* rxBuffer)
{
  uint8_t len = rxBuffer[1];
  uint8_t crc = crc8(&rxBuffer[2], len-1);
  return (crc == rxBuffer[len+1]);
}

static void crossfireSendPulses(void* ctx, int16_t* channels, uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  crossfireSetupMixerScheduler(module);

  auto data = (CrossfirePulsesData*)mod_st->user_data;
  uint8_t endpoint = 0;  
#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) endpoint = TELEMETRY_ENDPOINT_SPORT;
#endif
  setupPulsesCrossfire(module, data, endpoint, channels, nChannels);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(drv_ctx, data->pulses, data->length);
}

static int crossfireGetByte(void* ctx, uint8_t* data)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto drv_ctx = modulePortGetCtx(mod_st->rx);
  return drv->getByte(drv_ctx, data);
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

static void crossfireProcessData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
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
    if (_checkFrameCRC(buffer)) {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, *len);
      }
#endif
      auto mod_st = (etx_module_state_t*)ctx;
      processCrossfireTelemetryFrame(modulePortGetModule(mod_st));
      *len = 0;
    }
    else {
      TRACE("[XF] CRC error ");
      _seekStart(buffer, len); // adjusts len
    }
  }
}

static const etx_serial_init crsfSerialParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
};

static void* crossfireInit(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init params(crsfSerialParams);

#if defined(INTERNAL_MODULE_CRSF)
  if (module == INTERNAL_MODULE) {

    params.baudrate = INT_CROSSFIRE_BAUDRATE;
    INTERNAL_MODULE_ON();

    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_INTERNAL_UART,
                                  ETX_MOD_DIR_TX_RX, &params);
    mod_st->user_data = (void*)&intmodulePulsesData.crossfire;
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {

    params.baudrate = EXT_CROSSFIRE_BAUDRATE;
    EXTERNAL_MODULE_ON();

    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SPORT,
                                  ETX_MOD_DIR_TX_RX, &params);

    mod_st->user_data = (void*)&extmodulePulsesData.crossfire;
  }
#endif

  if (mod_st) {
    mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD(module));
  }

  return (void*)mod_st;
}

static void crossfireDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

#if defined(INTERNAL_MODULE_CRSF)
  if (module == INTERNAL_MODULE) {
    INTERNAL_MODULE_OFF();
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {
    EXTERNAL_MODULE_OFF();
  }
#endif
  
  mixerSchedulerSetPeriod(module, 0);
  modulePortDeInit(mod_st);
}

const etx_proto_driver_t CrossfireDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInit,
  .deinit = crossfireDeInit,
  .sendPulses = crossfireSendPulses,
  .getByte = crossfireGetByte,
  .processData = crossfireProcessData,
};
