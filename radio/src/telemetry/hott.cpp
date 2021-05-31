/*
 * Copyright (C) OpenTX
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

/* Telemetry 
packet[0]    = TX RSSI value
packet[1]    = TX LQI value
packet[2]    = Telemetry type
packet[3]    = Telemetry page
packet[4-13] = Telemetry data

Telemetry type: 0x00 RX, 0x09 Vario, 0x0A GPS, 0x0B not seen, 0x0C ESC, 0x0D GAM, 0x0E EAM, 0x80 Text config mode
Telemetry page: 0x00..0x04 for all telemetry except Text config mode which is 0x00..0x12. The pages can be present or not, they also do not have to follow each others.

Here is a description of the RX telemetry:
 [2] = 0x00 -> Telemetry type RX
 [3] = 0x00 -> Telemetry page 0
 [4] = ?? looks to be a copy of [7]
 [5] = RX_Voltage*10 in V
 [6] = Temperature-20 in °C
 [7] = RX_RSSI CC2500 formated (a<128:a/2-71dBm, a>=128:(a-256)/2-71dBm)
 [8] = RX_LQI in %
 [9] = RX_MIN_Voltage*10 in V
 [10,11] = [11]*256+[10]=max lost packet time in ms, max value seems 2s=0x7D0
 [12] = 0x00 ??
 [13] = ?? looks to be a copy of [7]
*/

struct HottSensor
{
  const uint16_t id;
  const char * name;
  const TelemetryUnit unit;
  const uint8_t precision;
};

// telemetry frames
enum
{
  HOTT_PAGE_00 = 0x00,
  HOTT_PAGE_01 = 0x01,
  HOTT_PAGE_02 = 0x02,
  HOTT_PAGE_03 = 0x03,
  HOTT_PAGE_04 = 0x04,
};

enum
{
  HOTT_TELEM_RX    = 0x00,
  HOTT_TELEM_VARIO = 0x09,
  HOTT_TELEM_GPS   = 0x0A,
  HOTT_TELEM_ESC   = 0x0C,
  HOTT_TELEM_GAM   = 0x0D,
  HOTT_TELEM_EAM   = 0x0E,
  HOTT_TELEM_TEXT  = 0x80,
};

// telemetry sensors ID
enum
{
  HOTT_ID_RX_VOLTAGE = 0x0004,  // RX_Batt Voltage
  HOTT_ID_TEMP1      = 0x0005,  // RX Temperature sensor
  HOTT_ID_VARIO      = 0x0006,  // Vario sensor
  HOTT_ID_ALT        = 0x0007,  // Alt sensor
  HOTT_ID_HDG        = 0x0008,  // Heading sensor
  HOTT_ID_GSPD       = 0x0008,  // Ground speed sensor
  HOTT_ID_GPS_LAT_LONG = 0x0009,  // GPS sensor
  HOTT_ID_GPS_COUNT  = 0x000A,  // GPS sat count
  HOTT_ID_CELS       = 0x000B,  // Cels L sensor
  HOTT_ID_CELS_H     = 0x000C,  // Cels H sensor
  HOTT_ID_VOLT1      = 0x000D,  // Voltage sensor
  HOTT_ID_VOLT2      = 0x000E,  // Voltage sensor
  HOTT_ID_AMP1       = 0x000F,  // Amp sensor
  HOTT_ID_AMP2       = 0x0010,  // Amp sensor
  HOTT_ID_RPM1       = 0x0011,  // RPM sensor
  HOTT_ID_RPM2       = 0x0012,  // RPM sensor
  HOTT_ID_ESC_VIN    = 0x0013,  // ESC Volt
  HOTT_ID_ESC_CUR    = 0x0014,  // ESC Amp
  HOTT_ID_ESC_TBEC   = 0x0015,  // BEC Temp
  HOTT_ID_ESC_BCUR   = 0x0016,  // BEC Cur
  HOTT_ID_ESC_VBEC   = 0x0017,  // BEC Volt
  HOTT_ID_BCAPA      = 0x0018,  // Batt capacity
  HOTT_ID_TEMP2      = 0x0019,  // Temperature sensor 2
  HOTT_ID_AIR_SPEED  = 0x001A,
  HOTT_ID_VOLT3      = 0x001B,  // Voltage sensor
  HOTT_ID_FUEL       = 0x001C,  // Fuel percentage

