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

#if defined(PCBX12S)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"6P", STR_CHAR_POT"S2", STR_CHAR_TRIM"L1", STR_CHAR_TRIM"L2", STR_CHAR_SLIDER"LS", STR_CHAR_SLIDER"RS", STR_CHAR_POT"JSx", STR_CHAR_POT"JSy"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG", STR_CHAR_SWITCH"SH", STR_CHAR_SWITCH"SI", STR_CHAR_SWITCH"SJ"
#elif defined(PCBX10)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"6P", STR_CHAR_POT"S2", STR_CHAR_POT"EX1", STR_CHAR_POT"EX2", STR_CHAR_POT"EX3", STR_CHAR_POT"EX4", STR_CHAR_SLIDER"LS", STR_CHAR_SLIDER"RS", STR_CHAR_POT"JSx", STR_CHAR_POT"JSy"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG", STR_CHAR_SWITCH"SH", STR_CHAR_SWITCH"SI", STR_CHAR_SWITCH"SJ"
#elif defined(PCBX9E)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"F1", STR_CHAR_POT"F2", STR_CHAR_POT"F3", STR_CHAR_POT"F4", STR_CHAR_SLIDER"S1", STR_CHAR_SLIDER"S2", STR_CHAR_SLIDER"LS", STR_CHAR_SLIDER"RS"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG", STR_CHAR_SWITCH"SH", STR_CHAR_SWITCH"SI", STR_CHAR_SWITCH"SJ", STR_CHAR_SWITCH"SK", STR_CHAR_SWITCH"SL", STR_CHAR_SWITCH"SM", STR_CHAR_SWITCH"SN", STR_CHAR_SWITCH"SO", STR_CHAR_SWITCH"SP", STR_CHAR_SWITCH"SQ\0", STR_CHAR_SWITCH"SR\0"
#elif defined(PCBXLITE)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF"
#elif defined(RADIO_TPRO)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2"
  #define TR_SW_VSRCRAW                                                      \
    STR_CHAR_SWITCH                                                          \
    "SA", STR_CHAR_SWITCH "SB", STR_CHAR_SWITCH "SC", STR_CHAR_SWITCH "SD",  \
        STR_CHAR_SWITCH "SW1", STR_CHAR_SWITCH "SW2", STR_CHAR_SWITCH "SW3", \
        STR_CHAR_SWITCH "SW4", STR_CHAR_SWITCH "SW5", STR_CHAR_SWITCH "SW6"
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define TR_POTS_VSRCRAW STR_CHAR_POT "S1", STR_CHAR_POT "S2"
  #define TR_SW_VSRCRAW                                                   \
    STR_CHAR_SWITCH "SA", STR_CHAR_SWITCH "SB", STR_CHAR_SWITCH "SC",     \
        STR_CHAR_SWITCH "SD", STR_CHAR_SWITCH "SG", STR_CHAR_SWITCH "SH", \
        STR_CHAR_SWITCH "SI", STR_CHAR_SWITCH "SJ"
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define TR_POTS_VSRCRAW STR_CHAR_POT "S1", STR_CHAR_POT "S2"
  #define TR_SW_VSRCRAW                     \
    STR_CHAR_SWITCH "SA", STR_CHAR_SWITCH "SB", STR_CHAR_SWITCH  \
                    "SC", STR_CHAR_SWITCH "SD", STR_CHAR_SWITCH  \
                    "SE", STR_CHAR_SWITCH "SF", STR_CHAR_SWITCH  \
                    "SI", STR_CHAR_SWITCH "SJ"
#elif defined(RADIO_ZORRO)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH "SA", STR_CHAR_SWITCH "SB", STR_CHAR_SWITCH "SC", STR_CHAR_SWITCH "SD", STR_CHAR_SWITCH "SE", STR_CHAR_SWITCH "SF", STR_CHAR_SWITCH "SG", STR_CHAR_SWITCH "SH"
#elif defined(PCBX7)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SH", STR_CHAR_SWITCH"SI", STR_CHAR_SWITCH"SJ"
#elif defined(PCBX9LITES)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG"
#elif defined(PCBX9LITE)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE"
#elif defined(PCBTARANIS)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2", STR_CHAR_POT"S3", STR_CHAR_SLIDER"LS", STR_CHAR_SLIDER"RS"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG", STR_CHAR_SWITCH"SH", STR_CHAR_SWITCH"SI"
#elif defined(PCBNV14)
  #define TR_POTS_VSRCRAW              STR_CHAR_POT"S1", STR_CHAR_POT"S2"
  #define TR_SW_VSRCRAW                STR_CHAR_SWITCH"SA", STR_CHAR_SWITCH"SB", STR_CHAR_SWITCH"SC", STR_CHAR_SWITCH"SD", STR_CHAR_SWITCH"SE", STR_CHAR_SWITCH"SF", STR_CHAR_SWITCH"SG", STR_CHAR_SWITCH"SH"
