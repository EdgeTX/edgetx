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
#include "spektrum.h"
#include "hal/module_port.h"
#include "tasks/mixer_task.h"

/*
 * Documentation of the Spektrum protocol is available under
 * https://www.spektrumrc.com/ProdInfo/Files/SPM_Telemetry_Developers_Specs.pdf
 * https://github.com/SpektrumRC/SpektrumDocumentation/blob/master/Telemetry/spektrumTelemetrySensors.h
 *
 * Multi module adds two byte header of 0xAA [RSSI of telemetry packet] [16 byte message]
 */

/*
 * The 16 byte message is formated as follows
 * [ic2 address] [secondary id] [14 byte ic2 address (type) specific data]
 * we translate the secondary to the instance of the sensor
 *
 * OpenTX Mapping
 *
 *  instance = secondary id  (almost always 0)
 *  subid = 0 (always 0)
 *  id = i2c address <<8 | start byte
 */

#define I2C_PSEUDO_TX 0xf0

#define I2C_PSEUDO_TX_RSSI   (I2C_PSEUDO_TX << 8 | 0)
#define I2C_PSEUDO_TX_BIND   (I2C_PSEUDO_TX << 8 | 4)
#define I2C_PSEUDO_TX_FM     (I2C_PSEUDO_TX << 8 | 8)
#define I2C_PSEUDO_TX_CELLS  (I2C_PSEUDO_TX << 8 | 10)

#define SPEKTRUM_TELEMETRY_LENGTH 18
#define DSM_BIND_PACKET_LENGTH 12

#define I2C_NODATA                    0x00  // No data in packet, but telemetry is alive
#define I2C_VOLTAGE                   0x01  // High-Voltage sensor (INTERNAL)
#define I2C_TEMPERATURE               0x02  // Temperature Sensor (INTERNAL)
#define I2C_HIGH_CURRENT              0x03  // Amps (INTERNAL)
#define I2C_FLITECTRL                 0x05  // Flight Controller SAFE + AS3X
#define I2C_FWD_PGM                   0x09  // Forward Programming
#define I2C_PBOX                      0x0A  // PowerBox
#define I2C_LAPTIMER                  0x0B  // Lap Timer
#define I2C_TEXTGEN                   0x0C  // Text Generator 
#define I2C_AIRSPEED                  0x11  // Air Speed (Eagle Tree Sensor)
#define I2C_ALTITUDE                  0x12  // Altitude (Eagle Tree Sensor)
#define I2C_GMETER                    0x14  // G-Force (Eagle Tree Sensor)
#define	I2C_JETCAT			              0x15	// Turbine interface (Eagle Tree)
#define I2C_GPS_LOC                   0x16  // GPS Location Data (Eagle Tree)
#define I2C_GPS_STAT                  0x17  // GPS Status (Eagle Tree)
#define I2C_RX_BATT                   0x18  // Receiver Pack Capacity (Dual)
#define	I2C_JETCAT_2		              0x19	// Turbine interface, message 2 format (Eagle Tree)
#define I2C_GYRO                      0x1A  // 3-axis gyro
#define I2C_ATTMAG                    0x1B  // Attitude and Magnetic Compass
#define I2C_ESC                       0x20  // Electronic Speed Control
#define I2C_ALPHA6                    0x24  // Alpha6 Stabilizer - Blade Helis
#define I2C_GPS_BIN                   0x26  // GPS, binary format
#define I2C_REMOTE_ID                 0x27  // Spektrum SkyID/RemoteID
#define I2C_FP_BATT                   0x34  // Flight Battery Capacity (Dual)
#define I2C_CELLS                     0x3A  // 6S Cell Monitor (LiPo taps)
#define I2C_VARIO                     0x40  // Vario

// SMART_BAT is using fake I2C adresses compared to official Spektrum address
// because of subtype used only for this I2C address
#define I2C_SMART_BAT_BASE_ADDRESS    0x42
#define I2C_SMART_BAT_REALTIME        0x42  // Spektrum SMART Battery
#define I2C_SMART_BAT_CELLS_1_6       0x43
#define I2C_SMART_BAT_CELLS_7_12      0x44
#define I2C_SMART_BAT_CELLS_13_18     0x45
#define I2C_SMART_BAT_ID              0x4A
#define I2C_SMART_BAT_LIMITS          0x4B

#define I2C_RPM                       0x7E  // RPM sensor
#define I2C_QOS                       0x7F  // RxV + flight log data

// GPS flags definitions:
// Example:  B9 (1100 1001) = IS_NORTH, !IS_EAST, !GREATER_99, IS_FIX_VALID,
//                            !DATA_RECEIVED, NEGATIVE_ALT
#define GPS_INFO_FLAGS_IS_NORTH               0x01
#define GPS_INFO_FLAGS_IS_EAST                0x02
#define GPS_INFO_FLAGS_LONGITUDE_GREATER_99   0x04
#define GPS_INFO_FLAGS_GPS_FIX_VALID          0x08
#define GPS_INFO_FLAGS_GPS_DATA_RECEIVED      0x10
#define GPS_INFO_FLAGS_3D_FIX                 0x20
// #define	GPS_INFO_FLAGS_BIT6                   0x40 // ??
#define GPS_INFO_FLAGS_NEGATIVE_ALT           0x80

// FLITECTRL FLAGS
#define FLITECTRL_FLAGS_IS_AS3X_STAB      0x01
#define FLITECTRL_FLAGS_IS_ANGLE_DEMAND   0x02 
#define FLITECTRL_FLAGS_IS_AS3X_HEADING   0x04 
#define FLITECTRL_FLAGS_IS_SAFE_ENVELOPE  0x08 

// 1= Test sensor with captured telementry data
// See  replaceForTestingPackage(const uint8_t *packet) at the end of this file
#define TEST_CAPTURED_MESSAGE  0

enum SpektrumDataType : uint8_t {
  int8,
  int16,
  int32,
  uint8,
  uint16,
  uint32,
  uint8bcd,
  uint16bcd,
  uint32bcd,
  int16le,
  int32le,
  uint16le,
  uint32le,
  custom
};

struct SpektrumSensor {
  const uint8_t i2caddress;
  const uint8_t startByte:4;
  const SpektrumDataType dataType:4;
  const uint8_t precision;
  const TelemetryUnit unit;
  const char *name;
};

// clang-format off
#define SS(i2caddress,startByte,dataType,name,unit,precision) {i2caddress,startByte,dataType,precision,unit,name}

