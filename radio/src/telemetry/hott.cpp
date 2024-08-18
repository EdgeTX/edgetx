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

/* HoTT Telemetry 

HoTT telemetry comprises of one and up to eight devices connected to the receiver's one-wire serial bus. The devices
have unique ID's and each decive ID may be present on the bus only once. 

HoTT telemetry knows to operating modes, Binary mode (normal operating mode) and Text mode (configuration mode). 
Telemetry data is passed from the receiver via MPM to OpenTx in pages. The pages can be present or not, they 
also do not have to follow each others.

Telemetry device IDs: 
  0x00 RX, 0x09 Vario, 0x0A GPS, 0x0B not seen, 0x0C ESC, 0x0D GAM, 0x0E EAM, 0x80 Text config mode

Binary mode:
  Each page consists of a four byte header followed by ten bytes data plus a single byte warning code. 

  Generic telemetry page layout in binary mode:
    [0] = TX RSSI value, downlink signal strength (rx --> tx as seen by tx)
          CC2500 formated (a<128:a/2-71dBm, a>=128:(a-256)/2-71dBm)
    [1] = TX LQI value, downlink signal quality (rx --> tx as seen by tx)
    [2] = ID of device that sent the packet, see telemetry devices ID above 
    [3] = telemetry page number. 
    [4]..[13] = 10 data bytes
    [14] = RX, GAM, EAM, ESC, GPS, VARIO warnings

  Telemetry pages in binary mode: 
    0x00 for RX binary data
    0x01..0x04 for GAM, EAM, ESC, GPS, VARIO binary data 

  Every binary data frame sent by the RX has 10 data bytes.
  Page layout of the RX telemetry page 0 in binary mode
    [0] = TX RSSI value, downlink signal strength (rx --> tx as seen by tx)
          CC2500 formated (a<128:a/2-71dBm, a>=128:(a-256)/2-71dBm)
    [1] = TX LQI value, downlink signal quality (rx --> tx as seen by tx)
    [2] = ID of device that sent the packet, see telemetry devices ID above 
    [3] = telemetry page number. 
    [4]  = ?? looks to be a copy of [7]
    [5]  = RX_Voltage*10 in V
    [6]  = Temperature-20 in °C
    [7]  = Uplink signal strength (tx --> rx as seen by the rx): 
           CC2500 formated (a<128:a/2-71dBm, a>=128:(a-256)/2-71dBm)
    [8]  = Uplink signal quality (tx --> rx as seen by the tx) in percent
    [9]  = RX_MIN_Voltage*10 in V
    [10,11] = [11]*256+[10]=max lost packet time in ms, max value is 2s=0x7D0
    [12] = RX Event (1 = low battery voltage, 2 low/high temp warning, other = general event)
    [13] = ?? looks to be a copy of [7]
    [14] = RX, GAM, EAM, ESC, GPS, VARIO warnings

  Every binary data frame sent by GAM, EAM, ESC, GPS, VARIO has a length of 45 bytes with 40 bytes
  of device specific data, see e.g. 
  https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L240
  The 40 data bytes are sent in 10 byte segments through pages 1 to 4. 
  
  Page layout for GAM, EAM, ESC, GPS, VARIO telemetry pages 1 to 4:
   [0] = TX RSSI value, downlink signal strength (rx --> tx as seen by tx)
         CC2500 formated (a<128:a/2-71dBm, a>=128:(a-256)/2-71dBm)
   [1] = TX LQI value, downlink signal quality (rx --> tx as seen by tx)
   [2] = ID of device that sent the packet, see telemetry devices ID above 
   [3] = telemetry page number. 
   [4]..[13] = data bytes 4-13, data bytes 14-23, data bytes 24, 33, data bytes 34-42
   [14] = RX, GAM, EAM, ESC, GPS, VARIO warnings
  
Text Mode:
  Telemetry pages in text mode:
    0x00..0x12   

  Every text mode data frame sent by GAM, EAM, ESC, GPS, VARIO has a length of 173 bytes with 168 bytes
  of data representing 8 lines of 21 characters to display. The 168 bytes of data are sent through pages 
  0 to 19. 

RSSI:
The packet[0] TX RSSI and packet[7] RX RSSI values describe estimates of the signal level in the
currently active channel from TX to RX (RX values, uplinked control data) and RX to TX (TX values, 
downlinked telemetry data). RSSI values are based on the current gain setting in the RX chain 
and telemetry frames. The downlink (telemetry) uses lower transmission power. Therefore TX RSSI
and RX RSSI values are not comparable as different gain values may be used.
Usually TX RSSI even indicates stronger downlink signal strength compared to uplink signal strength
because the TX amplifies the signal higher due to the lower power of the downlink signal.

LQI:
LQI (link quality) values in packet[1] and packet[8] also take decodability, i.e. the effort
to extract meaningful data in account and are because of this the best indicator to determine the quality of 
the link. LQI values are presented asrelative numbers (0-100%). 

OpenTX RSSI value:
For Graupner HoTT OpenTX doesn't use the RX RSSI value but instead uses the more 
meaningful RX_LQI value in packet[8]. Hence the default OpenTX RSSI warnig and critical thresholds can
be adjusted to more appropriate values to indicate link problems.

Failsafe trigger:
Graupner receivers do not trigger failsafe based on RSSI and LQI directly. They do trigger
failsafe based on the maximum time no decodable data packet has been received. The default threshhold
seems to be 1000ms
*/

enum
{
  HOTT_PAGE_00 = 0x00,
  HOTT_PAGE_01 = 0x01,
  HOTT_PAGE_02 = 0x02,
  HOTT_PAGE_03 = 0x03,
  HOTT_PAGE_04 = 0x04,
};