#endif

// only special switches here
#define TR_VSWITCHES                   "---", TR_TRIMS_SWITCHES, TR_ON_ONE_SWITCHES

#define TR_VTRAINERMODES                                                \
        TR_VTRAINER_MASTER_OFF, TR_VTRAINER_MASTER_JACK, TR_VTRAINER_SLAVE_JACK, \
        TR_VTRAINER_MASTER_SBUS_MODULE, TR_VTRAINER_MASTER_CPPM_MODULE, \
        TR_VTRAINER_MASTER_BATTERY, TR_VTRAINER_BLUETOOTH, TR_VTRAINER_MULTI

#define TR_VSRCRAW                     "---", TR_STICKS_VSRCRAW, TR_POTS_VSRCRAW, TR_IMU_VSRCRAW "MAX", TR_CYC_VSRCRAW, TR_TRIMS_VSRCRAW, TR_SW_VSRCRAW, TR_EXTRA_VSRCRAW

#define TR_CRSF_BAUDRATE               "115k","400k","921k","1.87M","3.75M","5.25M"

#define TR_MODULE_R9M_LITE             "R9MLite"

#define TR_EXTERNAL_MODULE_PROTOCOLS \
  "OFF",                             \
    "PPM",                           \
    "XJT",                           \
    "ISRM",                          \
    "DSM2",                          \
    "CRSF",                          \
    "MULTI",                         \
    "R9M",                           \
    "R9M ACCESS",                    \
    TR_MODULE_R9M_LITE,              \
    "R9ML ACCESS",                   \
    "GHST",                          \
    "R9MLP ACCESS",                  \
    "SBUS",                          \
    "XJT Lite",                      \
    "FLYSKY",                        \
    TR("Lemon DSMP","LemonRx DSMP")

#define TR_INTERNAL_MODULE_PROTOCOLS   TR_EXTERNAL_MODULE_PROTOCOLS

#define TR_FLYSKY_PROTOCOLS            "AFHDS3","AFHDS2A"
#define TR_XJT_ACCST_RF_PROTOCOLS      "D16","D8","LR12"
#define TR_ISRM_RF_PROTOCOLS           "ACCESS","D16","LR12"

#define TR_SPORT_MODES                 "S.PORT","F.PORT","FBUS(FPORT2)"
#define TR_R9M_PXX2_RF_PROTOCOLS       "ACCESS","FCC","EU","Flex"
#define TR_R9M_REGION                  "FCC","EU","868MHz","915MHz"
#define TR_R9M_LITE_FCC_POWER_VALUES   "(100mW)"
#define TR_R9M_LITE_LBT_POWER_VALUES   "25mW 8CH","25mW 16CH","100mW NoTele"
#define TR_R9M_FCC_POWER_VALUES        "10mW","100mW","500mW","1W (auto)"
#define TR_R9M_LBT_POWER_VALUES        "25mW 8CH","25mW 16CH","200mW NoTele","500mW NoTele"

#define TR_DSM_PROTOCOLS               "LP45","DSM2","DSMX"

