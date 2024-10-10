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

#pragma once

#define FRSKY_SPORT_PACKET_SIZE   9

// Enumerate FrSky packet codes
#define LINKPKT                   0xfe
#define USRPKT                    0xfd
#define A11PKT                    0xfc
#define A12PKT                    0xfb
#define A21PKT                    0xfa
#define A22PKT                    0xf9
#define ALRM_REQUEST              0xf8
#define RSSI1PKT                  0xf7
#define RSSI2PKT                  0xf6
#define RSSI_REQUEST              0xf1

// FrSky PRIM IDs (1 byte)
#define DATA_FRAME                0x10

// FrSky old DATA IDs (1 byte)
#define GPS_ALT_BP_ID             0x01
#define TEMP1_ID                  0x02
#define RPM_ID                    0x03
#define FUEL_ID                   0x04
#define TEMP2_ID                  0x05
#define VOLTS_ID                  0x06
#define GPS_ALT_AP_ID             0x09
#define BARO_ALT_BP_ID            0x10
#define GPS_SPEED_BP_ID           0x11
#define GPS_LONG_BP_ID            0x12
#define GPS_LAT_BP_ID             0x13
#define GPS_COURS_BP_ID           0x14
#define GPS_DAY_MONTH_ID          0x15
#define GPS_YEAR_ID               0x16
#define GPS_HOUR_MIN_ID           0x17
#define GPS_SEC_ID                0x18
#define GPS_SPEED_AP_ID           0x19
#define GPS_LONG_AP_ID            0x1A
#define GPS_LAT_AP_ID             0x1B
#define GPS_COURS_AP_ID           0x1C
#define BARO_ALT_AP_ID            0x21
#define GPS_LONG_EW_ID            0x22
#define GPS_LAT_NS_ID             0x23
#define ACCEL_X_ID                0x24
#define ACCEL_Y_ID                0x25
#define ACCEL_Z_ID                0x26
#define CURRENT_ID                0x28
#define VARIO_ID                  0x30
#define VFAS_ID                   0x39
#define VOLTS_BP_ID               0x3A
#define VOLTS_AP_ID               0x3B
#define FRSKY_LAST_ID             0x3F
#define D_RSSI_ID                 0xF0
#define D_A1_ID                   0xF1
#define D_A2_ID                   0xF2

#define VFAS_D_HIPREC_OFFSET      2000