enum DeviceID
{
  HOTT_TELEM_RX    = 0x00,
  HOTT_TELEM_VARIO = 0x09,
  HOTT_TELEM_GPS   = 0x0A,
  HOTT_TELEM_ESC   = 0x0C,
  HOTT_TELEM_GAM   = 0x0D,
  HOTT_TELEM_EAM   = 0x0E,
  HOTT_TELEM_TEXT  = 0x80,
  HOTT_TELEM_TX    = 0xFF
} ;

// telemetry sensors ID
enum TelemetrySensorID
{
  // TX from 0xFF01
  HOTT_ID_TX_RSSI_DL  = (HOTT_TELEM_TX << 8) + 1,    // downlink signal strength (rx --> tx as seen by tx) 
  HOTT_ID_TX_LQI_DL   = (HOTT_TELEM_TX << 8) + 2,    // downlink signal quality (rx --> tx s seen by tx)

  // RX from 0x0001
  HOTT_ID_RX_RSSI_UL  = (HOTT_TELEM_RX << 8) + 1,    // uplink signal strength (tx --> rx as seen by rx)
  HOTT_ID_RX_LQI_UL   = (HOTT_TELEM_RX << 8) + 2,    // uplink signal quality (tx --> rx as seen by rx)
  HOTT_ID_RX_VLT      = (HOTT_TELEM_RX << 8) + 3,    // RX battery voltage
  HOTT_ID_RX_TMP      = (HOTT_TELEM_RX << 8) + 4,    // RX temperature
  HOTT_ID_RX_BAT_MIN  = (HOTT_TELEM_RX << 8) + 5,    // RX lowest rx voltage
  HOTT_ID_RX_VPCK     = (HOTT_TELEM_RX << 8) + 6,    // RX VPack
  HOTT_ID_RX_EVENT    = (HOTT_TELEM_RX << 8) + 7,    // RX event + GAM, EAM, VARIO, GPS, ESC warnings

  // Vario from 0x0901
  HOTT_ID_VARIO_ALT   = (HOTT_TELEM_VARIO << 8) + 1, // Vario altitude
  HOTT_ID_VARIO_VV    = (HOTT_TELEM_VARIO << 8) + 2, // Vario vertical velocity
  HOTT_ID_VARIO_HDG   = (HOTT_TELEM_VARIO << 8) + 3, // Vario heading

  // GPS from 0x0A01
  HOTT_ID_GPS_HDG     = (HOTT_TELEM_GPS << 8) + 1,  // GPS heading
  HOTT_ID_GPS_SPEED   = (HOTT_TELEM_GPS << 8) + 2,  // GPS ground speed
  HOTT_ID_GPS_LL      = (HOTT_TELEM_GPS << 8) + 3,  // GPS lat/long
  HOTT_ID_GPS_DST     = (HOTT_TELEM_GPS << 8) + 4,  // GPS lat/long
  HOTT_ID_GPS_ALT     = (HOTT_TELEM_GPS << 8) + 5,  // GPS altitude
  HOTT_ID_GPS_VV      = (HOTT_TELEM_GPS << 8) + 6,  // GPS distance
  HOTT_ID_GPS_NSATS   = (HOTT_TELEM_GPS << 8) + 7,  // GPS number of satellites

  //ESC from 0x0C01
  HOTT_ID_ESC_VLT     = (HOTT_TELEM_ESC << 8) + 1,  // ESC battery voltage
  HOTT_ID_ESC_CAP     = (HOTT_TELEM_ESC << 8) + 2,  // ESC battery capacity consumed
  HOTT_ID_ESC_TMP     = (HOTT_TELEM_ESC << 8) + 3,  // ESC temperature
  HOTT_ID_ESC_CUR     = (HOTT_TELEM_ESC << 8) + 4,  // ESC current
  HOTT_ID_ESC_RPM     = (HOTT_TELEM_ESC << 8) + 5,  // ESC motor rpm 
  HOTT_ID_ESC_BEC_VLT = (HOTT_TELEM_ESC << 8) + 6,  // ESC BEC voltage
  HOTT_ID_ESC_BEC_CUR = (HOTT_TELEM_ESC << 8) + 7,  // ESC BEC current
  HOTT_ID_ESC_BEC_TMP = (HOTT_TELEM_ESC << 8) + 8,  // BEC BEC temperature
  HOTT_ID_ESC_MOT_TMP = (HOTT_TELEM_ESC << 8) + 9,  // ESC motor or external temperature

  // GAM from 0x0D01
  HOTT_ID_GAM_CELS    = (HOTT_TELEM_GAM << 8) + 1,  // GAM Cells
  HOTT_ID_GAM_VLT1    = (HOTT_TELEM_GAM << 8) + 2,  // GAM voltage battery 1
  HOTT_ID_GAM_VLT2    = (HOTT_TELEM_GAM << 8) + 3,  // GAM volatge battery 2
  HOTT_ID_GAM_TMP1    = (HOTT_TELEM_GAM << 8) + 4,  // GAM temperature 1
  HOTT_ID_GAM_TMP2    = (HOTT_TELEM_GAM << 8) + 5,  // GAM temperature 2
  HOTT_ID_GAM_FUEL    = (HOTT_TELEM_GAM << 8) + 6,  // GAM fuel percentage  
  HOTT_ID_GAM_RPM1    = (HOTT_TELEM_GAM << 8) + 7,  // GAM rpm 1  
  HOTT_ID_GAM_ALT     = (HOTT_TELEM_GAM << 8) + 8,  // GAM altitude
  HOTT_ID_GAM_VV      = (HOTT_TELEM_GAM << 8) + 9,  // GAM vertical velocity
  HOTT_ID_GAM_CUR     = (HOTT_TELEM_GAM << 8) + 10, // GAM current  
  HOTT_ID_GAM_VLT3    = (HOTT_TELEM_GAM << 8) + 11, // GAM voltage battery 3
  HOTT_ID_GAM_CAP     = (HOTT_TELEM_GAM << 8) + 12, // GAM battery capacity
  HOTT_ID_GAM_SPEED   = (HOTT_TELEM_GAM << 8) + 13, // GAM speed
  HOTT_ID_GAM_RPM2    = (HOTT_TELEM_GAM << 8) + 14, // GAM rmp 2
  
