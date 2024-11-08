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

#if !defined(SIMU)
#include "stm32_exti_driver.h"
#include "stm32_hal_ll.h"
#endif

#include "edgetx.h"
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

#define MODULE_ALIVE_TIMEOUT  50                      // if the module has sent a valid frame within 500ms it is declared alive
static tmr10ms_t lastAlive[NUM_MODULES];              // last time stamp module sent CRSF frames
static bool moduleAlive[NUM_MODULES];                 // module alive status

uint8_t createCrossfireBindFrame(uint8_t moduleIdx, uint8_t * frame)
{
  uint8_t * buf = frame;
  *buf++ = UART_SYNC;                                 /* device address */
  *buf++ = 7;                                         /* frame length */
  *buf++ = COMMAND_ID;                                /* cmd type */
  if (TELEMETRY_STREAMING())
    *buf++ = RECEIVER_ADDRESS;                        /* Destination is receiver (unbind) */
  else
    *buf++ = MODULE_ADDRESS;                          /* Destination is module */
  *buf++ = RADIO_ADDRESS;                             /* Origin Address */
  *buf++ = SUBCOMMAND_CRSF;                           /* sub command */
  *buf++ = SUBCOMMAND_CRSF_BIND;                      /* initiate bind */
  *buf++ = crc8_BA(frame + 2, 5);
  *buf++ = crc8(frame + 2, 6);
  return buf - frame;
}

