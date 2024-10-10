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

//
//  Data based on MPM firmware version 1.3.3.33 multi.txt
//

#if defined(MULTIMODULE) or defined(SIMU)

#pragma once

//
// Common list of supported Multi protocols
//
enum ModuleSubtypeMulti {
  MODULE_SUBTYPE_MULTI_FIRST = 0,
  MODULE_SUBTYPE_MULTI_FLYSKY = MODULE_SUBTYPE_MULTI_FIRST,
  MODULE_SUBTYPE_MULTI_HUBSAN, //1
  MODULE_SUBTYPE_MULTI_FRSKY,  //2
  MODULE_SUBTYPE_MULTI_HISKY,  //3
  MODULE_SUBTYPE_MULTI_V2X2,   //4
  MODULE_SUBTYPE_MULTI_DSM2,   //5
  MODULE_SUBTYPE_MULTI_DEVO,
  MODULE_SUBTYPE_MULTI_YD717,
  MODULE_SUBTYPE_MULTI_KN,
  MODULE_SUBTYPE_MULTI_SYMAX,
  MODULE_SUBTYPE_MULTI_SLT,    //10
  MODULE_SUBTYPE_MULTI_CX10,
  MODULE_SUBTYPE_MULTI_CG023,
  MODULE_SUBTYPE_MULTI_BAYANG,
  MODULE_SUBTYPE_MULTI_FRSKYX,
  MODULE_SUBTYPE_MULTI_ESky,   //15
  MODULE_SUBTYPE_MULTI_MT99XX,
  MODULE_SUBTYPE_MULTI_MJXQ,
  MODULE_SUBTYPE_MULTI_SHENQI,
  MODULE_SUBTYPE_MULTI_FY326,
  MODULE_SUBTYPE_MULTI_FUTABA, //20
  MODULE_SUBTYPE_MULTI_J6PRO,
  MODULE_SUBTYPE_MULTI_FQ777,
  MODULE_SUBTYPE_MULTI_ASSAN,
  MODULE_SUBTYPE_MULTI_FRSKYV,
  MODULE_SUBTYPE_MULTI_HONTAI, //25
  MODULE_SUBTYPE_MULTI_OLRS,
  MODULE_SUBTYPE_MULTI_FS_AFHDS2A,
  MODULE_SUBTYPE_MULTI_Q2X2,
  MODULE_SUBTYPE_MULTI_WK_2X01,
  MODULE_SUBTYPE_MULTI_Q303,   //30
  MODULE_SUBTYPE_MULTI_GW008,
  MODULE_SUBTYPE_MULTI_DM002,
  MODULE_SUBTYPE_MULTI_CABELL,
  MODULE_SUBTYPE_MULTI_ESKY150,
  MODULE_SUBTYPE_MULTI_H83D,   //35
  MODULE_SUBTYPE_MULTI_CORONA,
  MODULE_SUBTYPE_MULTI_CFLIE,
  MODULE_SUBTYPE_MULTI_HITEC,
  MODULE_SUBTYPE_MULTI_WFLY,
  MODULE_SUBTYPE_MULTI_BUGS,   //40
  MODULE_SUBTYPE_MULTI_BUGS_MINI,
  MODULE_SUBTYPE_MULTI_TRAXXAS,
  MODULE_SUBTYPE_MULTI_NCC1701,
  MODULE_SUBTYPE_MULTI_E01X,
  MODULE_SUBTYPE_MULTI_V911S,  //45
  MODULE_SUBTYPE_MULTI_GD00X,
  MODULE_SUBTYPE_MULTI_V761,
  MODULE_SUBTYPE_MULTI_KF606,
  MODULE_SUBTYPE_MULTI_REDPINE,
  MODULE_SUBTYPE_MULTI_POTENSIC, //50
  MODULE_SUBTYPE_MULTI_ZSX,
  MODULE_SUBTYPE_MULTI_HEIGHT,
  MODULE_SUBTYPE_MULTI_SCANNER,
  MODULE_SUBTYPE_MULTI_FRSKYX_RX,
  MODULE_SUBTYPE_MULTI_AFHDS2A_RX, //55
  MODULE_SUBTYPE_MULTI_HOTT,
  MODULE_SUBTYPE_MULTI_FX,
  MODULE_SUBTYPE_MULTI_BAYANG_RX,
  MODULE_SUBTYPE_MULTI_PELIKAN,
  MODULE_SUBTYPE_MULTI_TIGER,  //60
  MODULE_SUBTYPE_MULTI_XK,
  MODULE_SUBTYPE_MULTI_XN297DUMP,
  MODULE_SUBTYPE_MULTI_FRSKYX2,
  MODULE_SUBTYPE_MULTI_FRSKY_R9,
  MODULE_SUBTYPE_MULTI_PROPEL, //65
  MODULE_SUBTYPE_MULTI_FRSKYL,
  MODULE_SUBTYPE_MULTI_SKYARTEC,
  MODULE_SUBTYPE_MULTI_ESKY150V2,
  MODULE_SUBTYPE_MULTI_DSM_RX,
  MODULE_SUBTYPE_MULTI_JJRC345, //70
  MODULE_SUBTYPE_MULTI_Q90C,
  MODULE_SUBTYPE_MULTI_KYOSHO,
  MODULE_SUBTYPE_MULTI_RLINK,
  MODULE_SUBTYPE_MULTI_ELRS,
  MODULE_SUBTYPE_MULTI_REALACC, //75
  MODULE_SUBTYPE_MULTI_OMP,
  MODULE_SUBTYPE_MULTI_MLINK,
  MODULE_SUBTYPE_MULTI_WFLY2,
  MODULE_SUBTYPE_MULTI_E016HV2,
  MODULE_SUBTYPE_MULTI_E010R5,  //80
  MODULE_SUBTYPE_MULTI_LOLI,
  MODULE_SUBTYPE_MULTI_E129,
  MODULE_SUBTYPE_MULTI_JOYSWAY,
  MODULE_SUBTYPE_MULTI_E016H,
  MODULE_SUBTYPE_MULTI_CONFIG,  //85
  MODULE_SUBTYPE_MULTI_IKEAANSLUTA,
  MODULE_SUBTYPE_MULTI_WILLIFM,
  MODULE_SUBTYPE_MULTI_LOSI,
  MODULE_SUBTYPE_MULTI_MOULDKG,
  MODULE_SUBTYPE_MULTI_XERALL,  //90
  MODULE_SUBTYPE_MULTI_MT99XX2,
  MODULE_SUBTYPE_MULTI_KYOSHO2,
  MODULE_SUBTYPE_MULTI_SCORPIO,
  //
  // spare entries - don't touch,
  // just add to known protocols
  //
  MODULE_SUBTYPE_MULTI_NN1,
  MODULE_SUBTYPE_MULTI_NN2,
  MODULE_SUBTYPE_MULTI_NN3,
  MODULE_SUBTYPE_MULTI_NN4,
  MODULE_SUBTYPE_MULTI_NN5,
  MODULE_SUBTYPE_MULTI_NN6,
  MODULE_SUBTYPE_MULTI_NN7,
  MODULE_SUBTYPE_MULTI_NN8,
  MODULE_SUBTYPE_MULTI_NN9,
  MODULE_SUBTYPE_MULTI_NN10,
  MODULE_SUBTYPE_MULTI_LAST = MODULE_SUBTYPE_MULTI_NN10,
  //
  // Sentinel
  //
  MODULE_SUBTYPE_MULTI_SENTINEL = 0xfe,
  //
  // most likely no longer required
  //
  MM_RF_CUSTOM_SELECTED = 0xff
};

