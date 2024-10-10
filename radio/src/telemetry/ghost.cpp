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

#include "ghost.h"
#include "ghost_menu.h"

#include "edgetx.h"

const char * const ghstRfProfileValue[GHST_RF_PROFILE_COUNT] = { "Auto", "Norm", "Race", "Pure", "Long", "Unused", "Race2", "Pure2" };
const char * const ghstVtxBandName[GHST_VTX_BAND_COUNT] = { "- - -" , "IRC", "Race", "BandE", "BandB", "BandA" };

struct GhostSensor
{
  const uint16_t id;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

// clang-format off
#define GS(id,name,unit,precision) {id,unit,precision,name}

// telemetry sensors ID
enum
{
  GHOST_ID_RX_RSSI = 0x0001,            // Rx-side RSSI
  GHOST_ID_RX_LQ = 0x0002,              // Rx-side link quality
  GHOST_ID_RX_SNR = 0x0003,             // Rx-side signal to noise
  GHOST_ID_FRAME_RATE = 0x0004,         // Tx-side frame rate
  GHOST_ID_TX_POWER = 0x0005,           // Tx-side power output
  GHOST_ID_RF_MODE = 0x0006,            // Tx-side frame rate
  GHOST_ID_TOTAL_LATENCY = 0x0007,      // Tx-side total latency
  GHOST_ID_VTX_FREQ = 0x0008,           // Vtx Frequency (in MHz)
  GHOST_ID_VTX_POWER = 0x0009,          // Vtx Power (in mW)
  GHOST_ID_VTX_CHAN = 0x000A,           // Vtx Channel
  GHOST_ID_VTX_BAND = 0x000B,           // Vtx Band

  GHOST_ID_PACK_VOLTS = 0x000C,         // Battery Pack Voltage
  GHOST_ID_PACK_AMPS = 0x000D,          // Battery Pack Current
  GHOST_ID_PACK_MAH = 0x000E,           // Battery Pack mAh consumed