   //EAM from 0x0E01
  HOTT_ID_EAM_CELS_L  = (HOTT_TELEM_EAM << 8) + 1,  // EAM Cells L voltage
  HOTT_ID_EAM_CELS_H  = (HOTT_TELEM_EAM << 8) + 2,  // EAM Cells H voltage
  HOTT_ID_EAM_VLT1    = (HOTT_TELEM_EAM << 8) + 3,  // EAM Voltage battery 1
  HOTT_ID_EAM_VLT2    = (HOTT_TELEM_EAM << 8) + 4,  // EAM Voltage battery 2
  HOTT_ID_EAM_TMP1    = (HOTT_TELEM_EAM << 8) + 5,  // EAM temperature 1
  HOTT_ID_EAM_TMP2    = (HOTT_TELEM_EAM << 8) + 6,  // EAM temperature 2
  HOTT_ID_EAM_ALT     = (HOTT_TELEM_EAM << 8) + 7,  // EAM altitude
  HOTT_ID_EAM_CUR     = (HOTT_TELEM_EAM << 8) + 8,  // EAM current  
  HOTT_ID_EAM_VLT3    = (HOTT_TELEM_EAM << 8) + 9,  // EAM battery voltage 3
  HOTT_ID_EAM_CAP     = (HOTT_TELEM_EAM << 8) + 10, // EAM Batt capacity
  HOTT_ID_EAM_VV      = (HOTT_TELEM_EAM << 8) + 11, // EAM vertical velcocity
  HOTT_ID_EAM_RPM     = (HOTT_TELEM_EAM << 8) + 12, // EAM rpm  
  HOTT_ID_EAM_SPEED   = (HOTT_TELEM_EAM << 8) + 13  // EAM speed
};

struct HottSensor
{
  const uint16_t id;
  const TelemetryUnit unit;
  const uint8_t precision;
  const char * name;
};

// clang-format off
#define HS(id,name,unit,precision) {id,unit,precision,name}

