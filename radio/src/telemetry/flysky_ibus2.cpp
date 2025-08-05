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
#include <math.h>

#define FLYSKY_TELEMETRY_LENGTH (2+7*4) 
#define ALT_PRECISION 15
#define RX_CMD_CODE_IBUS2_SET_PARAM          ( 0x7025 )
#define RX_CMD_CODE_IBUS2_GET_PARAM          ( 0x7026 )
#define PRESSURE_MASK 0x7FFFF
#define REMAP_CONST 0x1000 // Some part of OpenTX does not like sensor with id and instance 0, remap to 0x1000
#define IBUS2_CALIB_SET_PARAM  0x1234
#define IBUS2_CALIB_IBC01     0x0003
#define IBUS2_CALIB_RPM       0xD001

enum Ibus2SensorOnLine {
  IBUS2_SENSOR_RPM,
  IBUS2_SENSOR_GPS,
  IBUS2_SENSOR_IBC,
  IBUS2_SENSOR_PRES,
  IBUS2_SENSOR_NUM,
};

enum 
{
    GPS_MSG_TYPE_PACK1 = 1,
    GPS_MSG_TYPE_PACK2,
    GPS_MSG_TYPE_PACK3,
    GPS_MSG_TYPE_PACK4,
    GPS_MSG_TYPE_CALI = 0x0F,
    GPS_MSG_TYPE_NUM,
};

enum 
{
    GPS_STATE_GET_POSITION = 3,
    GPS_STATE_NUM,
};

typedef struct
{
	unsigned char NbSatellites;
	unsigned char PositionStatus;	//-
	unsigned char DateDay;
	unsigned char DateMonth;
	unsigned short DateYear; // Year from 2000
	unsigned short Speed; // In 1/100th of meters
	unsigned long UTCTime; // In milliseconds from midnight
	int32_t 		  Latitude; // In 1/1000000th of degree from -90 to +90 degrees
	int32_t			  Longitude; // In 1/1000000th of degree from -180 to +180 degrees
	int32_t			  Direction; // True direction in 1/10 of degree (0=North)
	int32_t   		  Altitude; // In 1/100th of meters
	float		  Pitch;	//-
	float		  Roll;		//-
	float		  Yaw;		//-
	
	int32_t   		  Distance; //-
	int16_t           Acceleration;//acc
	int16_t 		  HeightChange;//
} Ibus2Gps_t;

typedef struct 
{
	int32_t 		  Latitude; // In 1/1000000th of degree from -90 to +90 degrees
	int32_t			  Longitude; // In 1/1000000th of degree from -180 to +180 degrees
  int32_t   		  Altitude; // In 1/100th of meters
  int32_t       Boar_Altitude;
} StartPos_t;

typedef struct
{
  short Voltage; //unit 0.1V
	short Current; // unit 0.1A
	uint16_t UsedCapacity; //unit 1mAh
	uint16_t RunTime;   //unit 1second
	short AverageCurrent;   //unit 0.1A
	short MaxVoltage;   //unit 0.1V
	short MinVoltage;   //unit 0.1V
	short MaxCurrent;   //unit 0.1A
} Ibus2Ibc_t;

typedef struct flysky_ibus2
{
  uint8_t id;
  uint16_t type;
  uint8_t ParameterData[16];
} Ibus2Param_t;


Ibus2Gps_t GPSData = {0};
Ibus2Ibc_t IBCDate = {0};
StartPos_t StartPos = {0};
Ibus2Param_t Ibus2DevPara = {0};