  HOTT_TX_RSSI_ID    = 0xFF00,  // Pseudo id outside 1 byte range of Hott sensors
  HOTT_TX_LQI_ID     = 0xFF01,  // Pseudo id outside 1 byte range of Hott sensors
  HOTT_RX_RSSI_ID    = 0xFF02,  // Pseudo id outside 1 byte range of Hott sensors
  HOTT_RX_LQI_ID     = 0xFF03,  // Pseudo id outside 1 byte range of Hott sensors
};

const HottSensor hottSensors[] = {
  //HOTT_TELEM_RX
  {HOTT_ID_RX_VOLTAGE,   STR_SENSOR_BATT,               UNIT_VOLTS,             1},  // RX_Batt Voltage
  {HOTT_ID_TEMP1,        STR_SENSOR_TEMP1,              UNIT_CELSIUS,           0},  // Temperature sensor 1
  {HOTT_ID_TEMP2,        STR_TEMP2,              UNIT_CELSIUS,           0},  // Temperature sensor 2
  {HOTT_ID_VARIO,        STR_VSPD,               UNIT_METERS_PER_SECOND, 2},  // Vario sensor
  {HOTT_ID_ALT,          STR_ALT,                UNIT_METERS,            0},  // Alt sensor
  {HOTT_ID_HDG,          STR_HDG,                UNIT_DEGREE,            0},  // Heading sensor
  {HOTT_ID_GSPD,         STR_GSPD,               UNIT_KMH,               0},  // Ground speed sensor
  {HOTT_ID_GPS_LAT_LONG, STR_GPS,                UNIT_GPS,               0},  // GPS position
  {HOTT_ID_GPS_COUNT,    STR_SATELLITES,         UNIT_RAW,               0},  // GPS sat count
  {HOTT_ID_VOLT1,        STR_A1,                 UNIT_VOLTS,             1},  // Voltage sensor 1
  {HOTT_ID_VOLT2,        STR_A2,                 UNIT_VOLTS,             1},  // Voltage sensor 2
  {HOTT_ID_VOLT3,        STR_A3,                 UNIT_VOLTS,             1},  // Voltage sensor 3
  {HOTT_ID_AMP1,         STR_CURR,               UNIT_AMPS,              1},  // Amp sensor 1
  {HOTT_ID_AMP2,         STR_CURR,               UNIT_AMPS,              1},  // Amp sensor 2
  {HOTT_ID_RPM1,         STR_RPM,                UNIT_RPMS,              0},  // RPM1
  {HOTT_ID_RPM2,         STR_RPM2,               UNIT_RPMS,              0},  // RPM2
  {HOTT_ID_ESC_VIN,      STR_ESC_VIN,            UNIT_VOLTS,             1},  // ESC VIN
  {HOTT_ID_BCAPA,        STR_SMART_BAT_CAPACITY, UNIT_MAH,               0},  // Batt Capa
  {HOTT_ID_ESC_CUR,      STR_ESC_CUR,            UNIT_AMPS,              1},  // ESC Amp
  {HOTT_ID_ESC_TBEC,     STR_ESC_TBEC,           UNIT_CELSIUS,           0},  // ESC BEC Temp
  {HOTT_ID_ESC_BCUR,     STR_ESC_BCUR,           UNIT_AMPS,              1},  // ESC BEC Amp
  {HOTT_ID_ESC_VBEC,     STR_ESC_VBEC,           UNIT_VOLTS,             1},  // ESC BEC Volt
  {HOTT_ID_CELS,         STR_CELLS,              UNIT_CELLS,             2},  // Batt cells
  {HOTT_ID_CELS_H,       STR_CELLS,              UNIT_CELLS,             2},  // Batt H cells
  {HOTT_ID_AIR_SPEED,    STR_ASPD,               UNIT_KMH,               0},  // Air speed
  {HOTT_ID_FUEL,         STR_FUEL,               UNIT_PERCENT,           0},  // Fuel

  {HOTT_TX_RSSI_ID,      STR_SENSOR_TX_RSSI,            UNIT_DB,                0},  // Pseudo id outside 1 byte range of Hott sensors
  {HOTT_TX_LQI_ID,       STR_SENSOR_TX_QUALITY,         UNIT_RAW,               0},  // Pseudo id outside 1 byte range of Hott sensors
  {HOTT_RX_RSSI_ID,      STR_SENSOR_RSSI,               UNIT_DB,                0},  // RX RSSI
  {HOTT_RX_LQI_ID,       STR_SENSOR_RX_QUALITY,         UNIT_RAW,               0},  // RX LQI
  {0x00,           NULL,                    UNIT_RAW,               0},  // sentinel
};

