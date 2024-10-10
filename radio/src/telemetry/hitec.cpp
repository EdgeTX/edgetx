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

/* Full telemetry 
packet[0] = TX RSSI value
packet[1] = TX LQI value
packet[2] = frame number
packet[3-7] telemetry data

The frame number takes the following values: 0x00, 0x11, 0x12, ..., 0x1C. The frames can be present or not, they also do not have to follow each others.
Here is a description of the telemetry data for each frame number:
- frame 0x00
data byte 0 -> 0x00				unknown
data byte 1 -> 0x00				unknown
data byte 2 -> 0x00				unknown
data byte 3 -> RX Batt Volt_H
data byte 4 -> RX Batt Volt_L => RX Batt=(Volt_H*256+Volt_L)/28
- frame 0x11
data byte 0 -> 0xAF				start of frame
data byte 1 -> 0x00				unknown
data byte 2 -> 0x2D				station type 0x2D=standard station nitro or electric, 0xAC=advanced station
data byte 3 -> RX Batt Volt_H
data byte 4 -> RX Batt Volt_L => RX Batt=(Volt_H*256+Volt_L)/28
- frame 0x12
data byte 0 -> Lat_sec_H		GPS : latitude second
data byte 1 -> Lat_sec_L		signed int : 1/100 of second
data byte 2 -> Lat_deg_min_H	GPS : latitude degree.minute
data byte 3 -> Lat_deg_min_L	signed int : +=North, - = south
data byte 4 -> Time_second		GPS Time
- frame 0x13
data byte 0 -> 					GPS Longitude second
data byte 1 -> 					signed int : 1/100 of second
data byte 2 -> 					GPS Longitude degree.minute
data byte 3 -> 					signed int : +=Est, - = west
data byte 4 -> Temp2			Temperature2=Temp2-40°C
- frame 0x14
data byte 0 -> Speed_H
data byte 1 -> Speed_L			GPS Speed=Speed_H*256+Speed_L km/h
data byte 2 -> Alti_sea_H
data byte 3 -> Alti_sea_L		GPS Altitude=Alti_sea_H*256+Alti_sea_L m
data byte 4 -> Temp1			Temperature1=Temp1-40°C
- frame 0x15
data byte 0 -> FUEL
data byte 1 -> RPM1_L
data byte 2 -> RPM1_H			RPM1=RPM1_H*256+RPM1_L
data byte 3 -> RPM2_L
data byte 4 -> RPM2_H			RPM2=RPM2_H*256+RPM2_L
- frame 0x16
data byte 0 -> Date_year		GPS Date
data byte 1 -> Date_month
data byte 2 -> Date_day
data byte 3 -> Time_hour		GPS Time
data byte 4 -> Time_min
- frame 0x17
data byte 0 -> COURSEH
data byte 1 -> COURSEL			GPS heading = COURSEH*256+COURSEL in degrees
data byte 2 -> Count			GPS satellites
data byte 3 -> Temp3			Temperature3=Temp2-40°C
data byte 4 -> Temp4			Temperature4=Temp3-40°C
- frame 0x18
data byte 0 -> Volt_L			Volt=(Volt_H*256+Volt_L)/10 V
data byte 1 -> Volt_H
data byte 2 -> AMP_L
data byte 3 -> AMP_H			Amp=(AMP1_*256+AMP_L -180)/14 in signed A
- frame 0x19					Servo sensor
data byte 0 -> AMP_Servo1		Amp=AMP_Servo1/10 in A
data byte 1 -> AMP_Servo2		Amp=AMP_Servo2/10 in A
data byte 2 -> AMP_Servo3		Amp=AMP_Servo3/10 in A
data byte 3 -> AMP_Servo4		Amp=AMP_Servo4/10 in A
- frame 0x1A
data byte 2 -> ASpeed_H			Air speed=ASpeed_H*256+ASpeed_L km/h
data byte 3 -> ASpeed_L
- frame 0x1B					Variometer sensor
data byte 0 -> Alti1H
data byte 1 -> Alti1L			Altitude unfiltered
data byte 2 -> Alti2H
data byte 3 -> Alti2L			Altitude filtered
- frame 0x1C					Unknown
- frame 0x22					Unknown
*/

#define HITEC_TELEMETRY_LENGTH 8

