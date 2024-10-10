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
// Single source of Multi protocol defintions:
// - MultiProtoDefs.h contains the protocol list and names as per Multi.txt
//   Widely used in Companion/Simulator and radio firmware code
// - MutliSubtypesDef.h contains subtype definitions and generates data structurse
//   for Companion and Simulator/radio code in radio/src/gui/gui_common.cpp and
//   companion/src/firmwares/multiprotocols.cpp
//

//
// include list of Multi protocosl
//
#include "MultiProtoDefs.h"

//
// This distinction circumvents the different ways of handling translations of menu texts
// and sets the defines STRLIST and PROTODEF such that the correct Companion/Simulator and radio code
// data structures are generated
//
#if defined(CPN)
  #define STRLIST(x, ...) static const QStringList x __VA_ARGS__;
  #define PROTODEF const Multiprotocols multiProtocols

  #define STR_MULTI_OPTION                     QT_TRANSLATE_NOOP("Multiprotocols", "Option value")
  #define STR_MULTI_RFTUNE                     QT_TRANSLATE_NOOP("Multiprotocols", "CC2500 frequency fine tune")
  #define STR_MULTI_VIDFREQ                    QT_TRANSLATE_NOOP("Multiprotocols", "Video TX frequency")
  #define STR_MULTI_FIXEDID                    QT_TRANSLATE_NOOP("Multiprotocols", "Fixed ID value")
  #define STR_MULTI_TELEMETRY                  QT_TRANSLATE_NOOP("Multiprotocols", "Telemetry")
  #define STR_MULTI_SERVOFREQ                  QT_TRANSLATE_NOOP("Multiprotocols", "Servo update rate")
  #define STR_MULTI_MAX_THROW                  QT_TRANSLATE_NOOP("Multiprotocols", "Enable max throw")
  #define STR_MULTI_RFCHAN                     QT_TRANSLATE_NOOP("Multiprotocols", "Select RF channel")
  #define STR_MULTI_RFPOWER                    QT_TRANSLATE_NOOP("Multiprotocols", "RF power")

  #define STR_MULTI_DEFAULT                    QT_TRANSLATE_NOOP("Multiprotocols", "DEFAULT")
  STRLIST(NO_SUBTYPE, {STR_MULTI_DEFAULT})
#else
  #define STRLIST(x, ...) const char* const x[] = __VA_ARGS__;
  #define PROTODEF const mm_protocol_definition multi_protocols[]

  const char* const mm_options_strings::options[] = {
    nullptr,
    STR_MULTI_OPTION,
    STR_MULTI_RFTUNE,
    STR_MULTI_VIDFREQ,
    STR_MULTI_FIXEDID,
    STR_MULTI_TELEMETRY,
    STR_MULTI_SERVOFREQ,
    STR_MULTI_MAX_THROW,
    STR_MULTI_RFCHAN,
    STR_MULTI_RFPOWER,
    STR_MULTI_WBUS               // probably not used by MPM anymore
  };

  #define NO_SUBTYPE nullptr
#endif