// IMPORTANT: Keep the sensor table incremtally sorted by i2caddress
const SpektrumSensor spektrumSensors[] = {
  // 0x01 High voltage internal sensor
  SS(I2C_VOLTAGE,      0,  int16,     STR_SENSOR_A1,                UNIT_VOLTS,     2), // 0.01V increments 

  // 0x02 Temperature internal sensor
  SS(I2C_TEMPERATURE,  0,  int16,     STR_SENSOR_TEMP1,             UNIT_FAHRENHEIT, 1), // Temperature in degrees Fahrenheit

  // 0x03 High current internal sensor (0x03), Resolution: 300A / 2048 = 0.196791 A/count
  SS(I2C_HIGH_CURRENT, 0,  int16,     STR_SENSOR_CURR,              UNIT_AMPS,      1), // Range: +/- 150A 

  // 0x0A Powerbox (also mentioned as 0x7D but that is also transmitter frame data)
  SS(I2C_PBOX,         0,  uint16,    STR_SENSOR_BATT1_VOLTAGE,     UNIT_VOLTS,     2),  // Volts, 0.01v
  SS(I2C_PBOX,         2,  uint16,    STR_SENSOR_BATT2_VOLTAGE,     UNIT_VOLTS,     2),
  SS(I2C_PBOX,         4,  uint16,    STR_SENSOR_BATT1_CONSUMPTION, UNIT_MAH,       0),  // mAh, 1mAh
  SS(I2C_PBOX,         6,  uint16,    STR_SENSOR_BATT2_CONSUMPTION, UNIT_MAH,       0),

  // 0x0B Lap Timer
  SS(I2C_LAPTIMER,     0, uint8,      STR_SENSOR_LAP_NUMBER,        UNIT_RAW,       0), // Lap last finished
  SS(I2C_LAPTIMER,     0, uint8,      STR_SENSOR_GATE_NUMBER,       UNIT_RAW,       0), // Last gate passed
  SS(I2C_LAPTIMER,     0, uint32,     STR_SENSOR_LAP_TIME,          UNIT_SECONDS,   3), // Time of lap in 1ms increments
  SS(I2C_LAPTIMER,     0, uint32,     STR_SENSOR_GATE_TIME,         UNIT_SECONDS,   3), // Duration between last 2 gates

  // Text Generator
//SS(I2C_TEXTGEN,      0, uint32,     STR_SENSOR_FLIGHT_MODE,       UNIT_TEXT,      0),

  // 0x11 AirSpeed
  SS(I2C_AIRSPEED,     0,  int16,     STR_SENSOR_ASPD,              UNIT_KMH,       0), // 1 km/h increments
//SS(I2C_AIRSPEED,     2,  int16,     STR_SENSOR_MAX_ASPD ?,        UNIT_KMH,       0), // 1 km/h increments

  // 0x012 Altitude
  SS(I2C_ALTITUDE,     0,  int16,     STR_SENSOR_ALT,               UNIT_METERS,    1), // .1m increments
//SS(I2C_ALTITUDE,     2,  int16,     STR_SENSOR_MAX_ALT ?,         UNIT_METERS,    1), // .1m increments

  // 0x14 G-Force
  SS(I2C_GMETER,       0,  int16,     STR_SENSOR_ACCX,              UNIT_G,         2), // force is reported as .01G increments
  SS(I2C_GMETER,       2,  int16,     STR_SENSOR_ACCY,              UNIT_G,         2), // 		Range = +/-4000 (+/- 40G) in Pro model
  SS(I2C_GMETER,       4,  int16,     STR_SENSOR_ACCZ,              UNIT_G,         2), // 		Range = +/-800 (+/- 8G) in Standard model
//SS(I2C_GMETER,       6,  int16,     STR_SENSOR_MAX_ACCX ?,        UNIT_G,         2), // abs(max G X-axis)   FORE/AFT
//SS(I2C_GMETER,       8,  int16,     STR_SENSOR_MAX_ACCY ?,        UNIT_G,         2), // abs (max G Y-axis)  LEFT/RIGHT
//SS(I2C_GMETER,      10,  int16,     STR_SENSOR_MAX_ACCZ ?,        UNIT_G,         2), // max G Z-axis        WING SPAR LOAD
//SS(I2C_GMETER,      12,  int16,     STR_SENSOR_MIN_ACCZ ?,        UNIT_G,         2), // min G Z-axis        WING SPAR LOAD

  // 0x15,  JETCAT/TURBINE, BCD Encoded values
  // TODO: Add decoding of status information
//SS(I2C_JETCAT,       0,  uint8,     STR_SENSOR_STATUS,            UNIT_BITFIELD,  0),
  SS(I2C_JETCAT,       1,  uint8bcd,  STR_SENSOR_THROTTLE,          UNIT_PERCENT,   0),
  SS(I2C_JETCAT,       2,  uint16bcd, STR_SENSOR_A1,                UNIT_VOLTS,     2),
  SS(I2C_JETCAT,       4,  uint16bcd, STR_SENSOR_A2,                UNIT_VOLTS,     2),
  SS(I2C_JETCAT,       6,  uint32bcd, STR_SENSOR_RPM,               UNIT_RPMS,      0),
  SS(I2C_JETCAT,      10,  uint16bcd, STR_SENSOR_TEMP1,             UNIT_CELSIUS,   0),

  // 0x16  GPS LOG
  SS(I2C_GPS_LOC,      0,  uint16bcd,  STR_SENSOR_GPSALT,           UNIT_METERS,    1), // Atl-Low BCD 3.1
  SS(I2C_GPS_LOC,      2,  uint32bcd,  STR_SENSOR_GPS,              UNIT_GPS,       0), // LAT and LON
  SS(I2C_GPS_LOC,     10,  uint16bcd,  STR_SENSOR_HDG,              UNIT_DEGREE,    1), // Course BCD 3.1
//SS(I2C_GPS_LOC,     12,  uint8bcd,   STR_SENSOR_HDOP ?,           UNIT_RAW,       1), // HDOP BCD 1.1

  // 0x17  GPS STAT
  SS(I2C_GPS_STAT,     0,  uint16bcd, STR_SENSOR_GSPD,              UNIT_KTS,       1), // BCD 3.1
  SS(I2C_GPS_STAT,     2,  uint32bcd, STR_SENSOR_GPSDATETIME,       UNIT_DATETIME,  1), // BCD 6.1 HH:MM:SS.S
  SS(I2C_GPS_STAT,     6,  uint8bcd,  STR_SENSOR_SATELLITES,        UNIT_RAW,       0), // BCD 2.0
 
  // 0x18  Dual Batt Capacity monitor - Receiver
  SS(I2C_RX_BATT,      0,  int16,     STR_SENSOR_BATT1_CURRENT,     UNIT_AMPS,      2), // Instantaneous current, 0.01A (0-327.66A)
  SS(I2C_RX_BATT,      2,  uint16,    STR_SENSOR_BATT1_CONSUMPTION, UNIT_MAH,       1), // Integrated mAh used, 0.1mAh (0-3276.6mAh)
  SS(I2C_RX_BATT,      4,  uint16,    STR_SENSOR_BATT1_VOLTAGE,     UNIT_VOLTS,     2), // Volts, 0.01V increments (0-16.00V)
  SS(I2C_RX_BATT,      6,  int16,     STR_SENSOR_BATT1_CURRENT,     UNIT_AMPS,      2), // Instantaneous current, 0.01A (0-327.66A)
  SS(I2C_RX_BATT,      8,  uint16,    STR_SENSOR_BATT1_CONSUMPTION, UNIT_MAH,       1), // Integrated mAh used, 0.1mAh (0-3276.6mAh)
  SS(I2C_RX_BATT,     10,  uint16,    STR_SENSOR_BATT1_VOLTAGE,     UNIT_VOLTS,     2), // Volts, 0.01V increments (0-16.00V)

  // 0x19 Jetcat flow rate
//SS(I2C_JETCAT_2,     0,  uint16bcd, STR_SENSOR_FUEL_CONSUMPTION,  UNIT_MILLILITERS_PER_MINUTE, 1), missing ml/min
  SS(I2C_JETCAT_2,     2,  uint32bcd, STR_SENSOR_FUEL,              UNIT_MILLILITERS, 1),

  // 0x1a Gyro
  SS(I2C_GYRO,         0,  int16,     STR_SENSOR_GYROX,             UNIT_DEGREE,    1), // Units are 0.1 deg/sec
  SS(I2C_GYRO,         2,  int16,     STR_SENSOR_GYROY,             UNIT_DEGREE,    1),
  SS(I2C_GYRO,         4,  int16,     STR_SENSOR_GYROZ,             UNIT_DEGREE,    1),
 //SS(I2C_GYRO,        6,  int16,     STR_SENSOR_MAX_GYROX ?,       UNIT_DEGREE,    1), // abs (max)
 //SS(I2C_GYRO,        8,  int16,     STR_SENSOR_MAX_GYROY ?,       UNIT_DEGREE,    1),
 //SS(I2C_GYRO,       10,  int16,     STR_SENSOR_MAX_GYROZ ?,       UNIT_DEGREE,    1),

  // 0x1b Attitude & Mag Compass
  // mag Units are tbd so probably no sensor in existance, ignore them for now
  SS(I2C_ATTMAG,       0,  int16,     STR_SENSOR_ROLL,              UNIT_DEGREE,    1), // Attitude, 3 axes.
  SS(I2C_ATTMAG,       2,  int16,     STR_SENSOR_PITCH,             UNIT_DEGREE,    1), // Units are 0.1 deg
  SS(I2C_ATTMAG,       4,  int16,     STR_SENSOR_YAW,               UNIT_DEGREE,    1),
//SS(I2C_ATTMAG,       6,  int16,     STR_SENSOR_MAGX ?,            ??,             1), // Magnetic Compass, 3 axes
//SS(I2C_ATTMAG,       8,  int16,     STR_SENSOR_MAGY ?,            ??,             1), // Units are 0.1mG
//SS(I2C_ATTMAG,      10,  int16,     STR_SENSOR_MAXZ ?,            ??,             1),
//SS(I2C_ATTMAG,      12,  int16,     STR_SENSOR_HDG ?,             UNIT_DEGREE,    1), // Heading, 0.1deg

  // 0x20 Smart ESC telemetry
  SS(I2C_ESC,          0,  uint16,    STR_SENSOR_ESC_RPM2,          UNIT_RPMS,      0), // Electrical RPM, 10RPM (0-655340 RPM)
  SS(I2C_ESC,          2,  uint16,    STR_SENSOR_ESC_VIN,           UNIT_VOLTS,     2), // Volts, 0.01v (0-655.34V)
  SS(I2C_ESC,          4,  uint16,    STR_SENSOR_ESC_TFET,          UNIT_CELSIUS,   1), // Temperature, 0.1C (0-6553.4C)
  SS(I2C_ESC,          6,  uint16,    STR_SENSOR_ESC_CUR,           UNIT_AMPS,      2), // Current, 10mA (0-655.34A)
  SS(I2C_ESC,          8,  uint16,    STR_SENSOR_ESC_TBEC,          UNIT_CELSIUS,   1), // Temperature, 0.1C (0-6553.4C)
  SS(I2C_ESC,         10,  uint8,     STR_SENSOR_ESC_BCUR,          UNIT_AMPS,      1), // BEC Current, 100mA (0-25.4A)
  SS(I2C_ESC,         11,  uint8,     STR_SENSOR_ESC_VBEC,          UNIT_VOLTS,     2), // BEC Volts, 0.05V (0-12.70V)
  SS(I2C_ESC,         12,  uint8,     STR_SENSOR_ESC_THR,           UNIT_PERCENT,   0), // 0.5% (0-100%)
  SS(I2C_ESC,         13,  uint8,     STR_SENSOR_ESC_POUT,          UNIT_PERCENT,   0), // Power Output, 0.5% (0-127%)

  // 0x26 GPS Binary
  SS(I2C_GPS_BIN,      0,  uint16,    STR_SENSOR_GPSALT,            UNIT_METERS,    0), // altitude m, 1000m offset
  SS(I2C_GPS_BIN,      2,  int32,     STR_SENSOR_GPS,               UNIT_GPS,       0), // latitude+long degree / 10,000,000
  SS(I2C_GPS_BIN,     10,  uint16,    STR_SENSOR_HDG,               UNIT_DEGREE,    1), // heading/course degree / 10
  SS(I2C_GPS_BIN,     12,  uint8,     STR_SENSOR_GSPD,              UNIT_KMH,       0), // ground speed Km/h
  SS(I2C_GPS_BIN,     13,  uint8,     STR_SENSOR_SATELLITES,        UNIT_RAW,       0), // count

  // 0x34 Dual Batt Capacity monitor - Flight Pack
  // The difference with sensor 0x18 (RX capacity monitor) is the consumption magnitude. 
  // RX (0x18) is up to 3.2A, and flight pack is up to 32A (x 10 or 1 decimal place)
  // Right now they have the same sensor name... could this sensors (0x18 and 0x34) be used at the same time? 
  SS(I2C_FP_BATT,       0,  int16le,     STR_SENSOR_BATT1_CURRENT,     UNIT_AMPS,     1), // Instantaneous current, 0.1A (0-3276.6A)
  SS(I2C_FP_BATT,       2,  int16le,     STR_SENSOR_BATT1_CONSUMPTION, UNIT_MAH,      0), // Integrated mAh used, 1mAh (0-32.766Ah)
  SS(I2C_FP_BATT,       4,  int16le,     STR_SENSOR_BATT1_TEMP,        UNIT_CELSIUS,  1), // Temperature, 0.1C (0-150C)
  SS(I2C_FP_BATT,       6,  int16le,     STR_SENSOR_BATT2_CURRENT,     UNIT_AMPS,     1), // Instantaneous current, 0.1A (0-3276.6A)
  SS(I2C_FP_BATT,       8,  int16le,     STR_SENSOR_BATT2_CONSUMPTION, UNIT_MAH,      0), // Integrated mAh used, 1mAh (0-32.766Ah)
  SS(I2C_FP_BATT,      10,  int16le,     STR_SENSOR_BATT2_TEMP,        UNIT_CELSIUS,  1), // Temperature, 0.1C (0-150C)

  // Tank pressure + custom input bits (ignore for now)
//SS(0x38,              0,  uint16,    STR_SENSOR_STATUS_BITS,      UNIT_BITFIELD,  0),
//SS(0x38,              0,  uint16,    STR_SENSOR_PRESSSURE,        UNIT_PSI,       1),

  // 0x3A Lipo 6s Monitor Cells
  SS(I2C_CELLS,        0,  uint16,    STR_SENSOR_CL01,              UNIT_VOLTS,     2), // Voltage across cell 1, .01V steps
  SS(I2C_CELLS,        2,  uint16,    STR_SENSOR_CL02,              UNIT_VOLTS,     2),
  SS(I2C_CELLS,        4,  uint16,    STR_SENSOR_CL03,              UNIT_VOLTS,     2),
  SS(I2C_CELLS,        6,  uint16,    STR_SENSOR_CL04,              UNIT_VOLTS,     2),
  SS(I2C_CELLS,        8,  uint16,    STR_SENSOR_CL05,              UNIT_VOLTS,     2),
  SS(I2C_CELLS,       10,  uint16,    STR_SENSOR_CL06,              UNIT_VOLTS,     2),
  SS(I2C_CELLS,       12,  uint16,    STR_SENSOR_TEMP2,             UNIT_CELSIUS,   1), // Temperature, 0.1C (0-655.34C)

  // 0x40 Vario-S
  SS(I2C_VARIO,         0,  int16,     STR_SENSOR_ALT,               UNIT_METERS,            1),
  SS(I2C_VARIO,         2,  int16,     STR_SENSOR_VSPD,              UNIT_METERS_PER_SECOND, 1),

  // 0x42 Smartbat
//SS(I2C_SMART_BAT_REALTIME,     1,  int8,      STR_SMART_BAT_BTMP,    UNIT_CELSIUS,             0),  // disabled because sensor is a duplicate of cells sensors ones
  SS(I2C_SMART_BAT_REALTIME,     2,  uint32le,  STR_SENSOR_SMART_BAT_BCUR,    UNIT_MAH,    0),
  SS(I2C_SMART_BAT_REALTIME,     6,  uint16le,  STR_SENSOR_SMART_BAT_BCAP,    UNIT_MAH,    0),
  SS(I2C_SMART_BAT_REALTIME,     8,  uint16le,  STR_SENSOR_SMART_BAT_MIN_CEL, UNIT_VOLTS,  2),
  SS(I2C_SMART_BAT_REALTIME,    10,  uint16le,  STR_SENSOR_SMART_BAT_MAX_CEL, UNIT_VOLTS,  2),
//SS(I2C_SMART_BAT_REALTIME,    12,  uint16le,  "RFU[2]",                     UNIT_RAW,    0),   // disabled to save sensors slots

  SS(I2C_SMART_BAT_CELLS_1_6,    1,  int8,      STR_SENSOR_SMART_BAT_BTMP,    UNIT_CELSIUS,  0),
  SS(I2C_SMART_BAT_CELLS_1_6,    2,  uint16le,  STR_SENSOR_CL01,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_1_6,    4,  uint16le,  STR_SENSOR_CL02,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_1_6,    6,  uint16le,  STR_SENSOR_CL03,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_1_6,    8,  uint16le,  STR_SENSOR_CL04,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_1_6,   10,  uint16le,  STR_SENSOR_CL05,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_1_6,   12,  uint16le,  STR_SENSOR_CL06,              UNIT_VOLTS,    2),

  SS(I2C_SMART_BAT_CELLS_7_12,   1,  int8,      STR_SENSOR_SMART_BAT_BTMP,    UNIT_CELSIUS,  0),
  SS(I2C_SMART_BAT_CELLS_7_12,   2,  uint16le,  STR_SENSOR_CL07,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_7_12,   4,  uint16le,  STR_SENSOR_CL08,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_7_12,   6,  uint16le,  STR_SENSOR_CL09,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_7_12,   8,  uint16le,  STR_SENSOR_CL10,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_7_12,  10,  uint16le,  STR_SENSOR_CL11,              UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_7_12,  12,  uint16le,  STR_SENSOR_CL12,              UNIT_VOLTS,    2),

  SS(I2C_SMART_BAT_CELLS_13_18,  1,  int8,     STR_SENSOR_SMART_BAT_BTMP,     UNIT_CELSIUS,  0),
  SS(I2C_SMART_BAT_CELLS_13_18,  2,  uint16le, STR_SENSOR_CL13,               UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_13_18,  4,  uint16le, STR_SENSOR_CL14,               UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_13_18,  6,  uint16le, STR_SENSOR_CL15,               UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_13_18,  8,  uint16le, STR_SENSOR_CL16,               UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_13_18, 10,  uint16le, STR_SENSOR_CL17,               UNIT_VOLTS,    2),
  SS(I2C_SMART_BAT_CELLS_13_18, 12,  uint16le, STR_SENSOR_CL18,               UNIT_VOLTS,    2),

  //SS(I2C_SMART_BAT_ID,              1,  uint8,  "chemistery",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_ID,              2,  uint8,  "number of cells",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_ID,              3,  uint8,  "manufacturer code",  UNIT_RAW, 0),   // disabled to save sensors slots
  SS(I2C_SMART_BAT_ID,              4,  uint16le,  STR_SENSOR_SMART_BAT_CYCLES,  UNIT_RAW,                 0),
  //SS(I2C_SMART_BAT_ID,              6,  uint8,  "uniqueID[8]",  UNIT_RAW, 0),   // disabled to save sensors slots

  //SS(I2C_SMART_BAT_LIMITS,          1,  uint8,  "rfu",  UNIT_RAW, 0),   // disabled to save sensors slots
  SS(I2C_SMART_BAT_LIMITS,          2,  uint16le,  STR_SENSOR_SMART_BAT_CAPACITY,UNIT_MAH,                 0),
  //SS(I2C_SMART_BAT_LIMITS,          4,  uint16le,  "dischargeCurrentRating",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_LIMITS,          6,  uint16le,  "overDischarge_mV",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_LIMITS,          8,  uint16le,  "zeroCapacity_mV",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_LIMITS,          10,  uint16le,  "fullyCharged_mV",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_LIMITS,          12,  uint8,  "minWorkingTemp",  UNIT_RAW, 0),   // disabled to save sensors slots
  //SS(I2C_SMART_BAT_LIMITS,          13,  uint8,  "maxWorkingTemp",  UNIT_RAW, 0),   // disabled to save sensors slots

  // 0x50-0x56 custom 3rd party sensors
  //SS(0x50, 0, int16, STR_SENSOR_)

  // 0x7d are transmitter channels frame data [7], probably only available on the Spektrum
  // telemetry bus on the model itself

  // 0x7E RPM/Volts/Temperature
  SS(I2C_RPM,          0,  uint16,    STR_SENSOR_RPM,               UNIT_RPMS,       0), // microseconds between pulse leading edges
  SS(I2C_RPM,          2,  uint16,    STR_SENSOR_A3,                UNIT_VOLTS,      2), // 0.01V increments (typically flight pack voltage)
  SS(I2C_RPM,          4,  int16,     STR_SENSOR_TEMP2,             UNIT_FAHRENHEIT, 0), // Temperature in degrees F.

  // 0x7f, QoS DATA, also called Flight Log
  SS(I2C_QOS,          0,  uint16,    STR_SENSOR_QOS_A,             UNIT_RAW,       0), // A - Antenna Fades on Receiver A
  SS(I2C_QOS,          2,  uint16,    STR_SENSOR_QOS_B,             UNIT_RAW,       0), // B - Antenna Fades on Receiver B
  SS(I2C_QOS,          4,  uint16,    STR_SENSOR_QOS_L,             UNIT_RAW,       0), // L - Antenna Fades on left Receiver
  SS(I2C_QOS,          6,  uint16,    STR_SENSOR_QOS_R,             UNIT_RAW,       0), // R - Antenna Fades on right Receiver
  SS(I2C_QOS,          8,  uint16,    STR_SENSOR_QOS_F,             UNIT_RAW,       0), // F - Frame losses.
  SS(I2C_QOS,          10, uint16,    STR_SENSOR_QOS_H,             UNIT_RAW,       0), // H - Holds
  SS(I2C_QOS,          12, uint16,    STR_SENSOR_A2,                UNIT_VOLTS,     2), // Volts, .01V increment.

  SS(I2C_PSEUDO_TX,    0,  uint8,     STR_SENSOR_TX_RSSI,           UNIT_RAW,       0),
  SS(I2C_PSEUDO_TX,    4,  uint32,    STR_SENSOR_BIND,              UNIT_RAW,       0),
  SS(I2C_PSEUDO_TX,    8,  uint32,    STR_SENSOR_FLIGHT_MODE,       UNIT_TEXT,      0),
  SS(I2C_PSEUDO_TX,    10, uint32,    STR_SENSOR_CELLS,             UNIT_CELLS,     2),
  SS(0,                0,  int16,     NULL,                         UNIT_RAW,       0) //sentinel
};
// clang-format on