#define TR_MULTI_PROTOCOLS             "FlySky","Hubsan","FrSky","Hisky","V2x2","DSM","Devo","YD717","KN","SymaX","SLT","CX10","CG023", \
        "Bayang","ESky","MT99XX","MJXq","Shenqi","FY326","Futaba","J6 Pro","FQ777","Assan","Hontai","OpenLrs","FlSky2A","Q2x2","Walkera", \
        "Q303","GW008","DM002","Cabell","Esky150","H8 3D","Corona","CFlie","Hitec","WFly","Bugs","BugMini","Traxxas","NCC1701","E01X", \
        "V911S","GD00X","V761","KF606","Redpine","Potensi","ZSX","Height","Scanner","FrSkyRX","FS2A_RX","HoTT","FX816","BayanRX","Pelikan","Tiger", \
        "XK","XN297DU","FrSkyX2","FrSkyR9","Propel","FrSkyL","Skyartc","ESky-v2","DSM RX","JJRC345","Q90C","Kyosho","RadLink","ExpLRS","Realacc", \
        "OMP","M-Link","Wfly 2","E016Hv2","E010r5","LOLI","E129","JOYSWAY","E016H","Config","IKEA","WILLIFM","Losi","MouldKg","Xerall","MT99XX2"

#define TR_MULTI_POWER                 "1.6mW","2.0mW","2.5mW","3.2mW","4.0mW","5.0mW","6.3mW","7.9mW","10mW","13mW","16mW","20mW","25mW","32mW","40mW","50mW"
#define TR_MULTI_WBUS_MODE             "WBUS","PPM"

#define TR_AFHDS3_PROTOCOLS            "PWM/IBUS","PWM/SBUS","PPM/IBUS","PPM/SBUS"
#define TR_AFHDS3_POWERS               "25 mW","100 mW","500 mW","1 W","2 W"
#define TR_FLYSKY_PULSE_PROTO          "PWM","PPM"
#define TR_FLYSKY_SERIAL_PROTO         "iBUS","SBUS"
#define TR_PPM_POL                     "-","+"
#define TR_PWR_OFF_DELAYS              "0s","1s","2s","4s"

