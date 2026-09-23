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
#include "mixer_scheduler.h"
#include "hal/module_driver.h"
#include "hal/module_port.h"
#include "telemetry/telemetry.h"

#include "mavlink.h"
#include "telemetry/mavlink.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "common/mavlink.h"
#pragma GCC diagnostic pop

// Supported link baudrates
const uint32_t MAVLINK_BAUDRATES[MAVLINK_BAUDRATE_COUNT] = {
  115200, 400000, 921600
};

// RC_CHANNELS_OVERRIDE send period (microseconds), one per MAVLINK_BAUDRATES
static const uint16_t MAVLINK_PERIODS[MAVLINK_BAUDRATE_COUNT] = {
  16000,  // 115200 ->  62 Hz
  4000,   // 400000 -> 250 Hz
  2000,   // 921600 -> 500 Hz
};

#define MAVLINK_GCS_SYSID         255
#define MAVLINK_GCS_COMPID        MAV_COMP_ID_MISSIONPLANNER
#define MAVLINK_HEARTBEAT_TIMEOUT 300

static const etx_serial_init mavlinkSerialParams = {
  .baudrate = 0,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

struct MavlinkState {
  uint8_t   targetSysId;
  uint8_t   targetCompId;
  bool      targetKnown;

  uint16_t  frameAccum;
  tmr10ms_t lastRateCalc;
  tmr10ms_t lastHeartbeat;

  MavlinkModuleStatus pub;
};

static MavlinkState mavlinkState[NUM_MODULES];

static uint16_t channelToUs(int16_t value)
{
  int32_t us = 1500 + ((int32_t)value * 500) / 1024;
  return (uint16_t)limit<int32_t>(1000, us, 2000);
}

static void mavlinkRxTrigger(void* param);

static void* mavlinkInit(uint8_t module)
{
  etx_serial_init params(mavlinkSerialParams);

  uint8_t brIdx = g_model.moduleData[module].mavlink.telemetryBaudrate;
  if (brIdx >= MAVLINK_BAUDRATE_COUNT) brIdx = 0;

  auto port = modulePortFind(module, ETX_MOD_TYPE_SERIAL, ETX_MOD_PORT_SPORT,
                             params.polarity, params.direction);
  if (port && port->port == ETX_MOD_PORT_SPORT) brIdx = 0;

  params.baudrate = MAVLINK_BAUDRATES[brIdx];

  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &params, false);
  if (!mod_st) return nullptr;

  auto& st = mavlinkState[module];
  memset(&st, 0, sizeof(st));

  st.targetSysId = 1;
  st.targetCompId = MAV_COMP_ID_AUTOPILOT1;
  st.lastRateCalc = get_tmr10ms();

  auto& rx_count = getTelemetryRxBufferCount(module);
  rx_count = 0;

  auto rxdrv = modulePortGetSerialDrv(mod_st->rx);
  auto rxctx = modulePortGetCtx(mod_st->rx);
  if (rxdrv && rxctx && !rxdrv->getByte && rxdrv->setIdleCb &&
      rxdrv->copyRxBuffer) {
    rxdrv->setIdleCb(rxctx, mavlinkRxTrigger, mod_st);
  }

  mixerSchedulerSetPeriod(module, MAVLINK_PERIODS[brIdx]);
  return (void*)mod_st;
}

static void mavlinkDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;

  auto rxdrv = modulePortGetSerialDrv(mod_st->rx);
  auto rxctx = modulePortGetCtx(mod_st->rx);
  if (rxdrv && rxctx && rxdrv->setIdleCb) {
    rxdrv->setIdleCb(rxctx, nullptr, nullptr);
  }

  modulePortDeInit(mod_st);
}

static void mavlinkSendPulses(void* ctx, uint8_t* buffer, int16_t* channels,
                              uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto& st = mavlinkState[module];

  // nChannels is hardcoded to 16
  // https://github.com/EdgeTX/edgetx/blob/main/radio/src/pulses/pulses.cpp#L572
  (void)nChannels;
  uint8_t count = sentModuleChannels(module);
  if (count > 18) count = 18;

  uint16_t ch[18];
  for (uint8_t i = 0; i < 18; i++) {
    ch[i] = (i < count) ? channelToUs(channels[i]) : 0;
  }

  mavlink_message_t msg;
  mavlink_msg_rc_channels_override_pack(
      MAVLINK_GCS_SYSID, MAVLINK_GCS_COMPID, &msg, st.targetSysId,
      st.targetCompId, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7],
      ch[8], ch[9], ch[10], ch[11], ch[12], ch[13], ch[14], ch[15], ch[16],
      ch[17]);

  uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  if (drv && drv->sendBuffer) {
    drv->sendBuffer(drv_ctx, buffer, len);
  }

  // Recompute the published link rate roughly once per second.
  auto now = get_tmr10ms();
  if ((tmr10ms_t)(now - st.lastRateCalc) >= 100) {
    st.pub.framesPerSecond = st.frameAccum;
    st.pub.linkUp = (tmr10ms_t)(now - st.lastHeartbeat) < MAVLINK_HEARTBEAT_TIMEOUT;
    st.pub.targetSysId = st.targetSysId;
    st.pub.targetCompId = st.targetCompId;
    st.frameAccum = 0;
    st.lastRateCalc = now;
  }
}

static void mavlinkHandleMessage(uint8_t module, const mavlink_message_t* msg)
{
  auto& st = mavlinkState[module];
  st.frameAccum++;

  switch (msg->msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: {
      mavlink_heartbeat_t hb;
      mavlink_msg_heartbeat_decode(msg, &hb);

      if (hb.type != MAV_TYPE_GCS) {
        st.targetSysId = msg->sysid;
        st.targetCompId = msg->compid;
        st.targetKnown = true;
      }
      st.lastHeartbeat = get_tmr10ms();
    } break;

    default:
      break;
  }

  mavlinkTelemetryDecode(module, msg);
}

static inline void mavlinkParseByte(uint8_t module, uint8_t data)
{
  mavlink_message_t msg;
  mavlink_status_t status;
  if (mavlink_parse_char(module, data, &msg, &status)) {
    mavlinkHandleMessage(module, &msg);
  }
}

static void mavlinkProcessData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  (void)buffer;
  (void)len;
  auto mod_st = (etx_module_state_t*)ctx;
  mavlinkParseByte(modulePortGetModule(mod_st), data);
}

static void mavlinkProcessFrame(void* ctx, uint8_t* frame, uint8_t flen, uint8_t* buf, uint8_t* len)
{
  (void)buf;
  (void)len;
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  for (uint8_t i = 0; i < flen; i++) {
    mavlinkParseByte(module, frame[i]);
  }
}

static void mavlinkRxTrigger(void* param)
{
  auto mod_st = (etx_module_state_t*)param;
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);
  if (!drv || !ctx || !drv->getBufferedBytes) return;
  if (drv->getBufferedBytes(ctx) == 0) return;
  telemetryFrameTrigger_ISR(modulePortGetModule(mod_st), &MavlinkDriver);
}

const etx_proto_driver_t MavlinkDriver = {
    .protocol = PROTOCOL_CHANNELS_MAVLINK,
    .init = mavlinkInit,
    .deinit = mavlinkDeInit,
    .sendPulses = mavlinkSendPulses,
    .processData = mavlinkProcessData,
    .processFrame = mavlinkProcessFrame,
    .onConfigChange = nullptr,
    .txCompleted = modulePortSerialTxCompleted,
};

const MavlinkModuleStatus* mavlinkGetStatus(uint8_t module)
{
  if (module >= NUM_MODULES) return nullptr;
  return &mavlinkState[module].pub;
}
