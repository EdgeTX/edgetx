/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "flysky_nv14.h"

#define FLYSKY_FIXED_RX_VOLTAGE (uint8_t)(FLYSKY_SENSOR_RX_VOLTAGE + (uint8_t)0xA0)

#define MIN_SNR 8
#define MAX_SNR 45

#define FIXED_PRECISION 15
#define FIXED(val) (val << FIXED_PRECISION)
#define DECIMAL(val) (val >> FIXED_PRECISION)
#define R_DIV_G_MUL_10_Q15 UINT64_C(9591506)
#define INV_LOG2_E_Q1DOT31 UINT64_C(0x58b90bfc) // Inverse log base 2 of e

struct FlyskyNv14Sensor {
  const uint16_t id;
  const uint8_t subId;
  const char * name;
  const TelemetryUnit unit;
  const uint8_t precision;
  const uint8_t offset;
  const uint8_t bytes;
  const bool issigned;
};


union nv14SensorData {
  uint8_t UINT8;
  uint16_t UINT16;
  int16_t INT16;
  uint32_t UINT32;
};



const FlyskyNv14Sensor Nv14Sensor[]=
{
    {FLYSKY_FIXED_RX_VOLTAGE,  0, STR_SENSOR_A3,          UNIT_VOLTS,         2, 0, 2, false},
    {FLYSKY_SENSOR_RX_SIGNAL,  0, STR_SENSOR_RX_QUALITY,  UNIT_RAW,           0, 0, 2, false},
    {FLYSKY_SENSOR_RX_RSSI,    0, STR_SENSOR_RSSI,        UNIT_DB,            0, 0, 2, true,},
    {FLYSKY_SENSOR_RX_NOISE,   0, STR_SENSOR_RX_NOISE,    UNIT_DB,            0, 0, 2, true},
    {FLYSKY_SENSOR_RX_SNR,     0, STR_SENSOR_RX_SNR,      UNIT_DB,            0, 0, 2, false},
    {FLYSKY_SENSOR_RX_SNR,     1, STR_SENSOR_RX_QUALITY,  UNIT_PERCENT,       0, 0, 2, false},
    {FLYSKY_SENSOR_TEMP,       0, STR_SENSOR_TEMP1,       UNIT_CELSIUS,       1, 0, 2, true},
    {FLYSKY_SENSOR_EXT_VOLTAGE,0, STR_SENSOR_A3,          UNIT_VOLTS,         2, 0, 2, false},
    {FLYSKY_SENSOR_MOTO_RPM,   0, STR_SENSOR_RPM,         UNIT_RPMS,          0, 0, 2, false},
    {FLYSKY_SENSOR_PRESURRE,   0, STR_SENSOR_PRES,        UNIT_RAW,           1, 0, 2, false},
    {FLYSKY_SENSOR_PRESURRE,   1, STR_SENSOR_ALT,         UNIT_METERS,        0, 0, 2, true},
    {FLYSKY_SENSOR_GPS,        1, STR_SENSOR_SATELLITES,  UNIT_RAW,           0, 0, 1, false},
    {FLYSKY_SENSOR_GPS,        2, STR_SENSOR_GPS,         UNIT_GPS_LATITUDE,  0, 1, 4, true},
    {FLYSKY_SENSOR_GPS,        3, STR_SENSOR_GPS,         UNIT_GPS_LONGITUDE, 0, 5, 4, true},
    {FLYSKY_SENSOR_GPS,        4, STR_SENSOR_ALT,         UNIT_METERS,        0, 8, 2, true},
    {FLYSKY_SENSOR_GPS,        5, STR_SENSOR_GSPD,        UNIT_KMH,           1, 10, 2, false},
    {FLYSKY_SENSOR_GPS,        6, STR_SENSOR_HDG,         UNIT_DEGREE,        3, 12, 2, false},

};

FlyskyNv14Sensor defaultNv14Sensor = {0, 0, "UNKNOWN", UNIT_RAW, 0, 0, 2, false};

const signed short tAltitude[225]=
{
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
signed short CalculateAltitude( unsigned int Pressure, unsigned int SeaLevelPressure )
{
  unsigned int Index;
  signed int Altitude1;
  signed int Altitude2;
  unsigned int Decimal;
  unsigned long Ratio;

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
  if( Altitude1 >= 0 )
  {
      return( ( Altitude1 + 1 ) / 2 );
  }
  else
  {
      return( ( Altitude1 - 1 ) / 2 );
  }
}

const FlyskyNv14Sensor* getFlyskyNv14Sensor(uint16_t id, uint8_t subId)
{
  for(unsigned index = 0; index < *(&Nv14Sensor + 1) - Nv14Sensor; index++ ){
    if (Nv14Sensor[index].id == id && Nv14Sensor[index].subId == subId) {
      return &Nv14Sensor[index];
    }
  }
    return &defaultNv14Sensor;
}

void flySkyNv14SetDefault(int index, uint8_t id, uint8_t subId, uint8_t instance)
{
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    telemetrySensor.id = id;
    telemetrySensor.subId = subId;
    telemetrySensor.instance = instance;
    const FlyskyNv14Sensor* sensor = getFlyskyNv14Sensor(id, subId);
    telemetrySensor.init(sensor->name, sensor->unit, sensor->precision);
    storageDirty(EE_MODEL);
}



int32_t GetSensorValueFlySkyNv14(const FlyskyNv14Sensor* sensor, const uint8_t * data){
  int32_t value = 0;
  const nv14SensorData* sensorData = reinterpret_cast<const nv14SensorData*>(data + sensor->offset);
  if(sensor->bytes == 1) value = sensorData->UINT8;
  else if(sensor->bytes == 2) value = sensor->issigned ? sensorData->INT16 : sensorData->UINT16;
  else if(sensor->bytes == 4) value = sensorData->UINT32;

  if(sensor->id == FLYSKY_SENSOR_RX_RSSI) {
    if(value < -200) value = -200;
#if defined (PCBNV14)
    if(!g_model.rssiAlarms.flysky_telemetry)
#endif
    {
        value += 200;
        value /= 2;
    }
    telemetryData.rssi.set(value);
  }
  if(sensor->id == FLYSKY_SENSOR_PRESURRE && sensor->subId != 0){
    value = CalculateAltitude(value, 101325);
    value = (value+500)/1000;
  }
  return value;
}

void flySkyNv14ProcessTelemetryPacket(const uint8_t * ptr, uint8_t sensorID )
{
  uint8_t instnace = *ptr++;
  if(sensorID == FLYSKY_SENSOR_RX_VOLTAGE) sensorID = FLYSKY_FIXED_RX_VOLTAGE;
  for (const FlyskyNv14Sensor sensor : Nv14Sensor) {
  if (sensor.id == sensorID) {
  int32_t value = GetSensorValueFlySkyNv14(&sensor, ptr);
  setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_NV14, sensor.id, sensor.subId, instnace, value, sensor.unit, sensor.precision);
}
    }

    telemetryStreaming = TELEMETRY_TIMEOUT10ms;
}