#define TR_SENSOR_RSSI                      "RSSI"
#define TR_SENSOR_R9PW                      "R9PW"
#define TR_SENSOR_RAS                       "SWR"
#define TR_SENSOR_A1                        "A1"
#define TR_SENSOR_A2                        "A2"
#define TR_SENSOR_A3                        "A3"
#define TR_SENSOR_A4                        "A4"
#define TR_SENSOR_BATT                      "RxBt"
#define TR_SENSOR_ALT                       "Alt"
#define TR_SENSOR_TEMP1                     "Tmp1"
#define TR_SENSOR_TEMP2                     "Tmp2"
#define TR_SENSOR_TEMP3                     "Tmp3"
#define TR_SENSOR_TEMP4                     "Tmp4"
#define TR_SENSOR_RPM2                      "RPM2"
#define TR_SENSOR_PRES                      "Pres"
#define TR_SENSOR_ODO1                      "Odo1"
#define TR_SENSOR_ODO2                      "Odo2"
#define TR_SENSOR_TXV                       "TX_V"
#define TR_SENSOR_CURR_SERVO1               "CSv1"
#define TR_SENSOR_CURR_SERVO2               "CSv2"
#define TR_SENSOR_CURR_SERVO3               "CSv3"
#define TR_SENSOR_CURR_SERVO4               "CSv4"
#define TR_SENSOR_DIST                      "Dist"
#define TR_SENSOR_ARM                       "Arm"
#define TR_SENSOR_C50                       "C50"
#define TR_SENSOR_C200                      "C200"
#define TR_SENSOR_RPM                       "RPM"
#define TR_SENSOR_FUEL                      "Fuel"
#define TR_SENSOR_VSPD                      "VSpd"
#define TR_SENSOR_ACCX                      "AccX"
#define TR_SENSOR_ACCY                      "AccY"
#define TR_SENSOR_ACCZ                      "AccZ"
#define TR_SENSOR_GYROX                     "GYRX"
#define TR_SENSOR_GYROY                     "GYRY"
#define TR_SENSOR_GYROZ                     "GYRZ"
#define TR_SENSOR_CURR                      "Curr"
#define TR_SENSOR_CAPACITY                  "Capa"
#define TR_SENSOR_VFAS                      "VFAS"
#define TR_SENSOR_ASPD                      "ASpd"
#define TR_SENSOR_GSPD                      "GSpd"
#define TR_SENSOR_HDG                       "Hdg"
#define TR_SENSOR_SATELLITES                "Sats"
#define TR_SENSOR_CELLS                     "Cels"
#define TR_SENSOR_GPSALT                    "GAlt"
#define TR_SENSOR_GPSDATETIME               "Date"
#define TR_SENSOR_GPS                       "GPS"
#define TR_SENSOR_BATT1_VOLTAGE             "RB1V"
#define TR_SENSOR_BATT2_VOLTAGE             "RB2V"
#define TR_SENSOR_BATT1_CURRENT             "RB1A"
#define TR_SENSOR_BATT2_CURRENT             "RB2A"
#define TR_SENSOR_BATT1_CONSUMPTION         "RB1C"
#define TR_SENSOR_BATT2_CONSUMPTION         "RB2C"
#define TR_SENSOR_BATT1_TEMP                "RB1T"
#define TR_SENSOR_BATT2_TEMP                "RB2T"
#define TR_SENSOR_RB_STATE                  "RBS"
#define TR_SENSOR_CHANS_STATE               "RBCS"
#define TR_SENSOR_RX_RSSI1                  "1RSS"
#define TR_SENSOR_RX_RSSI2                  "2RSS"
#define TR_SENSOR_RX_QUALITY                "RQly"
#define TR_SENSOR_RX_SNR                    "RSNR"
#define TR_SENSOR_RX_SIGNAL                 "Sgnl"
#define TR_SENSOR_RX_NOISE                  "RNse"
#define TR_SENSOR_ANTENNA                   "ANT"
#define TR_SENSOR_RF_MODE                   "RFMD"
#define TR_SENSOR_TX_POWER                  "TPWR"
#define TR_SENSOR_TX_RSSI                   "TRSS"
#define TR_SENSOR_TX_QUALITY                "TQly"
#define TR_SENSOR_RX_RSSI_PERC              "RRSP"
#define TR_SENSOR_RX_RF_POWER               "RPWR"
#define TR_SENSOR_TX_RSSI_PERC              "TRSP"
#define TR_SENSOR_TX_RF_POWER               "TPWR"
#define TR_SENSOR_TX_FPS                    "TFPS"
#define TR_SENSOR_TX_SNR                    "TSNR"
#define TR_SENSOR_TX_NOISE                  "TNse"
#define TR_SENSOR_PITCH                     "Ptch"
#define TR_SENSOR_ROLL                      "Roll"
#define TR_SENSOR_YAW                       "Yaw"
#define TR_SENSOR_FLIGHT_MODE               "FM"
#define TR_SENSOR_THROTTLE                  "Thr"
#define TR_SENSOR_QOS_A                     "FdeA"
#define TR_SENSOR_QOS_B                     "FdeB"
#define TR_SENSOR_QOS_L                     "FdeL"
#define TR_SENSOR_QOS_R                     "FdeR"
#define TR_SENSOR_QOS_F                     "FLss"
#define TR_SENSOR_QOS_H                     "Hold"
#define TR_SENSOR_BIND                      "BIND"
#define TR_SENSOR_LAP_NUMBER                "Lap "
#define TR_SENSOR_GATE_NUMBER               "Gate"
#define TR_SENSOR_LAP_TIME                  "LapT"
#define TR_SENSOR_GATE_TIME                 "GteT"
#define TR_SENSOR_ESC_VOLTAGE               "EscV"
#define TR_SENSOR_ESC_CURRENT               "EscA"
#define TR_SENSOR_ESC_RPM                   "EscR"
#define TR_SENSOR_ESC_CONSUMPTION           "EscC"
#define TR_SENSOR_ESC_TEMP                  "EscT"
#define TR_SENSOR_SD1_CHANNEL               "Chan"
#define TR_SENSOR_GASSUIT_TEMP1             "GTp1"
#define TR_SENSOR_GASSUIT_TEMP2             "GTp2"
#define TR_SENSOR_GASSUIT_RPM               "GRPM"
#define TR_SENSOR_GASSUIT_FLOW              "GFlo"
#define TR_SENSOR_GASSUIT_CONS              "GFue"
#define TR_SENSOR_GASSUIT_RES_VOL           "GRVl"
#define TR_SENSOR_GASSUIT_RES_PERC          "GRPc"
#define TR_SENSOR_GASSUIT_MAX_FLOW          "GMFl"
#define TR_SENSOR_GASSUIT_AVG_FLOW          "GAFl"
#define TR_SENSOR_SBEC_VOLTAGE              "BecV"
#define TR_SENSOR_SBEC_CURRENT              "BecA"

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

