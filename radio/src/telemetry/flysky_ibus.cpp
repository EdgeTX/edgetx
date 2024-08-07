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

/*
 *  TXID + RXID are already skipped in MULTI module to save memory+transmission time, format from Multi is:
 *  AA or AC | TX_RSSI | sensor ...
 */


#define FLYSKY_TELEMETRY_LENGTH (2+7*4)        // Should it be 2+7*6???
#define ALT_PRECISION 15
#define R_DIV_G_MUL_10_Q15 (uint64_t)9591506
#define INV_LOG2_E_Q1DOT31 (uint64_t)0x58b90bfc // Inverse log base 2 of e
#define PRESSURE_MASK 0x7FFFF
#define REMAP_CONST 0x1000 // Some part of OpenTX does not like sensor with id and instance 0, remap to 0x1000

struct FlySkySensor
{
  const uint16_t type;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

// telemetry sensors type
enum
{
  SENSOR_TYPE_RX_VOL= 0x00 | REMAP_CONST, // RX supply voltage, remapped
  SENSOR_TYPE_TEMPERATURE = 0x01,         // Temperature
  SENSOR_TYPE_MOT = 0x02,                 // RPM
  SENSOR_TYPE_EXT_VOL = 0x03,             // Sensor voltage
  SENSOR_TYPE_BVD = 0x103,                // built-in voltage measurement of the receiver (the highest bit of ID is 1), which distinguishes independent voltage sensors.
  SENSOR_TYPE_GYROSCOPE_1_AXIS = 0x04,    // signed short in 0.1 degrees per second, 0x8000 if unknown
  SENSOR_TYPE_GPS = 0x40,                 // GPS
  SENSOR_TYPE_PRES = 0x41,                // Pressure
  SENSOR_TYPE_ALT = 0x1041,               // virtual

  SENSOR_TYPE_RF_MODULE = 0x56,           // Ext flysky rf module sensor(voltage and temperature)
  SENSOR_TYPE_RF_MODULE_TEMP = 0x1056,    // virtual
  SENSOR_TYPE_RF_MODULE_VOL = 0x2056,     // virtual
  SENSOR_TYPE_RF_MODULE_POWER = 0x3056,   // virtual
//  SENSOR_TYPE_RF_MODULE_RAW =   0x4056,   // virtual

  SENSOR_TYPE_TX_V = 0x7F,                // TX Voltage

  SENSOR_TYPE_ANALOG_SERVO  =0xF0,        // IBUS2 device
	SENSOR_TYPE_DIGITAL_SERVO  =0xF1,       // IBUS2 device

  SENSOR_TYPE_RX_SNR = 0xFA,              // SNR
  SENSOR_TYPE_RX_NOISE = 0xFB,            // Noise
  SENSOR_TYPE_RX_RSSI = 0xFC,             // RSSI
  SENSOR_TYPE_RX_ERR_RATE = 0xFE,         // Error rate
  SENSOR_TYPE_SIGNAL_STRENGTH = 0xFE,     // Error rate

  SENSOR_TYPE_END = 0xFF,
  /////////////////////////////////////////////////////////////////////////////

  AFHDS2A_ID_BAT_CURR = 0x05,           // battery current A * 100
  AFHDS2A_ID_FUEL = 0x06,               // remaining battery percentage / mah drawn otherwise or fuel level no unit!
  AFHDS2A_ID_RPM = 0x07,                // throttle value / battery capacity
  AFHDS2A_ID_CMP_HEAD = 0x08,           // Heading  0..360 deg, 0=north 2bytes
  AFHDS2A_ID_CLIMB_RATE = 0x09,         // 2 bytes m/s *100 signed
  AFHDS2A_ID_COG = 0x0A,                // 2 bytes  Course over ground(NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. unknown max uint
  AFHDS2A_ID_GPS_STATUS = 0x0B,         // 2 bytes
  AFHDS2A_ID_ACC_X = 0x0C,              // 2 bytes m/s *100 signed
  AFHDS2A_ID_ACC_Y = 0x0D,              // 2 bytes m/s *100 signed
  AFHDS2A_ID_ACC_Z = 0x0E,              // 2 bytes m/s *100 signed
  AFHDS2A_ID_ROLL = 0x0F,               // 2 bytes deg *100 signed
  AFHDS2A_ID_PITCH = 0x10,              // 2 bytes deg *100 signed
  AFHDS2A_ID_YAW = 0x11,                // 2 bytes deg *100 signed
  AFHDS2A_ID_VERTICAL_SPEED = 0x12,     // 2 bytes m/s *100 signed
  AFHDS2A_ID_GROUND_SPEED = 0x13,       // 2 bytes m/s *100 different unit than build-in sensor
  AFHDS2A_ID_GPS_DIST = 0x14,           // 2 bytes distance from home m unsigned
  AFHDS2A_ID_ARMED = 0x15,              // 2 bytes
  AFHDS2A_ID_FLIGHT_MODE = 0x16,        // 2 bytes

