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

struct FrSkySportSensor {
  const uint16_t firstId;
  const uint8_t idCnt:6;
  const uint8_t subId:2;
  const uint8_t prec:2;
  const TelemetryUnit unit:6;
  const char * name;
};

// clang-format off
#define FS(firstId,lastId,subId,name,unit,prec) {firstId,lastId-firstId,subId,prec,unit,name}

const FrSkySportSensor sportSensors[] = {
  FS( VALID_FRAME_RATE_ID,        VALID_FRAME_RATE_ID,      0, STR_SENSOR_VFR,                UNIT_PERCENT,     0 ),
  FS( RSSI_ID,                    RSSI_ID,                  0, STR_SENSOR_RSSI,               UNIT_DB,          0 ),
#if defined(MULTIMODULE)
  FS( TX_RSSI_ID,                 TX_RSSI_ID,               0, STR_SENSOR_TX_RSSI,            UNIT_DB ,         0 ),
  FS( TX_LQI_ID,                  TX_LQI_ID,                0, STR_SENSOR_TX_QUALITY,         UNIT_RAW,         0 ),
#endif
  FS( ADC1_ID,                    ADC1_ID,                  0, STR_SENSOR_A1,                 UNIT_VOLTS,       1 ),
  FS( ADC2_ID,                    ADC2_ID,                  0, STR_SENSOR_A2,                 UNIT_VOLTS,       1 ),
  FS( A3_FIRST_ID,                A3_LAST_ID,               0, STR_SENSOR_A3,                 UNIT_VOLTS,       2 ),
  FS( A4_FIRST_ID,                A4_LAST_ID,               0, STR_SENSOR_A4,                 UNIT_VOLTS,       2 ),
  FS( BATT_ID,                    BATT_ID,                  0, STR_SENSOR_BATT,               UNIT_VOLTS,       1 ),
  FS( R9_PWR_ID,                  R9_PWR_ID,                0, STR_SENSOR_R9PW,               UNIT_MILLIWATTS,  0 ),
  FS( T1_FIRST_ID,                T1_LAST_ID,               0, STR_SENSOR_TEMP1,              UNIT_CELSIUS,     0 ),
  FS( T2_FIRST_ID,                T2_LAST_ID,               0, STR_SENSOR_TEMP2,              UNIT_CELSIUS,     0 ),
  FS( RPM_FIRST_ID,               RPM_LAST_ID,              0, STR_SENSOR_RPM,                UNIT_RPMS,        0 ),
  FS( FUEL_FIRST_ID,              FUEL_LAST_ID,             0, STR_SENSOR_FUEL,               UNIT_PERCENT,     0 ),
  FS( ALT_FIRST_ID,               ALT_LAST_ID,              0, STR_SENSOR_ALT,                UNIT_METERS,      2 ),
  FS( VARIO_FIRST_ID,             VARIO_LAST_ID,            0, STR_SENSOR_VSPD,               UNIT_METERS_PER_SECOND, 2 ),
  FS( ACCX_FIRST_ID,              ACCX_LAST_ID,             0, STR_SENSOR_ACCX,               UNIT_G,           3 ),
  FS( ACCY_FIRST_ID,              ACCY_LAST_ID,             0, STR_SENSOR_ACCY,               UNIT_G,           3 ),
  FS( ACCZ_FIRST_ID,              ACCZ_LAST_ID,             0, STR_SENSOR_ACCZ,               UNIT_G,           3 ),
  FS( ANGLE_FIRST_ID,             ANGLE_LAST_ID,            0, STR_SENSOR_ROLL,               UNIT_DEGREE,      2 ),
  FS( ANGLE_FIRST_ID,             ANGLE_LAST_ID,            1, STR_SENSOR_PITCH,              UNIT_DEGREE,      2 ),
  FS( CURR_FIRST_ID,              CURR_LAST_ID,             0, STR_SENSOR_CURR,               UNIT_AMPS,        1 ),
  FS( VFAS_FIRST_ID,              VFAS_LAST_ID,             0, STR_SENSOR_VFAS,               UNIT_VOLTS,       2 ),
  FS( AIR_SPEED_FIRST_ID,         AIR_SPEED_LAST_ID,        0, STR_SENSOR_ASPD,               UNIT_KTS,         1 ),
  FS( GPS_SPEED_FIRST_ID,         GPS_SPEED_LAST_ID,        0, STR_SENSOR_GSPD,               UNIT_KTS,         3 ),
  FS( CELLS_FIRST_ID,             CELLS_LAST_ID,            0, STR_SENSOR_CELLS,              UNIT_CELLS,       2 ),
  FS( GPS_ALT_FIRST_ID,           GPS_ALT_LAST_ID,          0, STR_SENSOR_GPSALT,             UNIT_METERS,      2 ),
  FS( GPS_TIME_DATE_FIRST_ID,     GPS_TIME_DATE_LAST_ID,    0, STR_SENSOR_GPSDATETIME,        UNIT_DATETIME,    0 ),
  FS( GPS_LONG_LATI_FIRST_ID,     GPS_LONG_LATI_LAST_ID,    0, STR_SENSOR_GPS,                UNIT_GPS,         0 ),
  FS( FUEL_QTY_FIRST_ID,          FUEL_QTY_LAST_ID,         0, STR_SENSOR_FUEL,               UNIT_MILLILITERS, 2 ),
  FS( GPS_COURS_FIRST_ID,         GPS_COURS_LAST_ID,        0, STR_SENSOR_HDG,                UNIT_DEGREE,      2 ),
  FS( RBOX_BATT1_FIRST_ID,        RBOX_BATT1_LAST_ID,       0, STR_SENSOR_BATT1_VOLTAGE,      UNIT_VOLTS,       3 ),
  FS( RBOX_BATT2_FIRST_ID,        RBOX_BATT2_LAST_ID,       0, STR_SENSOR_BATT2_VOLTAGE,      UNIT_VOLTS,       3 ),
  FS( RBOX_BATT1_FIRST_ID,        RBOX_BATT1_LAST_ID,       1, STR_SENSOR_BATT1_CURRENT,      UNIT_AMPS,        2 ),
  FS( RBOX_BATT2_FIRST_ID,        RBOX_BATT2_LAST_ID,       1, STR_SENSOR_BATT2_CURRENT,      UNIT_AMPS,        2 ),
  FS( RBOX_CNSP_FIRST_ID,         RBOX_CNSP_LAST_ID,        0, STR_SENSOR_BATT1_CONSUMPTION,  UNIT_MAH,         0 ),
  FS( RBOX_CNSP_FIRST_ID,         RBOX_CNSP_LAST_ID,        1, STR_SENSOR_BATT2_CONSUMPTION,  UNIT_MAH,         0 ),
  FS( RBOX_STATE_FIRST_ID,        RBOX_STATE_LAST_ID,       0, STR_SENSOR_CHANS_STATE,        UNIT_TEXT,        0 ),
  FS( RBOX_STATE_FIRST_ID,        RBOX_STATE_LAST_ID,       1, STR_SENSOR_RB_STATE,           UNIT_TEXT,        0 ),
  FS( SD1_FIRST_ID,               SD1_LAST_ID,              0, STR_SENSOR_SD1_CHANNEL,        UNIT_RAW,         0 ),
  FS( ESC_POWER_FIRST_ID,         ESC_POWER_LAST_ID,        0, STR_SENSOR_ESC_VOLTAGE,        UNIT_VOLTS,       2 ),
  FS( ESC_POWER_FIRST_ID,         ESC_POWER_LAST_ID,        1, STR_SENSOR_ESC_CURRENT,        UNIT_AMPS,        2 ),
  FS( ESC_RPM_CONS_FIRST_ID,      ESC_RPM_CONS_LAST_ID,     0, STR_SENSOR_ESC_RPM,            UNIT_RPMS,        0 ),
  FS( ESC_RPM_CONS_FIRST_ID,      ESC_RPM_CONS_LAST_ID,     1, STR_SENSOR_ESC_CONSUMPTION,    UNIT_MAH,         0 ),
  FS( ESC_TEMPERATURE_FIRST_ID,   ESC_TEMPERATURE_LAST_ID,  0, STR_SENSOR_ESC_TEMP,           UNIT_CELSIUS,     0 ),
  FS( GASSUIT_TEMP1_FIRST_ID,     GASSUIT_TEMP1_LAST_ID,    0, STR_SENSOR_GASSUIT_TEMP1,      UNIT_CELSIUS,     0 ),
  FS( GASSUIT_TEMP2_FIRST_ID,     GASSUIT_TEMP2_LAST_ID,    0, STR_SENSOR_GASSUIT_TEMP2,      UNIT_CELSIUS,     0 ),
  FS( GASSUIT_SPEED_FIRST_ID,     GASSUIT_SPEED_LAST_ID,    0, STR_SENSOR_GASSUIT_RPM,        UNIT_RPMS,        0 ),
  FS( GASSUIT_RES_VOL_FIRST_ID,   GASSUIT_RES_VOL_LAST_ID,  0, STR_SENSOR_GASSUIT_RES_VOL,    UNIT_MILLILITERS, 0 ),
  FS( GASSUIT_RES_PERC_FIRST_ID,  GASSUIT_RES_PERC_LAST_ID, 0, STR_SENSOR_GASSUIT_RES_PERC,   UNIT_PERCENT,     0 ),
  FS( GASSUIT_FLOW_FIRST_ID,      GASSUIT_FLOW_LAST_ID,     0, STR_SENSOR_GASSUIT_FLOW,       UNIT_MILLILITERS_PER_MINUTE, 0 ),
  FS( GASSUIT_MAX_FLOW_FIRST_ID,  GASSUIT_MAX_FLOW_LAST_ID, 0, STR_SENSOR_GASSUIT_MAX_FLOW,   UNIT_MILLILITERS_PER_MINUTE, 0 ),
  FS( GASSUIT_AVG_FLOW_FIRST_ID,  GASSUIT_AVG_FLOW_LAST_ID, 0, STR_SENSOR_GASSUIT_AVG_FLOW,   UNIT_MILLILITERS_PER_MINUTE, 0 ),
  FS( SBEC_POWER_FIRST_ID,        SBEC_POWER_LAST_ID,       0, STR_SENSOR_SBEC_VOLTAGE,       UNIT_VOLTS,       2 ),
  FS( SBEC_POWER_FIRST_ID,        SBEC_POWER_LAST_ID,       1, STR_SENSOR_SBEC_CURRENT,       UNIT_AMPS,        2 ),
  FS( RB3040_OUTPUT_FIRST_ID,     RB3040_OUTPUT_LAST_ID,    0, STR_SENSOR_RB3040_EXTRA_STATE, UNIT_TEXT,        0 ),
  FS( RB3040_CH1_2_FIRST_ID,      RB3040_CH1_2_LAST_ID,     0, STR_SENSOR_RB3040_CHANNEL1,    UNIT_AMPS,        2 ),
  FS( RB3040_CH1_2_FIRST_ID,      RB3040_CH1_2_LAST_ID,     1, STR_SENSOR_RB3040_CHANNEL2,    UNIT_AMPS,        2 ),
  FS( RB3040_CH3_4_FIRST_ID,      RB3040_CH3_4_LAST_ID,     0, STR_SENSOR_RB3040_CHANNEL3,    UNIT_AMPS,        2 ),
  FS( RB3040_CH3_4_FIRST_ID,      RB3040_CH3_4_LAST_ID,     1, STR_SENSOR_RB3040_CHANNEL4,    UNIT_AMPS,        2 ),
  FS( RB3040_CH5_6_FIRST_ID,      RB3040_CH5_6_LAST_ID,     0, STR_SENSOR_RB3040_CHANNEL5,    UNIT_AMPS,        2 ),
  FS( RB3040_CH5_6_FIRST_ID,      RB3040_CH5_6_LAST_ID,     1, STR_SENSOR_RB3040_CHANNEL6,    UNIT_AMPS,        2 ),
  FS( RB3040_CH7_8_FIRST_ID,      RB3040_CH7_8_LAST_ID,     0, STR_SENSOR_RB3040_CHANNEL7,    UNIT_AMPS,        2 ),
  FS( RB3040_CH7_8_FIRST_ID,      RB3040_CH7_8_LAST_ID,     1, STR_SENSOR_RB3040_CHANNEL8,    UNIT_AMPS,        2 ),
  FS( SERVO_FIRST_ID,             SERVO_LAST_ID,            0, STR_SENSOR_SERVO_CURRENT,      UNIT_AMPS,        1 ),
  FS( SERVO_FIRST_ID,             SERVO_LAST_ID,            1, STR_SENSOR_SERVO_VOLTAGE,      UNIT_VOLTS,       1 ),
  FS( SERVO_FIRST_ID,             SERVO_LAST_ID,            2, STR_SENSOR_SERVO_TEMPERATURE,  UNIT_CELSIUS,     0 ),
  FS( SERVO_FIRST_ID,             SERVO_LAST_ID,            3, STR_SENSOR_SERVO_STATUS,       UNIT_TEXT,        0 ),
  FS( 0,                          0,                        0, nullptr,                       UNIT_RAW,         0 ) // sentinel
};
// clang-format on