  GHOST_ID_GPS_LAT = 0x000F,            // GPS Latitude
  GHOST_ID_GPS_LONG = 0x0010,           // GPS Longitude
  GHOST_ID_GPS_ALT = 0x0011,            // GPS Altitude
  GHOST_ID_GPS_HDG = 0x0012,            // GPS Heading
  GHOST_ID_GPS_GSPD = 0x0013,           // GPS Ground Speed
  GHOST_ID_GPS_SATS = 0x0014            // GPS Satellite Count
};

const GhostSensor ghostSensors[] = {
  GS(GHOST_ID_RX_RSSI,         STR_SENSOR_RSSI,             UNIT_DB,                0),
  GS(GHOST_ID_RX_LQ,           STR_SENSOR_RX_QUALITY,       UNIT_PERCENT,           0),
  GS(GHOST_ID_RX_SNR,          STR_SENSOR_RX_SNR,           UNIT_DB,                0),

  GS(GHOST_ID_FRAME_RATE,      STR_SENSOR_FRAME_RATE,       UNIT_RAW,               0),
  GS(GHOST_ID_TX_POWER,        STR_SENSOR_TX_POWER,         UNIT_MILLIWATTS,        0),
  GS(GHOST_ID_RF_MODE,         STR_SENSOR_RF_MODE,          UNIT_TEXT,              0),
  GS(GHOST_ID_TOTAL_LATENCY,   STR_SENSOR_TOTAL_LATENCY,    UNIT_RAW,               0),

  GS(GHOST_ID_VTX_FREQ,        STR_SENSOR_VTX_FREQ,         UNIT_RAW,               0),
  GS(GHOST_ID_VTX_POWER,       STR_SENSOR_VTX_PWR,          UNIT_RAW,               0),
  GS(GHOST_ID_VTX_CHAN,        STR_SENSOR_VTX_CHAN,         UNIT_RAW,               0),
  GS(GHOST_ID_VTX_BAND,        STR_SENSOR_VTX_BAND,         UNIT_TEXT,              0),

  GS(GHOST_ID_PACK_VOLTS,      STR_SENSOR_BATT,             UNIT_VOLTS,             2),
  GS(GHOST_ID_PACK_AMPS,       STR_SENSOR_CURR,             UNIT_AMPS,              2),
  GS(GHOST_ID_PACK_MAH,        STR_SENSOR_CAPACITY,         UNIT_MAH,               0),

  GS(GHOST_ID_GPS_LAT,         STR_GPS,                     UNIT_GPS_LATITUDE,      0),
  GS(GHOST_ID_GPS_LONG,        STR_GPS,                     UNIT_GPS_LONGITUDE,     0),
  GS(GHOST_ID_GPS_GSPD,        STR_SENSOR_GSPD,             UNIT_KMH,               1),
  GS(GHOST_ID_GPS_HDG,         STR_SENSOR_HDG,              UNIT_DEGREE,            3),
  GS(GHOST_ID_GPS_ALT,         STR_SENSOR_GPSALT,           UNIT_METERS,            0),
  GS(GHOST_ID_GPS_SATS,        STR_SENSOR_SATELLITES,       UNIT_RAW,               0),

  GS(0x00,                     NULL,                        UNIT_RAW,               0),
};
// clang-format on

const GhostSensor *getGhostSensor(uint8_t id)
{
  for (const GhostSensor * sensor = ghostSensors; sensor->id; sensor++) {
    if (id == sensor->id)
      return sensor;
  }
  return nullptr;
}

void processGhostTelemetryValue(uint8_t index, int32_t value)
{
  if (!TELEMETRY_STREAMING())
    return;

  const GhostSensor *sensor = getGhostSensor(index);
  if (sensor) {
    uint16_t id = sensor->id;
    if (id == GHOST_ID_GPS_LONG) {
      id = GHOST_ID_GPS_LAT;
    }
    setTelemetryValue(PROTOCOL_TELEMETRY_GHOST, id, 0, 0, value, sensor->unit,
                      sensor->precision);
  }
}

void processGhostTelemetryValueString(const GhostSensor * sensor, const char * str)
{
  if (TELEMETRY_STREAMING()) {
    setTelemetryText(PROTOCOL_TELEMETRY_GHOST, sensor->id, 0, 0, str);
  }
}

static bool checkGhostTelemetryFrameCRC(const uint8_t* frame, uint32_t len)
{
  uint8_t crc = crc8(frame, len - 1);
  return (crc == frame[len - 1]);
}

// hifirst
static uint16_t _get_u16(const uint8_t* frame, uint8_t offset)
{
  return (frame[offset] << 8) | frame[offset + 1];
}

// lofirst
static uint16_t _get_u16le(const uint8_t* frame, uint8_t offset)
{
  return (frame[offset + 1] << 8) | frame[offset];
}

// hifirst
static uint32_t _get_s32(const uint8_t* frame, uint8_t offset)
{
  uint32_t val = 0;
  for (int i = 0; i < 4; ++i)
    val <<= 8, val |= frame[offset + i];
  return val;
}

// lofirst
static uint32_t _get_s32le(const uint8_t* frame, uint8_t offset)
{
  uint32_t val = 0;
  for (int i = 0; i < 4; ++i)
    val <<= 8, val |= frame[offset + 3 - i];
  return val;
}

void processGhostTelemetryFrame(uint8_t module, uint8_t* buffer, uint32_t length)
{
  uint8_t frame_len = buffer[1];
  auto frame = buffer + 2;
  if (!checkGhostTelemetryFrameCRC(frame, frame_len)) {
    TRACE("[GS] CRC error");
    return;
  }

  uint8_t id = frame[0];
  switch(id) {
    case GHST_DL_OPENTX_SYNC:
    {
      uint32_t update_interval = _get_s32(frame, 1);
      int32_t  offset = _get_s32(frame, 5);

      // values are in units of 100ns
      update_interval /= 10;
      offset /= 10;

      getModuleSyncStatus(module).update(update_interval, offset);
    }
    break;

    case GHST_DL_LINK_STAT:
    {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, length);
      }
#endif
      // RSSI is a negative value, but sent as a positive integer.
      uint8_t rssiVal = min<uint8_t>(frame[1], 120);
      uint8_t lqVal = min<uint8_t>(frame[2], 100);
      uint8_t snrVal = min<uint8_t>(frame[3], 100);

      processGhostTelemetryValue(GHOST_ID_RX_RSSI, - rssiVal);
      processGhostTelemetryValue(GHOST_ID_RX_LQ, lqVal);
      processGhostTelemetryValue(GHOST_ID_RX_SNR, snrVal);

      // give OpenTx the LQ value, not RSSI
      if (lqVal) {
        telemetryData.rssi.set(lqVal);
        telemetryStreaming = TELEMETRY_TIMEOUT10ms;
      }
      else {
        telemetryData.rssi.reset();
        telemetryStreaming = 0;
      }

      processGhostTelemetryValue(GHOST_ID_TX_POWER, _get_u16(frame, 4));
      processGhostTelemetryValue(GHOST_ID_FRAME_RATE, _get_u16(frame, 6));
      processGhostTelemetryValue(GHOST_ID_TOTAL_LATENCY, _get_u16(frame, 8));
      uint8_t rfModeEnum = min<uint8_t>(frame[10], GHST_RF_PROFILE_MAX);

      // RF mode string, one char at a time
      const GhostSensor * sensor = getGhostSensor(GHOST_ID_RF_MODE);
      const char * rfModeString = ghstRfProfileValue[rfModeEnum];
      processGhostTelemetryValueString(sensor, rfModeString);
      break;
    }