struct FlySkySensor
{
  const uint16_t type;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

enum
{
	IBDT_INT_VOLTAGE          = 0x00 | REMAP_CONST, 
	IBDT_TEMPERATURE          = 0x01,    // Temperature
	IBDT_ROTATION_SPEED       = 0x02,    // RPM
	IBDT_EXT_VOLTAGE          = 0x03,    // Sensor voltage
	IBDT_GPS                  = 0x40,    // GPS
	IBDT_PRESSURE             = 0x41,    // Pressure
	IBDT_COMPASS              = 0x42,
	IBDT_IBC01				  = 0x43,    // Voltage and current sensor
	IBDT_REDUNDANT_RECEIVER   = 0x44,
	IBDT_REDUNDANT_RECEIVER_E = 0x45,
	IBDT_FLIGHT               = 0x70,
	IBDT_TX_VOLTAGE           = 0x7F,
  IBDT_PRESSURE_SENSOR   	  = 0x80,
  IBDT_AIRSPEED_SENSOR   	  = 0x81,
  IBDT_EXT_BVD              = 0xA0,
	IBDT_HUB_1_PORTS_PWM      = 0xE0,    // 1-port hub in PWM mode
	IBDT_HUB_2_PORTS_PWM      = 0xE1,    // 2-port hub in PWM mode
	IBDT_HUB_3_PORTS_PWM      = 0xE2,    // 3-port hub in PWM mode
	IBDT_HUB_4_PORTS_PWM      = 0xE3,    // 4-port hub in PWM mode
	IBDT_HUB_5_PORTS_PWM      = 0xE4,    // 5-port hub in PWM mode
	IBDT_HUB_6_PORTS_PWM      = 0xE5,    // 6-port hub in PWM mode
	IBDT_HUB_7_PORTS_PWM      = 0xE6,    // 7-port hub in PWM mode
	IBDT_8_PORTS_PWM_ADAPTER  = 0xE7,    // 8-port PWM adapter
	IBDT_9_PORTS_PWM_ADAPTER  = 0xE8,    // 9-port PWM adapter
	IBDT_10_PORTS_PWM_ADAPTER = 0xE9,    // 10-port PWM adapter
	IBDT_11_PORTS_PWM_ADAPTER = 0xEA,    // 11-port PWM adapter
	IBDT_12_PORTS_PWM_ADAPTER = 0xEB,    // 12-port PWM adapter
	IBDT_13_PORTS_PWM_ADAPTER = 0xEC,    // 13-port PWM adapter
	IBDT_14_PORTS_PWM_ADAPTER = 0xED,    // 14-port PWM adapter
	IBDT_15_PORTS_PWM_ADAPTER = 0xEE,    // 15-port PWM adapter
	IBDT_16_PORTS_PWM_ADAPTER = 0xEF,    // 16-port PWM adapter
	IBDT_17_PORTS_PWM_ADAPTER = 0xF0,    // 17-port PWM adapter
	IBDT_HUB_1_PORTS_HUB      = 0xF1,    // 1-port hub in hub mode
	IBDT_HUB_2_PORTS_HUB      = 0xF2,    // 2-port hub in hub mode
	IBDT_HUB_3_PORTS_HUB      = 0xF3,    // 3-port hub in hub mode
	IBDT_HUB_4_PORTS_HUB      = 0xF4,    // 4-port hub in hub mode
	IBDT_HUB_5_PORTS_HUB      = 0xF5,    // 5-port hub in hub mode
	IBDT_HUB_6_PORTS_HUB      = 0xF6,    // 6-port hub in hub mode
	IBDT_HUB_7_PORTS_HUB      = 0xF7,    // 7-port hub in hub mode
	IBDT_DIGITAL_SERVO        = 0xF8,
	IBDT_SNR                  = 0xFA,
	IBDT_BK_NOISE             = 0xFB,
	IBDT_RSSI                 = 0xFC,
	IBDT_SERVO_HUB            = 0xFD,    // i-Bus 1 only
	IBDT_SIGNAL_STRENGTH      = 0xFE,	
	IBDT_NONE                 = 0xFF,

  VIRTUAL_ALT               = 0x1041,               // virtual
  VIRTUAL_REL_ALT           = 0x1141,               // virtual
  VIRTUAL_GPS_STAS          = 0x1040,
  VIRTUAL_GPS_TIME           = 0x1140,
  VIRTUAL_GPS_LAT           = 0x1240,
  VIRTUAL_GPS_LON           = 0x1340,
  VIRTUAL_GPS_KMH           = 0x1440,
  VIRTUAL_GPS_PITCH         = 0x1540,
  VIRTUAL_GPS_ROLL          = 0x1640,
  VIRTUAL_GPS_YAW           = 0x1740,
  VIRTUAL_GPS_DIST          = 0x1840,
  VIRTUAL_GPS_ACC           = 0x1940,
  VIRTUAL_GPS_SPEED          = 0x1A40,
  VIRTUAL_GPS_ALT           = 0x1B40,
  VIRTUAL_GPS_REL_ALT           = 0x1C40,