const HottSensor hottSensors[] = {
  // TX
  HS( HOTT_ID_TX_RSSI_DL,   STR_SENSOR_HOTT_ID_TX_RSSI_DL,  UNIT_DB, 0),                	// downlink signal strength (rx --> tx as seen by tx) 
  HS( HOTT_ID_TX_LQI_DL,    STR_SENSOR_HOTT_ID_TX_LQI_DL,   UNIT_RAW, 0),               	// downlink signal quality (rx --> tx s seen by tx)

  // RX
  HS( HOTT_ID_RX_RSSI_UL,   STR_SENSOR_HOTT_ID_RX_RSSI_UL,  UNIT_DB, 0 ),               	// uplink signal strength (tx --> rx as seen by rx)
  HS( HOTT_ID_RX_LQI_UL,    STR_SENSOR_HOTT_ID_RX_LQI_UL,   UNIT_RAW, 0 ),              	// uplink signal quality (tx --> rx as seen by rx)
  HS( HOTT_ID_RX_VLT,       STR_SENSOR_HOTT_ID_RX_VLT,      UNIT_VOLTS, 1 ),            	// RX battery voltage
  HS( HOTT_ID_RX_TMP,       STR_SENSOR_HOTT_ID_RX_TMP,      UNIT_CELSIUS, 0 ),          	// RX temperature
  HS( HOTT_ID_RX_BAT_MIN,   STR_SENSOR_HOTT_ID_RX_BAT_MIN,  UNIT_VOLTS, 1 ),            	// RX lowest rx voltage
  HS( HOTT_ID_RX_VPCK,      STR_SENSOR_HOTT_ID_RX_VPCK,     UNIT_MS, 0 ),               	// RX VPack
  HS( HOTT_ID_RX_EVENT,     STR_SENSOR_HOTT_ID_RX_EVENT,    UNIT_RAW, 0 ),              	// RX event and other devices warnings
  
  // Vario 
  HS( HOTT_ID_VARIO_ALT,    STR_SENSOR_HOTT_ID_VARIO_ALT,   UNIT_METERS, 0 ),           	// Vario altitude
  HS( HOTT_ID_VARIO_VV,     STR_SENSOR_HOTT_ID_VARIO_VV,    UNIT_METERS_PER_SECOND, 2 ),	// Vario vertical velocity
  HS( HOTT_ID_VARIO_HDG,    STR_SENSOR_HOTT_ID_VARIO_HDG,   UNIT_DEGREE, 0 ),           	// Vario heading

  // GPS
  HS( HOTT_ID_GPS_HDG,      STR_SENSOR_HOTT_ID_GPS_HDG,     UNIT_DEGREE, 0 ),           	// GPS heading
  HS( HOTT_ID_GPS_SPEED,    STR_SENSOR_HOTT_ID_GPS_SPEED,   UNIT_KMH,  0 ) ,            	// GPS ground speed
  HS( HOTT_ID_GPS_LL,       STR_SENSOR_HOTT_ID_GPS_LL,      UNIT_GPS, 0 ),              	// GPS lat/long
  HS( HOTT_ID_GPS_DST,      STR_SENSOR_HOTT_ID_GPS_DST,     UNIT_METERS, 0 ),           	// GPS distance
  HS( HOTT_ID_GPS_ALT,      STR_SENSOR_HOTT_ID_GPS_ALT,     UNIT_METERS, 0 ),           	// GPS altitude
  HS( HOTT_ID_GPS_VV,       STR_SENSOR_HOTT_ID_GPS_VV,      UNIT_METERS_PER_SECOND, 2), 	// GPS vertical velocity
  HS( HOTT_ID_GPS_NSATS,    STR_SENSOR_HOTT_ID_GPS_NSATS,   UNIT_RAW, 0 ),              	// GPS number of satellites

  // ESC
  HS( HOTT_ID_ESC_VLT,      STR_SENSOR_HOTT_ID_ESC_VLT,     UNIT_VOLTS, 1 ),            	// ESC battery voltage
  HS( HOTT_ID_ESC_CAP,      STR_SENSOR_HOTT_ID_ESC_CAP,     UNIT_MAH, 0 ),              	// ESC battery capacity consumed
  HS( HOTT_ID_ESC_TMP,      STR_SENSOR_HOTT_ID_ESC_TMP,     UNIT_CELSIUS, 0 ),          	// ESC temperature
  HS( HOTT_ID_ESC_CUR,      STR_SENSOR_HOTT_ID_ESC_CUR,     UNIT_AMPS, 1 ),             	// ESC current
  HS( HOTT_ID_ESC_RPM,      STR_SENSOR_HOTT_ID_ESC_RPM,     UNIT_RPMS, 0 ),             	// ESC motor rpm 
  HS( HOTT_ID_ESC_BEC_VLT,  STR_SENSOR_HOTT_ID_ESC_BEC_VLT, UNIT_VOLTS, 1 ),            	// ESC BEC voltage
  HS( HOTT_ID_ESC_BEC_CUR,  STR_SENSOR_HOTT_ID_ESC_BEC_CUR, UNIT_AMPS, 1 ),             	// ESC BEC current
  HS( HOTT_ID_ESC_BEC_TMP,  STR_SENSOR_HOTT_ID_ESC_BEC_TMP, UNIT_CELSIUS, 0 ),          	// BEC BEC temperature
  HS( HOTT_ID_ESC_MOT_TMP,  STR_SENSOR_HOTT_ID_ESC_MOT_TMP, UNIT_CELSIUS, 0 ),         	  // ESC motor or external temperature

  // GAM
  HS( HOTT_ID_GAM_CELS,     STR_SENSOR_HOTT_ID_GAM_CELS,    UNIT_CELLS, 2),             	// GAM Cels L
  HS( HOTT_ID_GAM_VLT1,     STR_SENSOR_HOTT_ID_GAM_VLT1,    UNIT_VOLTS, 1 ),            	// GAM voltage battery 1
  HS( HOTT_ID_GAM_VLT2,     STR_SENSOR_HOTT_ID_GAM_VLT2,    UNIT_VOLTS, 1 ),            	// GAM volatge battery 2
  HS( HOTT_ID_GAM_TMP1,     STR_SENSOR_HOTT_ID_GAM_TMP1,    UNIT_CELSIUS, 0 ),          	// GAM temperature 1
  HS( HOTT_ID_GAM_TMP2,     STR_SENSOR_HOTT_ID_GAM_TMP2,    UNIT_CELSIUS, 0 ),          	// GAM temperature 2
  HS( HOTT_ID_GAM_FUEL,     STR_SENSOR_HOTT_ID_GAM_FUEL,    UNIT_PERCENT, 0),           	// GAM fuel percentage  
  HS( HOTT_ID_GAM_RPM1,     STR_SENSOR_HOTT_ID_GAM_RPM1,    UNIT_RPMS, 0 ),             	// GAM rpm 1  
  HS( HOTT_ID_GAM_ALT,      STR_SENSOR_HOTT_ID_GAM_ALT,     UNIT_METERS, 0 ),           	// GAM altitude
  HS( HOTT_ID_GAM_VV,       STR_SENSOR_HOTT_ID_GAM_VV,      UNIT_METERS_PER_SECOND, 2 ),	// GAM vertical velocity
  HS( HOTT_ID_GAM_CUR,      STR_SENSOR_HOTT_ID_GAM_CUR,     UNIT_AMPS, 1 ),             	// GAM current  
  HS( HOTT_ID_GAM_VLT3,     STR_SENSOR_HOTT_ID_GAM_VLT3,    UNIT_VOLTS, 1 ),            	// GAM voltage battery 3
  HS( HOTT_ID_GAM_CAP,      STR_SENSOR_HOTT_ID_GAM_CAP,     UNIT_MAH, 0 ),              	// GAM battery capacity
  HS( HOTT_ID_GAM_SPEED,    STR_SENSOR_HOTT_ID_GAM_SPEED,   UNIT_KMH,  0 ) ,            	// GAM speed
  HS( HOTT_ID_GAM_RPM2,     STR_SENSOR_HOTT_ID_GAM_RPM2,    UNIT_RPMS, 0 ),             	// GAM rmp 2

  //EAM
  HS( HOTT_ID_EAM_CELS_L,   STR_SENSOR_HOTT_ID_EAM_CELS_L,  UNIT_CELLS, 2),             	// EAM Cells L voltage
  HS( HOTT_ID_EAM_CELS_H,   STR_SENSOR_HOTT_ID_EAM_CELS_H,  UNIT_CELLS, 2),             	// EAM Cells H voltage
  HS( HOTT_ID_EAM_VLT1,     STR_SENSOR_HOTT_ID_EAM_VLT1,    UNIT_VOLTS, 1 ),            	// EAM Voltage battery 1
  HS( HOTT_ID_EAM_VLT2,     STR_SENSOR_HOTT_ID_EAM_VLT2,    UNIT_VOLTS, 1 ),            	// EAM Voltage battery 2
  HS( HOTT_ID_EAM_TMP1,     STR_SENSOR_HOTT_ID_EAM_TMP1,    UNIT_CELSIUS, 0 ),          	// EAM temperature 1
  HS( HOTT_ID_EAM_TMP2,     STR_SENSOR_HOTT_ID_EAM_TMP2,    UNIT_CELSIUS, 0 ),          	// EAM temperature 2
  HS( HOTT_ID_EAM_ALT,      STR_SENSOR_HOTT_ID_EAM_ALT,     UNIT_METERS, 0 ),           	// EAM altitude
  HS( HOTT_ID_EAM_CUR,      STR_SENSOR_HOTT_ID_EAM_CUR,     UNIT_AMPS, 1 ),             	// EAM current  
  HS( HOTT_ID_EAM_VLT3,     STR_SENSOR_HOTT_ID_EAM_VLT3,    UNIT_VOLTS, 1 ),            	// EAM battery voltage 3
  HS( HOTT_ID_EAM_CAP,      STR_SENSOR_HOTT_ID_EAM_CAP,     UNIT_MAH, 0 ),              	// EAM Batt capacity
  HS( HOTT_ID_EAM_VV,       STR_SENSOR_HOTT_ID_EAM_VV,      UNIT_METERS_PER_SECOND, 2 ),	// EAM vertical velcocity
  HS( HOTT_ID_EAM_RPM,      STR_SENSOR_HOTT_ID_EAM_RPM,     UNIT_RPMS, 0 ),              	// EAM rpm  
  HS( HOTT_ID_EAM_SPEED,    STR_SENSOR_HOTT_ID_EAM_SPEED,   UNIT_KMH,  0 ) ,            	// EAM speed
  
  // sentinel
  HS(0x00,                  NULL,                           UNIT_RAW, 0)                  // sentinel
};
// clang-format on