const HottSensor * getHottSensor(uint16_t id)
{
  for (const HottSensor * sensor = hottSensors; sensor->id; sensor++) {
    if (id == sensor->id)
      return sensor;
  }
  return nullptr;
}

int16_t processHoTTdBm(uint8_t value)
{
  if (value >= 128) {
    value = 256 - value;
  }
  return value / 2 -71;
}

void processHottPacket(const uint8_t * packet)
{
  // Set TX RSSI Value
  setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_TX_RSSI_ID, 0, 0, processHoTTdBm(packet[0]), UNIT_RAW, 0);
  // Set TX LQI  Value
  setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_TX_LQI_ID, 0, 0, packet[1], UNIT_RAW, 0);

#if defined(LUA)
#define HOTT_MENU_NBR_PAGE 0x13
  // Config menu consists of the different telem pages put all together
  //   [3] = config mennu page 0x00 to 0x12.
  //   Page X [4] = seems like all the telem pages with the same value are going together to make the full config menu text. Seen so far 'a', 'b', 'c', 'd'
  //   Page X [5..13] = 9 ascii chars to be displayed, char is highlighted when ascii|0x80
  //   Screen display is 21 characters large which means that once the first 21 chars are filled go to the begining of the next line
  //   Menu commands are sent through TX packets:
  //     packet[28]= 0xXF=>no key press, 0xXD=>down, 0xXB=>up, 0xX9=>enter, 0xXE=>right, 0xX7=>left with X=0, 9=Vario, A=GPS, B=Unknown, C=ESC, D=GAM, or E=EAM
  //     packet[29]= 0xX1/0xX9 with X=0 or X counting 0,1,1,2,2,..,9,9
  if (Multi_Buffer && memcmp(Multi_Buffer, "HoTT", 4) == 0) {
    // HoTT Lua script is running
    if (Multi_Buffer[4] == 0xFF) {
      // Init
      memset(&Multi_Buffer[6], ' ', HOTT_MENU_NBR_PAGE * 9); // Clear text buffer
    }

    if (packet[2] == HOTT_TELEM_TEXT && packet[3] < HOTT_MENU_NBR_PAGE && (Multi_Buffer[5] & 0x80) && (Multi_Buffer[5] & 0x0F) >= 0x07) {
      Multi_Buffer[4] = packet[4];                             // Store detected sensors
      memcpy(&Multi_Buffer[6 + packet[3] * 9], &packet[5], 9); // Store the received page in the buffer
    }
    return;
  }