// Alt Low and High needs to be combined (in 2 diff packets)
static uint8_t gpsAltHigh = 0;
static bool varioTelemetry = false;
static bool flightPackTelemetry = false;

// Helper function declared later
static void processAS3XPacket(const uint8_t *packet);
static void processAlpha6Packet(const uint8_t *packet);
static void adjustTimeFromUTC(uint8_t hour, uint8_t min, uint8_t sec,
                              struct gtm *tp);

#if TEST_CAPTURED_MESSAGE
static uint8_t replaceForTestingPackage(const uint8_t *packet);
#endif

static int32_t bcdToInt8(uint8_t bcd)
{
  return (bcd & 0xf) + 10 * (bcd >> 4 & 0xf);
}

static int32_t bcdToInt16(uint16_t bcd)
{
  return (bcd & 0xf) + 10 * (bcd >> 4 & 0xf) + 100 * (bcd >> 8 & 0xf) +
         1000 * (bcd >> 12 & 0xf);
}

static int32_t bcdToInt32(uint32_t bcd)
{
  return bcdToInt16(bcd >> 16) + 10000 * bcdToInt16(bcd);
}

// Spektrum uses Big Endian data types
static int32_t spektrumGetValue(const uint8_t *packet, int startByte, SpektrumDataType type)
{
  const uint8_t * data = packet + startByte;
  switch (type) {
    case uint8:
      return *((uint8_t *) (data));
    case int8:
      return *((int8_t *) (data));
    case int16:
      return (int16_t) ((uint16_t) (data[1] + (data[0] << 8)));
    case uint16:
      return ((uint16_t) (data[1] + (data[0] << 8)));
    case int32:
      return ((int32_t) (data[3] + (data[2] << 8) + (data[1] << 16) + (data[0] << 24)));
    case uint32:
      return ((uint32_t) (data[3] + (data[2] << 8) + (data[1] << 16) + (data[0] << 24)));
    case uint16bcd:
      return bcdToInt16(*(uint16_t *)data);
    case uint8bcd:
      return bcdToInt8(*(uint8_t *)data);
    case uint32bcd:
      return bcdToInt32(*(uint32_t *)data);
    case int16le:
      return (int16_t) ((int16_t) (data[0] + (data[1] << 8)));
    case uint16le:
      return (uint16_t) ((uint16_t) (data[0] + (data[1] << 8)));
    case int32le:
      return ((int32_t) (data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24)));
    case uint32le:
      return ((uint32_t) (data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24)));
    default:
      return -1;
  }
}