struct HitecSensor
{
  const uint16_t id;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

// telemetry frames
enum
{
  HITEC_FRAME_00 = 0x00,
  HITEC_FRAME_11 = 0x11,
  HITEC_FRAME_12 = 0x12,
  HITEC_FRAME_13 = 0x13,
  HITEC_FRAME_14 = 0x14,
  HITEC_FRAME_15 = 0x15,
  HITEC_FRAME_16 = 0x16,
  HITEC_FRAME_17 = 0x17,
  HITEC_FRAME_18 = 0x18,
  HITEC_FRAME_19 = 0x19,
  HITEC_FRAME_1A = 0x1A,
  HITEC_FRAME_1B = 0x1B,
  HITEC_FRAME_1C = 0x1C,
  HITEC_FRAME_22 = 0x22,
};

// telemetry sensors ID
enum
{
  HITEC_ID_RX_VOLTAGE = 0x0003,    // RX_Batt Voltage
  HITEC_ID_GPS_LAT_LONG = 0x1200,    // GPS latitude longitude
  HITEC_ID_TEMP2 = 0x1304,    // Temperature sensor 2
  HITEC_ID_GPS_SPEED = 0x1400,    // GPS speed
  HITEC_ID_GPS_ALTITUDE = 0x1402,    // GPS altitude sea level
  HITEC_ID_TEMP1 = 0x1404,    // Temperature sensor 1
  HITEC_ID_FUEL = 0x1500,    // Fuel
  HITEC_ID_RPM1 = 0x1501,    // RPM1
  HITEC_ID_RPM2 = 0x1503,    // RPM2
  HITEC_ID_GPS_DATETIME = 0x1600,    // GPS date time
  HITEC_ID_GPS_HEADING = 0x1700,    // GPS heading
  HITEC_ID_GPS_COUNT = 0x1702,    // GPS count
  HITEC_ID_TEMP3 = 0x1703,    // Temperature sensor 3
  HITEC_ID_TEMP4 = 0x1704,    // Temperature sensor 4
  HITEC_ID_VOLTAGE = 0x1800,    // Voltage sensor
  HITEC_ID_AMP = 0x1802,    // Amp sensor
  HITEC_ID_C50 = 0x1803,    // Amp sensor C50
  HITEC_ID_C200 = 0x1804,    // Amp sensor C200
  HITEC_ID_AMP_S1 = 0x1900,    // Amp servo 1 sensor
  HITEC_ID_AMP_S2 = 0x1901,    // Amp servo 2 sensor
  HITEC_ID_AMP_S3 = 0x1902,    // Amp servo 3 sensor
  HITEC_ID_AMP_S4 = 0x1903,    // Amp servo 4 sensor
  HITEC_ID_AIR_SPEED = 0x1A02,    // Air speed
  HITEC_ID_VARIO = 0x1B00,    // Vario
  HITEC_ID_ALT = 0x1B02,    // Vario
  HITEC_ID_TX_RSSI = 0xFF00,    // Pseudo id outside 1 byte range of Hitec sensors
  HITEC_ID_TX_LQI = 0xFF01,    // Pseudo id outside 1 byte range of Hitec sensors
};

// clang-format off
#define HS(id,name,unit,precision) {id,unit,precision,name}

const HitecSensor hitecSensors[] = {
  //frame 00
  HS(HITEC_ID_RX_VOLTAGE,   STR_SENSOR_BATT,       UNIT_VOLTS,             2),  // RX_Batt Voltage
  //frame 11
  //frame 12
  HS(HITEC_ID_GPS_LAT_LONG, STR_SENSOR_GPS,        UNIT_GPS,               0),  // GPS position
  //frame 13
  HS(HITEC_ID_TEMP2,        STR_SENSOR_TEMP2,      UNIT_CELSIUS,           0),  // Temperature sensor 2
  //frame 14
  HS(HITEC_ID_GPS_SPEED,    STR_SENSOR_GSPD,       UNIT_KMH,               0),  // GPS speed
  HS(HITEC_ID_GPS_ALTITUDE, STR_SENSOR_GPSALT,     UNIT_METERS,            0),  // GPS altitude sea level
  HS(HITEC_ID_TEMP1,        STR_SENSOR_TEMP1,      UNIT_CELSIUS,           0),  // Temperature sensor 1
  //frame 15
  HS(HITEC_ID_FUEL,         STR_SENSOR_FUEL,       UNIT_PERCENT,           0),  // Fuel
  HS(HITEC_ID_RPM1,         STR_SENSOR_RPM,        UNIT_RPMS,              0),  // RPM1
  HS(HITEC_ID_RPM2,         STR_SENSOR_RPM2,       UNIT_RPMS,              0),  // RPM2
  //frame 16
  HS(HITEC_ID_GPS_DATETIME, STR_SENSOR_GPS,        UNIT_DATETIME,          0),  // GPS date time
  //frame 17
  HS(HITEC_ID_GPS_HEADING,  STR_SENSOR_HDG,        UNIT_DEGREE,            0),  // GPS Heading
  HS(HITEC_ID_GPS_COUNT,    STR_SENSOR_SATELLITES, UNIT_RAW,               0),  // GPS count
  HS(HITEC_ID_TEMP3,        STR_SENSOR_TEMP3,      UNIT_CELSIUS,           0),  // Temperature sensor 3
  HS(HITEC_ID_TEMP4,        STR_SENSOR_TEMP4,      UNIT_CELSIUS,           0),  // Temperature sensor 4
  //frame 18
  HS(HITEC_ID_VOLTAGE,      STR_SENSOR_A1,         UNIT_VOLTS,             1),  // Voltage sensor
  HS(HITEC_ID_AMP,          STR_SENSOR_CURR,       UNIT_AMPS,              0),  // Amp sensor
  HS(HITEC_ID_C50,          STR_SENSOR_C50,        UNIT_AMPS,              1),  // Amp sensor C50
  HS(HITEC_ID_C200,         STR_SENSOR_C200,       UNIT_AMPS,              0),  // Amp sensor C200
  //frame 19
  HS(HITEC_ID_AMP_S1, STR_SENSOR_CURR_SERVO1,      UNIT_AMPS,              1),  // Amp sensor
  HS(HITEC_ID_AMP_S2, STR_SENSOR_CURR_SERVO2,      UNIT_AMPS,              1),  // Amp sensor
  HS(HITEC_ID_AMP_S3, STR_SENSOR_CURR_SERVO3,      UNIT_AMPS,              1),  // Amp sensor
  HS(HITEC_ID_AMP_S4, STR_SENSOR_CURR_SERVO4,      UNIT_AMPS,              1),  // Amp sensor
  //frame 1A
  HS(HITEC_ID_AIR_SPEED,    STR_SENSOR_ASPD,       UNIT_KMH,               0),  // Air speed
  //frame 1B
  HS(HITEC_ID_VARIO,        STR_SENSOR_VSPD,       UNIT_METERS_PER_SECOND, 1),  // Vario
  HS(HITEC_ID_ALT,          STR_SENSOR_ALT,        UNIT_METERS,            1),  // Altitude

  HS(HITEC_ID_TX_RSSI,      STR_SENSOR_TX_RSSI,    UNIT_RAW,               0),  // Pseudo id outside 1 byte range of Hitec sensors
  HS(HITEC_ID_TX_LQI,       STR_SENSOR_TX_QUALITY, UNIT_RAW,               0),  // Pseudo id outside 1 byte range of Hitec sensors// Pseudo sensor for TLQI
  HS(0x00,                  NULL,                  UNIT_RAW,               0),  // sentinel
};
// clang-format on

const HitecSensor * getHitecSensor(uint16_t id)
{
  for (const HitecSensor * sensor = hitecSensors; sensor->id; sensor++) {
    if (id == sensor->id)
      return sensor;
  }
  return nullptr;
}

void processHitecPacket(const uint8_t * packet)
{
  static uint16_t rssi = 0, lqi = 0;
  // Set TX RSSI Value, reverse MULTIs scaling
  rssi = ((packet[0] * 10) + (rssi * 90)) / 100; // quick filtering
  setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TX_RSSI, 0, 0, rssi >> 1, UNIT_RAW, 0);
  telemetryData.rssi.set(rssi >> 1);
  if (packet[0] > 0) telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  // Set TX LQI  Value, reverse MULTIs scaling
  lqi = ((packet[1] * 10) + (lqi * 90)) / 100; // quick filtering
  setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TX_LQI, 0, 0, lqi, UNIT_RAW, 0);

  const HitecSensor * sensor;
  int32_t value, deg, min, sec, alt, amp;
  static uint8_t second = 0;
  static int32_t last_alt = 0;
  static uint16_t last_ms = 0;
  uint16_t current_ms;

  switch (packet[2]) {
    case HITEC_FRAME_00:
      value = (((packet[6] << 8) | packet[7]) * 100) / 28;
      sensor = getHitecSensor(HITEC_ID_RX_VOLTAGE);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_RX_VOLTAGE, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_11:
      value = (((packet[6] << 8) | packet[7]) * 100) / 28;
      sensor = getHitecSensor(HITEC_ID_RX_VOLTAGE);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_RX_VOLTAGE, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_12:
      //value=(packet[5]<<24)|(packet[6]<<16)|(packet[3]<<8)|packet[4];
      min = (int16_t) ((packet[5] << 8) | packet[6]);
      deg = min / 100;
      min = min - deg * 100;
      sec = (int16_t) ((packet[3] << 8) | packet[4]);
      value = deg * 1000000 + (min * 150000 + sec * 25) / 9;
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_LAT_LONG, 0, 0, value, UNIT_GPS_LATITUDE, 0);
      second = packet[7];
      return;
    case HITEC_FRAME_13:
      //value=(packet[5]<<24)|(packet[6]<<16)|(packet[3]<<8)|packet[4];
      min = (int16_t) ((packet[5] << 8) | packet[6]);
      deg = min / 100;
      min = min - deg * 100;
      sec = (int16_t) ((packet[3] << 8) | packet[4]);
      value = deg * 1000000 + (min * 150000 + sec * 25) / 9;
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_LAT_LONG, 0, 0, value, UNIT_GPS_LONGITUDE, 0);
      value = packet[7] - 40;
      sensor = getHitecSensor(HITEC_ID_TEMP2);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TEMP2, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_14:
      value = (packet[3] << 8) | packet[4];
      sensor = getHitecSensor(HITEC_ID_GPS_SPEED);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_SPEED, 0, 0, value, sensor->unit, sensor->precision);
      value = (packet[5] << 8) | packet[6];
      sensor = getHitecSensor(HITEC_ID_GPS_ALTITUDE);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_ALTITUDE, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[7] - 40;
      sensor = getHitecSensor(HITEC_ID_TEMP1);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TEMP1, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_15:
      value = packet[3] * 25;
      if (value > 100) value = 100;
      sensor = getHitecSensor(HITEC_ID_FUEL);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_FUEL, 0, 0, value, sensor->unit, sensor->precision);
      value = (packet[5] << 8) | packet[4];
      sensor = getHitecSensor(HITEC_ID_RPM1);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_RPM1, 0, 0, value, sensor->unit, sensor->precision);
      value = (packet[7] << 8) | packet[6];
      sensor = getHitecSensor(HITEC_ID_RPM2);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_RPM2, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_16:
      sensor = getHitecSensor(HITEC_ID_GPS_DATETIME);
      value = (packet[3] << 24) | (packet[4] << 16) | (packet[5] << 8) | 0x01;
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_DATETIME, 0, 0, value, sensor->unit, sensor->precision);
      value = (packet[6] << 24) | (packet[7] << 16) | (second << 8);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_DATETIME, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_17:
      value = (packet[3] << 8) | packet[4];
      if (value <= 359) { // Filter strange values received time to time
        sensor = getHitecSensor(HITEC_ID_GPS_HEADING);
        setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_HEADING, 0, 0, value, sensor->unit, sensor->precision);
      }
      value = packet[5];
      sensor = getHitecSensor(HITEC_ID_GPS_COUNT);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_GPS_COUNT, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[6] - 40;
      sensor = getHitecSensor(HITEC_ID_TEMP3);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TEMP3, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[7] - 40;
      sensor = getHitecSensor(HITEC_ID_TEMP4);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_TEMP4, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_18:
      value = (packet[4] << 8) | packet[3];
      if (value) value += 2; // Measured voltage seems to be 0.2V lower than real
      sensor = getHitecSensor(HITEC_ID_VOLTAGE);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_VOLTAGE, 0, 0, value, sensor->unit, sensor->precision);
      //I'm adding below 3 amp sensors but there is only one really since I don't know how to really calculate them
      value = (int16_t) ((packet[6] << 8) | packet[5]);
      sensor = getHitecSensor(HITEC_ID_AMP);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AMP, 0, 0, value, sensor->unit, sensor->precision);
      amp = ((value + 114.875) * 1.441) + 0.5;
      sensor = getHitecSensor(HITEC_ID_C50);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_C50, 0, 0, amp, sensor->unit, sensor->precision);
      amp = value * 3 + 165;
      sensor = getHitecSensor(HITEC_ID_C200);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_C200, 0, 0, amp, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_19:
      value = packet[3];
      sensor = getHitecSensor(HITEC_ID_AMP_S1);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AMP_S1, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[4];
      sensor = getHitecSensor(HITEC_ID_AMP_S2);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AMP_S2, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[5];
      sensor = getHitecSensor(HITEC_ID_AMP_S3);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AMP_S3, 0, 0, value, sensor->unit, sensor->precision);
      value = packet[6];
      sensor = getHitecSensor(HITEC_ID_AMP_S4);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AMP_S4, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_1A:
      value = (packet[5] << 8) | packet[6];
      sensor = getHitecSensor(HITEC_ID_AIR_SPEED);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_AIR_SPEED, 0, 0, value, sensor->unit, sensor->precision);
      return;
    case HITEC_FRAME_1B:
      alt = (int16_t) ((packet[3] << 8) | packet[4]);
      sensor = getHitecSensor(HITEC_ID_ALT);
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_ALT, 0, 0, alt, sensor->unit, sensor->precision);
      current_ms = RTOS_GET_MS();
      sensor = getHitecSensor(HITEC_ID_VARIO);
      value = (alt - last_alt) * 100;
      if ((current_ms - last_ms) < 1000)
        value /= (int32_t) (current_ms - last_ms);
      else
        value = 0;
      setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, HITEC_ID_VARIO, 0, 0, value, sensor->unit, sensor->precision);
      last_alt = alt;
      last_ms = current_ms;
      return;
    case HITEC_FRAME_1C:
    case HITEC_FRAME_22:
      return;
  }
  //unknown
  value = (packet[6] << 24) | (packet[5] << 16) | (packet[4] << 8) | packet[3];
  setTelemetryValue(PROTOCOL_TELEMETRY_HITEC, packet[2], 0, 0, value, UNIT_RAW, 0);
}