// FrSky new DATA IDs (2 bytes)
#define ALT_FIRST_ID              0x0100
#define ALT_LAST_ID               0x010F
#define VARIO_FIRST_ID            0x0110
#define VARIO_LAST_ID             0x011F
#define CURR_FIRST_ID             0x0200
#define CURR_LAST_ID              0x020F
#define VFAS_FIRST_ID             0x0210
#define VFAS_LAST_ID              0x021F
#define CELLS_FIRST_ID            0x0300
#define CELLS_LAST_ID             0x030F
#define T1_FIRST_ID               0x0400
#define T1_LAST_ID                0x040F
#define T2_FIRST_ID               0x0410
#define T2_LAST_ID                0x041F
#define RPM_FIRST_ID              0x0500
#define RPM_LAST_ID               0x050F
#define FUEL_FIRST_ID             0x0600
#define FUEL_LAST_ID              0x060F
#define ACCX_FIRST_ID             0x0700
#define ACCX_LAST_ID              0x070F
#define ACCY_FIRST_ID             0x0710
#define ACCY_LAST_ID              0x071F
#define ACCZ_FIRST_ID             0x0720
#define ACCZ_LAST_ID              0x072F
#define ANGLE_FIRST_ID            0x0730
#define ANGLE_LAST_ID             0x073F
#define GPS_LONG_LATI_FIRST_ID    0x0800
#define GPS_LONG_LATI_LAST_ID     0x080F
#define GPS_ALT_FIRST_ID          0x0820
#define GPS_ALT_LAST_ID           0x082F
#define GPS_SPEED_FIRST_ID        0x0830
#define GPS_SPEED_LAST_ID         0x083F
#define GPS_COURS_FIRST_ID        0x0840
#define GPS_COURS_LAST_ID         0x084F
#define GPS_TIME_DATE_FIRST_ID    0x0850
#define GPS_TIME_DATE_LAST_ID     0x085F
#define A3_FIRST_ID               0x0900
#define A3_LAST_ID                0x090F
#define A4_FIRST_ID               0x0910
#define A4_LAST_ID                0x091F
#define AIR_SPEED_FIRST_ID        0x0A00
#define AIR_SPEED_LAST_ID         0x0A0F
#define FUEL_QTY_FIRST_ID         0x0A10
#define FUEL_QTY_LAST_ID          0x0A1F
#define RBOX_BATT1_FIRST_ID       0x0B00
#define RBOX_BATT1_LAST_ID        0x0B0F
#define RBOX_BATT2_FIRST_ID       0x0B10
#define RBOX_BATT2_LAST_ID        0x0B1F
#define RBOX_STATE_FIRST_ID       0x0B20
#define RBOX_STATE_LAST_ID        0x0B2F
#define RBOX_CNSP_FIRST_ID        0x0B30
#define RBOX_CNSP_LAST_ID         0x0B3F
#define SD1_FIRST_ID              0x0B40
#define SD1_LAST_ID               0x0B4F
#define ESC_POWER_FIRST_ID        0x0B50
#define ESC_POWER_LAST_ID         0x0B5F
#define ESC_RPM_CONS_FIRST_ID     0x0B60
#define ESC_RPM_CONS_LAST_ID      0x0B6F
#define ESC_TEMPERATURE_FIRST_ID  0x0B70
#define ESC_TEMPERATURE_LAST_ID   0x0B7F
#define RB3040_OUTPUT_FIRST_ID    0x0B80
#define RB3040_OUTPUT_LAST_ID     0x0B8F
#define RB3040_CH1_2_FIRST_ID     0x0B90
#define RB3040_CH1_2_LAST_ID      0x0B9F
#define RB3040_CH3_4_FIRST_ID     0x0BA0
#define RB3040_CH3_4_LAST_ID      0x0BAF
#define RB3040_CH5_6_FIRST_ID     0x0BB0
#define RB3040_CH5_6_LAST_ID      0x0BBF
#define RB3040_CH7_8_FIRST_ID     0x0BC0
#define RB3040_CH7_8_LAST_ID      0x0BCF
#define X8R_FIRST_ID              0x0C20
#define X8R_LAST_ID               0x0C2F
#define S6R_FIRST_ID              0x0C30
#define S6R_LAST_ID               0x0C3F
#define GASSUIT_TEMP1_FIRST_ID    0x0D00
#define GASSUIT_TEMP1_LAST_ID     0x0D0F
#define GASSUIT_TEMP2_FIRST_ID    0x0D10
#define GASSUIT_TEMP2_LAST_ID     0x0D1F
#define GASSUIT_SPEED_FIRST_ID    0x0D20
#define GASSUIT_SPEED_LAST_ID     0x0D2F
#define GASSUIT_RES_VOL_FIRST_ID  0x0D30
#define GASSUIT_RES_VOL_LAST_ID   0x0D3F
#define GASSUIT_RES_PERC_FIRST_ID 0x0D40
#define GASSUIT_RES_PERC_LAST_ID  0x0D4F
#define GASSUIT_FLOW_FIRST_ID     0x0D50
#define GASSUIT_FLOW_LAST_ID      0x0D5F
#define GASSUIT_MAX_FLOW_FIRST_ID 0x0D60
#define GASSUIT_MAX_FLOW_LAST_ID  0x0D6F
#define GASSUIT_AVG_FLOW_FIRST_ID 0x0D70
#define GASSUIT_AVG_FLOW_LAST_ID  0x0D7F
#define SBEC_POWER_FIRST_ID       0x0E50
#define SBEC_POWER_LAST_ID        0x0E5F
#define DIY_FIRST_ID              0x5100
#define DIY_LAST_ID               0x52FF
#define DIY_STREAM_FIRST_ID       0x5000
#define DIY_STREAM_LAST_ID        0x50FF
#define SERVO_FIRST_ID            0x6800
#define SERVO_LAST_ID             0x680F
#define FACT_TEST_ID              0xF000
#define VALID_FRAME_RATE_ID       0xF010
#define RSSI_ID                   0xF101
#define ADC1_ID                   0xF102
#define ADC2_ID                   0xF103
#define BATT_ID                   0xF104
#define RAS_ID                    0xF105
#define XJT_VERSION_ID            0xF106
#define R9_PWR_ID                 0xF107
#define SP2UART_A_ID              0xFD00
#define SP2UART_B_ID              0xFD01

// Virtual IDs, value can be changed to anything only used for display
#define RX_LQI_ID                 0xFFFC
#define TX_LQI_ID                 0xFFFD
#define TX_RSSI_ID                0xFFFE

// Default sensor data IDs (Physical IDs + CRC)
#define DATA_ID_VARIO             0x00 // 0
#define DATA_ID_FLVSS             0xA1 // 1
#define DATA_ID_FAS               0x22 // 2
#define DATA_ID_GPS               0x83 // 3
#define DATA_ID_RPM               0xE4 // 4
#define DATA_ID_SP2UH             0x45 // 5
#define DATA_ID_SP2UR             0xC6 // 6