static bool isSpektrumValidValue(int32_t value, const SpektrumDataType type)
{
  switch (type) {
    case uint8:              
      return value != 0xff; 
    case uint16:
    case uint16le:
      return value != 0xffff;
    case int16:
    case int16le:
      return value != 0x7fff;
    case int32:
    case int32le:
      return value != 0x7fffffff;
    case uint32:
    case uint32le:
      return ((uint32_t) value) != 0xffffffff;
    default:
      return true;
  }
}

// Process GPS Stats Packet (BCD Date/Time and Altitude)
static void processGPSStatPacket(const uint8_t *packet, const uint16_t pseudoId, const uint8_t instance)
{
  //*********** GPS STAT *********************************
  // Example 0x17:  0  1    2  3  4  5    6    7
  //                25 00 | 00 28 15 17 | 06 | 01    
  //                Spd:002.5k, TimeUTC:17:15:28.00, Sats: 06, AltH=01      
  const uint8_t *packetData = packet + 4;  // Skip the header

  uint8_t sec = bcdToInt8(packetData[3]);
  uint8_t min = bcdToInt8(packetData[4]);
  uint8_t hour = bcdToInt8(packetData[5]);

  struct gtm td;
  adjustTimeFromUTC(hour, min, sec, &td);

  // Depending on the last byte it SETS DATE or TIME:  
  // DATE: HEX (YYMMDD01)   TIME: HEX(HHMMSS00)
  int32_t value = (td.tm_hour << 24) + (td.tm_min << 16) + (td.tm_sec << 8);
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance,
                    value, UNIT_DATETIME, 0);

  value = ((td.tm_year + 1900 - 2000) << 24) + ((td.tm_mon + 1) << 16) +
          (td.tm_mday << 8) + 0x01;
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance,
                    value, UNIT_DATETIME, 0);

  // Get Altitude High since we need to combine it with Alt-Low
  // Save the high part for later (0-99)
  gpsAltHigh = bcdToInt8(packetData[7]);
}