const HottSensor * getHottSensor(uint16_t id)
{
  for (const HottSensor * sensor = hottSensors; sensor->id; sensor++) {
    if (id == sensor->id)
      return sensor;
  }
  return nullptr;
}

int16_t processHoTTdBm(int16_t value)
  // The RSSI value read from the RSSI status register is a 2’s complement number. The following
  // procedure implements the CC2500 data sheet specification to convert the RSSI reading to an 
  // absolute power level (RSSI_dBm).
  // 1) Read the RSSI status register
  // 2) Convert the reading from a hexadecimal number to a decimal number (RSSI_dec)
  // 3) If RSSI_dec ≥ 128 then RSSI_dBm = (RSSI_dec - 256)/2 – RSSI_offset
  // 4) Else if RSSI_dec < 128 then RSSI_dBm = (RSSI_dec)/2 – RSSI_offset
{
  if (value >= 128) {
    value -= 256;
  }
  return value/2 - 71;
}

uint8_t processHoTTWarnings(const uint8_t * packet) {
  // Translates rx events to warnings and transfers GAM, EAM, GPS, VARIO, ESC warnings
  // 
  // Two types of warnings have to be considered:
  //  1. Rx events - these are critical events and should be treated with priority
  //  0      = no event
  //  1      = rx low battery warning based on the threshold set in the rx configuration
  //  2      = rx temp warning based on the threshold set in the rx configuration
  //  others = not sure, make it known as general event
  //
  // 2. GAM, EAM, VARIO, GPS, EAM - these may be informational but also flight critical
  //  0     = no warning
  //  other = warnings based on device specific setting either set by HoTT device configuration
  //          or external software, e.g. third party devices like YGE ESC's or SM Unisens/GPS Logger
  // 
  // For a list of warnings see HoTT_warnings.txt
  //
  // Rx events are passed from MPM in page 0, packet[12]
  // Other device warnings are passed in packet[14]
  // 
  // As only one warning can be transferred to the user the device warnigs are prioritized 
  // by the order RX, ESC, GAM, EAM, VARIO, GPS
  //
  // The final result is passed to the user for further processing in the HOTT_ID_RX_EVENT telemetry sensor. 
  // Users may process warnings with a combination of logical switches and special funtions to announce the warning
  // or lua scripts

  #define DEVICE  (packet[2])
  #define PAGE    (packet[3])
  #define RXTEMP  (packet[6] - 20)
  #define RXEVENT (packet[12])
  #define WARN    (packet[14])

  enum HoTTWarnDevices {                          // lists devices in order of priority
    HOTT_WARN_RX = 0,                             // if two or more devices issue warnigs in the same le
    HOTT_WARN_ESC,                                // polling cyc the device with the lower enum value wins
    HOTT_WARN_GAM,
    HOTT_WARN_EAM, 
    HOTT_WARN_VARIO,
    HOTT_WARN_GPS,
    HOTT_WARN_LAST                                // delimiter
  };

  static uint8_t warnings[HOTT_WARN_LAST] = {};   // to memorize the warnings status of each HoTT device
                                                  // until it gets updated again

  if ( PAGE == 0 && DEVICE ==  HOTT_TELEM_RX) {	  // sending device is RX
    switch (RXEVENT) {                            // fetch RX event parameter from RX telemetry data
      case 0:                                     // rx doesn't indicate an event
        warnings[HOTT_WARN_RX] = 0;               // translate to warning 0 (no warning)   
        break;     	                          	                        
      
      case 1:                                     // low rx battery event
        warnings[HOTT_WARN_RX] = 64;              // translate to warning 64
        break;

      case 2:                                     // rx temperature event, find reason
        warnings[HOTT_WARN_RX] =                  // check rx temp
          RXTEMP >= 50 ? 44: 43;                  // > 50 degrees, high temperature warning, translate to warning 44      
          break;                                  // < 50 degrees, low temperature warning, translate to warning 43 

      default: 
        warnings[HOTT_WARN_RX] = 53;              // other rx events -> translate to general receiver warning
    }  
  } else {
    if (PAGE > 0 && PAGE <= 4) {                  // sending device is other than rx
      switch (DEVICE) {
        case HOTT_TELEM_ESC:                      // sending device is ESC
          warnings[HOTT_WARN_ESC] = WARN;
          break;
        
        case HOTT_TELEM_GAM:                      // sending device is GAM
          warnings[HOTT_WARN_GAM] = WARN;
          break;
        
        case HOTT_TELEM_EAM:                      // sending device is EAM
          warnings[HOTT_WARN_EAM] = WARN;
          break;
        
        case HOTT_TELEM_VARIO:                    // sending device is VARIO
          warnings[HOTT_WARN_VARIO] = WARN;
          break;
        
        case HOTT_TELEM_GPS:                      // sending device is GPS
          warnings[HOTT_WARN_GPS] = WARN; 
          break;
      }
    }
  }

  for(uint8_t i = 0; i < HOTT_WARN_LAST; i++) {   // go through list of devices and see if warning was issued 
    uint8_t deviceWarn = warnings[i];             // using device priorities in the order of the HoTTWarnDevice enum
    
    if (deviceWarn > 0 &&                         // if warning was issued by device #i
        (deviceWarn < 27 || deviceWarn > 36))     // always exclude climb rate beeps (meant for tx's without voice output)
      return  deviceWarn;                         // return warning
  }                                                

  return 0;                                       // return 0 (no warning)
}

