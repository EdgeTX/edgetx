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

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */

#define TR_MIN_PLURAL2 2

#if defined(TRANSLATIONS_CZ) || defined(TRANSLATIONS_PL)
  #define TR_MAX_PLURAL2 4
  #define TR_USE_PLURAL2_SPECIAL_CASE 1
#else
  #define TR_MAX_PLURAL2 2
  #define TR_USE_PLURAL2_SPECIAL_CASE 0
#endif

// For this number of minute in the last decimal place singular form is used in plural
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_DA) || defined(TRANSLATIONS_DE) || defined(TRANSLATIONS_ES) || \
    defined(TRANSLATIONS_FI) || defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_JP) || defined(TRANSLATIONS_NL) || \
    defined(TRANSLATIONS_PT) || defined(TRANSLATIONS_SE) || defined(TRANSLATIONS_TW)
  #define TR_USE_SINGULAR_IN_PLURAL 1
#else
  #define TR_USE_SINGULAR_IN_PLURAL 0
#endif

// If the number of minutes is above this value PLURAL2 is used
#if defined(TRANSLATIONS_CZ)
  #define TR_USE_PLURAL2 20
#else
  #define TR_USE_PLURAL2 INT_MAX
#endif

constexpr int g_max_plural2 = TR_MAX_PLURAL2;
constexpr int g_min_plural2 = TR_MIN_PLURAL2;
constexpr int g_use_singular_in_plural = TR_USE_SINGULAR_IN_PLURAL;
constexpr int g_use_plural2_special_case = TR_USE_PLURAL2_SPECIAL_CASE;
constexpr int g_use_plural2 = TR_USE_PLURAL2;

extern const char CHR_HOUR;
extern const char CHR_INPUT;

#define STR_UPDATE_LIST STR_DELAYDOWN

#define STR_V (STR_VTELEMUNIT[1])
#define STR_A (STR_VTELEMUNIT[2])

// ACCESS STUFF
#define STR_SBUSIN                     "SBUS in"
#define STR_SBUSOUT                    "SBUS out"
#define STR_SPORT                      "S.PORT"
#define STR_FBUS                       "FBUS"
#define STR_SBUS24                     "SBUS24"