//
// Common definitions of Multi protocol subtype options (only for protocols that do have subtypes)
//
STRLIST(STR_SUBTYPE_FLYSKY,    {"Std","V9x9","V6x6","V912","CX20"})
STRLIST(STR_SUBTYPE_HUBSAN,    {"H107","H301","H501"})
STRLIST(STR_SUBTYPE_FRSKYD,    {"D8","Cloned"})
STRLIST(STR_SUBTYPE_FRSKYX,    {"D16","D16 8ch","LBT(EU)","LBT 8ch","Cloned", "Cloned 8ch"})
STRLIST(STR_SUBTYPE_HISKY,     {"Std","HK310"})
STRLIST(STR_SUBTYPE_V2X2,      {"Std","JXD506","MR101"})
STRLIST(STR_SUBTYPE_DSM,       {"2 1F","2 2F","X 1F","X 2F","Auto","R 1F"})
STRLIST(STR_SUBTYPE_DEVO,      {"8ch","10ch","12ch","6ch","7ch"})
STRLIST(STR_SUBTYPE_YD717,     {"Std","SkyWlkr","Syma X4","XINXUN","NIHUI"})
STRLIST(STR_SUBTYPE_KN,        {"WLtoys","FeiLun"})
STRLIST(STR_SUBTYPE_SYMAX,     {"Std","X5C"})
STRLIST(STR_SUBTYPE_SLT,       {"V1_6ch","V2_8ch","Q100","Q200","MR100"})
STRLIST(STR_SUBTYPE_CX10,      {"Green","Blue","DM007","-","JC3015a","JC3015b","MK33041"})
STRLIST(STR_SUBTYPE_CG023,     {"Std","YD829"})
STRLIST(STR_SUBTYPE_BAYANG,    {"Std","H8S3D","X16 AH","IRDrone","DHD D4","QX100"})
STRLIST(STR_SUBTYPE_ESky,      {"Std","ET4"})
STRLIST(STR_SUBTYPE_MT99,      {"MT99","H7","YZ","LS","FY805","A180","Dragon","F949G"})
STRLIST(STR_SUBTYPE_MJXQ,      {"WLH08","X600","X800","H26D","E010","H26WH","Phoenix"})
STRLIST(STR_SUBTYPE_FY326,     {"Std","FY319"})
STRLIST(STR_SUBTYPE_FUTABA,    {"SFHSS"})
STRLIST(STR_SUBTYPE_HONTAI,    {"Std","JJRC X1","X5C1","FQ_951"})
STRLIST(STR_SUBTYPE_AFHDS2A,   {"PWM,IBUS","PPM,IBUS","PWM,SBUS","PPM,SBUS","PWM,IB16","PPM,IB16","PWM,SB16","PPM,SB16"})
STRLIST(STR_SUBTYPE_Q2X2,      {"Q222","Q242","Q282"})
STRLIST(STR_SUBTYPE_WK2x01,    {"WK2801","WK2401","W6_5_1","W6_6_1","W6_HeL","W6_HeI"})
STRLIST(STR_SUBTYPE_Q303,      {"Std","CX35","CX10D","CX10WD"})
STRLIST(STR_SUBTYPE_CABELL,    {"V3","V3 Telm","-","-","-","-","F-Safe","Unbind"})
STRLIST(STR_SUBTYPE_ESKY150,   {"4ch","7ch"})
STRLIST(STR_SUBTYPE_H83D,      {"Std","H20H","H20Mini","H30Mini"})
STRLIST(STR_SUBTYPE_CORONA,    {"V1","V2","FD V3"})
STRLIST(STR_SUBTYPE_HITEC,     {"Optima","Opt Hub","Minima"})
STRLIST(STR_SUBTYPE_WFLY,      {"WFR0x"})
STRLIST(STR_SUBTYPE_BUGS_MINI, {"Std","Bugs3H"})
STRLIST(STR_SUBTYPE_TRAXXAS,   {"6519"})
STRLIST(STR_SUBTYPE_E01X,      {"E012","E015"})
STRLIST(STR_SUBTYPE_V911S,     {"Std","E119"})
STRLIST(STR_SUBTYPE_GD00X,     {"GD_V1","GD_V2"})
STRLIST(STR_SUBTYPE_V761,      {"3ch","4ch","TOPRC"})
STRLIST(STR_SUBTYPE_KF606,     {"KF606","MIG320","ZCZ50"})
STRLIST(STR_SUBTYPE_REDPINE,   {"Fast","Slow"})
STRLIST(STR_SUBTYPE_POTENSIC,  {"A20"})
STRLIST(STR_SUBTYPE_ZSX,       {"280JJRC"})
STRLIST(STR_SUBTYPE_HEIGHT,    {"5ch","8ch"})
STRLIST(STR_SUBTYPE_FRSKYX_RX, {"Multi","CloneTX","EraseTX","CPPM"})
STRLIST(STR_SUBTYPE_HOTT,      {"Sync","No_Sync"})
STRLIST(STR_SUBTYPE_FX,        {"816","620","9630"})
STRLIST(STR_SUBTYPE_PELIKAN,   {"Pro","Lite","SCX24"})
STRLIST(STR_SUBTYPE_XK,        {"X450","X420"})
STRLIST(STR_SUBTYPE_XN297DUMP, {"250K","1M","2M","AUTO","NRF","CC2500"})
STRLIST(STR_SUBTYPE_FRSKYR9,   {"915MHz","868MHz","915 8ch","868 8ch","FCC","--","FCC 8ch","-- 8ch"})
STRLIST(STR_SUBTYPE_PROPEL,    {"74-Z"})
STRLIST(STR_SUBTYPE_FRSKYL,    {"LR12","LR12 6ch"})
STRLIST(STR_SUBTYPE_ESKY150V2, {"150 V2"})
STRLIST(STR_SUBTYPE_JJRC345,   {"Std","SkyTmbr"})
STRLIST(STR_SUBTYPE_KYOSHO,    {"FHSS","Hype"})
STRLIST(STR_SUBTYPE_KYOSHO2,   {"KT-17"})
STRLIST(STR_SUBTYPE_RLINK,     {"Surface","Air","DumboRC"})
STRLIST(STR_SUBTYPE_ELRS,      {"N/A WIP"})
STRLIST(STR_SUBTYPE_REALACC,   {"R11"})
STRLIST(STR_SUBTYPE_WFLY2,     {"RF20x"})
STRLIST(STR_SUBTYPE_MOULDKG,   {"Analog","Digital"})
STRLIST(STR_SUBTYPE_MT992,     {"PA18"})
STRLIST(STR_SUBTYPE_RX,        {"Multi","CPPM"})
STRLIST(STR_SUBTYPE_DSM_RX,    {"Multi","CloneTX","EraseTX","CPPM"})
STRLIST(STR_SUBTYPE_E129,      {"STD","C186"})