  AFHDS2A_ID_ODO1 = 0x7C,               // Odometer1
  AFHDS2A_ID_ODO2 = 0x7D,               // Odometer2
  AFHDS2A_ID_SPE = 0x7E,                // Speed 2 bytes km/h

  AFHDS2A_ID_GPS_LAT = 0x80,            // 4bytes signed WGS84 in degrees * 1E7
  AFHDS2A_ID_GPS_LON = 0x81,            // 4bytes signed WGS84 in degrees * 1E7
  AFHDS2A_ID_GPS_ALT = 0x82,            // 4bytes signed!!! GPS alt m*100

  AFHDS2A_ID_ALT = 0x83,                // 4bytes signed!!! Alt m*100

  AFHDS2A_ID_RX_SIG_AFHDS3  = 0xF7,    // SIG
  AFHDS2A_ID_RX_SNR_AFHDS3  = 0xF8,    // SNR
  AFHDS2A_ID_ALT_FLYSKY     = 0xF9,    // Altitude 2 bytes signed in m - used in FlySky native TX

  // AC type telemetry with multiple values in one packet
  AFHDS2A_ID_GPS_FULL = 0xFD,
  AFHDS2A_ID_VOLT_FULL = 0xF0,
  AFHDS2A_ID_ACC_FULL = 0xEF,
  AFHDS2A_ID_TX_RSSI = 0x200,           // Pseudo id outside 1 byte range of FlySky sensors
};

// telemetry sensors ID
enum
{
  AFHDS2A_ID_VOLTAGE = 0x00,    // Internal Voltage
  AFHDS2A_ID_TEMPERATURE = 0x01,    // Temperature
  AFHDS2A_ID_MOT = 0x02,    // RPM
  AFHDS2A_ID_EXTV = 0x03,    // External Voltage
  AFHDS2A_ID_CELL_VOLTAGE = 0x04,    // Avg Cell voltage

  AFHDS3_FRM_TEMP           = 0x57,    //virtual
  AFHDS3_FRM_EXT_V          = 0x58,    //virtual

  AFHDS2A_ID_TX_V = 0x7F,    // TX Voltage

  AFHDS2A_ID_S84 = 0x84,
  AFHDS2A_ID_S85 = 0x85,
  AFHDS2A_ID_S86 = 0x86,
  AFHDS2A_ID_S87 = 0x87,
  AFHDS2A_ID_S88 = 0x88,
  AFHDS2A_ID_S89 = 0x89,
  AFHDS2A_ID_S8a = 0x8A,