  VIRTUAL_IBC_VOLTS          = 0x1043,
  VIRTUAL_IBC_CURR           = 0x1143,
  VIRTUAL_IBC_CAPA           = 0x1243,
  VIRTUAL_IBC_RUMTIME        = 0x1343,
  VIRTUAL_IBC_AVG_CURR       = 0x1443,
};

// clang-format off
#define FS(type,name,unit,precision) {type,unit,precision,name}

extern int32_t getALT(uint32_t value);

void flyskyIbus2GPS(const uint8_t * pData, uint8_t len, uint8_t id);
void flyskyIbus2IBC(const uint8_t * pData, uint8_t len, uint8_t id);
void setIbus2Param(uint8_t* pData);
void getIbus2Param(uint8_t* pData);

void sendFlyskytelemtry(uint16_t type, uint8_t id, int32_t value);

int32_t GpsSensorDistanceGet();
int32_t GpsSensorHeighthGet();

uint8_t flyskyGpsId = 0;
uint8_t flyskyIbcId = 0;
uint8_t flyskyRpmId = 0;
bool reset_gps_dist = false;
bool reset_gps_alt = false;
bool reset_baro_alt = false;

uint32_t gps_update_tick = 0;
uint32_t ibc_update_tick = 0;
uint32_t rpm_update_tick = 0;
uint32_t pres_update_tick = 0;

const FlySkySensor flySkySensors[] = {
  // flysky start
  FS(IBDT_INT_VOLTAGE,           STR_SENSOR_BATT,       UNIT_VOLTS,   2),
  FS(IBDT_TEMPERATURE,           STR_SENSOR_TEMP1,    UNIT_CELSIUS, 1),
  FS(IBDT_ROTATION_SPEED,        STR_SENSOR_RPM,      UNIT_RAW,     0),
  FS(IBDT_EXT_VOLTAGE,           STR_SENSOR_A3,       UNIT_VOLTS,   2),
  FS(IBDT_GPS,                   STR_SENSOR_GPS,      UNIT_GPS,     2),
  
  FS(VIRTUAL_GPS_STAS,           STR_SENSOR_SATELLITES,         UNIT_RAW,     0),
  FS(VIRTUAL_GPS_TIME,           STR_SENSOR_GPSDATETIME,         UNIT_DATETIME,     0),
  FS(VIRTUAL_GPS_LAT,            STR_SENSOR_GPS,         UNIT_GPS_LATITUDE,     0),
  FS(VIRTUAL_GPS_LON,            STR_SENSOR_GPS,         UNIT_GPS_LONGITUDE,     0),
  FS(VIRTUAL_GPS_KMH,            STR_SENSOR_GSPD,         UNIT_KMH,     1),
  FS(VIRTUAL_GPS_PITCH,          STR_SENSOR_PITCH,         UNIT_DEGREE,     1),
  FS(VIRTUAL_GPS_ROLL,           STR_SENSOR_ROLL,         UNIT_DEGREE,     1),
  FS(VIRTUAL_GPS_YAW,            STR_SENSOR_YAW,         UNIT_DEGREE,     1),
  FS(VIRTUAL_GPS_DIST,           STR_SENSOR_DIST,         UNIT_METERS,     2),
  FS(VIRTUAL_GPS_ACC,            STR_SENSOR_ACC,         UNIT_RAW,     0),
  FS(VIRTUAL_GPS_SPEED,          STR_SENSOR_GSPD,       UNIT_SPEED,    2),
  FS(VIRTUAL_GPS_ALT,            STR_SENSOR_GPSALT,       UNIT_METERS,    0),
  FS(VIRTUAL_GPS_REL_ALT,        "G_RH",                  UNIT_METERS, 0),

  FS(IBDT_PRESSURE,              STR_SENSOR_PRES,     UNIT_RAW,     2),
  FS(VIRTUAL_ALT,                STR_SENSOR_ALT,      UNIT_METERS,  2),
  FS(VIRTUAL_REL_ALT,            "P_RH",      UNIT_METERS,  2),
  FS(IBDT_COMPASS,               "COMPASS",           UNIT_RAW,     2),
  FS(IBDT_IBC01,                 "IBC",               UNIT_RAW,     2),
  FS(VIRTUAL_IBC_VOLTS,          STR_SENSOR_BATT1_VOLTAGE,  UNIT_VOLTS, 1),
  FS(VIRTUAL_IBC_CURR,           STR_SENSOR_CURR,  UNIT_AMPS, 1),
  FS(VIRTUAL_IBC_CAPA,           STR_SENSOR_CAPACITY,  UNIT_MAH, 0),
  FS(VIRTUAL_IBC_AVG_CURR,      "AVG_CURR",  UNIT_AMPS, 1),

  FS(IBDT_REDUNDANT_RECEIVER,    "REDUNDANT",         UNIT_RAW,     0),
  FS(IBDT_REDUNDANT_RECEIVER_E,  "REDUNDANT_E",       UNIT_RAW,     0),
  FS(IBDT_FLIGHT,                "FLIGHT",            UNIT_RAW,     0),
  FS(IBDT_TX_VOLTAGE,            STR_SENSOR_TXV,      UNIT_VOLTS,   2),
  FS(IBDT_PRESSURE_SENSOR,       "PRESSURE_SENSOR",   UNIT_RAW,     2),
  FS(IBDT_AIRSPEED_SENSOR,       "AIRSPEED_SENSOR",   UNIT_RAW,     2),
  FS(IBDT_EXT_BVD,               "BVD",               UNIT_VOLTS,   2),
  FS(IBDT_HUB_1_PORTS_PWM,         "HUB_PWM1",          UNIT_RAW,     0),
  FS(IBDT_HUB_2_PORTS_PWM,         "HUB_PWM2",          UNIT_RAW,     0),
  FS(IBDT_HUB_3_PORTS_PWM,         "HUB_PWM3",          UNIT_RAW,     0),
  FS(IBDT_HUB_4_PORTS_PWM,         "HUB_PWM4",          UNIT_RAW,     0),
  FS(IBDT_HUB_5_PORTS_PWM,         "HUB_PWM5",          UNIT_RAW,     0),
  FS(IBDT_HUB_6_PORTS_PWM,         "HUB_PWM6",          UNIT_RAW,     0),
  FS(IBDT_HUB_7_PORTS_PWM,         "HUB_PWM7",          UNIT_RAW,     0),
  FS(IBDT_8_PORTS_PWM_ADAPTER,     "ADAPTER_8",         UNIT_RAW,     0),
  FS(IBDT_9_PORTS_PWM_ADAPTER,     "ADAPTER_9",         UNIT_RAW,     0),
  FS(IBDT_10_PORTS_PWM_ADAPTER,    "ADAPTER_10",        UNIT_RAW,     0),
  FS(IBDT_11_PORTS_PWM_ADAPTER,    "ADAPTER_11",        UNIT_RAW,     0),
  FS(IBDT_12_PORTS_PWM_ADAPTER,    "ADAPTER_12",        UNIT_RAW,     0),
  FS(IBDT_13_PORTS_PWM_ADAPTER,    "ADAPTER_13",        UNIT_RAW,     0),
  FS(IBDT_14_PORTS_PWM_ADAPTER,    "ADAPTER_14",        UNIT_RAW,     0),
  FS(IBDT_15_PORTS_PWM_ADAPTER,    "ADAPTER_15",        UNIT_RAW,     0),
  FS(IBDT_16_PORTS_PWM_ADAPTER,    "ADAPTER_16",        UNIT_RAW,     0),
  FS(IBDT_17_PORTS_PWM_ADAPTER,    "ADAPTER_17",        UNIT_RAW,     0),
  FS(IBDT_HUB_1_PORTS_HUB,         "HUB1",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_2_PORTS_HUB,         "HUB2",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_3_PORTS_HUB,         "HUB3",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_4_PORTS_HUB,         "HUB4",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_5_PORTS_HUB,         "HUB5",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_6_PORTS_HUB,         "HUB6",              UNIT_VOLTS,     3),
  FS(IBDT_HUB_7_PORTS_HUB,         "HUB7",              UNIT_VOLTS,     3),
  FS(IBDT_DIGITAL_SERVO,           "DSERVO",            UNIT_RAW,     0),
  FS(IBDT_SNR,                   STR_SENSOR_RX_SNR,   UNIT_DB,      0),
  FS(IBDT_BK_NOISE,              STR_SENSOR_RX_NOISE, UNIT_DBM,     0),
  FS(IBDT_RSSI,                  STR_SENSOR_RSSI,     UNIT_DBM,     0),
  FS(IBDT_SERVO_HUB,               "SERVO_HUB",         UNIT_RAW,     0),
  FS(IBDT_SIGNAL_STRENGTH,       STR_SENSOR_RX_QUALITY, UNIT_PERCENT, 0),
  FS(IBDT_NONE,                  NULL,                UNIT_RAW,     0),
};

inline int setFlyskyTelemetryValue( int16_t type, uint8_t instance, int32_t value, uint32_t unit, uint32_t prec)
{
  return setTelemetryValue(PROTOCOL_TELEMETRY_FLYSKY_IBUS2, type, 0, instance, value, unit, prec );
}


void processFlySkyIbus2AFHDS3Sensor(const uint8_t * packet, uint8_t len )
{
  uint16_t type = (packet[0] << 8) | packet[1];
  uint8_t id = packet[2];
  int32_t value=0;
  type = type ? type : IBDT_INT_VOLTAGE;  // Remapped

  // TRACE("[IBUS] type x%02X, len %d", type, len);
  if(len == 1)
  {
    value = packet[3];  // type 0xfe SIGNAL_STRENGTH
  }
  else if (len == 2)
  {
      value = (packet[4] << 8) | packet[3];
  }
  else if(len == 4)
  {
    value = (packet[6] << 24) | (packet[5] << 16) | (packet[4] << 8) | packet[3]; // PRESSURE
  } else {
    // gps & ibc01
  }

  if (IBDT_BK_NOISE == type || IBDT_RSSI == type)
  {
    value  = -value;
  }  

  if ( (IBDT_EXT_VOLTAGE == type) )
  {
    if ( id&0x80 )
    {
      type = IBDT_EXT_BVD;
    }
  }
  else if(IBDT_RSSI == type || IBDT_BK_NOISE == type || IBDT_SNR == type)
  {
    if( value>=0 )
      value = (value+2)/4;
    else
      value = (value-2)/4;
  }
  else if (IBDT_SIGNAL_STRENGTH == type)
  {
    telemetryData.rssi.set( value );
    if (value > 0) telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  }
  else if (IBDT_PRESSURE == type)
  {
      int32_t alt = getALT(value);
      // int16_t temp = (value >> 19);
      if (reset_baro_alt) {
        reset_baro_alt = !reset_baro_alt;
        StartPos.Boar_Altitude = alt;
      } else {
        int32_t RH =  alt - StartPos.Boar_Altitude;
        type = VIRTUAL_REL_ALT;
        sendFlyskytelemtry(type, id, RH);
        // TRACE("[IBUS2] RH = %d", RH);
      }

      type = VIRTUAL_ALT;
      sendFlyskytelemtry(type, id, alt);

      pres_update_tick = timersGetMsTick();
      value &= PRESSURE_MASK;
      type = IBDT_PRESSURE;
      sendFlyskytelemtry(type, id, value);
      return;
  } else if (IBDT_ROTATION_SPEED == type) {
    // Adjust the rotational speed based on the number of blades
    // RPM = value;
    flyskyRpmId = id;
    rpm_update_tick = timersGetMsTick();
  } else if (IBDT_GPS == type) {
    flyskyGpsId = id;
    flyskyIbus2GPS(&packet[3], len, id);
    return;
  } else if (IBDT_IBC01 == type) {
    flyskyIbus2IBC(&packet[3], len, id);
    return;
  }

  if(IBDT_TEMPERATURE == type)
  {
    value -= 400; // Temperature sensors have 40 degree offset
  }
 
  sendFlyskytelemtry(type, id, value);
}

void flyskyIbus2GPS(const uint8_t * pData, uint8_t len, uint8_t id) {
  static uint16_t	PSValue;
  uint16_t n = 0;
  int32_t value = 0;
  uint16_t type = 0;
  static bool get_start = true; // Get the location at startup

  gps_update_tick = timersGetMsTick();

  PSValue = pData[n] | (pData[n+1] << 8);
  if( GPS_MSG_TYPE_PACK1 == (PSValue & 0x000f) )
  {
    GPSData.NbSatellites 	= (PSValue >> 4) & 0x003F ;	n++;
    GPSData.PositionStatus 	= (PSValue >> 10) & 0x003F ;n++;
    value = GPSData.NbSatellites;
    type = VIRTUAL_GPS_STAS;
    sendFlyskytelemtry(type, id, value);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.DateDay 		= (PSValue & 0x001F);
    GPSData.DateMonth 		= (PSValue >> 5) & 0x000F;	n++;
    GPSData.DateYear 		= (PSValue >> 9) & 0x007F;	n++;
    value = GPSData.DateYear << 24 | GPSData.DateMonth << 16 | GPSData.DateDay << 8 | 0xff ;
    type = VIRTUAL_GPS_TIME;
    sendFlyskytelemtry(type, id, value);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.Speed 			= (PSValue);					n++;n++;
    value = GPSData.Speed;
    type = VIRTUAL_GPS_SPEED;
    sendFlyskytelemtry(type, id, value);

    GPSData.UTCTime 		= pData[n] | (pData[n+1] << 8) | (pData[n+2] << 16) | (pData[n+3] << 24);
    n++;n++;n++;n++;
    // value = GPSData.UTCTime;
    uint32_t total_seconds = GPSData.UTCTime;
    uint8_t sec = total_seconds % 60;
    uint32_t total_minutes = total_seconds / 60;
    uint8_t min = total_minutes % 60;
    uint8_t hour = total_minutes / 60;
    type = VIRTUAL_GPS_TIME;
    value = 0x00;
    value = hour << 24 | min << 16 | sec << 8;
    sendFlyskytelemtry(type, id, value);
    // TRACE("[IBUS2] utc %d %d %d %d ", GPSData.DateYear, GPSData.DateMonth, GPSData.DateDay, GPSData.UTCTime);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.Altitude 		= (PSValue);					n++;n++;
    type = VIRTUAL_GPS_ALT;
    value = PSValue;
    sendFlyskytelemtry(type, id, value);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.Direction 		= (PSValue);					n++;n++;

    type = VIRTUAL_GPS_REL_ALT;
    if (!get_start) {
      value = GpsSensorHeighthGet();
    } else {
      value = 0;
    }
    sendFlyskytelemtry(type, id, value);
  }
  else if( GPS_MSG_TYPE_PACK2 == (PSValue & 0x000f) )
  {
    GPSData.Pitch 			= (PSValue >> 4) & 0x0FFF ;		n++;n++;
    value = GPSData.Pitch - 1800;
    type = VIRTUAL_GPS_PITCH;
    sendFlyskytelemtry(type, id, value);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.Roll 			= (PSValue);					n++;n++;
    value = GPSData.Roll - 1800;
    type = VIRTUAL_GPS_ROLL;
    sendFlyskytelemtry(type, id, value);

    PSValue = pData[n] | (pData[n+1] << 8);
    GPSData.Yaw 			= (PSValue);					n++;n++;
    value = 3600 - GPSData.Yaw;
    type = VIRTUAL_GPS_YAW;
    sendFlyskytelemtry(type, id, value);

    GPSData.Latitude 		= (pData[n] | (pData[n+1] << 8) | (pData[n+2] << 16) | (pData[n+3] << 24));
    value = GPSData.Latitude;
    type = VIRTUAL_GPS_LAT;
    sendFlyskytelemtry(type, id, value);

    n++;n++;n++;n++;
    GPSData.Longitude 		= (pData[n] | (pData[n+1] << 8) | (pData[n+2] << 16) | (pData[n+3] << 24));
    value = GPSData.Longitude;
    type = VIRTUAL_GPS_LON;
    sendFlyskytelemtry(type, id, value);

    
    type = VIRTUAL_GPS_DIST;
    if (!get_start) {
      value = GpsSensorDistanceGet();
    } else {
      value = 0;
    }
    sendFlyskytelemtry(type, id, value);
    
  }
    
  if (get_start && GPSData.PositionStatus == 3) {
    get_start = false;
    StartPos.Altitude = GPSData.Altitude;
    StartPos.Latitude = GPSData.Latitude;
    StartPos.Longitude = GPSData.Longitude;
  } else {
    if (reset_gps_alt) {
      StartPos.Altitude = GPSData.Altitude;
      reset_gps_alt = false;
    }
    if (reset_gps_dist) {
      StartPos.Latitude = GPSData.Latitude;
      StartPos.Longitude = GPSData.Longitude;
      reset_gps_dist = false;
    }
  }
}

void flyskyIbus2IBC(const uint8_t * pData, uint8_t len, uint8_t id) {
  int32_t value = 0;
  uint16_t type = 0;
  memcpy(&IBCDate, pData, sizeof(IBCDate));

  ibc_update_tick = timersGetMsTick();
  
  type = VIRTUAL_IBC_VOLTS;
  value = IBCDate.Voltage;
  sendFlyskytelemtry(type, id, value);

  type = VIRTUAL_IBC_CURR;
  value = IBCDate.Current;
  sendFlyskytelemtry(type, id, value);

  type = VIRTUAL_IBC_CAPA;
  value = IBCDate.UsedCapacity;
  sendFlyskytelemtry(type, id, value);

  type = VIRTUAL_IBC_AVG_CURR;
  value = IBCDate.AverageCurrent;
  sendFlyskytelemtry(type, id, value);

  flyskyIbcId = id;
}

uint8_t getIbcVoltags(void) {
  return IBCDate.Voltage;
}

void sendFlyskytelemtry(uint16_t type, uint8_t id, int32_t value) {
  for (const FlySkySensor * sensor = flySkySensors; sensor->type; sensor++)
  {
    if (sensor->type != type) continue;

    if (sensor->unit == UNIT_VOLTS) value = (int16_t) value; // Voltage types are unsigned 16bit integers

    //  Remapped to single GPS sensor:
    if (type == VIRTUAL_GPS_LON) type = VIRTUAL_GPS_LAT;
    setFlyskyTelemetryValue(type, id, value, sensor->unit, sensor->precision);
    return;
  }
}

const FlySkySensor * getFlySkyIbus2Sensor(uint16_t id)
{
  for (const FlySkySensor * sensor = flySkySensors; sensor->type; sensor++) {
    if (id == sensor->type)
      return sensor;
  }
  return nullptr;
}

void flySkyIbus2SetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const FlySkySensor * sensor = getFlySkyIbus2Sensor(id);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    uint8_t prec = min<uint8_t>(2, sensor->precision);
    telemetrySensor.init(sensor->name, unit, prec);

