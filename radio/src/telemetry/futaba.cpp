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

#define FUTABA_SLOTS 32

struct FutabaSensor {
  const uint16_t id;
  const uint8_t subId;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char* name;
};

// clang-format off
const FutabaSensor futabaSensors[] = {
  {FUTABA_RX_VOLTAGE,  0, UNIT_VOLTS,   1, STR_SENSOR_BATT},
  {FUTABA_RX_LQ,       0, UNIT_PERCENT, 0, STR_SENSOR_RX_QUALITY},
  {FUTABA_RX_BARS,     0, UNIT_RAW,     0, STR_SENSOR_RX_SIGNAL},
  {FUTABA_TX_RSSI,     0, UNIT_DBM,     0, STR_SENSOR_TX_RSSI},
  {FUTABA_SBS_TEMP,    0, UNIT_CELSIUS, 0, STR_SENSOR_TEMP1},
  {FUTABA_SBS_TEMP125, 0, UNIT_CELSIUS, 0, STR_SENSOR_TEMP1},
  {FUTABA_SBS_RPM,     0, UNIT_RPMS,    0, STR_SENSOR_RPM},
  {FUTABA_SBS_VOLT,    0, UNIT_VOLTS,   1, STR_SENSOR_A3},
  {FUTABA_SBS_VOLT,    1, UNIT_VOLTS,   1, STR_SENSOR_A4},
  {FUTABA_SBS_CURR,    0, UNIT_AMPS,    2, STR_SENSOR_CURR},
  {FUTABA_SBS_CURR,    1, UNIT_VOLTS,   2, STR_SENSOR_VFAS},
  {FUTABA_SBS_CURR,    2, UNIT_MAH,     0, STR_SENSOR_CAPACITY},
  {0,                  0, UNIT_RAW,     0, nullptr},
};
// clang-format on

static const FutabaSensor* getFutabaSensor(uint16_t id, uint8_t subId)
{
  for (const FutabaSensor* sensor = futabaSensors; sensor->id; sensor++) {
    if (id == sensor->id && subId == sensor->subId) return sensor;
  }
  return nullptr;
}

static bool isSbsType(uint16_t id)
{
  return id >= FUTABA_SBS_TEMP && id <= FUTABA_SBS_LAST;
}

static uint8_t sbsSlots(uint16_t id)
{
  switch (id) {
    case FUTABA_SBS_VOLT:
      return 2;
    case FUTABA_SBS_CURR:
      return 3;
    default:
      return 1;
  }
}

// The last 2 bytes of each slot (b0 << 8 | b1), and the slots seen: a
// sensor spanning slots from two replies is decoded from both
static uint16_t slotData[FUTABA_SLOTS];
static uint32_t slotSeen, slotActive;

static uint16_t slotBigEndian(uint8_t slot) { return slotData[slot]; }

static uint16_t slotLittleEndian(uint8_t slot)
{
  return slotData[slot] >> 8 | (slotData[slot] & 0xFF) << 8;
}

static void setSbs(uint16_t id, uint8_t subId, uint8_t slot, int32_t value)
{
  const FutabaSensor* sensor = getFutabaSensor(id, subId);
  if (sensor)
    setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, id, subId, slot, value,
                      sensor->unit, sensor->precision);
}

// The values of a sensor of type id from its first slot; formats as the
// sensors send them (the same as the MPM's test radio page)
static void decodeSbs(uint16_t id, uint8_t slot)
{
  switch (id) {
    case FUTABA_SBS_TEMP:
      setSbs(id, 0, slot, (int32_t)(slotLittleEndian(slot) & 0x7FFF) - 100);
      break;
    case FUTABA_SBS_TEMP125:
      setSbs(id, 0, slot, slotBigEndian(slot) & 0x3FFF);
      break;
    case FUTABA_SBS_RPM:
      setSbs(id, 0, slot, (int32_t)slotLittleEndian(slot) * 6);
      break;
    case FUTABA_SBS_VOLT:
      setSbs(id, 0, slot, slotBigEndian(slot) & 0x1FFF);
      setSbs(id, 1, slot, slotBigEndian(slot + 1) & 0x1FFF);
      break;
    case FUTABA_SBS_CURR:
      setSbs(id, 0, slot, slotBigEndian(slot) & 0x3FFF);
      setSbs(id, 1, slot, slotBigEndian(slot + 1));
      setSbs(id, 2, slot, slotBigEndian(slot + 2));
      break;
  }
}

// The slots of this reply (first..last): each S.BUS2 sensor of the model
// whose slots all have data and one of them is in this reply, decoded once
// per first slot (a type with several values: one sensor per value); the
// slots with data no sensor claims, as raw sensors
static void processSlots(uint8_t first, uint8_t last)
{
  uint32_t claimed = 0, decoded = 0;
  uint32_t inReply = (last >= 31 ? 0xFFFFFFFFu : (1u << (last + 1)) - 1) &
                     ~((1u << first) - 1);

  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    const TelemetrySensor& sensor = g_model.telemetrySensors[i];
    if (sensor.type != TELEM_TYPE_CUSTOM || !isSbsType(sensor.id)) continue;
    uint8_t slot = sensor.instance, n = sbsSlots(sensor.id);
    if (slot == 0 || slot + n > FUTABA_SLOTS) continue;
    uint32_t span = ((1u << n) - 1) << slot;
    claimed |= span;
    if ((decoded >> slot & 1) || (span & slotSeen) != span || !(span & inReply))
      continue;
    decoded |= 1u << slot;
    decodeSbs(sensor.id, slot);
  }

  // A raw slot keeps updating once it had data (an empty slot: 0)
  for (uint8_t slot = max<uint8_t>(first, 1); slot <= last; slot++) {
    if (slotData[slot]) slotActive |= 1u << slot;
    if ((claimed >> slot & 1) || !(slotActive >> slot & 1)) continue;
    setTelemetryValue(PROTOCOL_TELEMETRY_FUTABA, FUTABA_SLOT_RAW, 0, slot,
                      slotData[slot], UNIT_RAW, 0);
  }
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
  uint8_t slots = (len - 3) / 2;
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
  if (first == 0 && slots >= 1) {
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

  if (slots > 0 && first < FUTABA_SLOTS) {
    uint8_t last = min<uint8_t>(first + slots - 1, FUTABA_SLOTS - 1);
    for (uint8_t slot = first; slot <= last; slot++) {
      const uint8_t* b = data + 3 + 2 * (slot - first);
      slotData[slot] = b[0] << 8 | b[1];
      slotSeen |= 1u << slot;
    }
    processSlots(first, last);
  }

  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
}

void futabaSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor& telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const FutabaSensor* sensor = getFutabaSensor(id, subId);
  if (sensor) {
    telemetrySensor.init(sensor->name, sensor->unit, sensor->precision);
  } else if (id == FUTABA_SLOT_RAW) {
    char label[TELEM_LABEL_LEN + 1];
    snprintf(label, sizeof(label), "S%u", instance);
    telemetrySensor.init(label, UNIT_RAW, 0);
  } else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}