  AFHDS2A_ID_RX_SNR = 0xFA,    // SNR
  AFHDS2A_ID_RX_NOISE = 0xFB,    // Noise
  AFHDS2A_ID_RX_RSSI = 0xFC,    // RSSI
  AFHDS2A_ID_RX_ERR_RATE = 0xFE,    // Error rate
  AFHDS2A_ID_END = 0xFF,
};

// clang-format off
#define FS(type,name,unit,precision) {type,unit,precision,name}

const FlySkySensor flySkySensors[] = {
  // flysky start
  FS( SENSOR_TYPE_RX_VOL,              STR_SENSOR_A1,             UNIT_VOLTS,             2 ),  // RX Voltage (remapped, really 0x0)
  FS( SENSOR_TYPE_TEMPERATURE,         STR_SENSOR_TEMP1,          UNIT_CELSIUS,           1 ),  // Temperature
  FS( SENSOR_TYPE_MOT,                 STR_SENSOR_RPM,            UNIT_RAW,               0 ),  // RPM
  FS( SENSOR_TYPE_EXT_VOL,             STR_SENSOR_A3,             UNIT_VOLTS,             2 ),  // External voltage
  FS( SENSOR_TYPE_BVD,                 "BVD",                     UNIT_VOLTS,             2 ),  // BVD
  FS( SENSOR_TYPE_GYROSCOPE_1_AXIS,    STR_SENSOR_CELLS,          UNIT_DEGREE,            1 ),  //
  FS( SENSOR_TYPE_PRES,                STR_SENSOR_PRES,           UNIT_RAW,               2 ),  // 4 bytes In fact Temperature + Pressure -> Altitude
  FS( SENSOR_TYPE_ALT,                 STR_SENSOR_ALT,            UNIT_METERS,            2 ),
  FS( SENSOR_TYPE_RF_MODULE_TEMP,      STR_SENSOR_TEMP2,          UNIT_CELSIUS,           0 ),  // 1 bytes temperature
  FS( SENSOR_TYPE_RF_MODULE_VOL,       STR_SENSOR_TXV,            UNIT_VOLTS,             2 ),  // 2 bytes voltage
  FS( SENSOR_TYPE_RF_MODULE_POWER,     STR_SENSOR_TX_POWER,       UNIT_DBM,               0 ),  // 2 bytes DBM
//  FS( SENSOR_TYPE_RF_MODULE_RAW,       STR_SENSOR_TX_POWER,       UNIT_RAW,               0 ),  // 2 bytes DBM
  FS( SENSOR_TYPE_TX_V,                STR_SENSOR_TXV,            UNIT_VOLTS,             2 ),  // TX Voltage
  FS( SENSOR_TYPE_RX_SNR,              STR_SENSOR_RX_SNR,         UNIT_DB,                0 ),  // RX SNR
  FS( SENSOR_TYPE_RX_NOISE,            STR_SENSOR_RX_NOISE,       UNIT_DBM,               0 ),  // RX Noise
  FS( SENSOR_TYPE_RX_RSSI,             STR_SENSOR_RSSI,           UNIT_DBM,               0 ),  // RX RSSI (0xfc)
  FS( SENSOR_TYPE_RX_ERR_RATE,         STR_SENSOR_RX_QUALITY,     UNIT_PERCENT,           0 ),  // RX error rate
  ////////////////////////////////////////////////////////////////

  FS( AFHDS2A_ID_BAT_CURR,              STR_SENSOR_CURR,          UNIT_AMPS,              2 ),  // battery current A * 100
  FS( AFHDS2A_ID_FUEL,                  STR_SENSOR_CAPACITY,      UNIT_RAW,               0 ),  // remaining battery percentage / mah drawn otherwise or fuel level no unit!
  FS( AFHDS2A_ID_RPM,                   STR_SENSOR_RPM,           UNIT_RAW,               0 ),  // throttle value / battery capacity
  FS( AFHDS2A_ID_CMP_HEAD,              STR_SENSOR_HDG,           UNIT_DEGREE,            0 ),  // Heading  0..360 deg, 0=north 2bytes
  FS( AFHDS2A_ID_CLIMB_RATE,            STR_SENSOR_VSPD,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100
  FS( AFHDS2A_ID_COG,                   STR_SENSOR_HDG,           UNIT_DEGREE,            2 ),  // 2 bytes  Course over ground(NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. unknown max uint
  FS( AFHDS2A_ID_GPS_STATUS,            STR_SENSOR_SATELLITES,    UNIT_RAW,               0 ),  // 2 bytes
  FS( AFHDS2A_ID_ACC_X,                 STR_SENSOR_ACCX,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100 signed
  FS( AFHDS2A_ID_ACC_Y,                 STR_SENSOR_ACCY,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100 signed
  FS( AFHDS2A_ID_ACC_Z,                 STR_SENSOR_ACCZ,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100 signed
  FS( AFHDS2A_ID_ROLL,                  STR_SENSOR_ROLL,          UNIT_DEGREE,            2 ),  // 2 bytes deg *100 signed
  FS( AFHDS2A_ID_PITCH,                 STR_SENSOR_PITCH,         UNIT_DEGREE,            2 ),  // 2 bytes deg *100 signed
  FS( AFHDS2A_ID_YAW,                   STR_SENSOR_YAW,           UNIT_DEGREE,            2 ),  // 2 bytes deg *100 signed
  FS( AFHDS2A_ID_VERTICAL_SPEED,        STR_SENSOR_VSPD,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100
  FS( AFHDS2A_ID_GROUND_SPEED,          STR_SENSOR_GSPD,          UNIT_METERS_PER_SECOND, 2 ),  // 2 bytes m/s *100 different unit than build-in sensor
  FS( AFHDS2A_ID_GPS_DIST,              STR_SENSOR_DIST,          UNIT_METERS,            0 ),  // 2 bytes dist from home m unsigned
  FS( AFHDS2A_ID_ARMED,                 STR_SENSOR_ARM,           UNIT_RAW,               0 ),  // 2 bytes
  FS( AFHDS2A_ID_FLIGHT_MODE,           STR_SENSOR_FLIGHT_MODE,   UNIT_RAW,               0 ),  // 2 bytes index
  FS( SENSOR_TYPE_PRES | 0x100,         STR_SENSOR_TEMP2,         UNIT_CELSIUS,           1 ),  // 2 bytes Temperature
  FS( AFHDS2A_ID_ODO1,                  STR_SENSOR_ODO1,          UNIT_METERS,            2 ),  // 2 bytes Odometer1 -- some magic with 330 needed
  FS( AFHDS2A_ID_ODO2,                  STR_SENSOR_ODO2,          UNIT_METERS,            2 ),  // 2 bytes Odometer2 -- some magic with 330 needed
  FS( AFHDS2A_ID_SPE,                   STR_SENSOR_ASPD,          UNIT_KMH,               2 ),  // 2 bytes Speed km/h -- some magic with 330 needed
  FS( AFHDS2A_ID_GPS_LAT,               STR_SENSOR_GPS,           UNIT_GPS,               0 ),  // 4 bytes signed WGS84 in degrees * 1E7
//FS( AFHDS2A_ID_GPS_LON,               STR_SENSOR_GPS,           UNIT_GPS,               0 ),  // 4 bytes signed WGS84 in degrees * 1E7
  FS( AFHDS2A_ID_GPS_ALT,               STR_SENSOR_GPSALT,        UNIT_METERS,            2 ),  // 4 bytes signed GPS alt m*100
  FS( AFHDS2A_ID_ALT,                   STR_SENSOR_ALT,           UNIT_METERS,            2 ),  // 4 bytes signed Alt m*100
  FS( AFHDS2A_ID_RX_SIG_AFHDS3,         STR_SENSOR_RX_QUALITY,    UNIT_PERCENT,           0 ),  // RX error rate
  FS( AFHDS2A_ID_RX_SNR_AFHDS3,         STR_SENSOR_RX_SNR,        UNIT_DB,                1 ),  // RX SNR
  FS( AFHDS2A_ID_TX_RSSI,               STR_SENSOR_TX_RSSI,       UNIT_DBM,               0 ),  // Pseudo sensor for TRSSI

  FS( 0x00,                            NULL,                      UNIT_RAW,               0 ),  // sentinel
};
// clang-format on

int32_t getALT(uint32_t value);
inline int setFlyskyTelemetryValue( int16_t type, uint8_t instance, int32_t value, uint32_t unit, uint32_t prec)
{
  return setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, type, 0, instance, value, unit, prec );
}

void processFlySkyAFHDS3Sensor(const uint8_t * packet, uint8_t len )
{
  uint16_t type = (packet[0] << 8) | packet[1];
  type = type ? type : SENSOR_TYPE_RX_VOL;  // Remapped
  uint8_t id = packet[2];
  int32_t value=0;

  if(len == 1)
  {
    value = packet[3];
  }
  else if (len == 2)
  {
      value = (packet[4] << 8) | packet[3];
  }
  else if(len == 4)
  {
    value = (packet[6] << 24) | (packet[5] << 16) | (packet[4] << 8) | packet[3];
  }
  else
  {
    if( SENSOR_TYPE_GPS == type )
    {
      //INRM301 does not support GPS temporarily.
      return;
    }
    else if( SENSOR_TYPE_RF_MODULE == type )
    {
      uint8_t data1[] = { (uint8_t)(SENSOR_TYPE_RF_MODULE_TEMP>>8), (uint8_t)SENSOR_TYPE_RF_MODULE_TEMP, id, packet[3] };
      uint8_t data2[] = { (uint8_t)(SENSOR_TYPE_RF_MODULE_VOL>>8), (uint8_t)SENSOR_TYPE_RF_MODULE_VOL, id, packet[4], packet[5] };
      uint8_t data3[] = { (uint8_t)(SENSOR_TYPE_RF_MODULE_POWER>>8), (uint8_t)SENSOR_TYPE_RF_MODULE_POWER, id, packet[8], packet[9] };
//      uint8_t data4[] = { (uint8_t)(SENSOR_TYPE_RF_MODULE_RAW>>8), (uint8_t)SENSOR_TYPE_RF_MODULE_RAW, id, packet[6] & 0x07};

      processFlySkyAFHDS3Sensor(data1, 1 );
      processFlySkyAFHDS3Sensor(data2, 2 );
      processFlySkyAFHDS3Sensor(data3, 2 );
//      processFlySkyAFHDS3Sensor(data4, 1 );
      return;
    }
    else
      return;
  }

  if (SENSOR_TYPE_RX_NOISE == type || SENSOR_TYPE_RX_RSSI == type)
  {
    value  = -value;
  }  

  if ( (SENSOR_TYPE_EXT_VOL == type) )
  {
    if ( id&0x80 )
    {
      type = SENSOR_TYPE_BVD;
    }
  }
  else if(SENSOR_TYPE_RX_RSSI == type || SENSOR_TYPE_RX_NOISE == type || SENSOR_TYPE_RX_SNR == type || SENSOR_TYPE_RF_MODULE_POWER == type)
  {
    if( value>=0 )
      value = (value+2)/4;
    else
      value = (value-2)/4;
  }
  else if (SENSOR_TYPE_RX_ERR_RATE == type)
  {
    telemetryData.rssi.set( value );
    if (value > 0) telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  }
  else if (SENSOR_TYPE_PRES == type)
  {
      int32_t alt = getALT(value);
      int16_t temp = (value >> 19);

      uint8_t data1[] = { (uint8_t)(SENSOR_TYPE_ALT>>8), (uint8_t)(SENSOR_TYPE_ALT&0xff), id, (uint8_t)alt, (uint8_t)(alt>>8), (uint8_t)(alt>>16), (uint8_t)(alt>>24) };
      uint8_t data2[] = { (uint8_t)(SENSOR_TYPE_TEMPERATURE>>8), (uint8_t)(SENSOR_TYPE_TEMPERATURE&0xff), id, (uint8_t)temp, (uint8_t)(temp>>8) };
      processFlySkyAFHDS3Sensor(data1, 4 );
      processFlySkyAFHDS3Sensor(data2, 2 );
      value &= PRESSURE_MASK;
  }

  if(SENSOR_TYPE_TEMPERATURE == type)
  {
    value -= 400; // Temperature sensors have 40 degree offset
  }

  for (const FlySkySensor * sensor = flySkySensors; sensor->type; sensor++)
  {
    if (sensor->type != type) continue;

    if (sensor->unit == UNIT_VOLTS) value = (int16_t) value; // Voltage types are unsigned 16bit integers

    setFlyskyTelemetryValue(type, id, value, sensor->unit, sensor->precision);
    return;
  }
  //unknown
  setFlyskyTelemetryValue(type, id, value, UNIT_RAW, 0);
}


void processFlySkySensor(const uint8_t * packet, uint8_t type)
{
  uint8_t buffer[8];
  uint16_t id = packet[0];
  const uint8_t instance = packet[1];
  int32_t value;

  //Load most likely value
  if (type == 0xAA)
    value = (packet[3] << 8) | packet[2];
  else
    value = (packet[6] << 24) | (packet[5] << 16) | (packet[4] << 8) | packet[3];

  id = id ? id : SENSOR_TYPE_RX_VOL;  // Remapped

  if (id == AFHDS2A_ID_RX_NOISE || id == AFHDS2A_ID_RX_RSSI) {
    value  = 135 - value;
  }
  else if (id == AFHDS2A_ID_RX_ERR_RATE) {
    value = 100 - value;
    telemetryData.rssi.set(value);
    if (value > 0) telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  }
  else if(id == AFHDS2A_ID_RX_SIG_AFHDS3) {
    telemetryData.rssi.set(value);
    if(value>0) telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  }
  else if (id == SENSOR_TYPE_PRES && value) {
    // Extract temperature to a new sensor
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, id | 0x100, 0, instance, ((value >> 19) - 400), UNIT_CELSIUS, 1);
    // Extract alt to a new sensor
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, AFHDS2A_ID_ALT, 0, instance, getALT(value), UNIT_METERS, 2);
    value &= PRESSURE_MASK;
  }
  else if ((id >= AFHDS2A_ID_ACC_X && id <= AFHDS2A_ID_VERTICAL_SPEED) || id == AFHDS2A_ID_CLIMB_RATE || id == AFHDS2A_ID_ALT_FLYSKY) {
    value = (int16_t) value; // Signed value
  }
  else if (id == AFHDS2A_ID_GPS_STATUS) {
    value = value >> 8;
  }
  else if (id == AFHDS2A_ID_GPS_FULL) {
    //(AC FRAME)[ID][inst][size][fix][sats][LAT]x4[LON]x4[ALT]x4
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, AFHDS2A_ID_GPS_STATUS, 0, instance, packet[4], UNIT_RAW, 0);

    for (uint8_t sensorID = AFHDS2A_ID_GPS_LAT; sensorID <= AFHDS2A_ID_GPS_ALT; sensorID++) {
      int index = 5 + (sensorID - AFHDS2A_ID_GPS_LAT) * 4;
      buffer[0] = sensorID;
      buffer[1] = instance;
      buffer[2] = 4;
      memcpy(buffer + 3, packet + index, 4);
      processFlySkySensor(buffer, 0xAC);
    }

    return;
  } else if (id == AFHDS2A_ID_GPS_LAT) {
    uint8_t instance2 = 0;  // Assume one instance, RX would only have one GPS
    value = value / 10;
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, AFHDS2A_ID_GPS_LAT, 0,
                      instance2, value, UNIT_GPS_LATITUDE, 0);
    return;
  } else if (id == AFHDS2A_ID_GPS_LON) {  // Remapped to single GPS sensor:
                                          // AFHDS2A_ID_GPS_LAT
    uint8_t instance2 = 0;
    value = value / 10;
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, AFHDS2A_ID_GPS_LAT, 0,
                      instance2, value, UNIT_GPS_LONGITUDE, 0);
    return;
  } else if (id == AFHDS2A_ID_VOLT_FULL) {
    //(AC FRAME)[ID][inst][size][ACC_X]x2[ACC_Y]x2[ACC_Z]x2[ROLL]x2[PITCH]x2[YAW]x2
    for (uint8_t sensorID = AFHDS2A_ID_EXTV; sensorID <= AFHDS2A_ID_RPM; sensorID++) {
      int index = 3 + (sensorID - AFHDS2A_ID_EXTV) * 2;
      buffer[0] = sensorID;
      buffer[1] = instance;
      buffer[2] = packet[index];
      buffer[3] = packet[index + 1];
      processFlySkySensor(buffer, 0xAA);
    }
    return;
  } else if (id == AFHDS2A_ID_ACC_FULL) {
    //(AC FRAME)[ID][inst][size]
    for (uint8_t sensorID = AFHDS2A_ID_ACC_X; sensorID <= AFHDS2A_ID_YAW; sensorID++) {
      int index = 3 + (sensorID - AFHDS2A_ID_ACC_X) * 2;
      buffer[0] = sensorID;
      buffer[1] = instance;
      buffer[2] = packet[index];
      buffer[3] = packet[index + 1];
      processFlySkySensor(buffer, 0xAA);
    }
    return;
  }
  for (const FlySkySensor * sensor = flySkySensors; sensor->type; sensor++) {
    if (sensor->type != id) continue;
    if (sensor->unit == UNIT_CELSIUS) value -= 400; // Temperature sensors have 40 degree offset
    else if (sensor->unit == UNIT_VOLTS) value = (int16_t) value; // Voltage types are unsigned 16bit integers
    setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, id, 0, instance, value, sensor->unit, sensor->precision);
    return;
  }
  //unknown
  setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS, id, 0, instance, value, UNIT_RAW, 0);
}