void processHitecTelemetryData(uint8_t data, uint8_t * rxBuffer, uint8_t &rxBufferCount)
{
  if (rxBufferCount == 0)
    return;

  if (data != 0xAA) {
    TRACE("[HITEC] invalid start byte 0x%02X", data);
    rxBufferCount = 0;
    return;
  }

  if (rxBuffer[3] == HITEC_FRAME_00 || (rxBuffer[3] >= HITEC_FRAME_11 && rxBuffer[3] <= HITEC_FRAME_1C) || rxBuffer[3] == HITEC_FRAME_22) {
    TRACE("[HITEC] Frame 0x%02X", rxBuffer[3]);
  }
  else {
    TRACE("[HITEC] wrong frame 0x%02X", rxBuffer[3]);
    rxBufferCount = 0;
    return;
  }

  if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
    rxBuffer[rxBufferCount++] = data;
  }
  else {
    TRACE("[HITEC] array size %d error", rxBufferCount);
    rxBufferCount = 0;
    return;
  }

  if (rxBufferCount >= HITEC_TELEMETRY_LENGTH) {
    // debug print the content of the packets
#if 0
    debugPrintf(" rssi 0x%02X lqi 0x%02X: ",
                rxBuffer[1], rxBuffer[2]);
    for (int i=0; i<5; i++) {
      debugPrintf("%02X ", rxBuffer[4+i]);
    }
    debugPrintf(CRLF);
#endif
    processHitecPacket(rxBuffer + 1);
    rxBufferCount = 0;
  }
}

void hitecSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const HitecSensor * sensor = getHitecSensor(id);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    uint8_t prec = min<uint8_t>(2, sensor->precision);
    telemetrySensor.init(sensor->name, unit, prec);
    if (unit == UNIT_RPMS) {
      telemetrySensor.custom.ratio = 1;
      telemetrySensor.custom.offset = 1;
    }
  }
  else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}