const FrSkySportSensor * getFrSkySportSensor(uint16_t id, uint8_t subId=0)
{
  for (const FrSkySportSensor * sensor = sportSensors; sensor->firstId; sensor++) {
    if (id >= sensor->firstId && id <= (sensor->firstId + sensor->idCnt) && subId == sensor->subId) {
      return sensor;
    }
  }
  return nullptr;
}

bool checkSportPacket(const uint8_t * packet)
{
  short crc = 0;
  for (int i = 1; i < FRSKY_SPORT_PACKET_SIZE; ++i) {
    crc += packet[i]; // 0-1FE
    crc += crc >> 8u;  // 0-1FF
    crc &= 0x00FFu;    // 0-FF
  }
  // TRACE("crc: 0x%02x", crc);
  return crc == 0x00FF;
}

#define SPORT_DATA_U8(packet)   (packet[4])
#define SPORT_DATA_S32(packet)  (*((int32_t *)(packet+4)))
#define SPORT_DATA_U32(packet)  (*((uint32_t *)(packet+4)))
#define HUB_DATA_U16(packet)    (*((uint16_t *)(packet+4)))

uint16_t servosState;
uint16_t rboxState;

void sportProcessTelemetryPacket(uint16_t id, uint8_t subId, uint8_t instance,
                                 uint32_t data, TelemetryUnit unit)
{
  const TelemetryProtocol proto = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  const FrSkySportSensor * sensor = getFrSkySportSensor(id, subId);
  uint8_t precision = 255;
  if (sensor) {
    if (unit == UNIT_RAW) {
      unit = sensor->unit;
    }
    precision = sensor->prec;
  }
  if (unit == UNIT_CELLS) {
    uint8_t cellsCount = (data & 0xF0u) >> 4u;
    uint8_t cellIndex = (data & 0x0Fu);
    if (cellIndex >= MAX_CELLS) return;
    uint32_t mask = (cellsCount << 24u) + (cellIndex << 16u);
    uint32_t value = mask + (((data & 0x000FFF00u) >> 8u) / 5);
    setTelemetryValue(proto, id, subId, instance, value, unit, precision);
    if (cellIndex + 1 < cellsCount) {
      mask += (1 << 16u);
      value = mask + (((data & 0xFFF00000u) >> 20u) / 5);
      setTelemetryValue(proto, id, subId, instance, value, unit, precision);
    }
  }
  else {
    setTelemetryValue(proto, id, subId, instance, data, unit, precision);
  }
}