void processFlySkyPacket(const uint8_t * packet)
{
  // Set TX RSSI Value, reverse MULTIs scaling
  setFlyskyTelemetryValue(AFHDS2A_ID_TX_RSSI, 0, packet[0], UNIT_RAW, 0);

  const uint8_t * buffer = packet + 1;
  int sensor = 0;
  while (sensor++ < 7) {
    if (*buffer == SENSOR_TYPE_END) break;
    processFlySkySensor(buffer, 0xAA);
    buffer += 4;
  }
}

void processFlySkyPacketAC(const uint8_t * packet)
{
  // Set TX RSSI Value, reverse MULTIs scaling
  setFlyskyTelemetryValue(AFHDS2A_ID_TX_RSSI, 0, packet[0], UNIT_RAW, 0);
  const uint8_t * buffer = packet + 1;
  while (buffer - packet < 26) //28 + 1(multi TX rssi) - 3(ac header)
  {
    if (*buffer == SENSOR_TYPE_END) break;
    uint8_t size = buffer[2];
    processFlySkySensor(buffer, 0xAC);
    buffer += size + 3;
  }
}

void processFlySkyTelemetryData(uint8_t data, uint8_t * rxBuffer, uint8_t &rxBufferCount)
{
  if (rxBufferCount == 0)
    return;

  if (data == 2 || data == 4) {
    TRACE("[IBUS] Packet 0x%02X", data);
  }
  else {
    TRACE("[IBUS] invalid start byte 0x%02X", data);
    rxBufferCount = 0;
    return;
  }

  if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
    rxBuffer[rxBufferCount++] = data;
  }
  else {
    TRACE("[IBUS] array size %d error", rxBufferCount);
    rxBufferCount = 0;
  }

  if (rxBufferCount >= FLYSKY_TELEMETRY_LENGTH) {
    // debug print the content of the packets
#if 0
    debugPrintf(", rssi 0x%02X: ", rxBuffer[1]);
    for (int i=0; i<7; i++) {
      debugPrintf("[%02X %02X %02X%02X] ", rxBuffer[i*4+2], rxBuffer[i*4 + 3],
                  rxBuffer[i*4 + 4], rxBuffer[i*4 + 5]);
    }
    debugPrintf(CRLF);
#endif
    if (data == 0xAA) processFlySkyPacket(rxBuffer + 1);
    else if (data == 0xAC) processFlySkyPacketAC(rxBuffer + 1);
    rxBufferCount = 0;
  }
}