    if (unit == UNIT_RPMS) {
      telemetrySensor.custom.ratio = 1;
      telemetrySensor.custom.offset = 1;
    } else if (unit == UNIT_GPS_LATITUDE || unit == UNIT_GPS_LONGITUDE) {
      telemetrySensor.unit = UNIT_GPS;
    }
  }
  else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}

double deg2rad(double deg) {
  return deg * M_PI / 180.0;
}

static double GetDistance(double lat1, double lng1, double lat2, double lng2)
{
  lat1 =lat1/1000000; lng1= lng1/1000000;
  lat2 =lat2/1000000; lng2= lng2/1000000;
  
  double radLat1 = deg2rad(lat1);
  double radLat2 = deg2rad(lat2);
  
  double a = radLat1 - radLat2;
  double b = deg2rad(lng1) - deg2rad(lng2);
  
  float s = 2.0f * asin(sqrt(pow(sin(a/2),2) 
                    + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
  
  s = s * EARTH_RADIUS * 100; //cm
  return s;
}

int32_t GpsSensorDistanceGet()
{
  double lats ,lngs, late, lnge;
  
  lats = StartPos.Latitude;
  lngs = StartPos.Longitude;        
  late = GPSData.Latitude;
  lnge = GPSData.Longitude;
  return GetDistance(lats, lngs, late, lnge);
}

int32_t GpsSensorHeighthGet() 
{
  return GPSData.Altitude - StartPos.Altitude;
}

void flySkyIbus2CalGpsGyro(uint8_t* packet, uint8_t* len) 
{
  Ibus2DevPara.type = IBUS2_CALIB_SET_PARAM;
  Ibus2DevPara.id = flyskyGpsId;
  Ibus2DevPara.ParameterData[0] = 0x55;
  Ibus2DevPara.ParameterData[1] = 0x01;
  Ibus2DevPara.ParameterData[2] = 0x01;
  setIbus2Param(packet);
  *len = 22;
}

void flySkyIbus2CalGpsAlt() 
{
  reset_gps_alt = true;
  reset_baro_alt = true;
}

void flySkyIbus2CalGpsDist() 
{
  reset_gps_dist = true;
}

void flySkyIbus2ReadParamRPM(uint8_t* packet, uint8_t* len) 
{
  Ibus2DevPara.type = IBUS2_CALIB_RPM;
  Ibus2DevPara.id = flyskyRpmId;
  getIbus2Param(packet);
  *len = 6;
}

void flySkyIbus2CalibIBC(uint8_t* packet, uint8_t* len, short voltags) 
{
  Ibus2DevPara.type = IBUS2_CALIB_SET_PARAM;
  Ibus2DevPara.id = flyskyIbcId;
  Ibus2DevPara.ParameterData[0] = (uint8_t)(voltags & 0xff);
  Ibus2DevPara.ParameterData[1] = (uint8_t)(voltags >> 8);
  setIbus2Param(packet);
  *len = 22;
}

void setIbus2Param(uint8_t* pData) {
  uint8_t n = 0;
  pData[n++] = ( uint8_t )( RX_CMD_CODE_IBUS2_SET_PARAM );
	pData[n++] = ( uint8_t )( RX_CMD_CODE_IBUS2_SET_PARAM >> 8 );
	pData[n++] = 19; //data length
	pData[n++] = Ibus2DevPara.id;
	pData[n++] = Ibus2DevPara.type;
	pData[n++] = Ibus2DevPara.type >> 8;
	pData[n++] = Ibus2DevPara.ParameterData[0];
	pData[n++] = Ibus2DevPara.ParameterData[1];
	pData[n++] = Ibus2DevPara.ParameterData[2];
	pData[n++] = Ibus2DevPara.ParameterData[3];
	pData[n++] = Ibus2DevPara.ParameterData[4];
	pData[n++] = Ibus2DevPara.ParameterData[5];
	pData[n++] = Ibus2DevPara.ParameterData[6];
	pData[n++] = Ibus2DevPara.ParameterData[7];
	pData[n++] = Ibus2DevPara.ParameterData[8];
	pData[n++] = Ibus2DevPara.ParameterData[9];
	pData[n++] = Ibus2DevPara.ParameterData[10];
	pData[n++] = Ibus2DevPara.ParameterData[11];
	pData[n++] = Ibus2DevPara.ParameterData[12];
	pData[n++] = Ibus2DevPara.ParameterData[13];
	pData[n++] = Ibus2DevPara.ParameterData[14];
	pData[n++] = Ibus2DevPara.ParameterData[15];
}

void getIbus2Param(uint8_t* pData) {
  uint8_t n = 0;
  pData[n++] = ( uint8_t )(RX_CMD_CODE_IBUS2_GET_PARAM);
  pData[n++] = ( uint8_t )(RX_CMD_CODE_IBUS2_GET_PARAM >> 8);
  pData[n++] = 3;//data length
  pData[n++] = Ibus2DevPara.id;
	pData[n++] = Ibus2DevPara.type;
	pData[n++] = Ibus2DevPara.type >> 8;				  
}

bool ibc_state = false; 
void Ibus2ParamCheck(uint8_t* packet, uint8_t len) {
  if (len != 4) {
    return;
  }
  uint8_t id = packet[0];
  uint8_t code = packet[3];
  if (id == flyskyIbcId) {
    ibc_state = code;
  } else if (id == flyskyGpsId) {
    // todo
  } else if (id == flyskyRpmId) {
    // todo
  }
} 

bool getIbus2IbcState() {
  return ibc_state;
}

uint8_t flyskyIbus2SensorOnLine() {
  uint32_t now = timersGetMsTick();
  uint8_t sensor_on_line = 0;
  if (now - ibc_update_tick < 2000) {
    sensor_on_line |= 1 << IBUS2_SENSOR_IBC;
  }
  if (now - gps_update_tick < 2000) {
    sensor_on_line |= 1 << IBUS2_SENSOR_GPS;
  }
  if (now - rpm_update_tick < 2000) {
    sensor_on_line |= 1 << IBUS2_SENSOR_RPM;
  }
  if (now - pres_update_tick < 2000) {
    sensor_on_line |= 1 << IBUS2_SENSOR_PRES;
  }

  return sensor_on_line;
}