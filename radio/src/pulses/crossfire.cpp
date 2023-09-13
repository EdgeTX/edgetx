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

#include "stm32_gpio_driver.h"  // DEBUG PIN
#include "stm32_exti_driver.h"

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

#define MIN_FRAME_LEN 3

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

static void setupPulsesCrossfire(uint8_t module, uint8_t*& p_buf,
                                 uint8_t endpoint, int16_t* channels,
                                 uint8_t nChannels)
{
#if defined(LUA)
  if (outputTelemetryBuffer.destination == endpoint) {
    auto len = outputTelemetryBuffer.size;
    memcpy(p_buf, outputTelemetryBuffer.data, len);
    outputTelemetryBuffer.reset();
    p_buf += len;
  } else
#endif
  {
    if (moduleState[module].counter == CRSF_FRAME_MODELID) {
      p_buf += createCrossfireModelIDFrame(module, p_buf);
      moduleState[module].counter = CRSF_FRAME_MODELID_SENT;
    } else {
      /* TODO: nChannels */
      p_buf += createCrossfireChannelsFrame(p_buf, channels);
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

static void crossfireSendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  crossfireSetupMixerScheduler(module);

  uint8_t endpoint = 0;  
#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) endpoint = TELEMETRY_ENDPOINT_SPORT;
#endif
  auto p_buf = buffer;
  setupPulsesCrossfire(module, p_buf, endpoint, channels, nChannels);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(drv_ctx, buffer, p_buf - buffer);
}

static bool _lenIsSane(uint8_t len)
{
  // packet len must be at least 3 bytes (type+payload+crc) and 2 bytes < MAX (hdr+len)
  return (len > 2 && len < TELEMETRY_RX_PACKET_SIZE-1);
}

static void crossfireProcessFrame(void* ctx, uint8_t* frame, uint8_t flen,
                                  uint8_t* buf, uint8_t* len)
{
  if (flen < MIN_FRAME_LEN) return;

  if (frame[0] != RADIO_ADDRESS && frame[0] != UART_SYNC) {
    TRACE("[XF] address 0x%02X error", frame[0]);
    return;
  }

  uint8_t pkt_len = frame[1];
  if (!_lenIsSane(pkt_len) || pkt_len > flen - 2) {
    TRACE("[XF] length 0x%02X error", pkt_len);
    return;
  }

  if (_checkFrameCRC(frame)) {
#if defined(BLUETOOTH) // TODO: generic telemetry mirror to BT
    if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
        bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
      bluetooth.write(frame, pkt_len);
    }
#endif
    auto mod_st = (etx_module_state_t*)ctx;
    *len = pkt_len + 2;
    memcpy(buf, frame, *len);
    processCrossfireTelemetryFrame(modulePortGetModule(mod_st));
  }
  else {
    TRACE("[XF] CRC error ");
  }
}

static const etx_serial_init crsfSerialParams = {
  .baudrate = 0,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

static void _crsf_extmodule_frame_received()
{
  telemetryFrameTrigger_ISR(EXTERNAL_MODULE, &CrossfireDriver);
}

static void crsfRxFrameLenghCheck(void* param)
{
  uint8_t destination, lengh;

  auto mod_st = (etx_module_state_t*)param;
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);

  auto buffered = drv->getBufferedBytes(ctx);
  if (buffered < 3) {
    drv->clearRxBuffer(ctx);
    return;  // Should not happen
  }

  drv->getLastByte(ctx, buffered, &destination);
  if (destination != RADIO_ADDRESS && destination != UART_SYNC) {
    drv->clearRxBuffer(ctx);
    return;  // Only process data sent to us
  }

  drv->getLastByte(ctx, buffered - 1, &lengh);
  if (buffered < (lengh + 2)) {
    // We are missing data, flush wrong data
    drv->clearRxBuffer(ctx);
    TRACE("[XF] missing bytes");
    return;
  }

#if !defined(SIMU)
  EXTI->SWIER = TELEMETRY_RX_FRAME_EXTI_LINE;
#endif
}

static void* crossfireInit(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init params(crsfSerialParams);

#if defined(INTERNAL_MODULE_CRSF)
  if (module == INTERNAL_MODULE) {

    params.baudrate = INT_CROSSFIRE_BAUDRATE;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params);
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {

    params.baudrate = EXT_CROSSFIRE_BAUDRATE;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &params);

    if (mod_st) {
      auto drv = modulePortGetSerialDrv(mod_st->rx);
      auto ctx = modulePortGetCtx(mod_st->rx);

      if (drv && ctx && drv->setIdleCb) {
        drv->setIdleCb(ctx, crsfRxFrameLenghCheck, mod_st);
        stm32_exti_enable(TELEMETRY_RX_FRAME_EXTI_LINE, 0, _crsf_extmodule_frame_received);
      }

      // DEBUG PIN: PPM as OUTPUT
      stm32_gpio_enable_clock(EXTMODULE_TX_GPIO);
      EXTMODULE_TX_GPIO->MODER |= (1 << 12);// PC.06
    }
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
  modulePortDeInit(mod_st);
}

const etx_proto_driver_t CrossfireDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInit,
  .deinit = crossfireDeInit,
  .sendPulses = crossfireSendPulses,
  .processData = nullptr,
  .processFrame = crossfireProcessFrame,
};