    case GHST_DL_VTX_STAT:
    {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, length);
      }
#endif
      uint8_t vtxBandEnum = min<uint8_t>(frame[6], GHST_VTX_BAND_MAX);

      const GhostSensor * sensor = getGhostSensor(GHOST_ID_VTX_BAND);
      const char * vtxBandString = ghstVtxBandName[vtxBandEnum];

      processGhostTelemetryValue(GHOST_ID_VTX_FREQ, _get_u16(frame, 2));
      processGhostTelemetryValue(GHOST_ID_VTX_POWER, _get_u16(frame, 4));
      processGhostTelemetryValue(GHOST_ID_VTX_CHAN, min<uint8_t>(frame[7], 8));
      processGhostTelemetryValueString(sensor, vtxBandString);
      break;
    }

    case GHST_DL_MENU_DESC:
    {
      GhostMenuFrame * packet;
      GhostMenuData * lineData;
      packet = (GhostMenuFrame * )buffer;
      lineData = (GhostMenuData *) &reusableBuffer.ghostMenu.line[packet->lineIndex];
      lineData->splitLine = 0;
      reusableBuffer.ghostMenu.menuStatus = packet->menuStatus;
      lineData->lineFlags = packet->lineFlags;
      for (uint8_t i = 0; i < GHST_MENU_CHARS; i++) {
        if (packet->menuText[i] == 0x7C) {
          lineData->menuText[i] = '\0';
          lineData->splitLine = i + 1;
        }
        else {
          lineData->menuText[i] = packet->menuText[i];
        }
      }
      lineData->menuText[GHST_MENU_CHARS] = '\0';
      break;
    }

    case GHST_DL_PACK_STAT: {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, length);
      }
#endif
      processGhostTelemetryValue(GHOST_ID_PACK_VOLTS, _get_u16le(frame, 1));
      processGhostTelemetryValue(GHOST_ID_PACK_AMPS, _get_u16le(frame, 3));
      processGhostTelemetryValue(GHOST_ID_PACK_MAH, _get_u16le(frame, 5) * 10);
      break;
    }

    case GHST_DL_GPS_PRIMARY: {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, length);
      }
#endif
      processGhostTelemetryValue(GHOST_ID_GPS_LAT, ((int32_t)_get_s32le(frame, 1)) / 10);  
      processGhostTelemetryValue(GHOST_ID_GPS_LONG, ((int32_t)_get_s32le(frame, 5)) / 10);
      processGhostTelemetryValue(GHOST_ID_GPS_ALT, (int16_t)_get_u16le(frame, 9));
      break; 
    }

    case GHST_DL_GPS_SECONDARY: {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, length);
      }
#endif
      processGhostTelemetryValue(GHOST_ID_GPS_HDG, _get_u16le(frame, 3) / 10);   

      // ground speed is passed via GHST as cm/s, converted to km/h for OpenTx
      processGhostTelemetryValue(GHOST_ID_GPS_GSPD, (_get_u16le(frame, 1) * 36 + 50) / 100);
      processGhostTelemetryValue(GHOST_ID_GPS_SATS, frame[5]);
      break; 
    }
    case GHST_DL_MAGBARO: {
      // Not implemented yet
      break;
    }
#if defined(LUA)
    default:
      // destination address and CRC are skipped
      if (luaInputTelemetryFifo && luaInputTelemetryFifo->hasSpace(length - 2) ) {
        for (uint8_t i = 1; i < length - 1; i++) {
          luaInputTelemetryFifo->push(buffer[i]);
        }
      }
      break;
#endif
  }
}

void ghostSetDefault(int index, uint8_t id, uint8_t subId)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.instance = subId;

  const GhostSensor * sensor = getGhostSensor(id);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE) {
      unit = UNIT_GPS;
    }
    uint8_t prec = min<uint8_t>(2, sensor->precision);
    telemetrySensor.init(sensor->name, unit, prec);
  }
  else
    telemetrySensor.init(id);

  storageDirty(EE_MODEL);
}
