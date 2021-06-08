/*
 * Copyright (C) EdgeTX
 *
 * (c) www.olliw.eu, OlliW, OlliW42
 *
 * Based on code named
 *   opentx - http://github.com/opentx/opentx
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
#include "telemetry/mavlink/mavlink_telem.h"

// -- CoOS RTOS mavlink task handlers --

RTOS_TASK_HANDLE mavlinkTaskId;
RTOS_DEFINE_STACK(mavlinkStack, MAVLINK_STACK_SIZE);

struct MavlinkTaskStat {
  uint16_t start = 0;
  uint16_t run = 0;
  uint16_t max = 0;
  uint16_t loop = 0;
};
struct MavlinkTaskStat mavlinkTaskStat;

uint16_t mavlinkTaskRunTime(void)
{
  return mavlinkTaskStat.run/2;
}

uint16_t mavlinkTaskRunTimeMax(void)
{
  return mavlinkTaskStat.max/2;
}

uint16_t mavlinkTaskLoop(void)
{
  return mavlinkTaskStat.loop/2;
}

TASK_FUNCTION(mavlinkTask)
{
  while (true) {
    uint16_t start_last = mavlinkTaskStat.start;
    mavlinkTaskStat.start = getTmr2MHz();

    mavlinkTelem.wakeup();

    mavlinkTaskStat.run = getTmr2MHz() - mavlinkTaskStat.start;
    if (mavlinkTaskStat.run > mavlinkTaskStat.max) mavlinkTaskStat.max = mavlinkTaskStat.run;
    mavlinkTaskStat.loop = (mavlinkTaskStat.start - start_last);

    RTOS_WAIT_TICKS(2);
  }
}

void mavlinkStart()
{
  RTOS_CREATE_TASK(mavlinkTaskId, mavlinkTask, "mavlink", mavlinkStack, MAVLINK_STACK_SIZE, MAVLINK_TASK_PRIO);
}

// -- AUX1, AUX2 handlers --

uint32_t _cvtBaudrate(uint16_t baud)
{
  switch (baud) {
    case 0: return 57600;
    case 1: return 115200;
    case 2: return 38400;
    case 3: return 19200;
  }
  return 57600;
}

uint32_t mavlinkTelemAuxBaudrate(void)
{
  return _cvtBaudrate(g_eeGeneral.mavlinkBaudrate);
}

uint32_t mavlinkTelemAux2Baudrate(void)
{
  return _cvtBaudrate(g_eeGeneral.mavlinkBaudrate2);
}

#if defined(AUX_SERIAL)
MAVLINK_RAM_SECTION Fifo<uint8_t, 2*512> auxSerialTxFifo;
MAVLINK_RAM_SECTION Fifo<uint8_t, 2*512> mavlinkTelemAuxSerialRxFifo;
#endif

#if defined(AUX2_SERIAL)
MAVLINK_RAM_SECTION Fifo<uint8_t, 2*512> aux2SerialTxFifo;
MAVLINK_RAM_SECTION Fifo<uint8_t, 2*512> mavlinkTelemAux2SerialRxFifo;
#endif

#if defined(TELEMETRY_MAVLINK) && defined(USB_SERIAL)
MAVLINK_RAM_SECTION Fifo<uint8_t, 2*512> mavlinkTelemUsbRxFifo;
#endif

#if defined(AUX_SERIAL)

uint32_t mavlinkTelem1Available(void)
{
  if (!mavlinkTelem.serial1_enabled) return 0;

//  if (auxSerialMode != UART_MODE_MAVLINK) return 0;
  return mavlinkTelemAuxSerialRxFifo.size();
}

// call only after check with mavlinkTelem2Available()
uint8_t mavlinkTelem1Getc(uint8_t* c)
{
  if (!mavlinkTelem.serial1_enabled) return 0;

  return mavlinkTelemAuxSerialRxFifo.pop(*c);
}

bool mavlinkTelem1HasSpace(uint16_t count)
{
  if (!mavlinkTelem.serial1_enabled) return 0;

//  if (auxSerialMode != UART_MODE_MAVLINK) return false;
  return auxSerialTxFifo.hasSpace(count);
}

bool mavlinkTelem1PutBuf(const uint8_t* buf, const uint16_t count)
{
  if (!mavlinkTelem.serial1_enabled || !buf) return false;

  if (!auxSerialTxFifo.hasSpace(count)) return false;
//  if (auxSerialMode != UART_MODE_MAVLINK || !buf || !auxSerialTxFifo.hasSpace(count)) {
//    return false;
//  }
  for (uint16_t i = 0; i < count; i++) auxSerialTxFifo.push(buf[i]);
  USART_ITConfig(AUX_SERIAL_USART, USART_IT_TXE, ENABLE);
  return true;
}

#else
uint32_t mavlinkTelem1Available(void){ return 0; }
uint8_t mavlinkTelem1Getc(uint8_t* c){ return 0; }
bool mavlinkTelem1HasSpace(uint16_t count){ return false; }
bool mavlinkTelem1PutBuf(const uint8_t* buf, const uint16_t count){ return false; }
#endif

#if defined(AUX2_SERIAL)

uint32_t mavlinkTelem2Available(void)
{
  if (!mavlinkTelem.serial2_enabled) return 0;

//  if (aux2SerialMode != UART_MODE_MAVLINK) return 0;
  return mavlinkTelemAux2SerialRxFifo.size();
}

// call only after check with mavlinkTelem2Available()
uint8_t mavlinkTelem2Getc(uint8_t* c)
{
  if (!mavlinkTelem.serial2_enabled) return 0;

  return mavlinkTelemAux2SerialRxFifo.pop(*c);
}

bool mavlinkTelem2HasSpace(uint16_t count)
{
  if (!mavlinkTelem.serial2_enabled) return 0;

//  if (aux2SerialMode != UART_MODE_MAVLINK) return false;
  return aux2SerialTxFifo.hasSpace(count);
}

bool mavlinkTelem2PutBuf(const uint8_t* buf, const uint16_t count)
{
  if (!mavlinkTelem.serial2_enabled || !buf) return false;

  if (!aux2SerialTxFifo.hasSpace(count)) return false;
//  if (aux2SerialMode != UART_MODE_MAVLINK || !buf || !aux2SerialTxFifo.hasSpace(count)) {
//    return false;
//  }
  for (uint16_t i = 0; i < count; i++) aux2SerialTxFifo.push(buf[i]);
  USART_ITConfig(AUX2_SERIAL_USART, USART_IT_TXE, ENABLE);
  return true;
}

#else
uint32_t mavlinkTelem2Available(void){ return 0; }
uint8_t mavlinkTelem2Getc(uint8_t* c){ return 0; }
bool mavlinkTelem2HasSpace(uint16_t count){ return false; }
bool mavlinkTelem2PutBuf(const uint8_t* buf, const uint16_t count){ return false; }
#endif

// -- USB handlers --

//TODO: is it really necessary to have this extra define?
#if defined(TELEMETRY_MAVLINK_USB_SERIAL)

uint32_t mavlinkTelem3Available(void)
{
  // TODO: check if MAVLink has been assigned to USB serial
  if (getSelectedUsbMode() != USB_SERIAL_MODE) return 0;
  return mavlinkTelemUsbRxFifo.size();
}

// call only after check with mavlinkTelem2Available()
uint8_t mavlinkTelem3Getc(uint8_t* c)
{
  return mavlinkTelemUsbRxFifo.pop(*c);
}

bool mavlinkTelem3HasSpace(uint16_t count)
{
  // TODO: check if MAVLink has been assigned to USB serial
  if (getSelectedUsbMode() != USB_SERIAL_MODE) return false;
  return true; //??
}

bool mavlinkTelem3PutBuf(const uint8_t* buf, const uint16_t count)
{
  // TODO: check if MAVLink has been assigned to USB serial
  if (getSelectedUsbMode() != USB_SERIAL_MODE || !buf) {
    return false;
  }
  for (uint16_t i = 0; i < count; i++) {
    usbSerialPutc(buf[i]);
  }
  return true;
}

#else
uint32_t mavlinkTelem3Available(void){ return 0; }
uint8_t mavlinkTelem3Getc(uint8_t* c){ return 0; }
bool mavlinkTelem3HasSpace(uint16_t count){ return false; }
bool mavlinkTelem3PutBuf(const uint8_t* buf, const uint16_t count){ return false; }
#endif

// -- MavlinkTelem stuff --

// map aux1,aux2 onto serial1 & serial2
void MavlinkTelem::map_serials(void)
{
  serial1_enabled = _aux1_enabled;
  serial2_enabled = _aux2_enabled;
}

void MavlinkTelem::telemetrySetValue(uint16_t id, uint8_t subId, uint8_t instance, int32_t value, uint32_t unit, uint32_t prec)
{
  if (g_model.mavlinkRssi) {
    if (!radio.is_receiving && !radio.is_receiving65 && !radio.is_receiving35) return;
  }

  if (g_model.mavlinkMimicSensors) {
    setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, id, subId, instance, value, unit, prec);
    telemetryStreaming = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT; //2 * TELEMETRY_TIMEOUT10ms; // 2 seconds
  }
}

// only for MAVLINK_MSG_ID_RADIO_STATUS, MAVLINK_MSG_ID_RC_CHANNELS, MAVLINK_MSG_ID_RC_CHANNELS_RAW
void MavlinkTelem::telemetrySetRssiValue(uint8_t rssi)
{
  if (g_model.mavlinkRssiScale > 0) {
    if (g_model.mavlinkRssiScale < 255) { //if not full range, respect  UINT8_MAX
      if (rssi == UINT8_MAX) rssi = 0;
    }
    if (rssi > g_model.mavlinkRssiScale) rssi = g_model.mavlinkRssiScale; //constrain
    rssi = (uint8_t)( ((uint16_t)rssi * 100) / g_model.mavlinkRssiScale); //scale to 0..99
  }
  else { //mavlink default
    if (rssi == UINT8_MAX) rssi = 0;
  }

  radio.rssi_scaled = rssi;

  if (g_model.mavlinkRssi) {
    if (!radio.is_receiving && !radio.is_receiving65 && !radio.is_receiving35) return;
  }

  if (g_model.mavlinkRssi) {
    telemetryData.rssi.set(rssi);
    telemetryStreaming = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT; //2 * TELEMETRY_TIMEOUT10ms; // 2 seconds
  }

  if (g_model.mavlinkRssi || g_model.mavlinkMimicSensors) {
    setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, RSSI_ID, 0, 1, (int32_t)rssi, UNIT_DB, 0);
    telemetryStreaming = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT; //2 * TELEMETRY_TIMEOUT10ms; // 2 seconds
  }
  //#if defined(MULTIMODULE)
  //{ TX_RSSI_ID, TX_RSSI_ID, 0, ZSTR_TX_RSSI   , UNIT_DB , 0 },
  //{ TX_LQI_ID , TX_LQI_ID,  0, ZSTR_TX_QUALITY, UNIT_RAW, 0 },
}

// is probably not needed, aren't they reset by telementryStreaming timeout?
void MavlinkTelem::telemetryResetRssiValue(void)
{
  if (radio.is_receiving || radio.is_receiving65 || radio.is_receiving35) return;

  radio.rssi_scaled = 0;

  if (g_model.mavlinkRssi)
    telemetryData.rssi.reset();

  if (g_model.mavlinkRssi || g_model.mavlinkMimicSensors)
    setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, RSSI_ID, 0, 1, 0, UNIT_DB, 0);
}

bool MavlinkTelem::telemetryVoiceEnabled(void)
{
  if (!g_model.mavlinkRssi && !g_model.mavlinkMimicSensors) return true;

  if (g_model.mavlinkRssi && !radio.rssi_voice_disabled) return true;

  return false;
}