bool sportProcessTelemetryPacket(uint8_t module, const uint8_t * packet, uint8_t len)
{
  if (!checkSportPacket(packet)) {
    TRACE("sportProcessTelemetryPacket(): checksum error ");
    DUMP(packet, FRSKY_SPORT_PACKET_SIZE);
    return false;
  }

  sportProcessTelemetryPacketWithoutCrc(module, 0, packet);
  return true;
}

static void sportSetServoState(uint16_t id, uint8_t subId, uint8_t instance, uint16_t value)
{
  const TelemetryProtocol proto = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  if (value == 0) {
    setTelemetryText(proto, id, subId, instance, "OK");
  }
  else {
    for (uint8_t i = 0; i < 16; i++) {
      if (value & (1u << i)) {
        char s[] = "CH__ KO";
        strAppendUnsigned(&s[2], i + 1, 2);
        setTelemetryText(proto, id, subId, instance, s);
        break;
      }
    }
  }
}

static void sportSetRBoxState(uint16_t id, uint8_t subId, uint8_t instance, uint16_t value)
{
  const TelemetryProtocol proto = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  if (value == 0) {
    setTelemetryText(proto, id, subId, instance, "Rx OK");
  }
  else {
    static const char * const RXS_STATUS[] = {
      "Rx1 Ovl",
      "Rx2 Ovl",
      "SBUS Ovl",
      "Rx1 FS",
      "Rx1 LF",
      "Rx2 FS",
      "Rx2 LF",
      "Rx1 Lost",
      "Rx2 Lost",
      "Rx1 NS",
      "Rx2 NS",
      "Rx3 FS",
      "Rx3 LF",
      "Rx3 Lost",
      "Rx3 NS"
    };
    for (uint8_t i = 0; i < DIM(RXS_STATUS); i++) {
      if (value & (1u << i)) {
        setTelemetryText(proto, id, subId, instance, RXS_STATUS[i]);
        break;
      }
    }
  }
}