// Telemetry sensor name definitions
#define STR_SENSOR_RSSI                      "RSSI"
#define STR_SENSOR_R9PW                      "R9PW"
#define STR_SENSOR_RAS                       "SWR"
#define STR_SENSOR_A1                        "A1"
#define STR_SENSOR_A2                        "A2"
#define STR_SENSOR_A3                        "A3"
#define STR_SENSOR_A4                        "A4"
#define STR_SENSOR_VOLT                      "Volt"
#define STR_SENSOR_BATT                      "RxBt"
#define STR_SENSOR_ALT                       "Alt"
#define STR_SENSOR_TEMP                      "Temp"
#define STR_SENSOR_TEMP1                     "Tmp1"
#define STR_SENSOR_TEMP2                     "Tmp2"
#define STR_SENSOR_TEMP3                     "Tmp3"
#define STR_SENSOR_TEMP4                     "Tmp4"
#define STR_SENSOR_RPM2                      "RPM2"
#define STR_SENSOR_PRES                      "Pres"
#define STR_SENSOR_ODO1                      "Odo1"
#define STR_SENSOR_ODO2                      "Odo2"
#define STR_SENSOR_TXV                       "TX_V"
#define STR_SENSOR_CURR_SERVO1               "CSv1"
#define STR_SENSOR_CURR_SERVO2               "CSv2"
#define STR_SENSOR_CURR_SERVO3               "CSv3"
#define STR_SENSOR_CURR_SERVO4               "CSv4"
#define STR_SENSOR_DIST                      "Dist"
#define STR_SENSOR_ARM                       "Arm"
#define STR_SENSOR_C50                       "C50"
#define STR_SENSOR_C200                      "C200"
#define STR_SENSOR_RPM                       "RPM"
#define STR_SENSOR_FUEL                      "Fuel"
#define STR_SENSOR_VSPD                      "VSpd"
#define STR_SENSOR_ACCX                      "AccX"
#define STR_SENSOR_ACCY                      "AccY"
#define STR_SENSOR_ACCZ                      "AccZ"
#define STR_SENSOR_ACC                       "Acc"
#define STR_SENSOR_GYROX                     "GYRX"
#define STR_SENSOR_GYROY                     "GYRY"
#define STR_SENSOR_GYROZ                     "GYRZ"
#define STR_SENSOR_CURR                      "Curr"
#define STR_SENSOR_CAPACITY                  "Capa"
#define STR_SENSOR_VFAS                      "VFAS"
#define STR_SENSOR_ASPD                      "ASpd"
#define STR_SENSOR_GSPD                      "GSpd"
#define STR_SENSOR_HDG                       "Hdg"
#define STR_SENSOR_SATELLITES                "Sats"
#define STR_SENSOR_CELLS                     "Cels"
#define STR_SENSOR_GPSALT                    "GAlt"
#define STR_SENSOR_GPSDATETIME               "Date"
#define STR_SENSOR_GPS                       "GPS"
#define STR_SENSOR_BATT1_VOLTAGE             "RB1V"
#define STR_SENSOR_BATT2_VOLTAGE             "RB2V"
#define STR_SENSOR_BATT1_CURRENT             "RB1A"
#define STR_SENSOR_BATT2_CURRENT             "RB2A"
#define STR_SENSOR_BATT1_CONSUMPTION         "RB1C"
#define STR_SENSOR_BATT2_CONSUMPTION         "RB2C"
#define STR_SENSOR_BATT1_TEMP                "RB1T"
#define STR_SENSOR_BATT2_TEMP                "RB2T"
#define STR_SENSOR_RB_STATE                  "RBS"
#define STR_SENSOR_CHANS_STATE               "RBCS"
#define STR_SENSOR_RX_RSSI1                  "1RSS"
#define STR_SENSOR_RX_RSSI2                  "2RSS"
#define STR_SENSOR_RX_QUALITY                "RQly"
#define STR_SENSOR_RX_SNR                    "RSNR"
#define STR_SENSOR_RX_SIGNAL                 "Sgnl"
#define STR_SENSOR_RX_NOISE                  "RNse"
#define STR_SENSOR_ANTENNA                   "ANT"
#define STR_SENSOR_RF_MODE                   "RFMD"
#define STR_SENSOR_TX_POWER                  "TPWR"
#define STR_SENSOR_TX_RSSI                   "TRSS"
#define STR_SENSOR_TX_QUALITY                "TQly"
#define STR_SENSOR_RX_RSSI_PERC              "RRSP"
#define STR_SENSOR_RX_RF_POWER               "RPWR"
#define STR_SENSOR_TX_RSSI_PERC              "TRSP"
#define STR_SENSOR_TX_FPS                    "TFPS"
#define STR_SENSOR_TX_SNR                    "TSNR"
#define STR_SENSOR_TX_NOISE                  "TNse"
#define STR_SENSOR_PITCH                     "Ptch"
#define STR_SENSOR_ROLL                      "Roll"
#define STR_SENSOR_YAW                       "Yaw"
#define STR_SENSOR_FLIGHT_MODE               "FM"
#define STR_SENSOR_THROTTLE                  "Thr"
#define STR_SENSOR_QOS_A                     "FdeA"
#define STR_SENSOR_QOS_B                     "FdeB"
#define STR_SENSOR_QOS_L                     "FdeL"
#define STR_SENSOR_QOS_R                     "FdeR"
#define STR_SENSOR_QOS_F                     "FLss"
#define STR_SENSOR_QOS_H                     "Hold"
#define STR_SENSOR_BIND                      "BIND"
#define STR_SENSOR_LAP_NUMBER                "Lap "
#define STR_SENSOR_GATE_NUMBER               "Gate"
#define STR_SENSOR_LAP_TIME                  "LapT"
#define STR_SENSOR_GATE_TIME                 "GteT"
#define STR_SENSOR_ESC_VOLTAGE               "EscV"
#define STR_SENSOR_ESC_CURRENT               "EscA"
#define STR_SENSOR_ESC_RPM                   "EscR"   //FrSky name
#define STR_SENSOR_ESC_RPM2                  "Erpm"   //Spektrum name
#define STR_SENSOR_ESC_CONSUMPTION           "EscC"
#define STR_SENSOR_ESC_TEMP                  "EscT"
#define STR_SENSOR_SD1_CHANNEL               "Chan"
#define STR_SENSOR_GASSUIT_TEMP1             "GTp1"
#define STR_SENSOR_GASSUIT_TEMP2             "GTp2"
#define STR_SENSOR_GASSUIT_RPM               "GRPM"
#define STR_SENSOR_GASSUIT_FLOW              "GFlo"
#define STR_SENSOR_GASSUIT_CONS              "GFue"
#define STR_SENSOR_GASSUIT_RES_VOL           "GRVl"
#define STR_SENSOR_GASSUIT_RES_PERC          "GRPc"
#define STR_SENSOR_GASSUIT_MAX_FLOW          "GMFl"
#define STR_SENSOR_GASSUIT_AVG_FLOW          "GAFl"
#define STR_SENSOR_SBEC_VOLTAGE              "BecV"
#define STR_SENSOR_SBEC_CURRENT              "BecA"
#define STR_SENSOR_LOSS                      "Loss" 
#define STR_SENSOR_FLOW                      "Flow"
#define STR_SENSOR_SPEED                     "Spd"
#define STR_SENSOR_FRAME_RATE                "FRat"
#define STR_SENSOR_TOTAL_LATENCY             "TLat"
#define STR_SENSOR_VTX_FREQ                  "VFrq"
#define STR_SENSOR_VTX_PWR                   "VPwr"
#define STR_SENSOR_VTX_CHAN                  "VChn"
#define STR_SENSOR_VTX_BAND                  "VBan"
#define STR_SENSOR_BATT_PERCENT              "Bat%"
#define STR_SENSOR_ESC_VIN                   "EVIN"
#define STR_SENSOR_ESC_TFET                  "TFET"
#define STR_SENSOR_ESC_CUR                   "ECUR"
#define STR_SENSOR_ESC_TBEC                  "TBEC"
#define STR_SENSOR_ESC_BCUR                  "CBEC"
#define STR_SENSOR_ESC_VBEC                  "VBEC"
#define STR_SENSOR_ESC_THR                   "ETHR"
#define STR_SENSOR_ESC_POUT                  "EOUT"
#define STR_SENSOR_SMART_BAT_BTMP            "BTmp"
#define STR_SENSOR_SMART_BAT_BCUR            "BCur"
#define STR_SENSOR_SMART_BAT_BCAP            "BUse"
#define STR_SENSOR_SMART_BAT_MIN_CEL         "CLMi"
#define STR_SENSOR_SMART_BAT_MAX_CEL         "CLMa"
#define STR_SENSOR_SMART_BAT_CYCLES          "Cycl"
#define STR_SENSOR_SMART_BAT_CAPACITY        "BCpT"
#define STR_SENSOR_CL01                      "Cel1"
#define STR_SENSOR_CL02                      "Cel2"
#define STR_SENSOR_CL03                      "Cel3"
#define STR_SENSOR_CL04                      "Cel4"
#define STR_SENSOR_CL05                      "Cel5"
#define STR_SENSOR_CL06                      "Cel6"
#define STR_SENSOR_CL07                      "Cel7"
#define STR_SENSOR_CL08                      "Cel8"
#define STR_SENSOR_CL09                      "Cel9"
#define STR_SENSOR_CL10                      "Cl10"
#define STR_SENSOR_CL11                      "Cl11"
#define STR_SENSOR_CL12                      "Cl12"
#define STR_SENSOR_CL13                      "Cl13"
#define STR_SENSOR_CL14                      "Cl14"
#define STR_SENSOR_CL15                      "Cl15"
#define STR_SENSOR_CL16                      "Cl16"
#define STR_SENSOR_CL17                      "Cl17"
#define STR_SENSOR_CL18                      "Cl18"
#define STR_SENSOR_VFR                       "VFR"
#define STR_SENSOR_RB3040_EXTRA_STATE        "RBES"
#define STR_SENSOR_RB3040_CHANNEL1           "CH1A"
#define STR_SENSOR_RB3040_CHANNEL2           "CH2A"
#define STR_SENSOR_RB3040_CHANNEL3           "CH3A"
#define STR_SENSOR_RB3040_CHANNEL4           "CH4A"
#define STR_SENSOR_RB3040_CHANNEL5           "CH5A"
#define STR_SENSOR_RB3040_CHANNEL6           "CH6A"
#define STR_SENSOR_RB3040_CHANNEL7           "CH7A"
#define STR_SENSOR_RB3040_CHANNEL8           "CH8A"
#define STR_SENSOR_SERVO_CURRENT             "SrvA"
#define STR_SENSOR_SERVO_VOLTAGE             "SrvV"
#define STR_SENSOR_SERVO_TEMPERATURE         "SrvT"
#define STR_SENSOR_SERVO_STATUS              "SrvS"
#define STR_SENSOR_SPECIAL                   "Spcl"