#endif

  struct HottGPSMinutes
  {
    uint16_t value: 14;
    uint16_t isNegative: 1;
    uint16_t spare: 1;
  };

  const HottSensor * sensor;
  int32_t value;
  static uint8_t prev_page = 0, prev_value = 0;
  static HottGPSMinutes min = {};
  int16_t deg = 0, sec = 0;

  switch (packet[2]) { // Telemetry type
    case HOTT_TELEM_RX:
      if (packet[3] == 0) { // Telemetry page: only page 0 is for RX
        // RX_VOLT
        value = packet[5];
        sensor = getHottSensor(HOTT_ID_RX_VOLTAGE);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_VOLTAGE, 0, 0, value, sensor->unit, sensor->precision);
        // RX_TEMP
        value = packet[6] - 20;
        sensor = getHottSensor(HOTT_ID_TEMP1);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP1, 0, 0, value, sensor->unit, sensor->precision);
        // RX_RSSI
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_RX_RSSI_ID, 0, 0, processHoTTdBm(packet[7]), UNIT_DB, 0);
        // RX_LQI
        telemetryData.rssi.set(packet[8]);
        if (packet[8] > 0)
          telemetryStreaming = TELEMETRY_TIMEOUT10ms;
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_RX_LQI_ID, 0, 0, packet[8], UNIT_RAW, 0);
      }
      break;

    case HOTT_TELEM_VARIO:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L240
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // packet[6 ] uint8_t altitude_L;          //#06 Altitude low uint8_t. In meters. A value of 500 means 0m
          // packet[7 ] uint8_t altitude_H;          //#07 Altitude high uint8_t
          value = packet[6] + (packet[7] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ALT, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);

          // packet[12] uint8_t climbrate_L;         //#12 Climb rate in m/s. Steps of 0.01m/s. Value of 30000 = 0.00 m/s
          // packet[13] uint8_t climbrate_H;         //#13 Climb rate in m/s
          value = packet[12] + (packet[13] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_VARIO);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);
          break;
        case HOTT_PAGE_02:
          // packet[12] uint8_t compass_direction;   //#42 Compass heading in 2� steps. 1 = 2�
          value = packet[12] * 2;
          sensor = getHottSensor(HOTT_ID_HDG);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_HDG, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);
          break;
        case HOTT_PAGE_03:
          break;
        case HOTT_PAGE_04:
          break;
      }
      break;

    case HOTT_TELEM_GPS:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L378
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // packet[7 ] uint8_t flight_direction; //#07 flight direction in 2 degreees/step (1 = 2degrees);
          value = packet[7] * 2;
          sensor = getHottSensor(HOTT_ID_HDG);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_HDG, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // packet[8 ] uint8_t gps_speed_L;   //08 km/h
          // packet[9 ] uint8_t gps_speed_H;   //#09
          value = packet[8] + (packet[9] << 8);
          sensor = getHottSensor(HOTT_ID_GSPD);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GSPD, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // packet[10] uint8_t pos_NS;        //#10 north = 0, south = 1
          // packet[11] uint8_t pos_NS_dm_L;   //#11 degree minutes ie N48�39�988
          // packet[12] uint8_t pos_NS_dm_H;   //#12
          // packet[13] uint8_t pos_NS_sec_L;  //#13 position seconds
          min.value = (int16_t) (packet[11] + (packet[12] << 8));
          if (packet[10] == 1) {
            min.isNegative = true;
          }
          break;

        case HOTT_PAGE_02:
          if (prev_page == ((HOTT_TELEM_GPS << 4) | HOTT_PAGE_01)) {
            // packet[4 ] uint8_t pos_NS_sec_H;  //#14
            deg = min.value / 100;
            min.value = min.value - deg * 100;
            sec = prev_value + (packet[4] << 8);
            value = deg * 1000000 + (min.value * 1000000 + sec * 100) / 60;
            if (min.isNegative) {
              value = -value;
            }
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_LAT_LONG, 0, HOTT_TELEM_GPS, value, UNIT_GPS_LATITUDE, 0);
          }

          // packet[5 ] uint8_t pos_EW;        //#15 east = 0, west = 1
          // packet[6 ] uint8_t pos_EW_dm_L;   //#16 degree minutes ie. E9�25�9360
          // packet[7 ] uint8_t pos_EW_dm_H;   //#17
          // packet[8 ] uint8_t pos_EW_sec_L;  //#18 position seconds
          // packet[9 ] uint8_t pos_EW_sec_H;  //#19
          min.value = (int16_t) (packet[6] + (packet[7] << 8));
          sec = (int16_t) (packet[8] + (packet[9] << 8));
          deg = min.value / 100;
          min.value = min.value - deg * 100;
          value = deg * 1000000 + (min.value * 1000000 + sec * 100) / 60;
          if (packet[5] == 1) {
            value = -value;
          }
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_LAT_LONG, 0, HOTT_TELEM_GPS, value, UNIT_GPS_LONGITUDE, 0);
          // packet[10] uint8_t home_distance_L;  //#20 meters
          // packet[11] uint8_t home_distance_H;  //#21

          // packet[12] uint8_t altitude_L;    //#22 meters. Value of 500 = 0m
          // packet[13] uint8_t altitude_H;    //#23
          value = packet[12] + (packet[13] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ALT, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // packet[4 ] uint8_t climbrate_L;   //#24 m/s 0.01m/s resolution. Value of 30000 = 0.00 m/s
          // packet[5 ] uint8_t climbrate_H;    //#25
          value = packet[4] + (packet[5] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_VARIO);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // packet[6 ] uint8_t climbrate3s;   //#26 climbrate in m/3s resolution, value of 120 = 0 m/3s
          // packet[7 ] uint8_t gps_satelites; //#27 sat count
          sensor = getHottSensor(HOTT_ID_GPS_COUNT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_COUNT, 0, HOTT_TELEM_GPS, packet[7], sensor->unit, sensor->precision);
          // packet[8 ] uint8_t gps_fix_char;  //#28 GPS fix character. display, 'D' = DGPS, '2' = 2D, '3' = 3D, '-' = no fix. Where appears this char???
          // packet[9 ] uint8_t home_direction;//#29 direction from starting point to Model position (2 degree steps)
          // packet[10] uint8_t angle_roll;    //#30 angle roll in 2 degree steps
          // packet[11] uint8_t angle_nick;    //#31 angle in 2degree steps
          // packet[12] uint8_t angle_compass; //#32 angle in 2degree steps. 1 = 2�, 255 = - 2� (1 uint8_t) North = 0�
          // packet[13] uint8_t gps_time_h;    //#33 UTC time hours
          break;

        case HOTT_PAGE_04:
          // packet[4 ] uint8_t gps_time_m;    //#34 UTC time minutes
          // packet[5 ] uint8_t gps_time_s;    //#35 UTC time seconds
          // packet[6 ] uint8_t gps_time_sss;  //#36 UTC time milliseconds
          // packet[7 ] uint8_t msl_altitude_L;//#37 mean sea level altitude
          // packet[8 ] uint8_t msl_altitude_H;//#38
          // packet[9 ] uint8_t vibration;     //#39 vibrations level in %
          // packet[10] uint8_t free_char1;    //#40 appears right to home distance
          // packet[11] uint8_t free_char2;    //#41 appears right to home direction
          // packet[12] uint8_t free_char3;    //#42 GPS ASCII D=DGPS 2=2D 3=3D -=No Fix
          break;
      }
      break;

    case HOTT_TELEM_ESC:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L454
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // packet[7 ] uint8_t input_v_L;      //#07 Input voltage low byte
          // packet[8 ] uint8_t input_v_H;      //#08
          value = packet[7] + (packet[8] << 8);
          sensor = getHottSensor(HOTT_ID_ESC_VIN);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_VIN, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          // packet[11] uint8_t batt_cap_L;     //#11 battery capacity in 10mAh steps
          // packet[12] uint8_t batt_cap_H;     //#12
          value = (packet[11] + (packet[12] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_BCAPA);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_BCAPA, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          // packet[13] uint8_t esc_temp;       //#13 ESC temperature
          value = packet[13] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP1, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_02:
          // packet[5 ] uint8_t current_L;      //#15 Current in 0.1 steps
          // packet[6 ] uint8_t current_H;      //#16
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_ESC_CUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_CUR, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          // packet[9 ] uint8_t rpm_L;          //#19 RPM in 10U/min steps
          // packet[10] uint8_t rpm_H;          //#20
          value = (packet[9] + (packet[10] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_RPM1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RPM1, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // packet[8 ] uint8_t bec_v;          //#28 BEC voltage
          value = packet[8];
          sensor = getHottSensor(HOTT_ID_ESC_VBEC);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_VBEC, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          // packet[10] uint8_t bec_current;    //#30 BEC current
          value = packet[10];
          sensor = getHottSensor(HOTT_ID_ESC_BCUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_BCUR, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_04:
          // packet[4 ] uint8_t bec_temp;       //#34 BEC temperature
          value = packet[4] - 20;
          sensor = getHottSensor(HOTT_ID_ESC_TBEC);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_TBEC, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          // packet[6 ] uint8_t motor_temp;     //#36 Motor or external sensor temperature
          value = packet[6] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP2, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;
      }
      break;

    case HOTT_TELEM_GAM:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L151
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          sensor = getHottSensor(HOTT_ID_CELS);
          // packet[7 ] uint8_t cell1_L;             //#07 cell 1 voltage lower value. 0.02V steps, 124=2.48V
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 0 << 16 | value, sensor->unit, sensor->precision);
          // packet[8 ] uint8_t cell2_L;             //#08
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 1 << 16 | value, sensor->unit, sensor->precision);
          // packet[9 ] uint8_t cell3_L;             //#09
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 2 << 16 | value, sensor->unit, sensor->precision);
          // packet[10] uint8_t cell4_L;             //#10
          value = packet[10] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 3 << 16 | value, sensor->unit, sensor->precision);
          // packet[11] uint8_t cell5_L;             //#11
          value = packet[11] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 4 << 16 | value, sensor->unit, sensor->precision);
          // packet[12] uint8_t cell6_L;             //#12
          value = packet[12] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_GAM, 5 << 16 | value, sensor->unit, sensor->precision);
          // packet[13] uint8_t batt1_L;             //#13 battery 1 voltage LSB value. 0.1V steps. 50 = 5.5V
          break;

        case HOTT_PAGE_02:
          if (prev_page == ((HOTT_TELEM_GAM << 4) | HOTT_PAGE_01)) {
            // packet[4 ] uint8_t batt1_H;             //#14
            value = prev_value + (packet[4] << 8);
            sensor = getHottSensor(HOTT_ID_VOLT2);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          }
          // packet[5 ] uint8_t batt2_L;             //#15 battery 2 voltage LSB value. 0.1V steps. 50 = 5.5V
          // packet[6 ] uint8_t batt2_H;             //#16
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_VOLT3);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT3, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[7 ] uint8_t temperature1;        //#17 temperature 1. offset of 20. a value of 20 = 0�C
          value = packet[7] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[8 ] uint8_t temperature2;        //#18 temperature 2. offset of 20. a value of 20 = 0�C
          value = packet[8] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[9 ] uint8_t fuel_procent;        //#19 Fuel capacity in %. Values 0--100, graphical display ranges: 0-25% 50% 75% 100%
          value = packet[9];
          sensor = getHottSensor(HOTT_ID_FUEL);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_FUEL, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[10] uint8_t fuel_ml_L;           //#20 Fuel in ml scale. Full = 65535!
          // packet[11] uint8_t fuel_ml_H;           //#21
          // packet[12] uint8_t rpm_L;               //#22 RPM in 10 RPM steps. 300 = 3000rpm
          // packet[13] uint8_t rpm_H;               //#23
          value = (packet[12] + (packet[13] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_RPM1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RPM1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // packet[4 ] uint8_t altitude_L;          //#24 altitude in meters. offset of 500, 500 = 0m
          // packet[5 ] uint8_t altitude_H;          //#25
          value = packet[4] + (packet[5] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ALT, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[6 ] uint8_t climbrate_L;         //#26 climb rate in 0.01m/s. Value of 30000 = 0.00 m/s
          // packet[7 ] uint8_t climbrate_H;         //#27
          value = packet[6] + (packet[7] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_VARIO);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[8 ] uint8_t climbrate3s;         //#28 climb rate in m/3sec. Value of 120 = 0m/3sec
          // packet[9 ] uint8_t current_L;           //#29 current in 0.1A steps
          // packet[10] uint8_t current_H;           //#30
          value = packet[9] + (packet[10] << 8);
          sensor = getHottSensor(HOTT_ID_AMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_AMP1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[11] uint8_t main_voltage_L;      //#31 Main power voltage using 0.1V steps
          // packet[12] uint8_t main_voltage_H;      //#32
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_VOLT1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[13] uint8_t batt_cap_L;          //#33 used battery capacity in 10mAh steps
          break;

        case HOTT_PAGE_04:
          // packet[4 ] uint8_t batt_cap_H;          //#34
          if (prev_page == ((HOTT_TELEM_GAM << 4) | HOTT_PAGE_03)) {
            value = (prev_value + (packet[4] << 8)) * 10;
            sensor = getHottSensor(HOTT_ID_BCAPA);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_BCAPA, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          }
          // packet[5 ] uint8_t speed_L;             //#35 (air?) speed in km/h(?) we are using ground speed here per default
          // packet[6 ] uint8_t speed_H;             //#36
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_AIR_SPEED);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_AIR_SPEED, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[7 ] uint8_t min_cell_volt;       //#37 minimum cell voltage in 2mV steps. 124 = 2,48V
          // packet[8 ] uint8_t min_cell_volt_num;   //#38 number of the cell with the lowest voltage
          // packet[9 ] uint8_t rpm2_L;              //#39 RPM in 10 RPM steps. 300 = 3000rpm
          // packet[10] uint8_t rpm2_H;              //#40
          value = (packet[9] + (packet[10] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_RPM2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RPM2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          // packet[11] uint8_t general_error_number;//#41 Voice error == 12. TODO: more docu
          // packet[12] uint8_t pressure;            //#42 Pressure up to 16bar. 0,1bar scale. 20 = 2bar
          break;
      }
      break;

    case HOTT_TELEM_EAM:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L288
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          sensor = getHottSensor(HOTT_ID_CELS);
          // packet[7 ] uint8_t cell1_L;             //#07 cell 1 voltage lower value. 0.02V steps, 124=2.48V
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 0 << 16 | value, sensor->unit, sensor->precision);
          // packet[8 ] uint8_t cell2_L;             //#08
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 1 << 16 | value, sensor->unit, sensor->precision);
          // packet[9 ] uint8_t cell3_L;             //#09
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 2 << 16 | value, sensor->unit, sensor->precision);
          // packet[10] uint8_t cell4_L;             //#10
          value = packet[10] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 3 << 16 | value, sensor->unit, sensor->precision);
          // packet[11] uint8_t cell5_L;             //#11
          value = packet[11] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 4 << 16 | value, sensor->unit, sensor->precision);
          // packet[12] uint8_t cell6_L;             //#12
          value = packet[12] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 5 << 16 | value, sensor->unit, sensor->precision);
          // packet[13] uint8_t cell7_L;             //#13
          // value = packet[13] << 1;
          // if(value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 6 << 16 | value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_02:
          sensor = getHottSensor(HOTT_ID_CELS);
          // packet[4 ] uint8_t cell1_H;             //#14 cell 1 voltage high value. 0.02V steps, 124=2.48V
          value = packet[4] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 0 << 16 | value, sensor->unit, sensor->precision);
          // packet[5 ] uint8_t cell2_H;             //#15
          value = packet[5] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 1 << 16 | value, sensor->unit, sensor->precision);
          // packet[6 ] uint8_t cell3_H;             //#16
          value = packet[6] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 2 << 16 | value, sensor->unit, sensor->precision);
          // packet[7 ] uint8_t cell4_H;             //#17
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 3 << 16 | value, sensor->unit, sensor->precision);
          // packet[8 ] uint8_t cell5_H;             //#18
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 4 << 16 | value, sensor->unit, sensor->precision);
          // packet[9 ] uint8_t cell6_H;             //#19
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS_H, 0, HOTT_TELEM_EAM, 5 << 16 | value, sensor->unit, sensor->precision);
          // packet[10] uint8_t cell7_H;             //#20
          //value = packet[10] << 1;
          //if(value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_CELS, 0, HOTT_TELEM_EAM, 13 << 16 | value, sensor->unit, sensor->precision);
          // packet[11] uint8_t batt1_voltage_L;     //#21 battery 1 voltage lower value in 100mv steps, 50=5V. optionally cell8_L value 0.02V steps
          // packet[12] uint8_t batt1_voltage_H;     //#22
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_VOLT2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT2, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[13] uint8_t batt2_voltage_L;     //#23 battery 2 voltage lower value in 100mv steps, 50=5V. optionally cell8_H value. 0.02V steps
          break;

        case HOTT_PAGE_03:
          if (prev_page == ((HOTT_TELEM_EAM << 4) | HOTT_PAGE_02)) {
            // packet[4 ] uint8_t batt2_voltage_H;     //#24
            value = prev_value + (packet[4] << 8);
            sensor = getHottSensor(HOTT_ID_VOLT3);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT3, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          }
          // packet[5 ] uint8_t temp1;               //#25 Temperature sensor 1. 20=0�, 46=26� - offset of 20.
          value = packet[5] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          // packet[6 ] uint8_t temp2;               //#26 temperature sensor 2
          value = packet[6] - 20;
          sensor = getHottSensor(HOTT_ID_TEMP2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TEMP2, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[7 ] uint8_t altitude_L;          //#27 Attitude lower value. unit: meters. Value of 500 = 0m
          // packet[8 ] uint8_t altitude_H;          //#28
          value = packet[7] + (packet[8] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ALT, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[9 ] uint8_t current_L;           //#29 Current in 0.1 steps
          // packet[10] uint8_t current_H;           //#30
          value = packet[9] + (packet[10] << 8);
          sensor = getHottSensor(HOTT_ID_AMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_AMP1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          // packet[11] uint8_t main_voltage_L;      //#31 Main power voltage (drive) in 0.1V steps
          // packet[12] uint8_t main_voltage_H;      //#32
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_VOLT1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VOLT1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          // packet[13] uint8_t batt_cap_L;          //#33 used battery capacity in 10mAh steps
          break;

        case HOTT_PAGE_04:
          // packet[4 ] uint8_t batt_cap_H;          //#34
          if (prev_page == ((HOTT_TELEM_EAM << 4) | HOTT_PAGE_03)) {
            value = (prev_value + (packet[4] << 8)) * 10;
            sensor = getHottSensor(HOTT_ID_BCAPA);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_BCAPA, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          }

          // packet[5 ] uint8_t climbrate_L;         //#35 climb rate in 0.01m/s. Value of 30000 = 0.00 m/s
          // packet[6 ] uint8_t climbrate_H;         //#36
          value = packet[5] + (packet[6] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_VARIO);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[7 ] uint8_t climbrate3s;         //#37 climbrate in m/3sec. Value of 120 = 0m/3sec
          // packet[8 ] uint8_t rpm_L;               //#38 RPM. Steps: 10 U/min
          // packet[9 ] uint8_t rpm_H;               //#39
          value = (packet[8] + (packet[9] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_RPM1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RPM1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          // packet[10] uint8_t electric_min;        //#40 Electric minutes. Time does start, when motor current is > 3 A
          // packet[11] uint8_t electric_sec;        //#41
          // packet[12] uint8_t speed_L;             //#42 (air?) speed in km/h. Steps 1km/h
          // packet[13] uint8_t speed_H;             //#43
          value = packet[12] + (packet[13] << 8);
          sensor = getHottSensor(HOTT_ID_AIR_SPEED);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_AIR_SPEED, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          break;
      }
      break;
  }
  prev_page = (packet[2] << 4) | packet[3];    // concatenate telemetry type and page
  prev_value = packet[13];                    // page overflow
}

void hottSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const HottSensor * sensor = getHottSensor(id);
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
