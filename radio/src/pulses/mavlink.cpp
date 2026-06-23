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
#include "crc.h"
#include "pulses.h"

#if !defined(SIMU)
  #include "stm32_exti_driver.h"
#endif

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

// Cooperative half-duplex timing: the module sends a DEBUG_VECT tagged with
// this name carrying x=refresh_rate_us, y=offset_us (see docs). Cross-repo
// contract with the ELRS MAVLinkHandset emitter.
#define MAVLINK_SYNC_TAG          "ETXSYNC"

#define MAV_TUNNEL_PAYLOAD_TYPE_ELRS_CRSF 0x454C

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
  uint8_t   brIdx;          // baud index, for the scheduler fallback period

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

#if !defined(SIMU) && defined(HARDWARE_EXTERNAL_MODULE)
static void mavlinkSoftIrqTrigger(void* param);
static void mavlinkExtmoduleFrameReceived();
#endif

static void* mavlinkInit(uint8_t module)
{
  etx_serial_init params(mavlinkSerialParams);

  uint8_t brIdx = g_model.moduleData[module].mavlink.telemetryBaudrate;
  if (brIdx >= MAVLINK_BAUDRATE_COUNT) brIdx = 0;
  params.baudrate = MAVLINK_BAUDRATES[brIdx];

  auto mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &params, false);
  if (!mod_st) return nullptr;

  auto& st = mavlinkState[module];
  memset(&st, 0, sizeof(st));

  st.brIdx = brIdx;
  st.targetSysId = 1;
  st.targetCompId = MAV_COMP_ID_AUTOPILOT1;
  st.lastRateCalc = get_tmr10ms();

  auto& rx_count = getTelemetryRxBufferCount(module);
  rx_count = 0;

  auto rxdrv = modulePortGetSerialDrv(mod_st->rx);
  auto rxctx = modulePortGetCtx(mod_st->rx);
  if (rxdrv && rxctx && rxdrv->setIdleCb) {
    if (!rxdrv->getByte && rxdrv->copyRxBuffer) {
      // USB-VCP (no byte poll): the idle callback runs in a FreeRTOS-safe
      // context, so trigger the frame path directly. (Unchanged.)
      rxdrv->setIdleCb(rxctx, mavlinkRxTrigger, mod_st);
    }
#if !defined(SIMU) && defined(HARDWARE_EXTERNAL_MODULE)
    else if (rxdrv->getByte && rxdrv->getBufferedBytes && rxdrv->copyRxBuffer) {
      // Physical half-duplex S.PORT: the USART IDLE IRQ runs at priority 0
      // where FreeRTOS calls are illegal, so bounce through an EXTI SWI to
      // reach telemetryFrameTrigger_ISR safely (mirrors CRSF).
      rxdrv->setIdleCb(rxctx, mavlinkSoftIrqTrigger, &mod_st->rx);
  #if defined(TELEMETRY_USE_CUSTOM_EXTI)
      stm32_exti_custom_enable(TELEMETRY_RX_FRAME_EXTI_LINE, 3,
                               mavlinkExtmoduleFrameReceived);
  #else
      stm32_exti_enable(TELEMETRY_RX_FRAME_EXTI_LINE, 0,
                        mavlinkExtmoduleFrameReceived);
  #endif
    }
#endif
  }

  mixerSchedulerSetPeriod(module, MAVLINK_PERIODS[brIdx]);
  return (void*)mod_st;
}

static void mavlinkDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;

  auto rxdrv = modulePortGetSerialDrv(mod_st->rx);
  auto rxctx = modulePortGetCtx(mod_st->rx);

#if !defined(SIMU) && defined(HARDWARE_EXTERNAL_MODULE)
  // Tear down the S.PORT EXTI trampoline if it was set up (getByte present);
  // the USB-VCP path never enabled it.
  if (rxdrv && rxctx && rxdrv->getByte && rxdrv->setIdleCb) {
  #if defined(TELEMETRY_USE_CUSTOM_EXTI)
    stm32_exti_custom_disable(TELEMETRY_RX_FRAME_EXTI_LINE);
  #else
    stm32_exti_disable(TELEMETRY_RX_FRAME_EXTI_LINE);
  #endif
  }
#endif

  if (rxdrv && rxctx && rxdrv->setIdleCb) {
    rxdrv->setIdleCb(rxctx, nullptr, nullptr);
  }

  modulePortDeInit(mod_st);
}

// Cooperative timing: if the module is feeding sync (DEBUG_VECT/ETXSYNC), let
// it steer our RC period so our TX lands in the half-duplex gap (mirrors CRSF);
// otherwise hold the fixed per-baud period. No-op for USB-VCP (no sync sent).
static void mavlinkSetupMixerScheduler(uint8_t module)
{
  ModuleSyncStatus& status = getModuleSyncStatus(module);
  if (status.isValid())
    mixerSchedulerSetPeriod(module, status.getAdjustedRefreshRate());
  else
    mixerSchedulerSetPeriod(module, MAVLINK_PERIODS[mavlinkState[module].brIdx]);
}

static uint8_t mavlinkTunnelBuf[128];
static uint8_t mavlinkTunnelLen = 0;

bool mavlinkTunnelStageAvailable()
{
  return mavlinkTunnelLen == 0;
}