//
// Generic subtypes for future use protocols - don't touch
//
STRLIST(STR_SUBTYPE_NN,        { SPARE_SUBTYPE_NAMES })

//
// Common data structure defining Multi protocol capabilites, i.e. number of subtypes,
// failsafe support yes/no, channel map disabled yes/no, reference to the suptype options strings above
// and further protocol options, e.g. RF tune, video frequency.
//
PROTODEF {
  {MODULE_SUBTYPE_MULTI_FLYSKY,     4, false, true,   STR_SUBTYPE_FLYSKY,    nullptr},
  {MODULE_SUBTYPE_MULTI_HUBSAN,     2, false, false,  STR_SUBTYPE_HUBSAN,    STR_MULTI_VIDFREQ},
  {MODULE_SUBTYPE_MULTI_FRSKY,      1, false, false,  STR_SUBTYPE_FRSKYD,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HISKY,      1, true,  true,   STR_SUBTYPE_HISKY,     nullptr},
  {MODULE_SUBTYPE_MULTI_V2X2,       2, false, false,  STR_SUBTYPE_V2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_DSM2,       5, false, true,   STR_SUBTYPE_DSM,       STR_MULTI_MAX_THROW},
  {MODULE_SUBTYPE_MULTI_DEVO,       4, true,  true,   STR_SUBTYPE_DEVO,      STR_MULTI_FIXEDID},
  {MODULE_SUBTYPE_MULTI_YD717,      4, false, false,  STR_SUBTYPE_YD717,     nullptr},
  {MODULE_SUBTYPE_MULTI_KN,         1, false, false,  STR_SUBTYPE_KN,        nullptr},
  {MODULE_SUBTYPE_MULTI_SYMAX,      1, false, false,  STR_SUBTYPE_SYMAX,     nullptr},
  {MODULE_SUBTYPE_MULTI_SLT,        4, false, true,   STR_SUBTYPE_SLT,       STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_CX10,       6, false, false,  STR_SUBTYPE_CX10,      nullptr},
  {MODULE_SUBTYPE_MULTI_CG023,      1, false, false,  STR_SUBTYPE_CG023,     nullptr},
  {MODULE_SUBTYPE_MULTI_BAYANG,     5, false, false,  STR_SUBTYPE_BAYANG,    STR_MULTI_TELEMETRY},
  {MODULE_SUBTYPE_MULTI_FRSKYX,     5, true,  false,  STR_SUBTYPE_FRSKYX,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ESky,       1, false, true,   STR_SUBTYPE_ESky,      nullptr},
  {MODULE_SUBTYPE_MULTI_MT99XX,     7, false, false,  STR_SUBTYPE_MT99,      nullptr},
  {MODULE_SUBTYPE_MULTI_MJXQ,       6, false, false,  STR_SUBTYPE_MJXQ,      nullptr},
  {MODULE_SUBTYPE_MULTI_SHENQI,     0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_FY326,      1, false, false,  STR_SUBTYPE_FY326,     nullptr},
  {MODULE_SUBTYPE_MULTI_FUTABA,     0, true,  true,   STR_SUBTYPE_FUTABA,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_J6PRO,      0, false, true,   NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_FQ777,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_ASSAN,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_FRSKYV,     0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HONTAI,     3, false, false,  STR_SUBTYPE_HONTAI,    nullptr},
  // MODULE_SUBTYPE_MULTI_OLRS non selectable and masked out for selection
  {MODULE_SUBTYPE_MULTI_FS_AFHDS2A, 7, true,  true,   STR_SUBTYPE_AFHDS2A,   STR_MULTI_SERVOFREQ},
  {MODULE_SUBTYPE_MULTI_Q2X2,       2, false, false,  STR_SUBTYPE_Q2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_WK_2X01,    5, true,  true,   STR_SUBTYPE_WK2x01,    nullptr},
  {MODULE_SUBTYPE_MULTI_Q303,       3, false, false,  STR_SUBTYPE_Q303,      nullptr},
  {MODULE_SUBTYPE_MULTI_GW008,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_DM002,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_CABELL,     7, false, false,  STR_SUBTYPE_CABELL,    STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_ESKY150,    1, false, false,  STR_SUBTYPE_ESKY150,   nullptr},
  {MODULE_SUBTYPE_MULTI_H83D,       3, false, false,  STR_SUBTYPE_H83D,      nullptr},
  {MODULE_SUBTYPE_MULTI_CORONA,     2, false, false,  STR_SUBTYPE_CORONA,    STR_MULTI_RFTUNE},
  // MODULE_SUBTYPE_MULTI_CFLIE non selectable and masked out for selection
  {MODULE_SUBTYPE_MULTI_HITEC,      2, false, false,  STR_SUBTYPE_HITEC,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_WFLY,       0, true,  false,  STR_SUBTYPE_WFLY,      nullptr},
  {MODULE_SUBTYPE_MULTI_BUGS,       0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_BUGS_MINI,  1, false, false,  STR_SUBTYPE_BUGS_MINI, nullptr},
  {MODULE_SUBTYPE_MULTI_TRAXXAS,    0, false, false,  STR_SUBTYPE_TRAXXAS,   nullptr},
  {MODULE_SUBTYPE_MULTI_NCC1701,    0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_E01X,       1, false, false,  STR_SUBTYPE_E01X,      nullptr},
  {MODULE_SUBTYPE_MULTI_V911S,      1, false, false,  STR_SUBTYPE_V911S,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_GD00X,      1, false, false,  STR_SUBTYPE_GD00X,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_V761,       2, false, false,  STR_SUBTYPE_V761,      nullptr},
  {MODULE_SUBTYPE_MULTI_KF606,      2, false, false,  STR_SUBTYPE_KF606,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_REDPINE,    1, false, false,  STR_SUBTYPE_REDPINE,   STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_POTENSIC,   0, false, false,  STR_SUBTYPE_POTENSIC,  nullptr},
  {MODULE_SUBTYPE_MULTI_ZSX,        0, false, false,  STR_SUBTYPE_ZSX,       nullptr},
  {MODULE_SUBTYPE_MULTI_HEIGHT,     1, false, false,  STR_SUBTYPE_HEIGHT,    nullptr},
  // MODULE_SUBTYPE_MULTI_SCANNER non selectable and masked out for selection
  {MODULE_SUBTYPE_MULTI_FRSKYX_RX,  3, false, false,  STR_SUBTYPE_FRSKYX_RX, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_AFHDS2A_RX, 1, false, false,  STR_SUBTYPE_RX,        nullptr}, //new
  {MODULE_SUBTYPE_MULTI_HOTT,       1, true,  false,  STR_SUBTYPE_HOTT,      STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FX,         2, false, false,  STR_SUBTYPE_FX,        nullptr},
  {MODULE_SUBTYPE_MULTI_BAYANG_RX,  1, false, false,  STR_SUBTYPE_RX,        nullptr}, //new
  {MODULE_SUBTYPE_MULTI_PELIKAN,    2, false, true,   STR_SUBTYPE_PELIKAN,   nullptr},
  {MODULE_SUBTYPE_MULTI_TIGER,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_XK,         1, false, false,  STR_SUBTYPE_XK,        STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_XN297DUMP,  5, false, false,  STR_SUBTYPE_XN297DUMP, STR_MULTI_RFCHAN},
  {MODULE_SUBTYPE_MULTI_FRSKYX2,    5, true,  false,  STR_SUBTYPE_FRSKYX,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FRSKY_R9,   7, true,  false,  STR_SUBTYPE_FRSKYR9,   nullptr},
  {MODULE_SUBTYPE_MULTI_PROPEL,     0, false, false,  STR_SUBTYPE_PROPEL,    nullptr},
  {MODULE_SUBTYPE_MULTI_FRSKYL,     1, false, false,  STR_SUBTYPE_FRSKYL,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_SKYARTEC,   0, false, true,   NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ESKY150V2,  0, false, true,   STR_SUBTYPE_ESKY150V2, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_DSM_RX,     3, false, true,   STR_SUBTYPE_DSM_RX,    nullptr}, //new
  {MODULE_SUBTYPE_MULTI_JJRC345,    1, false, false,  STR_SUBTYPE_JJRC345,   nullptr},
  {MODULE_SUBTYPE_MULTI_Q90C,       0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_KYOSHO,     1, false, true,   STR_SUBTYPE_KYOSHO,    nullptr},
  {MODULE_SUBTYPE_MULTI_RLINK,      2, false, false,  STR_SUBTYPE_RLINK,     STR_MULTI_RFTUNE},
  // MODULE_SUBTYPE_MULTI_ELRS non selectable and masked out for selection
  {MODULE_SUBTYPE_MULTI_REALACC,    0, false, false,  STR_SUBTYPE_REALACC,   nullptr},
  {MODULE_SUBTYPE_MULTI_OMP,        0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_MLINK,      0, true,  false,  NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_WFLY2,      0, true,  false,  STR_SUBTYPE_WFLY2,     STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_E016HV2,    0, false, false,  NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_E010R5,     0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_LOLI,       0, true,  false,  NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_E129,       1, false, false,  STR_SUBTYPE_E129,      nullptr}, //new
  {MODULE_SUBTYPE_MULTI_JOYSWAY,    0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_E016H,      0, false, false,  NO_SUBTYPE,            nullptr}, //new
  // MODULE_SUBTYPE_MULTI_CONFIG non selectable and masked out for selection
  // MODULE_SUBTYPE_MULTI_IKEAANSLUTA non selectable and masked out for selection
  // MODULE_SUBTYPE_MULTI_WILLIFM non selectable and masked out for selection
  {MODULE_SUBTYPE_MULTI_LOSI,       0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_MOULDKG,    1, false, false,  STR_SUBTYPE_MOULDKG,   STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_XERALL,     0, false, false,  NO_SUBTYPE,            nullptr}, //new
  {MODULE_SUBTYPE_MULTI_MT99XX2,    0, false, false,  STR_SUBTYPE_MT992,     nullptr},
  {MODULE_SUBTYPE_MULTI_KYOSHO2,    0, false, false,  STR_SUBTYPE_KYOSHO2,   nullptr},
  {MODULE_SUBTYPE_MULTI_SCORPIO,    0, false, true,   NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_NN1,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN2,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN3,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN4,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN5,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN6,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN7,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN8,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN9,        7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_NN10,       7, true,  true,   STR_SUBTYPE_NN,        STR_MULTI_OPTION},
  //
  // most likely no longer required
  //
  {MM_RF_CUSTOM_SELECTED,           0, true,  false,  NO_SUBTYPE,            STR_MULTI_OPTION},

  // Sentinel and default for protocols not listed above (MM_RF_CUSTOM is 0xff)
  {MODULE_SUBTYPE_MULTI_SENTINEL,   0, false, false,  NO_SUBTYPE,            nullptr},
};

#endif // MULTIMODULE