// Process GPS Location Packet (Lat/Long)
static void processGPSLocPacket(const uint8_t *packet, const uint16_t pseudoId, const uint8_t instance)
{
  //*********** GPS LOC *********************************
  // Example 0x16:  0  1    2  3  4  5    6  7  8  9    10 11   12   13
  //                97 00 | 54 71 12 28 | 40 80 09 82 | 85 14 | 13 | B9
  //                Alt: 009.7, LAT: 28o 12'7154, LON: -82 09 8040 Course: 148.5, HDOP 1.3 Flags= B9

  // Process LAT and LOG together
  const uint8_t *packetData = packet + 4;  // Skip the header

  uint8_t gpsFlags = packetData[13];

  // LATITUDE
  uint16_t fmin =
      bcdToInt8(packetData[2]) + (bcdToInt8(packetData[3]) * 100);
  uint8_t min = bcdToInt8(packetData[4]);
  uint8_t deg = bcdToInt8(packetData[5]);

  // formula from code in gps.cpp
  int32_t value = deg * 1000000UL + (min * 100000UL + fmin * 10UL) / 6;

  if ((gpsFlags & GPS_INFO_FLAGS_IS_NORTH) == 0) {  // SOUTH, negative
    value = -value;
  }
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance,
                    value, UNIT_GPS_LATITUDE, 0);

  // LONGITUDE
  fmin = bcdToInt8(packetData[6]) + (bcdToInt8(packetData[7]) * 100);
  min = bcdToInt8(packetData[8]);
  deg = bcdToInt8(packetData[9]);

  if ((gpsFlags & GPS_INFO_FLAGS_LONGITUDE_GREATER_99) != 0) {
    deg = deg + 100;
  }

  // formula from code in gps.cpp
  value = deg * 1000000UL + (min * 100000UL + fmin * 10UL) / 6;

  if ((gpsFlags & GPS_INFO_FLAGS_IS_EAST) == 0) {  // WEST, negative
    value = -value;
  }
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance,
                    value, UNIT_GPS_LONGITUDE, 0);
}

// Process Binary GPS Location Packet (Lat/Long)
static void processBinGPSLocPacket(const uint8_t *packet, const uint16_t pseudoId, const uint8_t instance)
{
  // LATITUDE
  int32_t value = spektrumGetValue(packet + 4, 2, int32);
  value = value / 10;
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance, value, UNIT_GPS_LATITUDE, 0);

  // LONGITUDE
  value = spektrumGetValue(packet + 4, 6, int32);
  value = value / 10;
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance, value, UNIT_GPS_LONGITUDE, 0);
}


void processSpektrumPacket(const uint8_t *packet)
{
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, I2C_PSEUDO_TX_RSSI, 0, 0,
                    packet[1], UNIT_RAW, 0);
  // highest bit indicates that TM1100 is in use, ignore it
  uint8_t i2cAddress = (packet[2] & 0x7f);

  uint8_t instance = packet[3];

  if (telemetryState == TELEMETRY_INIT) {  // Telemetry Reset?
    gpsAltHigh = 0;
    varioTelemetry = false;
    flightPackTelemetry = false;
  }

  if (i2cAddress == I2C_NODATA) {
    // Not a Sensor.. Telemetry is alive, but no data  (avoid creation of fake 0000,0002.. sensors)
    return; 
  }

#if TEST_CAPTURED_MESSAGE
  // Only for Testing when we don't have the sensor, but have sample data
  i2cAddress = replaceForTestingPackage(packet);
  instance = packet[3];
#endif

  if (i2cAddress == I2C_FWD_PGM) {
#if defined(LUA) && defined(MULTIMODULE)
    // Forward Programming
    if (Multi_Buffer && memcmp(Multi_Buffer, "DSM", 3) == 0) {
      // Multi_Buffer[0..2]=="DSM" -> Lua script is running
      // Multi_Buffer[3]==0x70 -> TX to RX data ready to be sent
      // Multi_Buffer[4..9]=6 bytes of TX to RX data
      // Multi_Buffer[10..25]=16 bytes of RX to TX data
      Multi_Buffer[10] = i2cAddress;
      memcpy(&Multi_Buffer[11], &packet[3], 15); // Store the received RX answer in the buffer
    }
#endif
    return; // Not a sensor
  }

#if defined(LUA) && defined(MULTIMODULE)
    // Generic way for LUA Script to request ANY Specktrum Telemety Raw Data
    // this can be used for TextGen or any other telemetry message

    if (Multi_Buffer && Multi_Buffer[3]==i2cAddress && Multi_Buffer[4]==0 && 
        memcmp(Multi_Buffer, "STR", 3) == 0) {
      // Multi_Buffer[0..2]=="STR" -> Lua script is running
      // Multi_Buffer[3]==i2C Addr -> I2C address of data requested in Lua script
      // Multi_Buffer[4]== Write Semaphore -> 0=Can Write Data. > 0, data not yet consumed
      // Multi_Buffer[5..20]=15 bytes of RX to TX data (any other data after i2c address)

      // Concurrency note: only going to write if Multi_Buffer[4]==0, that means that the
      // Lua script is ready for more data. Whithout this "semaphore", the lua script was getting messages
      // with mix of the old and new data. 

      memcpy(&Multi_Buffer[5], &packet[3], 16); // Store the received RX answer in the buffer
      Multi_Buffer[4]=i2cAddress; // Tell lua script data is ready
    }