void processHottPacket(const uint8_t * packet)
{
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

  const HottSensor *sensor;
  int32_t value;
  static uint8_t prev_page = 0, prev_value = 0;
  static HottGPSMinutes min = { 0, 0, 0 };
  int16_t deg = 0, sec = 0;

  // Set RX Event (HoTT warnings) 
  value = processHoTTWarnings(packet);
  sensor = getHottSensor(HOTT_ID_RX_EVENT);
  setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_EVENT, 0, 0, value, sensor->unit, sensor->precision);

  // Set TX RSSI Value
  value = processHoTTdBm(packet[0]);
  sensor = getHottSensor(HOTT_ID_TX_RSSI_DL);
  setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TX_RSSI_DL, 0, 0, value, sensor->unit, sensor->precision);

  // Set TX LQI  Value
  value = packet[1];
  sensor = getHottSensor(HOTT_ID_TX_LQI_DL);
  setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_TX_LQI_DL, 0, 0, value, sensor->unit, sensor->precision);

  switch (packet[2]) { // Telemetry type
    case HOTT_TELEM_RX:
      if (packet[3] == HOTT_PAGE_00) { // Telemetry page: only page 0 is for RX
        // rx battery voltage
        value = packet[5];
        sensor = getHottSensor(HOTT_ID_RX_VLT);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_VLT, 0, HOTT_TELEM_RX, value, sensor->unit, sensor->precision);

        // rx temperature
        value = packet[6] - 20;
        sensor = getHottSensor(HOTT_ID_RX_TMP);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_TMP, 0, HOTT_TELEM_RX, value, sensor->unit, sensor->precision);

        // uplink RSSI
        value = processHoTTdBm(packet[7]);
        sensor = getHottSensor(HOTT_ID_RX_RSSI_UL);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_RSSI_UL, 0, HOTT_TELEM_RX, value, sensor->unit, sensor->precision);

        // uplink quality
        value = packet[8];
        
        telemetryData.rssi.set(value);
        if (value > 0)
          telemetryStreaming = TELEMETRY_TIMEOUT10ms;

        sensor = getHottSensor(HOTT_ID_RX_LQI_UL);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_LQI_UL, 0, HOTT_TELEM_RX, value, sensor->unit, sensor->precision);

        // RX_lowest voltage
        value = packet[9];
        sensor = getHottSensor(HOTT_ID_RX_BAT_MIN);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_BAT_MIN, 0, HOTT_TELEM_RX, value, sensor->unit, sensor->precision);

        // RX_VPACK
        value = packet[10] + (packet[11] << 8);
        sensor = getHottSensor(HOTT_ID_RX_VPCK);
        setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_RX_VPCK, 0, 0, value, sensor->unit, sensor->precision);    
      }
      break;

    case HOTT_TELEM_VARIO:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L240
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // Vario altitude
          value = packet[6] + (packet[7] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_VARIO_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO_ALT, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);

          // Vario vertical velocity
          value = packet[12] + (packet[13] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_VARIO_VV);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO_VV, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_02:
          break;

        case HOTT_PAGE_03:
          break;

        case HOTT_PAGE_04:
          // Vario heading
          value = packet[12] * 2;
          sensor = getHottSensor(HOTT_ID_VARIO_HDG);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_VARIO_HDG, 0, HOTT_TELEM_VARIO, value, sensor->unit, sensor->precision);
          break;
      }
      break;

    case HOTT_TELEM_GPS:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L378
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // GPS heading
          value = packet[7] * 2;
          sensor = getHottSensor(HOTT_ID_GPS_HDG);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_HDG, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // GPS speed
          value = packet[8] + (packet[9] << 8);
          sensor = getHottSensor(HOTT_ID_GPS_SPEED);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_SPEED, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // GPS latitude
          min.value = (int16_t) (packet[11] + (packet[12] << 8));
          min.isNegative = (packet[10] == 1);
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
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_LL, 0, HOTT_TELEM_GPS, value, UNIT_GPS_LATITUDE, 0);
          }

          // GPS longitude
          min.value = (int16_t) (packet[6] + (packet[7] << 8));
          sec = (int16_t) (packet[8] + (packet[9] << 8));
          deg = min.value / 100;
          min.value = min.value - deg * 100;
          value = deg * 1000000 + (min.value * 1000000 + sec * 100) / 60;
          if (packet[5] == 1) {
            value = -value;
          }
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_LL, 0, HOTT_TELEM_GPS, value, UNIT_GPS_LONGITUDE, 0);
          
          // GPS distance to model
          value = packet[10] + (packet[11] << 8);
          sensor = getHottSensor(HOTT_ID_GPS_DST);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_DST, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // GPS altitude
          value = packet[12] + (packet[13] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_GPS_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_ALT, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // GPS vertical velocity
          value = packet[4] + (packet[5] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_GPS_VV);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_VV, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);

          // GPS number of satellites
          value = packet[7];
          sensor = getHottSensor(HOTT_ID_GPS_NSATS);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GPS_NSATS, 0, HOTT_TELEM_GPS, value, sensor->unit, sensor->precision);
          
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
          // ESC battery voltage
          value = packet[7] + (packet[8] << 8);
          sensor = getHottSensor(HOTT_ID_ESC_VLT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_VLT, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          
          // ESC battery capacity consumed
          value = (packet[11] + (packet[12] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_ESC_CAP);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_CAP, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
         
          // ESC temperature
          value = packet[13] - 20;
          sensor = getHottSensor(HOTT_ID_ESC_TMP);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_TMP, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_02:
          // ESC battery current
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_ESC_CUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_CUR, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          
          // ESC motor rpm
          value = (packet[9] + (packet[10] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_ESC_RPM);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_RPM, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // ESC BEC voltage
          value = packet[8];
          sensor = getHottSensor(HOTT_ID_ESC_BEC_VLT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_BEC_VLT, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          
          // ESC BEC current
          value = packet[10];
          sensor = getHottSensor(HOTT_ID_ESC_BEC_CUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_BEC_CUR, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_04:
          // ESC BEC temperature
          value = packet[4] - 20;
          sensor = getHottSensor(HOTT_ID_ESC_BEC_TMP);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_BEC_TMP, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          
          // ESC motor temp or external temperature sensor
          value = packet[6] - 20;
          sensor = getHottSensor(HOTT_ID_ESC_MOT_TMP);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_ESC_MOT_TMP, 0, HOTT_TELEM_ESC, value, sensor->unit, sensor->precision);
          break;
      }
      break;

    case HOTT_TELEM_GAM:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L151
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // GAM cell voltages
          sensor = getHottSensor(HOTT_ID_GAM_CELS);
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 0 << 16 | value, sensor->unit, sensor->precision);
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 1 << 16 | value, sensor->unit, sensor->precision);
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 2 << 16 | value, sensor->unit, sensor->precision);
          value = packet[10] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 3 << 16 | value, sensor->unit, sensor->precision);
          value = packet[11] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 4 << 16 | value, sensor->unit, sensor->precision);
          value = packet[12] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CELS, 0, HOTT_TELEM_GAM, 5 << 16 | value, sensor->unit, sensor->precision);
          
          // packet[13] uint8_t batt1_L;             //#13 battery 1 voltage LSB value. 0.1V steps. 50 = 5.5V
          break;

        case HOTT_PAGE_02:
          if (prev_page == ((HOTT_TELEM_GAM << 4) | HOTT_PAGE_01)) {
            // GAM voltage battery 1
            value = prev_value + (packet[4] << 8);
            sensor = getHottSensor(HOTT_ID_GAM_VLT1);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_VLT1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          }

          // GAM voltage battery 2
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_GAM_VLT2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_VLT2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // GAM temperature 1
          value = packet[7] - 20;
          sensor = getHottSensor(HOTT_ID_GAM_TMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_TMP1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // GAM temperature 2
          value = packet[8] - 20;
          sensor = getHottSensor(HOTT_ID_GAM_TMP2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_TMP2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // GAM fuel percentage
          value = packet[9];
          sensor = getHottSensor(HOTT_ID_GAM_FUEL);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_FUEL, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);

          // packet[10] uint8_t fuel_ml_L;           //#20 Fuel in ml scale. Full = 65535!
          // packet[11] uint8_t fuel_ml_H;           //#21
          
          // GAM rpm 1
          value = (packet[12] + (packet[13] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_GAM_RPM1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_RPM1, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_03:
          // GAM altitude
          value = packet[4] + (packet[5] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_GAM_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_ALT, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // GAM vertical velocity
          value = packet[6] + (packet[7] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_GAM_VV);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_VV, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // packet[8 ] uint8_t climbrate3s;         //#28 climb rate in m/3sec. Value of 120 = 0m/3sec

          // GAM current
          value = packet[9] + (packet[10] << 8);
          sensor = getHottSensor(HOTT_ID_GAM_CUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CUR, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // GAM voltage battery 3
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_GAM_VLT3);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_VLT3, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // packet[13] uint8_t batt_cap_L;          //#33 used battery capacity in 10mAh steps
          break;

        case HOTT_PAGE_04:
          // GAM capacity
          if (prev_page == ((HOTT_TELEM_GAM << 4) | HOTT_PAGE_03)) {
            value = (prev_value + (packet[4] << 8)) * 10;
            sensor = getHottSensor(HOTT_ID_GAM_CAP);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_CAP, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          }

          // GAM speed
          value = packet[5] + (packet[6] << 8);
          sensor = getHottSensor(HOTT_ID_GAM_SPEED);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_SPEED, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
          
          // packet[7 ] uint8_t min_cell_volt;       //#37 minimum cell voltage in 2mV steps. 124 = 2,48V
          // packet[8 ] uint8_t min_cell_volt_num;   //#38 number of the cell with the lowest voltage
          
          // GAM rpm 2
          value = (packet[9] + (packet[10] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_GAM_RPM2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_GAM_RPM2, 0, HOTT_TELEM_GAM, value, sensor->unit, sensor->precision);
         
          // packet[11] uint8_t general_error_number;//#41 Voice error == 12 (max. speed). TODO: more docu
          // packet[12] uint8_t pressure;            //#42 Pressure up to 16bar. 0,1bar scale. 20 = 2bar
          break;
      }
      break;

    case HOTT_TELEM_EAM:
      // https://github.com/betaflight/betaflight/blob/1d8a0e9fd61cf01df7b34805e84365df72d9d68d/src/main/telemetry/hott.h#L288
      switch (packet[3]) { // Telemetry page 1,2,3,4
        case HOTT_PAGE_01:
          // EAM cell voltages 1-7 (1 to 6 transferred)
          sensor = getHottSensor(HOTT_ID_EAM_CELS_L);
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 0 << 16 | value, sensor->unit, sensor->precision);
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 1 << 16 | value, sensor->unit, sensor->precision);
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 2 << 16 | value, sensor->unit, sensor->precision);
          value = packet[10] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 3 << 16 | value, sensor->unit, sensor->precision);
          value = packet[11] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 4 << 16 | value, sensor->unit, sensor->precision);
          value = packet[12] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 5 << 16 | value, sensor->unit, sensor->precision);
          // value = packet[13] << 1;
          // if(value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_L, 0, HOTT_TELEM_EAM, 6 << 16 | value, sensor->unit, sensor->precision);
          break;

        case HOTT_PAGE_02:
          // EAM cell voltages 8-14 (8 to 13  transferred)
          sensor = getHottSensor(HOTT_ID_EAM_CELS_H);
          value = packet[4] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 0 << 16 | value, sensor->unit, sensor->precision);
          value = packet[5] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 1 << 16 | value, sensor->unit, sensor->precision);
          value = packet[6] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 2 << 16 | value, sensor->unit, sensor->precision);
          value = packet[7] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 3 << 16 | value, sensor->unit, sensor->precision);
          value = packet[8] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 4 << 16 | value, sensor->unit, sensor->precision);
          value = packet[9] << 1;
          if (value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 5 << 16 | value, sensor->unit, sensor->precision);
          //value = packet[10] << 1;
          //if(value) setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CELS_H, 0, HOTT_TELEM_EAM, 13 << 16 | value, sensor->unit, sensor->precision);

          // EAM voltage battery 1
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_EAM_VLT1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_VLT1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[13] uint8_t batt2_voltage_L;     //#23 battery 2 voltage lower value in 100mv steps, 50=5V. optionally cell8_H value. 0.02V steps
          break;

        case HOTT_PAGE_03:
          // EAM voltage battery 2
          if (prev_page == ((HOTT_TELEM_EAM << 4) | HOTT_PAGE_02)) {
            value = prev_value + (packet[4] << 8);
            sensor = getHottSensor(HOTT_ID_EAM_VLT2);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_VLT2, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          }

          // EAM temperature 1
          value = packet[5] - 20;
          sensor = getHottSensor(HOTT_ID_EAM_TMP1);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_TMP1, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          
          // EAM temperature 2
          value = packet[6] - 20;
          sensor = getHottSensor(HOTT_ID_EAM_TMP2);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_TMP2, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          //EAM altitude
          value = packet[7] + (packet[8] << 8) - 500;
          sensor = getHottSensor(HOTT_ID_EAM_ALT);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_ALT, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // EAM current
          value = packet[9] + (packet[10] << 8);
          sensor = getHottSensor(HOTT_ID_EAM_CUR);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CUR, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          
          // EAM voltage battery 3 (main)
          value = packet[11] + (packet[12] << 8);
          sensor = getHottSensor(HOTT_ID_EAM_VLT3);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_VLT3, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          // packet[13] uint8_t batt_cap_L;          //#33 used battery capacity in 10mAh steps
          break;

        case HOTT_PAGE_04:
          // EAM capacity
          if (prev_page == ((HOTT_TELEM_EAM << 4) | HOTT_PAGE_03)) {
            value = (prev_value + (packet[4] << 8)) * 10;
            sensor = getHottSensor(HOTT_ID_EAM_CAP);
            setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_CAP, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          }

          // EAM vertical velocity
          value = packet[5] + (packet[6] << 8) - 30000;
          sensor = getHottSensor(HOTT_ID_EAM_VV);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_VV, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);

          // packet[7 ] uint8_t climbrate3s;         //#37 climbrate in m/3sec. Value of 120 = 0m/3sec

          // EAM rpm
          value = (packet[8] + (packet[9] << 8)) * 10;
          sensor = getHottSensor(HOTT_ID_EAM_RPM);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_RPM, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
          
          // packet[10] uint8_t electric_min;        //#40 Electric minutes. Time does start, when motor current is > 3 A
          // packet[11] uint8_t electric_sec;        //#41
          
          // EAM speed
          value = packet[12] + (packet[13] << 8);
          sensor = getHottSensor(HOTT_ID_EAM_SPEED);
          setTelemetryValue(PROTOCOL_TELEMETRY_HOTT, HOTT_ID_EAM_SPEED, 0, HOTT_TELEM_EAM, value, sensor->unit, sensor->precision);
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