#define TR_FSGROUPS                     "-","1","2","3"

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
#define STR_HOTT_ID_TX_RSSI_DL     "Tssi"
#define STR_HOTT_ID_TX_LQI_DL      "Tqly"
// RX
#define STR_HOTT_ID_RX_RSSI_UL     "Rssi"
#define STR_HOTT_ID_RX_LQI_UL      "Rqly"
#define STR_HOTT_ID_RX_VLT         "Rbt"
#define STR_HOTT_ID_RX_TMP         "Rtmp" 
#define STR_HOTT_ID_RX_BAT_MIN     "Rbtm"
#define STR_HOTT_ID_RX_VPCK        "Rvpk"
#define STR_HOTT_ID_RX_EVENT       "Revt"
// Vario
#define STR_HOTT_ID_VARIO_ALT      "Valt"
#define STR_HOTT_ID_VARIO_VV       "Vvv"
#define STR_HOTT_ID_VARIO_HDG      "Vhdg"
// GPS
#define STR_HOTT_ID_GPS_HDG        "GPhd"
#define STR_HOTT_ID_GPS_SPEED      "GPsp"
#define STR_HOTT_ID_GPS_LL         "GPS"
#define STR_HOTT_ID_GPS_DST        "GPdi"
#define STR_HOTT_ID_GPS_ALT        "GPal" 
#define STR_HOTT_ID_GPS_VV         "GPvv"
#define STR_HOTT_ID_GPS_NSATS      "GPns"
// ESC
#define STR_HOTT_ID_ESC_VLT        "ESu1"
#define STR_HOTT_ID_ESC_CAP        "EScp"
#define STR_HOTT_ID_ESC_TMP        "ESt1" 
#define STR_HOTT_ID_ESC_CUR        "ESi1"
#define STR_HOTT_ID_ESC_RPM        "ESrp"
#define STR_HOTT_ID_ESC_BEC_VLT    "ESu2"
#define STR_HOTT_ID_ESC_BEC_CUR    "ESi2" 
#define STR_HOTT_ID_ESC_BEC_TMP    "ESt2"
#define STR_HOTT_ID_ESC_MOT_TMP    "ESt3"
// GAM
#define STR_HOTT_ID_GAM_CELS       "GAcl"
#define STR_HOTT_ID_GAM_VLT1       "GAu1"
#define STR_HOTT_ID_GAM_VLT2       "GAu2"
#define STR_HOTT_ID_GAM_TMP1       "GAt1"
#define STR_HOTT_ID_GAM_TMP2       "GAt2"
#define STR_HOTT_ID_GAM_FUEL       "GAfl"
#define STR_HOTT_ID_GAM_RPM1       "GAr1"
#define STR_HOTT_ID_GAM_ALT        "GAal"
#define STR_HOTT_ID_GAM_VV         "GAvv"
#define STR_HOTT_ID_GAM_CUR        "GAi"
#define STR_HOTT_ID_GAM_VLT3       "GAu3"
#define STR_HOTT_ID_GAM_CAP        "GAcp"
#define STR_HOTT_ID_GAM_SPEED      "GAsp"
#define STR_HOTT_ID_GAM_RPM2       "GAr2"
// EAM
#define STR_HOTT_ID_EAM_CELS_L     "EAc1"
#define STR_HOTT_ID_EAM_CELS_H     "EAc2"
#define STR_HOTT_ID_EAM_VLT1       "EAu1"
#define STR_HOTT_ID_EAM_VLT2       "EAu2"
#define STR_HOTT_ID_EAM_TMP1       "EAt1"
#define STR_HOTT_ID_EAM_TMP2       "EAt2"
#define STR_HOTT_ID_EAM_ALT        "EAal"
#define STR_HOTT_ID_EAM_CUR        "EAi"
#define STR_HOTT_ID_EAM_VLT3       "EAu3"
#define STR_HOTT_ID_EAM_CAP        "EAcp"
#define STR_HOTT_ID_EAM_VV         "EAvv"
#define STR_HOTT_ID_EAM_RPM        "EArp"
#define STR_HOTT_ID_EAM_SPEED      "EAsp"