//
// Common list of Multi protocol names. Needs to match enum ModuleSubtypeMulti
//
#define KNOWN_PROTO_NAMES \
  "FlySky","Hubsan","FrSky D","Hisky","V2x2","DSM","Devo","YD717","KN","SymaX",\
  "SLT","CX10","CG023","Bayang","FrSky X","ESky","MT99XX","MJXq","Shenqi","FY326",\
  "Futaba","J6 Pro","FQ777","Assan","FrSky V","Hontai","OpenLrs","FlSky2A","Q2x2","Walkera", \
  "Q303","GW008","DM002","Cabell","Esky150","H8 3D","Corona","CFlie","Hitec","WFLY",\
  "Bugs","BugMini","Traxxas","NCC1701","E01X","V911S","GD00X","V761","KF606","Redpine",\
  "Potensi","ZSX","Height","Scanner","FrSkyRX","FS2A_RX","HoTT","FX","BayanRX","Pelikan",\
  "Tiger", "XK","XN297DU","FrSkyX2","FrSkyR9","Propel","FrSky L","Skyartc","ESkyV2","DSM_RX",\
  "JJRC345","Q90C","Kyosho","RadLink","ExpLRS","Realacc","OMP","M-Link","WFLY2","E016Hv2",\
  "E010r5","LOLI","E129","JOYSWAY","E016H","Config","IKEA","WILLIFM","Losi","MouldKg",\
  "Xerall","MT99XX2", "Kyosho2", "Scorpio"