static void sportSetRB3040State(uint16_t id, uint8_t subId, uint8_t instance, uint32_t value)
{
  const TelemetryProtocol proto = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  if (value == 0) {
    setTelemetryText(proto, id, subId, instance, "OK");
  }
  else {
    for (uint8_t i = 0; i < 9; i++) {
      if (value & (1u << i)) {
        if (i < 8) {
          char s[] = "CH__ KO";
          strAppendUnsigned(&s[2], i + 17, 2);
          setTelemetryText(proto, id, subId, instance, s);
          break;
        } else {
          setTelemetryText(proto, id, subId, instance, "S.P Ovl");
          break;
        }
      }
    }
  }
}

void sportProcessTelemetryPacketWithoutCrc(uint8_t module, uint8_t origin, const uint8_t * packet)
{
  uint8_t physicalId = packet[0] & 0x1F;
  uint8_t primId = packet[1];
  uint16_t dataId = *((uint16_t *)(packet+2));
  uint32_t data = SPORT_DATA_S32(packet);

#if defined(BLUETOOTH)
  if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
      bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
    bluetooth.forwardTelemetry(packet);
  }
#endif

  if (primId == DATA_FRAME) {
    uint8_t originMask = 0x01 << module;
    uint8_t instance = physicalId + (origin << 5);
    if (dataId == RSSI_ID) {
      data = SPORT_DATA_U8(packet);
      if (data > 0) {
        telemetryStreaming = TELEMETRY_TIMEOUT10ms; // reset counter only if valid packets are being detected
        telemetryData.telemetryValid |= originMask;
      }
      else {
        telemetryData.telemetryValid &= ~originMask;
        // one module may send RSSI(0) while the other is still streaming
        // in this case we don't want to update telemetryData.rssi
        return;
      }
      
      telemetryData.rssi.set(data);
    }
    else if (dataId == VALID_FRAME_RATE_ID) {
      data = 100 - SPORT_DATA_U8(packet);
    }
    else if (dataId == R9_PWR_ID) {
      // convert 'dBm' to 'mW'
      static const uint16_t r9pwrs[][2] = {{0, 1},    {5, 3},    {10, 10},
                                           {13, 20},  {14, 25},  {20, 100},
                                           {23, 200}, {27, 500}, {30, 1000}};
      uint32_t dBm = SPORT_DATA_U8(packet);
      for (auto & r9pwr : r9pwrs) {
        if (dBm == r9pwr[0]) {
          data = r9pwr[1];
          break;
        }
      }
    }
    else if (dataId == XJT_VERSION_ID) {
      telemetryData.xjtVersion = HUB_DATA_U16(packet);
      if (!isRasValueValid()) {
        telemetryData.setSwr(module, 0);
      }
    }
    else if (dataId == RAS_ID) {
      if (isRasValueValid()) {
        telemetryData.setSwr(module, SPORT_DATA_U8(packet));
      }
    }

    // here we discard the frame if it comes from an origin which has RSSI = 0
    // (RxBt and RSSI are sent in a loop by the module in some situations)
    if (TELEMETRY_STREAMING() &&
        // because when Rx is OFF it happens that some
        // old A1/A2 values are sent from the XJT module
        (telemetryData.telemetryValid & originMask)) {
      if ((dataId >> 8) == 0) {
        // The old FrSky IDs
        processHubPacket(dataId, HUB_DATA_U16(packet));
      } else if (!IS_HIDDEN_TELEMETRY_VALUE(dataId)) {
        if (dataId == ADC1_ID || dataId == ADC2_ID || dataId == BATT_ID || dataId == RAS_ID) {
          data = SPORT_DATA_U8(packet);
        }

        if (dataId >= GPS_LONG_LATI_FIRST_ID && dataId <= GPS_LONG_LATI_LAST_ID) {
          int32_t value = (data & 0x3fffffff);
          if (data & (1 << 30u))
            value = -value;
          value = (value * 5) / 3; // min/10000 => deg/1000000
          if (data & (1 << 31u))
            sportProcessTelemetryPacket(dataId, 0, instance, value, UNIT_GPS_LONGITUDE);
          else
            sportProcessTelemetryPacket(dataId, 0, instance, value, UNIT_GPS_LATITUDE);
        }
        else if (dataId >= RBOX_BATT1_FIRST_ID && dataId <= RBOX_BATT2_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0xFFFFu);
          sportProcessTelemetryPacket(dataId, 1, instance, data >> 16u);
        }
        else if (dataId >= RBOX_CNSP_FIRST_ID && dataId <= RBOX_CNSP_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0xFFFFu);
          sportProcessTelemetryPacket(dataId, 1, instance, data >> 16u);
        }
        else if (dataId >= RBOX_STATE_FIRST_ID && dataId <= RBOX_STATE_LAST_ID) {
          bool static isRB10 = false;
          uint16_t newServosState;

          if (servosState == 0 && (data & 0xFF00u) == 0xFF00u) {
            isRB10 = true;
          }
          if (isRB10) {
            newServosState = data & 0x00FFu; // 8ch only RB10
          }
          else {
            newServosState = data & 0xFFFFu;
          }
          if (newServosState != 0 && servosState == 0) {
            audioEvent(AU_SERVO_KO);
          }
          uint16_t newRboxState = data >> 16u;
          if ((newRboxState & 0x07u) && (rboxState & 0x07u) == 0) {
            audioEvent(AU_RX_OVERLOAD);
          }
          servosState = newServosState;
          rboxState = newRboxState;
          sportSetServoState(dataId, 0, instance, servosState);
          sportSetRBoxState(dataId, 1, instance, rboxState);
        }
        else if (dataId >= RB3040_OUTPUT_FIRST_ID && dataId <= RB3040_OUTPUT_LAST_ID) {
          sportSetRB3040State(dataId, 0, instance, data);
        }
        else if (dataId >= ESC_POWER_FIRST_ID && dataId <= ESC_POWER_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0xFFFFu);
          sportProcessTelemetryPacket(dataId, 1, instance, data >> 16u);
        }
        else if (dataId >= ESC_RPM_CONS_FIRST_ID && dataId <= ESC_RPM_CONS_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, 100 * (data & 0xFFFFu));
          sportProcessTelemetryPacket(dataId, 1, instance, data >> 16u);
        }
        else if (dataId >= ESC_TEMPERATURE_FIRST_ID && dataId <= ESC_TEMPERATURE_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0x00FFu);
        }
        else if (dataId >= SBEC_POWER_FIRST_ID && dataId <= SBEC_POWER_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, (data & 0xFFFFu) / 10);
          sportProcessTelemetryPacket(dataId, 1, instance, (data >> 16u) / 10);
        }
        else if (dataId >= DIY_STREAM_FIRST_ID && dataId <= DIY_STREAM_LAST_ID) {
#if defined(LUA)
          if (luaInputTelemetryFifo &&
              luaInputTelemetryFifo->hasSpace(sizeof(SportTelemetryPacket))) {

            SportTelemetryPacket luaPacket;
            luaPacket.physicalId = physicalId;
            luaPacket.primId = primId;
            luaPacket.dataId = dataId;
            luaPacket.value = data;
            for (uint8_t i=0; i<sizeof(SportTelemetryPacket); i++) {
              luaInputTelemetryFifo->push(luaPacket.raw[i]);
            }
          }
#endif
        }
        else if (dataId >= RB3040_CH1_2_FIRST_ID && dataId <= RB3040_CH7_8_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0xFFFFu);
          sportProcessTelemetryPacket(dataId, 1, instance, (data >> 16u) & 0xFFFFu);
        }
        else if (dataId >= SERVO_FIRST_ID && dataId <= SERVO_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, data & 0xFFu);
          sportProcessTelemetryPacket(dataId, 1, instance, (data >> 8u) & 0xFFu);
          sportProcessTelemetryPacket(dataId, 2, instance, (data >> 16u) & 0xFFu);
          uint8_t newServosState = data >> 24u;
          setTelemetryText(PROTOCOL_TELEMETRY_FRSKY_SPORT, dataId, 3, instance,
                           newServosState ? "STALL" : "OK");
          if (newServosState != 0 && servosState == 0) {
            audioEvent(AU_SERVO_KO);
            servosState = newServosState;
          }
        }
        else if (dataId >= ANGLE_FIRST_ID && dataId <= ANGLE_LAST_ID) {
          sportProcessTelemetryPacket(dataId, 0, instance, (int16_t) (data & 0xFFFFu));
          sportProcessTelemetryPacket(dataId, 1, instance, (int16_t) (data >> 16u));
        }
        else {
          sportProcessTelemetryPacket(dataId, 0, instance, data);
        }
      }
    }
  }
