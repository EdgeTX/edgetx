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
#include "futaba.h"
#include "sensor_names.h"

struct FutabaSensor {
  const uint16_t id;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char* name;
};

// clang-format off
const FutabaSensor futabaSensors[] = {
  {FUTABA_RX_VOLTAGE, UNIT_VOLTS,   1, STR_SENSOR_BATT},
  {FUTABA_RX_LQ,      UNIT_PERCENT, 0, STR_SENSOR_RX_QUALITY},
  {FUTABA_RX_BARS,    UNIT_RAW,     0, STR_SENSOR_RX_SIGNAL},
  {FUTABA_TX_RSSI,    UNIT_DBM,     0, STR_SENSOR_TX_RSSI},
  {0,                 UNIT_RAW,     0, nullptr},
};
// clang-format on

static const FutabaSensor* getFutabaSensor(uint16_t id)
{
  for (const FutabaSensor* sensor = futabaSensors; sensor->id; sensor++) {
    if (id == sensor->id) return sensor;
  }
  return nullptr;
}

// A link quality % seen this recently: RSSI follows it, not the bars
// (T-FHSS Surface sends it in every other reply, FASSTest 18CH never)
#define FUTABA_LQ_TIMEOUT 200  // 10 ms

static tmr10ms_t lastLqTime;
static bool lqSeen;

void processFutabaPacket(const uint8_t* data, uint8_t len)
{
  if (len < 3) return;

  uint8_t first = data[0];
  int8_t rssi = (int8_t)data[1];
  uint8_t lq = data[2];
  tmr10ms_t now = get_tmr10ms();

  setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, FUTABA_TX_RSSI, 0, 0, rssi,
                    UNIT_DBM, 0);

  if (lq != 0xFF) {
    setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, FUTABA_RX_LQ, 0, 0, lq,
                      UNIT_PERCENT, 0);
    telemetryData.rssi.set(lq);
    lastLqTime = now;
    lqSeen = true;
  }

  // Slot 0, the receiver's: byte 0 bits 7..6 = bars, bit 2 = 0: byte 1 =
  // its voltage in 0.1 V (1: something else, e.g. FASSTest's EXT voltage)
  if (first == 0 && len >= 5) {
    uint8_t bars = data[3] >> 6;
    setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, FUTABA_RX_BARS, 0, 0, bars,
                      UNIT_RAW, 0);
    if (!(data[3] & 0x04))
      setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, FUTABA_RX_VOLTAGE, 0, 0,
                        data[4], UNIT_VOLTS, 1);
    if (!lqSeen || (tmr10ms_t)(now - lastLqTime) > FUTABA_LQ_TIMEOUT) {
      lqSeen = false;
      telemetryData.rssi.set(bars * 100 / 3);
    }
  }

  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
}

void futabaSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor& telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const FutabaSensor* sensor = getFutabaSensor(id);
  if (sensor) {
    telemetrySensor.init(sensor->name, sensor->unit, sensor->precision);
  } else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}