#endif



  if (i2cAddress == I2C_TEXTGEN) {
    // TextGen now accessed via the new "Spectrum Telemetry Raw" LUA method  (See above code)
    // 0		byte:lineNumber	-- Line number to display (0 = title, 1-8 for general, 254 = Refresh backlight, 255 = Erase all text on screen)
	  // 1		char[13]        -- 0-terminated text when < 13 chars
    return;  // Not a sensor
  } // I2C_TEXTGEN

  else if (i2cAddress == I2C_FLITECTRL) {
    // AS3X + SAFE information: Flight Mode
    processAS3XPacket(packet);
    return; // not a sensor... this is to cleanup many auto-generated 05XX sensors
  } // I2C_FLITECTRL

  else if (i2cAddress == I2C_ALPHA6) {
    // Alpha6 Flight Controller (Blade Helis): Flight Mode
    processAlpha6Packet(packet);
    return; // not a sensor... this is to cleanup many auto-generated 24XX sensors
  } // I2C_ALPHA6

  else if (i2cAddress == I2C_SMART_BAT_BASE_ADDRESS) {
    // SmartBat Hack
    // use type to create virtual I2CAddresses
    i2cAddress = i2cAddress + (packet[4] >> 4);
  } // I2C_SMART_BAT_BASE_ADDRESS

  else if (i2cAddress == I2C_REMOTE_ID) { 
     if (instance == I2C_GPS_LOC || instance == I2C_GPS_STAT) {
      // RemoteID/SkyID GPS Data embeded in RemoteID packages
      // The format is exactly the same (with the exception of the I2C_ID and Instance), 
      // so we just need to continue processing it as if the frame was a GPS telemetry data.
      // The instance is populated with 0x16/0x17 when is GPS, and 0x00 when it is the
      // usual RemoteID data
      i2cAddress = instance;
      instance = 0;
     } else {
      // Currently we are not processing any other of the RemoteID telemetry frames
      // we can add in the future a new sensor(s) to record the Remote system ID if we want 
      // to log it as part of the telemetry data.
      return; // not a sensor... this is to cleanup many auto-generated 27XX sensors
     }
  } // I2C_REMOTE_ID


  bool handled = false;
  for (const SpektrumSensor * sensor = spektrumSensors; sensor->i2caddress; sensor++) {
    // Optimization... the sensor table is sorted incrementally by i2cAddress
    if (sensor->i2caddress < i2cAddress)  // haven't reach the sesnor def. keep going
      continue;
    if (sensor->i2caddress > i2cAddress)  // We past it, done
      break;  

    uint16_t pseudoId = (sensor->i2caddress << 8 | sensor->startByte);  
    handled = true;

    // Extract value, skip header
    int32_t value =
        spektrumGetValue(packet + 4, sensor->startByte, sensor->dataType);

    if (!isSpektrumValidValue(value, sensor->dataType))
      continue;

    // mV to VOLT PREC2 for Smart Batteries
    if ((i2cAddress >= I2C_SMART_BAT_REALTIME &&
          i2cAddress <= I2C_SMART_BAT_LIMITS) &&
        sensor->unit == UNIT_VOLTS) {
      if (value == -1) {
        continue;  // discard unavailable sensors (farzu: i think might not be needed.. previous validation)
      } else {
        value = value / 10;
      }
    } // I2C_SMART_BAT_REALTIME

    else if (i2cAddress == I2C_ESC) {
      if (sensor->unit == UNIT_RPMS) {   
        // RPM, 10RPM (0-655340 RPM)
        value = value * 10;
      }
      else if (sensor->startByte == 11) {
        // BEC Volts, 0.05V (0-12.70V)
        value = value * 5;
      }
      else if (sensor->startByte == 12 || sensor->startByte == 13) {
        // Throttle 0.5% (0-100%) or  Power 0.5% (0-127%)
        value = value / 2;
      }
    } // I2C_ESC

    else if (i2cAddress == I2C_CELLS && sensor->unit == UNIT_VOLTS) {
      if (value == 0x7FFF) continue;  // ignore NO-DATA

      // Map to FrSky style cell values (All Cells in a single Sensor)
      int cellIndex = (sensor->startByte / 2) << 16;
      uint32_t valueCells = cellIndex | value;
      setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, I2C_PSEUDO_TX_CELLS, 0, instance, valueCells, UNIT_CELLS, 2);
      
      // Continue to process regular Single Cell value
    } // I2C_CELLS

    else if (sensor->i2caddress == I2C_HIGH_CURRENT && sensor->unit == UNIT_AMPS) {
      // Spektrum's documents talks says: Resolution: 300A/2048 = 0.196791
      // A/tick Note that 300/2048 = 0,1464. DeviationTX also uses the
      // 0.196791 figure
      value = value * 196791 / 100000;
    } // I2C_HIGH_CURRENT

    else if (i2cAddress == I2C_QOS) {
      if (sensor->startByte == 0) {  // FdeA
        // Check if this looks like a LemonRX Transceiver, they use QoS Frame loss A as RSSI indicator(0-100)
        // farzu: new G2s has different signature, but i think using the Cyrf chip strength is
        //        more consistent across brands
        if (spektrumGetValue(packet + 4, 2, uint16) == 0x8000 &&
            spektrumGetValue(packet + 4, 4, uint16) == 0x8000 &&
            spektrumGetValue(packet + 4, 6, uint16) == 0x8000 &&
            spektrumGetValue(packet + 4, 8, uint16) == 0x8000) {
          telemetryData.rssi.set(value);
        }
        else {
          // Otherwise use the received signal strength of the telemetry packet as indicator
          // Range is 0-31, multiply by 3 to get an almost full reading for 0x1f, the maximum the cyrf chip reports
          telemetryData.rssi.set(packet[1] * 3);
        }
        telemetryStreaming = TELEMETRY_TIMEOUT10ms; // Telemery Alive
      } // FdeA
      else if (sensor->startByte == 8 || sensor->startByte == 10) { // Flss and Hold
        // Lemon-RX: F and H = 0x7FFF (alternative N0-DATA)
        if (value == 0x7FFF) continue; 
      }
    } // I2C_QOS

    else if (sensor->i2caddress == I2C_GPS_STAT && sensor->unit == UNIT_DATETIME) {
      // Process Date/Time together
      processGPSStatPacket(packet, pseudoId,  instance);
      continue;  // setTelemetryValue handled
    } // I2C_GPS_STAT

    else if (sensor->i2caddress == I2C_GPS_LOC) {
      if (sensor->startByte == 0) {
        // ALTITUDE LOW (METERS)
        uint8_t gpsFlags = packet[4 + 13];

        // Format Decimal: HHLLLL, for display will be HHLLL.L
        value = (gpsAltHigh * 1000) + value;

        if (gpsFlags & GPS_INFO_FLAGS_NEGATIVE_ALT) {
          value = -value;
        }
      }
      else if (sensor->unit == UNIT_GPS) {
        // Process LAT and LOG together
        processGPSLocPacket(packet, pseudoId,  instance);
        continue;  // setTelemetryValue handled
      }
    } // I2C_GPS_LOC

    else if (i2cAddress == I2C_GPS_BIN) {
      // GPS Binary
      if (sensor->startByte == 0) {
        //mstrens:  Altitude: to take care of 1000m offset
        value = value - 1000; 
      }
      else if (sensor->unit == UNIT_GPS)  {
         // Process LAT and LOG together
        processBinGPSLocPacket(packet, pseudoId, instance);
        continue; // setTelemetryValue handled
      }
    } // I2C_GPS_BIN

    else if (i2cAddress == I2C_FP_BATT) {
      flightPackTelemetry = true;
      // Lemon-RX G2: No Bat2: Current (-1.0 A)
      if (sensor->startByte == 6 && ((int16_t) value) == -10) {
        continue;
      }  
    } // I2C_FP_BATT

    else if (i2cAddress == I2C_PBOX) {
      if (flightPackTelemetry && (sensor->startByte == 4 ||  sensor->startByte == 6)) {
          // hide mAh Consumption already reported in Fligh Pack message 
          continue;
      }
      else if ((sensor->startByte == 0 || sensor->startByte == 4) && 
          spektrumGetValue(packet + 4, 0, uint16) == 0) {
          // No Bat1 Voltage, hide Voltage and Consumption
          continue;
      }
      else if ((sensor->startByte == 2 || sensor->startByte == 6) && 
          spektrumGetValue(packet + 4, 2, uint16) == 0) {
          // No Bat2 Voltage, hide Voltage and Consumption
          continue;
      }
    } // I2C_PBOX

    else if (i2cAddress == I2C_VARIO) {
      varioTelemetry = true;
    }
    else if (i2cAddress == I2C_ALTITUDE && varioTelemetry) {
      // Altitude already reported in vario
      continue; 
    }

    setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance, value, sensor->unit, sensor->precision);
  } // FOR

  if (!handled) {
    // If we see a sensor that is not handled at all, add the raw values of this sensor to show its existance to
    // the user and help debugging/implementing these sensors
    for (int startByte=0; startByte<14; startByte+=2) {
      int32_t value = spektrumGetValue(packet + 4, startByte, uint16);
      uint16_t pseudoId = i2cAddress << 8 | startByte;
      setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, pseudoId, 0, instance, value, UNIT_RAW, 0);
    }
  }
}