// RX stat labeling for settings/telemetry, range
// check and internal/external module setup screens 
#define STR_RXSTAT_LABEL_RSSI   "RSSI "
#define STR_RXSTAT_LABEL_RQLY   "Rqly "
#define STR_RXSTAT_LABEL_SIGNAL "Sgnl "
#define STR_RXSTAT_UNIT_PERCENT " %"
#define STR_RXSTAT_UNIT_DBM     " db"
#define STR_RXSTAT_UNIT_NOUNIT  ""

#define STR_CHAR_RIGHT     "\302\200"
#define STR_CHAR_LEFT      "\302\201"
#define STR_CHAR_UP        "\302\202"
#define STR_CHAR_DOWN      "\302\203"

#define STR_CHAR_DELTA     "\302\210"
#define STR_CHAR_STICK     "\302\211"
#define STR_CHAR_POT       "\302\212"
#define STR_CHAR_SLIDER    "\302\213"
#define STR_CHAR_SWITCH    "\302\214"
#define STR_CHAR_TRIM      "\302\215"
#define STR_CHAR_INPUT     "\302\216"
#define STR_CHAR_FUNCTION  "\302\217"
#define STR_CHAR_CYC       "\302\220"
#define STR_CHAR_TRAINER   "\302\221"
#define STR_CHAR_CHANNEL   "\302\222"
#define STR_CHAR_TELEMETRY "\302\223"
#define STR_CHAR_LUA       "\302\224"
#define STR_CHAR_LS        "\302\225"
#define STR_CHAR_CURVE     "\302\226"