#define SPARE_PROTO_NAMES \
  "NN 1","NN 2","NN 3","NN 4","NN 5","NN 6","NN 7","NN 8","NN 9","NN 10"
#define SPARE_SUBTYPE_NAMES \
  "SUB 1","SUB 2","SUB 3","SUB 4","SUB 5","SUB 6","SUB 7","SUB 8"
#define PROTO_NAMES   KNOWN_PROTO_NAMES,SPARE_PROTO_NAMES

//
// Common list of option names. Needs to match inline void getMultiOptionValues cases.
//
#define BAYANG_OPTION_TELEMETRY_NAMES     "Off","On","Off+Aux","On+Aux"
#define DSM2_OPTION_SERVOFREQ_NAMES       "22ms","11ms"
#define DSM_CLONE_NAMES                   "Normal","Cloned"

//
// Common list of protocols the MPM doesn't allow to be selected (not sent by MPM protocol scan)
//
inline bool isMultiProtocolSelectable(int protocol)
{
  return (protocol != MODULE_SUBTYPE_MULTI_CONFIG &&
          protocol != MODULE_SUBTYPE_MULTI_SCANNER
         );
}

//
// Common protocol specific option values
//
inline void getMultiOptionValues(int8_t multi_proto, int8_t &min, int8_t &max) {
  switch (multi_proto) {
    case MODULE_SUBTYPE_MULTI_DSM2:
      min = 0;    // STR_MULTI_SERVOFREQ: 22ms, 11ms
      max = 1;
      break;
    case MODULE_SUBTYPE_MULTI_BAYANG:
      min = 0;    // STR_MULTI_TELEMETRY: Off, On, Off+AUX, On+Aux
      max = 3;
      break;
    case MODULE_SUBTYPE_MULTI_OLRS:
      min = -1;
      max = 7;
      break;
    case MODULE_SUBTYPE_MULTI_FS_AFHDS2A:
      min = 0;    // STR_MULTI_SERVOFREQ: (50+optionvalue*5)ms
      max = 70;
      break;
    case MODULE_SUBTYPE_MULTI_XN297DUMP:
      min = -1;
      max = 84;
      break;
    default:
      min = -128; // all other option types, e.g STR_MULTI_RFTUNE
      max = 127;
      break;
  }
}

//
// Out of the ordinary max. numbers of model IDs
//
#define MODULE_SUBTYPE_MULTI_OLRS_RXNUM       4
#define MODULE_SUBTYPE_MULTI_BUGS_RXNUM       15
#define MODULE_SUBTYPE_MULTI_BUGS_MINI_RXNUM  15

//
// subtypes for Radiomaster T8 bind key and Radiomaster RTF use.
// see defines RTF, BIND_KEY and the protocol subtype list STRLIST()
// in MultiSubTypeDefs.h
//
#define MULTI_FRSKYD_SUBTYPE_D8         0
#define MULTI_FRSKYX_SUBTYPE_D16_FCC    0
#define MULTI_FRSKYX_SUBTYPE_D16_LBT    2
#define MULTI_FRSKYX2_SUBTYPE_D16_FCC   0
#define MULTI_FRSKYX2_SUBTYPE_D16_LBT   2

#endif // MULTIMODULE