const FlySkySensor * getFlySkySensor(uint16_t id)
{
  for (const FlySkySensor * sensor = flySkySensors; sensor->type; sensor++) {
    if (id == sensor->type)
      return sensor;
  }
  return nullptr;
}

void flySkySetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const FlySkySensor * sensor = getFlySkySensor(id);
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

uint16_t ibusTempToK(int16_t tempertureIbus)
{
  return (uint16_t) (tempertureIbus - 400) + 2731;
}

int32_t log2fix(uint32_t x)
{
  int32_t b = 1U << (ALT_PRECISION - 1);
  int32_t y = 0;
  while (x < 1U << ALT_PRECISION) {
    x <<= 1;
    y -= 1U << ALT_PRECISION;
  }

  while (x >= 2U << ALT_PRECISION) {
    x >>= 1;
    y += 1U << ALT_PRECISION;
  }

  uint64_t z = x;
  for (size_t i = 0; i < ALT_PRECISION; i++) {
    z = (z * z) >> ALT_PRECISION;
    if (z >= 2U << ALT_PRECISION) {
      z >>= 1;
      y += b;
    }
    b >>= 1;
  }
  return y;
}

#if 0
int32_t getALT(uint32_t value)
{
  uint32_t pressurePa = value & PRESSURE_MASK;
  if (pressurePa == 0) return 0;
  uint16_t temperatureK = ibusTempToK((uint16_t) (value >> 19));
  static uint32_t initPressure = 0;
  static uint16_t initTemperature = 0;
  if (initPressure <= 0) // use current pressure for ground altitude -> 0
  {
    initPressure = pressurePa;
    initTemperature = temperatureK;
  }
  int temperature = (initTemperature + temperatureK) >> 1; //div 2
  bool tempNegative = temperature < 0;
  if (tempNegative) temperature = temperature * -1;
  uint64_t helper = R_DIV_G_MUL_10_Q15;
  helper = helper * (uint64_t) temperature;
  helper = helper >> ALT_PRECISION;

  uint32_t po_to_p = (uint32_t)(initPressure << (ALT_PRECISION - 1));
  po_to_p = po_to_p / pressurePa;
  //shift missing bit
  po_to_p = po_to_p << 1;
  if (po_to_p == 0) return 0;
  uint64_t t = log2fix(po_to_p) * INV_LOG2_E_Q1DOT31;
  int32_t ln = t >> 31;

  bool neg = ln < 0;
  if (neg) ln = ln * -1;
  helper = helper * (uint64_t) ln;
  helper = helper >> ALT_PRECISION;
  int result = (int) helper;

  if (neg ^ tempNegative) result = result * -1;
  return result;
}
#else
/*==================================================================================================
table:
Function:
==================================================================================================*/
const int16_t tAltitude[225]=
{ // In half meter unit
    20558, 20357, 20158, 19962, 19768, 19576, 19387, 19200, 19015,  18831, 18650, 18471, 18294, 18119, 17946, 17774,
    17604, 17436, 17269, 17105, 16941, 16780, 16619, 16461, 16304,  16148, 15993, 15841, 15689, 15539, 15390, 15242,
    15096, 14950, 14806, 14664, 14522, 14381, 14242, 14104, 13966,  13830, 13695, 13561, 13428, 13296, 13165, 13035,
    12906, 12777, 12650, 12524, 12398, 12273, 12150, 12027, 11904,  11783, 11663, 11543, 11424, 11306, 11189, 11072,
    10956, 10841, 10726, 10613, 10500, 10387, 10276, 10165, 10054,   9945,  9836,  9727,  9620,  9512,  9406,  9300,
    9195,  9090,  8986,  8882,  8779,  8677,   8575,  8474,  8373,   8273,  8173,  8074,  7975,  7877,  7779,  7682,
    7585,  7489,  7394,  7298,  7204,  7109,   7015,  6922,  6829,   6737,  6645,  6553,  6462,  6371,  6281,  6191,
    6102,  6012,  5924,  5836,  5748,  5660,   5573,  5487,  5400,   5314,  5229,  5144,  5059,  4974,  4890,  4807,
    4723,  4640,  4557,  4475,  4393,  4312,   4230,  4149,  4069,   3988,  3908,  3829,  3749,  3670,  3591,  3513,
    3435,  3357,  3280,  3202,  3125,  3049,   2972,  2896,  2821,   2745,  2670,  2595,  2520,  2446,  2372,  2298,
    2224,  2151,  2078,  2005,  1933,   1861,  1789,  1717,  1645,   1574,  1503,  1432,  1362,  1292,  1222,  1152,
    1082,  1013,   944,   875,   806,   738,    670,   602,   534,    467,   399,   332,   265,   199,   132,    66,
     0,     -66,  -131,  -197,  -262,  -327,   -392,  -456,  -521,   -585,  -649,  -713,  -776,  -840,  -903,  -966,
    -1029,-1091, -1154, -1216, -1278, -1340,  -1402, -1463,  -1525, -1586, -1647, -1708, -1769, -1829, -1889, -1950,
    -2010
};
/*==================================================================================================
Name:
Function:
Input:
Output:
==================================================================================================*/
int32_t getALT(uint32_t Pressure)//CalculateAltitude( uint32_t Pressure, uint32_t SeaLevelPressure )
{
    uint32_t Index;
    int32_t Altitude1;
    int32_t Altitude2;
    uint32_t Decimal;
    uint64_t Ratio;
   uint32_t SeaLevelPressure=101320;
    Pressure = Pressure & PRESSURE_MASK;
    Ratio = ( ( ( unsigned long long ) Pressure << 16 ) + ( SeaLevelPressure / 2 ) ) / SeaLevelPressure;
    if( Ratio < ( ( 1 << 16 ) * 250 / 1000 ) )// 0.250 inclusive
    {
        Ratio = ( 1 << 16 ) * 250 / 1000;
    }
    else if( Ratio > ( 1 << 16 ) * 1125 / 1000 - 1 ) // 1.125 non-inclusive
    {
        Ratio = ( 1 << 16 ) * 1125 / 1000 - 1;
    }

    Ratio -= ( 1 << 16 ) * 250 / 1000; // from 0.000 (inclusive) to 0.875 (non-inclusive)
    Index = Ratio >> 8;
    Decimal = Ratio & ( ( 1 << 8 ) - 1 );
    Altitude1 = tAltitude[Index];
    Altitude2 = Altitude1 - tAltitude[Index + 1];
    Altitude1 = Altitude1 - ( Altitude2 * Decimal + ( 1 << 7 ) ) / ( 1 << 8 );
    Altitude1 *= 100;
    if( Altitude1 >= 0 )
    {
        return( ( Altitude1 + 1 ) / 2 );
    }
    else
    {
        return( ( Altitude1 - 1 ) / 2 );
    }
}
#endif