#define STR_CHAR_BW_GREATEREQUAL '}'
#define STR_CHAR_BW_DEGREE       '@'

//
// HoTT Telemetry sensor names by Hott device
//
// naming convention: Name of device in capital letters (1 or 2) followed by sensor name in lower case letters
//
// example: GPal = GPS altitude, GAal = GAM altitude, Valt = Vario altitude, GAt2 = GAM temperature sensor 2
//
// T  = transmitter
// R  = receiver
// V  = Vario
// G  = GPS
// ES = ESC
// EA = EAM
//
// ssi = signal strength indicator
// qly = quality
// bt  = battery
// evt = HoTT warnings
// btm = battery lowest voltage
// vpk = VPack
// al or alt = altitude
// vv  = vertical velocity (climb rate)
// hd  = heading
// sp  = speed
// PS  = GPS coordinates
// di  = direction
// ns  = number of satellites
// cp  = capacity
// u   = voltage (may be followed by numner if device offers more voltage sensors
// i   = current (may be followed by numner if device offers more current sensors
// tmp or t = temperature (single t may be followed by numner if device offers more temperature sensors
// rp or r  = temperature (single r may be followed by numner if device offers more temperature sensors
// fl = fuel
//  
// TX
#define STR_SENSOR_HOTT_ID_TX_RSSI_DL     "Tssi"
#define STR_SENSOR_HOTT_ID_TX_LQI_DL      "Tqly"
// RX
#define STR_SENSOR_HOTT_ID_RX_RSSI_UL     "Rssi"
#define STR_SENSOR_HOTT_ID_RX_LQI_UL      "Rqly"
#define STR_SENSOR_HOTT_ID_RX_VLT         "Rbt"
#define STR_SENSOR_HOTT_ID_RX_TMP         "Rtmp" 
#define STR_SENSOR_HOTT_ID_RX_BAT_MIN     "Rbtm"
#define STR_SENSOR_HOTT_ID_RX_VPCK        "Rvpk"
#define STR_SENSOR_HOTT_ID_RX_EVENT       "Revt"
// Vario
#define STR_SENSOR_HOTT_ID_VARIO_ALT      "Valt"
#define STR_SENSOR_HOTT_ID_VARIO_VV       "Vvv"
#define STR_SENSOR_HOTT_ID_VARIO_HDG      "Vhdg"
// GPS
#define STR_SENSOR_HOTT_ID_GPS_HDG        "GPhd"
#define STR_SENSOR_HOTT_ID_GPS_SPEED      "GPsp"
#define STR_SENSOR_HOTT_ID_GPS_LL         "GPS"
#define STR_SENSOR_HOTT_ID_GPS_DST        "GPdi"
#define STR_SENSOR_HOTT_ID_GPS_ALT        "GPal" 
#define STR_SENSOR_HOTT_ID_GPS_VV         "GPvv"
#define STR_SENSOR_HOTT_ID_GPS_NSATS      "GPns"
// ESC
#define STR_SENSOR_HOTT_ID_ESC_VLT        "ESu1"
#define STR_SENSOR_HOTT_ID_ESC_CAP        "EScp"
#define STR_SENSOR_HOTT_ID_ESC_TMP        "ESt1" 
#define STR_SENSOR_HOTT_ID_ESC_CUR        "ESi1"
#define STR_SENSOR_HOTT_ID_ESC_RPM        "ESrp"
#define STR_SENSOR_HOTT_ID_ESC_BEC_VLT    "ESu2"
#define STR_SENSOR_HOTT_ID_ESC_BEC_CUR    "ESi2" 
#define STR_SENSOR_HOTT_ID_ESC_BEC_TMP    "ESt2"
#define STR_SENSOR_HOTT_ID_ESC_MOT_TMP    "ESt3"
// GAM
#define STR_SENSOR_HOTT_ID_GAM_CELS       "GAcl"
#define STR_SENSOR_HOTT_ID_GAM_VLT1       "GAu1"
#define STR_SENSOR_HOTT_ID_GAM_VLT2       "GAu2"
#define STR_SENSOR_HOTT_ID_GAM_TMP1       "GAt1"
#define STR_SENSOR_HOTT_ID_GAM_TMP2       "GAt2"
#define STR_SENSOR_HOTT_ID_GAM_FUEL       "GAfl"
#define STR_SENSOR_HOTT_ID_GAM_RPM1       "GAr1"
#define STR_SENSOR_HOTT_ID_GAM_ALT        "GAal"
#define STR_SENSOR_HOTT_ID_GAM_VV         "GAvv"
#define STR_SENSOR_HOTT_ID_GAM_CUR        "GAi"
#define STR_SENSOR_HOTT_ID_GAM_VLT3       "GAu3"
#define STR_SENSOR_HOTT_ID_GAM_CAP        "GAcp"
#define STR_SENSOR_HOTT_ID_GAM_SPEED      "GAsp"
#define STR_SENSOR_HOTT_ID_GAM_RPM2       "GAr2"
// EAM
#define STR_SENSOR_HOTT_ID_EAM_CELS_L     "EAc1"
#define STR_SENSOR_HOTT_ID_EAM_CELS_H     "EAc2"
#define STR_SENSOR_HOTT_ID_EAM_VLT1       "EAu1"
#define STR_SENSOR_HOTT_ID_EAM_VLT2       "EAu2"
#define STR_SENSOR_HOTT_ID_EAM_TMP1       "EAt1"
#define STR_SENSOR_HOTT_ID_EAM_TMP2       "EAt2"
#define STR_SENSOR_HOTT_ID_EAM_ALT        "EAal"
#define STR_SENSOR_HOTT_ID_EAM_CUR        "EAi"
#define STR_SENSOR_HOTT_ID_EAM_VLT3       "EAu3"
#define STR_SENSOR_HOTT_ID_EAM_CAP        "EAcp"
#define STR_SENSOR_HOTT_ID_EAM_VV         "EAvv"
#define STR_SENSOR_HOTT_ID_EAM_RPM        "EArp"
#define STR_SENSOR_HOTT_ID_EAM_SPEED      "EAsp"