bool mavlinkTunnelStage(const uint8_t* frame, uint8_t len)
{
  if (mavlinkTunnelLen != 0) return false;
  if (len == 0 || len > sizeof(mavlinkTunnelBuf)) return false;
  memset(mavlinkTunnelBuf, 0, sizeof(mavlinkTunnelBuf));
  memcpy(mavlinkTunnelBuf, frame, len);
  mavlinkTunnelLen = len;
  return true;
}

static void mavlinkSendTunnel(etx_module_state_t* mod_st, uint8_t* buffer,
                              MavlinkState& st)
{
  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);

  mavlink_message_t msg;
  uint16_t framelen = mavlink_msg_tunnel_pack(
      MAVLINK_GCS_SYSID, MAVLINK_GCS_COMPID, &msg, st.targetSysId,
      st.targetCompId, MAV_TUNNEL_PAYLOAD_TYPE_ELRS_CRSF, mavlinkTunnelLen,
      mavlinkTunnelBuf);

  if (drv && drv->sendBuffer && framelen <= MODULE_BUFFER_SIZE) {
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    drv->sendBuffer(drv_ctx, buffer, len);
  }

  mavlinkTunnelLen = 0;
}

static void mavlinkSendPulses(void* ctx, uint8_t* buffer, int16_t* channels,
                              uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto& st = mavlinkState[module];

  mavlinkSetupMixerScheduler(module);

  if (mavlinkTunnelLen) {
    mavlinkSendTunnel(mod_st, buffer, st);
    return;
  }

  // nChannels is hardcoded to 16
  // https://github.com/EdgeTX/edgetx/blob/main/radio/src/pulses/pulses.cpp#L572
  (void)nChannels;
  uint8_t count = sentModuleChannels(module);
  if (count < 16) count = 16;
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

    case MAVLINK_MSG_ID_DEBUG_VECT: {
      // Cooperative-timing sync frame from the module (name-tagged so real
      // autopilot DEBUG_VECT traffic is ignored). x=refresh_rate_us,
      // y=offset_us, plain microseconds. Feeds the shared CRSF sync law.
      mavlink_debug_vect_t dv;
      mavlink_msg_debug_vect_decode(msg, &dv);
      if (strncmp(dv.name, MAVLINK_SYNC_TAG, sizeof(MAVLINK_SYNC_TAG) - 1) == 0) {
        getModuleSyncStatus(module).update((uint16_t)dv.x, (int16_t)dv.y);
      }
    } break;

    case MAVLINK_MSG_ID_TUNNEL: {
      mavlink_tunnel_t tn;
      mavlink_msg_tunnel_decode(msg, &tn);
      if (tn.payload_type != MAV_TUNNEL_PAYLOAD_TYPE_ELRS_CRSF) break;
      if (tn.payload_length < 4) break;
      if (crc8(tn.payload + 2, tn.payload_length - 3) !=
          tn.payload[tn.payload_length - 1])
        break;
      pushTelemetryDataToQueues(tn.payload + 1, tn.payload_length - 2);
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

// USB-VCP idle callback: runs in a FreeRTOS-safe context, so trigger directly.
static void mavlinkRxTrigger(void* param)
{
  auto mod_st = (etx_module_state_t*)param;
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);
  if (!drv || !ctx || !drv->getBufferedBytes) return;
  if (drv->getBufferedBytes(ctx) == 0) return;
  telemetryFrameTrigger_ISR(modulePortGetModule(mod_st), &MavlinkDriver);
}

#if !defined(SIMU) && defined(HARDWARE_EXTERNAL_MODULE)
// EXTI software-interrupt handler (FreeRTOS-safe priority) for the S.PORT path.
static void mavlinkExtmoduleFrameReceived()
{
  telemetryFrameTrigger_ISR(EXTERNAL_MODULE, &MavlinkDriver);
}

// S.PORT USART IDLE callback (IRQ priority 0): cannot call FreeRTOS directly,
// so bounce to the EXTI SWI above (mirrors CRSF's _soft_irq_trigger).
static void mavlinkSoftIrqTrigger(void* param)
{
  auto mod_rx = (etx_module_driver_t*)param;
  auto drv = modulePortGetSerialDrv(*mod_rx);
  auto ctx = modulePortGetCtx(*mod_rx);
  if (!drv || !ctx || !drv->getBufferedBytes) return;
  if (drv->getBufferedBytes(ctx) == 0) return;
#if defined(TELEMETRY_USE_CUSTOM_EXTI)
  stm32_exti_custom_trigger_swi(TELEMETRY_RX_FRAME_EXTI_LINE);
#else
  stm32_exti_trigger_swi(TELEMETRY_RX_FRAME_EXTI_LINE);
#endif
}
#endif

const etx_proto_driver_t MavlinkDriver = {
    .protocol = PROTOCOL_CHANNELS_MAVLINK,
    .init = mavlinkInit,
    .deinit = mavlinkDeInit,
    .sendPulses = mavlinkSendPulses,
    .processData = nullptr,  // frame-only (idle/EXTI) for both transports
    .processFrame = mavlinkProcessFrame,
    .onConfigChange = nullptr,
    .txCompleted = modulePortSerialTxCompleted,
};

const MavlinkModuleStatus* mavlinkGetStatus(uint8_t module)
{
  if (module >= NUM_MODULES) return nullptr;
  return &mavlinkState[module].pub;
}