// Parse the DSM2 bind response, Fields are as per http://www.rcgroups.com/forums/showpost.php?p=35692146&postcount=5191
// "I"  here means the multi module

/*
0-3   4 bytes -> Cyrf ID of the TX xor 0xFF but don't care...
4     1 byte -> RX version but don't care...
5     1 byte -> number of channels, example 0x06=6 channels
6     1 byte -> max DSM type allowed:
        0x01 => 1024 DSM2 1 packet => number of channels is <8 and no telemetry
        0x02 => 1024 DSM2 2 packets => either a number of channel >7 or telemetry enable RX
        0x12 => 2048 DSM2 2 packets => can be any number of channels with/without telemetry -> this mode might be supported following Mike's trials, note the channels should be duplicated between the packets which is not the case today
        0xa2 => 2048 DSMX 1 packet => number of channels is <8 and no telemetry
        0xb2 => 2048 DSMX 2 packets => can be any number of channels with/without telemetry -> this mode is only half supported since the channels should be duplicated between the packets which is not the case but might be supported following Mike's trials
7     0x00: not sure of the use of this byte since I've always seen it at 0...
8-9   2 bytes CRC but don't care...

 Examples:           DSM   #Chan  RXver
 Inductrix           0xa2   07     1
 LemonRX+Sat+tele    0xb2   07     1

 */
void processDSMBindPacket(uint8_t module, const uint8_t *packet)
{
  uint32_t debugval;

  if (g_model.moduleData[module].type == MODULE_TYPE_LEMON_DSMP) {

    // save flags
    g_model.moduleData[module].dsmp.flags = packet[0];

    // save number of channels
    uint8_t channels = packet[2];
    if (channels > 12) { channels = 12; }
    g_model.moduleData[module].channelsCount = channels - 8;

    TRACE("[SPK] DSMP bind packet: 0x%X / %i",
          packet[0] & 0x3F, packet[2]);

    storageDirty(EE_MODEL);

    moduleState[module].mode = MODULE_MODE_NORMAL;
    restartModuleAsync(module, 50);  // ~500ms
  }
#if defined(MULTIMODULE)
  else if (g_model.moduleData[module].type == MODULE_TYPE_MULTIMODULE &&
             g_model.moduleData[module].multi.rfProtocol ==
             MODULE_SUBTYPE_MULTI_DSM2 &&
             g_model.moduleData[module].subType == MM_RF_DSM2_SUBTYPE_AUTO) {

    // Only sets channel etc when in DSM/AUTO mode
    int channels = packet[5];
    if (channels > 12) {
      channels = 12;
    }
    else if (channels < 3) {
      channels = 3;
    }

    switch(packet[6]) {
      case 0xa2:
        g_model.moduleData[module].subType = MM_RF_DSM2_SUBTYPE_DSMX_22;
        break;
      case 0x12:
        g_model.moduleData[module].subType = MM_RF_DSM2_SUBTYPE_DSM2_11;
        if (channels == 7) {
          channels = 12;    // change the number of channels if 7
        }
        break;
      case 0x01:
      case 0x02:
        g_model.moduleData[module].subType = MM_RF_DSM2_SUBTYPE_DSM2_22;
        break;
      default: // 0xb2 or unknown
        g_model.moduleData[module].subType = MM_RF_DSM2_SUBTYPE_DSMX_11;
        if (channels == 7) {
          channels = 12;    // change the number of channels if 7
        }
        break;
    }

    g_model.moduleData[module].channelsCount = channels - 8;
    // clear the 11ms servo refresh rate flag
    g_model.moduleData[module].multi.optionValue &= 0xFD;

    storageDirty(EE_MODEL);
  }
#endif
  debugval = packet[7] << 24 | packet[6] << 16 | packet[5] << 8 | packet[4];

  /* log the bind packet as telemetry for quick debugging */
  setTelemetryValue(PROTOCOL_TELEMETRY_SPEKTRUM, I2C_PSEUDO_TX_BIND, 0, 0,
                    debugval, UNIT_RAW, 0);

  /* Finally stop binding as the rx just told us that it is bound */
  if (getModuleMode(module) == MODULE_MODE_BIND) {
    auto module_type = g_model.moduleData[module].type;
#if defined(MULTIMODULE)
    if (module_type == MODULE_TYPE_MULTIMODULE &&
        g_model.moduleData[module].multi.rfProtocol ==
            MODULE_SUBTYPE_MULTI_DSM2) {
      setMultiBindStatus(module, MULTI_BIND_FINISHED);
    } else
#endif
    if (module_type == MODULE_TYPE_LEMON_DSMP) {
      setModuleMode(module, MODULE_MODE_NORMAL);
    }
  }
}
  
void processSpektrumTelemetryData(uint8_t module, uint8_t data,
                                  uint8_t *rxBuffer, uint8_t &rxBufferCount)
{
  if (rxBufferCount == 0 && data != 0xAA) {
    TRACE("[SPK] invalid start byte 0x%02X", data);
    return;
  }

  if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
    rxBuffer[rxBufferCount++] = data;
  }
  else {
    TRACE("[SPK] array size %d error", rxBufferCount);
    rxBufferCount = 0;
  }

  if (rxBuffer[1] == 0x80 && rxBufferCount >= DSM_BIND_PACKET_LENGTH) {
    processDSMBindPacket(module, rxBuffer+2);
    rxBufferCount = 0;
    return;
  }

  if (rxBufferCount >= SPEKTRUM_TELEMETRY_LENGTH) {
    // Debug print content of Telemetry to console
#if 0
    debugPrintf("[SPK] Packet 0x%02X rssi 0x%02X: ic2 0x%02x, %02x: ",
                rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
    for (int i=4; i<SPEKTRUM_TELEMETRY_LENGTH; i+=4) {
      debugPrintf("%02X%02X %02X%02X  ", rxBuffer[i], rxBuffer[i + 1],
                  rxBuffer[i + 2], rxBuffer[i + 3]);
    }
    debugPrintf(CRLF);
#endif
    processSpektrumPacket(rxBuffer);
    rxBufferCount = 0;
  }
}

const SpektrumSensor *getSpektrumSensor(uint16_t pseudoId)
{
  uint8_t startByte = (uint8_t)(pseudoId & 0xff);
  uint8_t i2cadd = (uint8_t)(pseudoId >> 8);
  for (const SpektrumSensor *sensor = spektrumSensors; sensor->i2caddress;
       sensor++) {
    if (i2cadd == sensor->i2caddress && startByte == sensor->startByte) {
      return sensor;
    }
  }
  return nullptr;
}

void spektrumSetDefault(int index, uint16_t id, uint8_t subId, uint8_t instance)
{
  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
  telemetrySensor.id = id;
  telemetrySensor.subId = subId;
  telemetrySensor.instance = instance;

  const SpektrumSensor *sensor = getSpektrumSensor(id);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    uint8_t prec = min<uint8_t>(2, sensor->precision);
    telemetrySensor.init(sensor->name, unit, prec);

    if (unit == UNIT_RPMS) {
      telemetrySensor.custom.ratio = 1;
      telemetrySensor.custom.offset = 1;
    } else if (unit == UNIT_FAHRENHEIT) {
      if (!IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_CELSIUS;
      }
    } else if (unit == UNIT_CELSIUS) {
      if (IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_FAHRENHEIT;
      }
    } else if (unit == UNIT_METERS) {
      if (IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_FEET;
      }
    } else if (unit == UNIT_KMH) {
      if (IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_KTS;
      }
    } else if (unit == UNIT_METERS_PER_SECOND) {
      if (IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_FEET_PER_SECOND;
      }
    } else if (unit == UNIT_KTS) {
      if (!IS_IMPERIAL_ENABLE()) {
        telemetrySensor.unit = UNIT_KMH;
      }
    }
  } else {
    telemetrySensor.init(id);
  }

  storageDirty(EE_MODEL);
}

extern int __offtime(const gtime_t *t, long int offset, struct gtm *tp);