uint8_t createCrossfirePingFrame(uint8_t moduleIdx, uint8_t * frame)
{
  uint8_t * buf = frame;
  *buf++ = UART_SYNC;                                 /* device address */
  *buf++ = 4;                                         /* frame length */
  *buf++ = PING_DEVICES_ID;                           /* cmd type */
  *buf++ = BROADCAST_ADDRESS;                         /* Destination Address */
  *buf++ = RADIO_ADDRESS;                             /* Origin Address */
  *buf++ = crc8(frame + 2, 3);
  return buf - frame;
}

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
uint8_t createCrossfireChannelsFrame(uint8_t moduleIdx, uint8_t * frame, int16_t * pulses)
{
  //
  // sends channel data and also communicates commanded armed status in arming mode Switch.
  // frame len 24 -> arming mode CH5: module will use channel 5
  // frame len 25 -> arming mode Switch: send commanded armed status in extra byte after channel data
  // 
  ModuleData *md = &g_model.moduleData[moduleIdx];

  uint8_t armingMode = md->crsf.crsfArmingMode; // 0 = Channel mode, 1 = Switch mode
  uint8_t lenAdjust = (armingMode == ARMING_MODE_SWITCH) ? 1 : 0;

  uint8_t * buf = frame;
  *buf++ = MODULE_ADDRESS;
  *buf++ = 24 + lenAdjust;      // 1(ID) + 22(channel data) + (+1 extra byte if Switch mode) + 1(CRC)
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
  
  if (armingMode == ARMING_MODE_SWITCH) {
    swsrc_t sw =  md->crsf.crsfArmingTrigger;

    *buf++ = (sw != SWSRC_NONE) && getSwitch(sw, 0);  // commanded armed status in Switch mode
  }
  
  *buf++ = crc8(crc_start, 23 + lenAdjust);
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
    //
    // An ELRS module stores the RF parameters in a model specific way using the
    // modelID as index. If the module resets after it was initally initialized the modelID 
    // needs to be resent as otherwise the module assumes modelID 0 which leads to the
    // module using the stored RF parameters for the model with modelID 0. This is not only
    // annoying but also potentially dangerous as a receiver will no longer re-connect.
    //
    // Reasons for a module resetting might be:
    // - power surge
    // - internal non-recoverable error
    // - after flashing in WiFi mode
    // - putting the module in WiFi mode and exiting WiFi mode (LUA script)
    // 
    // This logic takes care of sending the modelID again after a module comes back to 
    // live after a module reset
    // 
    if(moduleState[module].counter != CRSF_FRAME_MODELID ) {            // skip the reset check logic if first init
      if((get_tmr10ms() - lastAlive[module]) > MODULE_ALIVE_TIMEOUT) {  // check if module has recently sent CRSF frames 
        moduleAlive[module] = false;                                    // no, declare it as dead  
      } else {
        if(moduleAlive[module] == false) {                              // if the module was dead and came back to live, e.g. reset
          moduleAlive[module] = true;                                   // declare the module as alive
          moduleState[module].counter = CRSF_FRAME_MODELID;             // and send it the modelID again 
        }
      }
    }

    if (moduleState[module].counter == CRSF_FRAME_MODELID) {
      TRACE("[XF] ModelID %d", g_model.header.modelId[module]);
      p_buf += createCrossfireModelIDFrame(module, p_buf);
      moduleState[module].counter = CRSF_FRAME_MODELID_SENT;
    } else if (moduleState[module].counter == CRSF_FRAME_MODELID_SENT && crossfireModuleStatus[module].queryCompleted == false) {
      p_buf += createCrossfirePingFrame(module, p_buf);
    } else if (moduleState[module].mode == MODULE_MODE_BIND) {
      p_buf += createCrossfireBindFrame(module, p_buf);
      moduleState[module].mode = MODULE_MODE_NORMAL;
    } else {
      /* TODO: nChannels */
      p_buf += createCrossfireChannelsFrame(module, p_buf, channels);
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
  uint8_t crc = crc8(&rxBuffer[2], len - 1);
  // TRACE("[XF] crc = %d; pkt = %d", crc, rxBuffer[len + 1]);
  return (crc == rxBuffer[len + 1]);
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

static bool _lenIsSane(uint32_t len)
{
  // packet len must be at least 3 bytes (type + payload + crc)
  // and 2 bytes < MAX (hdr + len)
  return (len > 2 && len < TELEMETRY_RX_PACKET_SIZE - 1);
}

static bool _validHdr(uint8_t* buf)
{
  return buf[0] == RADIO_ADDRESS || buf[0] == UART_SYNC;
}

static uint8_t* _processFrames(void* ctx, uint8_t* buf, uint8_t& len)
{
  uint8_t* p_buf = buf;
  while (len >= MIN_FRAME_LEN) {

    if (!_validHdr(p_buf)) {
      TRACE("[XF] skipping invalid start bytes");
      do { p_buf++; len--; } while(len > 0 && !_validHdr(p_buf));
      if (len < MIN_FRAME_LEN) break;
    }

    uint32_t pkt_len = p_buf[1] + 2;
    if (!_lenIsSane(pkt_len)) {
      TRACE("[XF] pkt len error (%d)", pkt_len);
      len = 0;
      break;
    }

    if (pkt_len > (uint32_t)len) {
      // incomplete packet
      break;
    }

    if (!_checkFrameCRC(p_buf)) {
      TRACE("[XF] CRC error ");
    } else {
#if defined(BLUETOOTH)
      // TODO: generic telemetry mirror to BT
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(p_buf, pkt_len);
      }
#endif
      auto mod_st = (etx_module_state_t*)ctx;
      auto module = modulePortGetModule(mod_st);
      lastAlive[module] = get_tmr10ms();                              // valid frame received, note timestamp
      processCrossfireTelemetryFrame(module, p_buf, pkt_len);
    }

    p_buf += pkt_len;
    len -= pkt_len;
  }
  
  return p_buf;
}

static void crossfireProcessFrame(void* ctx, uint8_t* frame, uint8_t frame_len,
                                  uint8_t* buf, uint8_t* p_len)
{
  if (frame_len < MIN_FRAME_LEN) return;

  uint8_t& len = *p_len;
  if (len == 0) {
    // buffer is empty: no re-assembly
    if (!_validHdr(frame)) {
      TRACE("[XF] invalid frame start");
      return;
    }

    // process frames directly out of RX buffer
    uint8_t* p_buf = _processFrames(ctx, frame, frame_len);
    if (frame_len > 0) {
      memcpy(buf, p_buf, frame_len);
      len = frame_len;
    }
  } else {
    uint32_t defrag_len = (uint32_t)len + (uint32_t)frame_len;
    if (defrag_len > TELEMETRY_RX_PACKET_SIZE) {
      TRACE("[XF] overflow (%d > %d)", defrag_len, TELEMETRY_RX_PACKET_SIZE);
      frame_len = TELEMETRY_RX_PACKET_SIZE - len;
      defrag_len = (uint32_t)len + (uint32_t)frame_len;
    }

    memcpy(buf + len, frame, frame_len);
    len = (uint8_t)defrag_len;

    uint8_t* p_buf = _processFrames(ctx, buf, len);
    if ((len > 0) && (p_buf != buf)) {
      memmove(buf, p_buf, len);
    }
  }
}

static const etx_serial_init crsfSerialParams = {
  .baudrate = 0,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

#if !defined(SIMU)

#if defined(INTERNAL_MODULE_CRSF)
static void _crsf_intmodule_frame_received(void*)
{
  telemetryFrameTrigger_ISR(INTERNAL_MODULE, &CrossfireDriver);
}
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
static void _crsf_extmodule_frame_received()
{
  telemetryFrameTrigger_ISR(EXTERNAL_MODULE, &CrossfireDriver);
}

// proxy trigger to avoid calling
// FreeRTOS methods from ISR with prio 0
static void _soft_irq_trigger(void*)
{
#ifndef STM32H7
  EXTI->SWIER = TELEMETRY_RX_FRAME_EXTI_LINE;
#else
  EXTI->SWIER1 = TELEMETRY_RX_FRAME_EXTI_LINE;
#endif
}
#endif

#endif // !SIMU

static void* crossfireInit(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init params(crsfSerialParams);

#if defined(INTERNAL_MODULE_CRSF)
  if (module == INTERNAL_MODULE) {
    params.baudrate = INT_CROSSFIRE_BAUDRATE;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, false);

    if (mod_st) {
      auto drv = modulePortGetSerialDrv(mod_st->rx);
      auto ctx = modulePortGetCtx(mod_st->rx);

      auto& rx_count = getTelemetryRxBufferCount(INTERNAL_MODULE);
      rx_count = 0;

#if !defined(SIMU)
      if (drv && ctx && drv->setIdleCb) {
        drv->setIdleCb(ctx, _crsf_intmodule_frame_received, nullptr);
      }
#endif
    }
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {
    params.baudrate = EXT_CROSSFIRE_BAUDRATE;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &params, false);

    if (mod_st) {
      auto drv = modulePortGetSerialDrv(mod_st->rx);
      auto ctx = modulePortGetCtx(mod_st->rx);

      auto& rx_count = getTelemetryRxBufferCount(EXTERNAL_MODULE);
      rx_count = 0;

#if !defined(SIMU)
      if (drv && ctx && drv->setIdleCb) {
        drv->setIdleCb(ctx, _soft_irq_trigger, nullptr);
        stm32_exti_enable(TELEMETRY_RX_FRAME_EXTI_LINE, 0,
                          _crsf_extmodule_frame_received);
      }
#endif
    }

    memset(&crossfireModuleStatus[module], 0, sizeof(crossfireModuleStatus[module]));
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

  memset(&crossfireModuleStatus[modulePortGetModule(mod_st)], 0,
         sizeof(CrossfireModuleStatus));

#if !defined(SIMU) && defined(HARDWARE_EXTERNAL_MODULE)
  if (mod_st && (modulePortGetModule(mod_st) == EXTERNAL_MODULE)) {
    auto drv = modulePortGetSerialDrv(mod_st->rx);
    auto ctx = modulePortGetCtx(mod_st->rx);
    if (drv && ctx && drv->setIdleCb) {
      stm32_exti_disable(TELEMETRY_RX_FRAME_EXTI_LINE);
    }
  }
#endif

  modulePortDeInit(mod_st);
}

const etx_proto_driver_t CrossfireDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInit,
  .deinit = crossfireDeInit,
  .sendPulses = crossfireSendPulses,
  .processData = nullptr,
  .processFrame = crossfireProcessFrame,
  .onConfigChange = nullptr,
};