#if defined(LUA)
  else if (primId == 0x32) {
    if (luaInputTelemetryFifo && luaInputTelemetryFifo->hasSpace(sizeof(SportTelemetryPacket))) {
      SportTelemetryPacket luaPacket;
      luaPacket.physicalId = physicalId;
      luaPacket.primId = primId;
      luaPacket.dataId = dataId;
      luaPacket.value = data;
      for (uint8_t i=0; i<sizeof(SportTelemetryPacket); i++) {
        luaInputTelemetryFifo->push(luaPacket.raw[i]);
      }
    }
  }
#endif
}

void frskySportSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const FrSkySportSensor * sensor = getFrSkySportSensor(id, subId);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    uint8_t prec = min<uint8_t>(2, sensor->prec);
    telemetrySensor.init(sensor->name, unit, prec);
    if (id >= ADC1_ID && id <= BATT_ID) {
      telemetrySensor.custom.ratio = 132;
      telemetrySensor.filter = 1;
    }
    else if (id >= CURR_FIRST_ID && id <= CURR_LAST_ID) {
      telemetrySensor.onlyPositive = 1;
    }
    else if (id >= ALT_FIRST_ID && id <= ALT_LAST_ID) {
      telemetrySensor.autoOffset = 1;
    }
    if (unit == UNIT_RPMS) {
      telemetrySensor.custom.ratio = 1;
      telemetrySensor.custom.offset = 1;
    }
    else if (unit == UNIT_METERS) {
      if (IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_FEET;
      }
    }
    else if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE) {
      telemetrySensor.unit = UNIT_GPS;
    }
  }
  else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}