static void adjustTimeFromUTC(uint8_t hour, uint8_t min, uint8_t sec,
                              struct gtm *tp)
{
  // Get current UTC date/time
  __offtime(&g_rtcTime, -timezoneOffsetSeconds(g_eeGeneral.timezone, g_eeGeneral.timezoneMinutes), tp);

  tp->tm_hour = hour;
  tp->tm_min = min;
  tp->tm_sec = sec;

  gtime_t newTime = gmktime(tp);
  __offtime(&newTime, +g_eeGeneral.timezone * 3600, tp);
}

// AS3X/SAFE flight Controller
// Contains Flight Mode, and Gyro Settings
static void processAS3XPacket(const uint8_t *packet)
{
    const uint8_t *packetData = packet + 4;  // Skip the header

    // uint8_t  state = packetData[1];
    uint8_t flags = packetData[0];
    // uint8_t  dataPage = packetData[3] & 0x0F;
    //  0=Gains, 1=Headings, 2=Angle Limits
    uint8_t flightMode = packetData[2] & 0x0F;

    char text[50];

    sprintf(text, "%d ", flightMode + 1);

    if (flags & FLITECTRL_FLAGS_IS_AS3X_STAB) {
    strcat(text, "AS3X");
    }

    // only one should show
    if (flags & FLITECTRL_FLAGS_IS_ANGLE_DEMAND) {
    strcat(text, " Level");
    } else if (flags & FLITECTRL_FLAGS_IS_SAFE_ENVELOPE) {
    strcat(text, " Envelope");
    } else if (flags & FLITECTRL_FLAGS_IS_AS3X_HEADING) {
    strcat(text, " Heading");
    }

    setTelemetryText(PROTOCOL_TELEMETRY_SPEKTRUM, I2C_PSEUDO_TX_FM, 0, 0, text);
}

// Alpha6 flight Controller  (Blade Helis based on AR636)
// Contains Flight Mode, and Gyro Settings
static void processAlpha6Packet(const uint8_t *packet)
{
  const uint8_t *packetData = packet + 4;  // Skip the header

  uint8_t status = packetData[2] & 0x0F;
  uint8_t flightMode = packetData[2] >> 4 & 0x0F;

  char text[50];

  sprintf(text, "%d ", flightMode);

  if (flightMode == 0) {
    strcat(text, "NOR");
  } else if (flightMode == 1) {
    strcat(text, "INT");
  } else if (flightMode == 2) {
    strcat(text, "ADV");
  } else if (flightMode == 5) {
    strcat(text, "PANIC");
  }

  if (status == 2) {
    strcat(text, " HOLD");
  }

  setTelemetryText(PROTOCOL_TELEMETRY_SPEKTRUM, I2C_PSEUDO_TX_FM, 0, 0, text);
}

#if TEST_CAPTURED_MESSAGE
// For Testing purposes, replace the package for data captured
static int testStep = 0;

// Keep track if we received real packets of this types
static bool real0x16 = false;
static bool real0x17 = false;
static bool real0x34 = false;
static bool real0x3A = false;
static bool real0x27 = false;

// *********** GPS LOC (BCD) ******************************
// Example 0x16:          0  1    2  3  4  5    6  7  8  9    10 11   12   13
//                16 00 | 97 00 | 54 71 12 28 | 40 80 09 82 | 85 14 | 13 | B9
//                Alt: 009.7, LAT: 28o 12'7154, LON: -82 09 8040 Course: 148.5, HDOP 1.3 Flags= B9
//static char test16data[] = {0x16, 0x00, 0x97, 0x00, 0x54, 0x71, 0x12, 0x28,
//                            0x40, 0x80, 0x09, 0x82, 0x85, 0x14, 0x13, 0xB9};

static char test16data[] = {0x16, 0x00, 0x97, 0x00, 0x54, 0x71, 0x12, 0x28,
                            0x40, 0x80, 0x09, 0x11, 0x85, 0x14, 0x13, 0xBD}; // > 99 Flag

// *********** GPS STAT (BCD) *****************************
// Example 0x17:          0  1    2  3  4  5    6    7
//                17 00 | 25 00 | 00 28 18 21 | 06 | 00    
//                Spd:002.5k, TimeUTC:21:18:28.00, Sats: 06, AltH=00
static char test17data[] = {0x17, 0x00, 0x25, 0x00, 0x00,
                            0x28, 0x18, 0x21, 0x06, 0x00};

// *********** Dual Flight pack monitor (Little-Endian)***************
// Example 0x34:          0  1    2  3    4  5    6  7    8  9    10 11 
//                34 00 | 2F 00 | 30 09 | 85 01 | 2B 00 | 07 0A | 81 01 
//                B1: 004.7A, 2352mAh, 38.9C   B2: 004.3A, 2567mAh, 38.5C  
static char test34data[] = {0x34, 0x00, 0x2F, 0x00, 0x30, 0x09, 0x85, 0x01, 
                                        0x2B, 0x00, 0x07, 0x0A, 0x81, 0x01 };

// *********** Lipo monitor (Big-Endian)***************
// Example 0x3A:          0  1    2  3    4  5    6  7    8  9    10 11   12 13
//                3A 00 | 01 9A | 01 9B | 01 9C | 01 9D | 7F FF | 7F FF | 0F AC 
//                         4.10V   4.11V   4.12V   4.12v   --      --     40.1C
static char test3Adata[] = {0x3A, 0x00, 0x01, 0x9A, 0x01, 0x9B, 0x01, 0x9C, 
                                        0x01, 0x9D, 0x7F, 0xFF, 0x7F, 0xFF,
                                        0x01, 0x91 };


// RemoteID (0x27), embeds Gps data in its frames, but by puting the real I2C frame
// address as the instance, everything else is the same
static char test27data_16[] = {0x27, 0x16, 0x97, 0x00, 0x54, 0x71, 0x12, 0x28,
                            0x40, 0x80, 0x09, 0x11, 0x85, 0x14, 0x13, 0xBD}; // > 99 Flag
static char test27data_17[] = {0x27, 0x17, 0x25, 0x00, 0x00,
                            0x28, 0x18, 0x21, 0x06, 0x00};

static uint8_t replaceForTestingPackage(const uint8_t *packet)
{
  uint8_t i2cAddress = packet[2] & 0x7f;

  // If we received a real package for the ones that we can Fake it, disable replacement
  if (i2cAddress == I2C_GPS_LOC) real0x16 = true;
  else if (i2cAddress == I2C_GPS_STAT) real0x17 = true;
  else if (i2cAddress == I2C_FP_BATT) real0x34 = true;
  else if (i2cAddress == I2C_CELLS) real0x3A = true;
  else if (i2cAddress == I2C_REMOTE_ID) real0x27 = true;
  
  // Only Substiture AS3X/SAFE I2C_FLITECTRL packages, since they are constantly brodcast
  if (i2cAddress != I2C_FLITECTRL) {  
    return i2cAddress;
  }

  switch (testStep) {
    case 0:
        // return original packet
        break;
    case 1: // return GSP LOG
        if (!real0x27) {
          test27data_16[4]=test27data_16[4]+1;
          test27data_16[8]=test27data_16[8]+1;
          memcpy((char *)packet + 2, test27data_16, 16);
          real0x16=true; // disable test of regular GPS frames, and use the RemoteID
        }
        if (!real0x16) {
          test16data[4]=test16data[4]+1;
          test16data[8]=test16data[8]+1;
          memcpy((char *)packet + 2, test16data, 16);
        }
        break;
    case 2: // Return GPS STAT
        if (!real0x27) {
          memcpy((char *)packet + 2, test27data_17, 10);
          real0x17=true; // disable test of regular GPS frames, and use the RemoteID
        }
        if (!real0x17) memcpy((char *)packet + 2, test17data, 10);
        break;
    case 3: // Return Dual Bat monitor
        if (!real0x34) memcpy((char *)packet + 2, test34data, 14);
        break;
    case 4: // Return LIPO monitor
        if (!real0x3A) memcpy((char *)packet + 2, test3Adata, 16);
        break;
  }

  testStep = (testStep + 1) % 5;
  

  return packet[2] & 0x7f;
}
#endif
