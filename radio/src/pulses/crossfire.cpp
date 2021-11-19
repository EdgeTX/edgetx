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

static void setupPulsesCrossfire(uint8_t idx, int16_t* channels, uint8_t nChannels)
{
  if (idx == INTERNAL_MODULE) {
    auto* p_data = &intmodulePulsesData.crossfire;
    setupPulsesCrossfire(idx, p_data, 0, channels, nChannels);
  } else if (telemetryProtocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
    auto* p_data = &extmodulePulsesData.crossfire;
    setupPulsesCrossfire(idx, p_data, TELEMETRY_ENDPOINT_SPORT, channels, nChannels);
  }
}

#if defined(INTERNAL_MODULE_CRSF)
static void intmoduleCRSF_rx(uint8_t data)
{
  intmoduleFifo.push(data);

#if !defined(SIMU)
  // wakeup mixer when rx buffer is quarter full (16 bytes)
  if (intmoduleFifo.size() >= 16) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(mixerTaskId.rtos_handle, 0, eNoAction,
                       &xHigherPriorityTaskWoken);

    // might effect a context switch on ISR exit
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

static const etx_serial_init intmoduleCrossfireInitParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
  .on_receive = intmoduleCRSF_rx,
  .on_error = nullptr,
};
#endif

static void* crossfireInit(uint8_t module)
{
  if (module == INTERNAL_MODULE) {
#if defined(INTERNAL_MODULE_CRSF)
    // serial port setup
    etx_serial_init params(intmoduleCrossfireInitParams);
    params.baudrate = CROSSFIRE_BAUDRATE;

    // wakeup mixer when rx buffer is quarter full (16 bytes)
    params.on_receive = intmoduleCRSF_rx;

    intmoduleFifo.clear();
    IntmoduleSerialDriver.init(&params);
#endif
  } else {
    // TODO: setup S.PORT
    //  - for now, the init is still done via telemetryWakeup()....
    telemetryInit(PROTOCOL_TELEMETRY_CROSSFIRE);
  }

  // mixer setup
  mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD);
  return (void*)(ulong)module;
}

static void crossfireDeInit(void* context)
{
  ulong module = (ulong)context;

  if (module == INTERNAL_MODULE) {
#if defined(INTERNAL_MODULE_CRSF)
    mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
    intmoduleStop();
    intmoduleFifo.clear();
#endif
  } else {
    mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
  }
}

static void crossfireSetupPulses(void* context, int16_t* channels, uint8_t nChannels)
{
  ulong module = (ulong)context;

  ModuleSyncStatus& status = getModuleSyncStatus(module);
  if (status.isValid()) {
    mixerSchedulerSetPeriod(module, status.getAdjustedRefreshRate());
  } else {
    mixerSchedulerSetPeriod(module, CROSSFIRE_PERIOD);
  }

  setupPulsesCrossfire(module, channels, nChannels);
}

static void crossfireSendPulses(void* context)
{
  ulong module = (ulong)context;

  if (module == INTERNAL_MODULE) {
#if defined(INTERNAL_MODULE_CRSF)
    IntmoduleSerialDriver.sendBuffer(intmodulePulsesData.crossfire.pulses,
                                     intmodulePulsesData.crossfire.length);
#endif
  } else {
    sportSendBuffer(extmodulePulsesData.crossfire.pulses,
                    extmodulePulsesData.crossfire.length);
  }
}

#include "hal/module_driver.h"

etx_module_driver_t CrossfireModuleDriver = {
  .protocol = PROTOCOL_CHANNELS_CROSSFIRE,
  .init = crossfireInit,
  .deinit = crossfireDeInit,
  .setupPulses = crossfireSetupPulses,
  .sendPulses = crossfireSendPulses
};
