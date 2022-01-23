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

#include <QObject>

#include "multiprotocols.h"
#include "radiodata.h"

#define STR_MPM_SUBTYPE                    QT_TRANSLATE_NOOP("Multiprotocols", "Subtype")
#define STR_MPM_VIDFREQ                    QT_TRANSLATE_NOOP("Multiprotocols", "Video TX frequency")
#define STR_MPM_RFTUNE                     QT_TRANSLATE_NOOP("Multiprotocols", "CC2500 frequency fine tune")
#define STR_MPM_RFPOWER                    QT_TRANSLATE_NOOP("Multiprotocols", "RF power")
#define STR_MPM_TELEMETRY                  QT_TRANSLATE_NOOP("Multiprotocols", "Telemetry")
#define STR_MPM_SERVOFREQ                  QT_TRANSLATE_NOOP("Multiprotocols", "Servo output frequency")
#define STR_MPM_OPTION                     QT_TRANSLATE_NOOP("Multiprotocols", "Option value")
#define STR_MPM_FIXEDID                    QT_TRANSLATE_NOOP("Multiprotocols", "Fixed ID value")
#define STR_MPM_DEFAULT                    QT_TRANSLATE_NOOP("Multiprotocols", "DEFAULT")

static const QStringList STR_SUBTYPE_CUSTOM    ({
                                                  STR_MPM_SUBTYPE " 0", STR_MPM_SUBTYPE " 1", STR_MPM_SUBTYPE " 2", STR_MPM_SUBTYPE " 3",
                                                  STR_MPM_SUBTYPE " 4", STR_MPM_SUBTYPE " 5", STR_MPM_SUBTYPE " 6", STR_MPM_SUBTYPE " 7"
                                                });
static const QStringList STR_SUBTYPE_FLYSKY    {"Standard", "V9x9", "V6x6", "V912", "CX20"};
static const QStringList STR_SUBTYPE_HUBSAN    {"H107", "H301", "H501"};
static const QStringList STR_SUBTYPE_FRSKY     {"D16", "D8", "D16 8ch", "V8", "D16 EU-LBT", "D16 EU-LBT 8ch", "D8 Cloned", "D16 Cloned"};
static const QStringList STR_SUBTYPE_HISKY     {"Standard", "HK310"};
static const QStringList STR_SUBTYPE_V2X2      {"Standard", "JXD506", "MR101"};
static const QStringList STR_SUBTYPE_DSM       {"DSM2 1F", "DSM2 2F", "DSMX 1F", "DSMX 2F"};
static const QStringList STR_SUBTYPE_DEVO      {"8 Channel", "10 Channel", "12 Channel", "6 Channel", "7 Channel"};
static const QStringList STR_SUBTYPE_YD717     {"Standard", "Skywalker", "Syma X4", "XINXUN", "NIHUI"};
static const QStringList STR_SUBTYPE_KN        {"WLtoys", "FeiLun"};
static const QStringList STR_SUBTYPE_SYMAX     {"Standard", "Syma X5C"};
static const QStringList STR_SUBTYPE_SLT       {"V1 (6 Channel)", "V2 (8 Channel)", "Q100", "Q200", "MR100"};
static const QStringList STR_SUBTYPE_CX10      {"Green", "Blue", "DM007", "-", "JC3015a", "JC3015b", "MK33041"};
static const QStringList STR_SUBTYPE_CG023     {"Standard", "YD829"};
static const QStringList STR_SUBTYPE_BAYANG    {"Standard", "H8S3D", "X16 AH", "IRDRONE", "DHD D4", "QX100"};
static const QStringList STR_SUBTYPE_ESky      {"Standard", "ET4"};
static const QStringList STR_SUBTYPE_MT99      {"MT99", "H7", "YZ", "LS", "FY805"};
static const QStringList STR_SUBTYPE_MJXQ      {"WLH08", "X600", "X800", "H26D", "E010", "H26WH", "Phoenix"};
static const QStringList STR_SUBTYPE_FY326     {"Standard", "FY319"};
static const QStringList STR_SUBTYPE_FUTABA    {"SFHSS"};
static const QStringList STR_SUBTYPE_HONTAI    {"Standard", "JJRC X1", "X5C1 Clone", "FQ777_951"};
static const QStringList STR_SUBTYPE_AFHDS2A   {"PWM and IBUS", "PPM and IBUS", "PWM and SBUS", "PPM and SBUS", "PWM and IBUS16", "PPM and IBUS16"};
static const QStringList STR_SUBTYPE_Q2X2      {"Q222", "Q242", "Q282"};
static const QStringList STR_SUBTYPE_WK2x01    {"WK2801", "WK2401", "W6_5_1", "W6_6_1", "W6_HEL", "W6_HEL_I"};
static const QStringList STR_SUBTYPE_Q303      {"Standard", "CX35", "CX10D", "CX10WD"};
static const QStringList STR_SUBTYPE_CABELL    {"Cabell V3", "Cab V3 Telem", "-", "-", "-", "-", "Set FailSafe", "Unbind"};
static const QStringList STR_SUBTYPE_ESKY150   {"4 Channel", "7 Channel"};
static const QStringList STR_SUBTYPE_H83D      {"H8 Mini 3D", "H20H", "H20 Mini", "H30 Mini"};
static const QStringList STR_SUBTYPE_CORONA    {"Corona V1", "Corona V2", "Flydream V3"};
static const QStringList STR_SUBTYPE_HITEC     {"Optima", "Optima Hub Telem", "Minima"};
static const QStringList STR_SUBTYPE_WFLY      {"WFR0x"};
static const QStringList STR_SUBTYPE_BUGS_MINI {"Standard", "Bugs 3H"};
static const QStringList STR_SUBTYPE_TRAXXAS   {"6519 RX"};
static const QStringList STR_SUBTYPE_E01X      {"E012", "E015", "E016H"};
static const QStringList STR_SUBTYPE_V911S     {"Standard", "E119"};
static const QStringList STR_SUBTYPE_GD00X     {"GD V1", "GD V2"};
static const QStringList STR_SUBTYPE_V761      {"3 Channel", "4 Channel"};
static const QStringList STR_SUBTYPE_REDPINE   {"Fast", "Slow"};
static const QStringList STR_SUBTYPE_POTENSIC  {"A20 Firefly"};
static const QStringList STR_SUBTYPE_ZSX       {"JJRC ZSX-280"};
static const QStringList STR_SUBTYPE_HEIGHT    {"5 Channel", "8 Channel"};
static const QStringList STR_SUBTYPE_FRSKYX_RX {"RX", "Clone TX"};
static const QStringList STR_SUBTYPE_HOTT      {"Sync", "No_Sync"};
static const QStringList STR_SUBTYPE_FX816     {"P38"};
static const QStringList STR_SUBTYPE_PELIKAN   {"Pro", "Lite"};
static const QStringList STR_SUBTYPE_XK        {"X450", "X420"};
static const QStringList STR_SUBTYPE_XN297DUMP {"250K", "1M", "2M", "AUTO", "NRF"};
static const QStringList STR_SUBTYPE_FRSKYX2   {"D16", "D16 8ch", "D16 EU-LBT", "D16 EU-LBT 8ch", "Cloned", "Cloned 8ch"};
static const QStringList STR_SUBTYPE_FRSKYR9   {"915 MHz", "868 MHz", "915 MHz 8-Channel", "868 MHz 8-Channel", "FCC", "---", "FCC 8-Channel"};
static const QStringList STR_SUBTYPE_PROPEL    {"74-Z"};
static const QStringList STR_SUBTYPE_FRSKYL    {"LR12", "LR12 6-Channel"};
static const QStringList STR_SUBTYPE_ESKY150V2 {"150 V2"};
static const QStringList STR_SUBTYPE_JJRC345   {"Standard", "SkyTumbler"};
static const QStringList STR_SUBTYPE_KYOSHO    {"FHSS", "Hype"};
static const QStringList STR_SUBTYPE_RLINK     {"Surface", "Air", "DumboRC"};
static const QStringList STR_SUBTYPE_ELRS      {"Not Available WIP"};
static const QStringList STR_SUBTYPE_REALACC   {"R11"};
static const QStringList STR_SUBTYPE_WFLY2     {"RF20x"};

static const QStringList NO_SUBTYPE            {STR_MPM_DEFAULT};


// Table is designed to be shared with gui_common_arm.cpp

const Multiprotocols multiProtocols {
  {MODULE_SUBTYPE_MPM_FLYSKY,     4, false,      STR_SUBTYPE_FLYSKY,    nullptr},
  {MODULE_SUBTYPE_MPM_HUBSAN,     2, false,      STR_SUBTYPE_HUBSAN,    STR_MPM_VIDFREQ},
  {MODULE_SUBTYPE_MPM_FRSKY,      7, false,      STR_SUBTYPE_FRSKY,     STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_HISKY,      1, false,      STR_SUBTYPE_HISKY,     nullptr},
  {MODULE_SUBTYPE_MPM_V2X2,       2, false,      STR_SUBTYPE_V2X2,      nullptr},
  {MODULE_SUBTYPE_MPM_DSM2,       3, false,      STR_SUBTYPE_DSM,       nullptr},
  {MODULE_SUBTYPE_MPM_DEVO,       4, false,      STR_SUBTYPE_DEVO,      STR_MPM_FIXEDID},
  {MODULE_SUBTYPE_MPM_YD717,      4, false,      STR_SUBTYPE_YD717,     nullptr},
  {MODULE_SUBTYPE_MPM_KN,         1, false,      STR_SUBTYPE_KN,        nullptr},
  {MODULE_SUBTYPE_MPM_SYMAX,      1, false,      STR_SUBTYPE_SYMAX,     nullptr},
  {MODULE_SUBTYPE_MPM_SLT,        4, false,      STR_SUBTYPE_SLT,       STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_CX10,       6, false,      STR_SUBTYPE_CX10,      nullptr},
  {MODULE_SUBTYPE_MPM_CG023,      1, false,      STR_SUBTYPE_CG023,     nullptr},
  {MODULE_SUBTYPE_MPM_BAYANG,     5, false,      STR_SUBTYPE_BAYANG,    STR_MPM_TELEMETRY},
  {MODULE_SUBTYPE_MPM_ESky,       1, false,      STR_SUBTYPE_ESky,      nullptr},
  {MODULE_SUBTYPE_MPM_MT99XX,     4, false,      STR_SUBTYPE_MT99,      nullptr},
  {MODULE_SUBTYPE_MPM_MJXQ,       6, false,      STR_SUBTYPE_MJXQ,      STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_FY326,      1, false,      STR_SUBTYPE_FY326,     nullptr},
  {MODULE_SUBTYPE_MPM_FUTABA,     0, true,       STR_SUBTYPE_FUTABA,    STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_J6PRO,      0, false,      NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MPM_HONTAI,     3, false,      STR_SUBTYPE_HONTAI,    nullptr},
  {MODULE_SUBTYPE_MPM_OLRS,       0, false,      NO_SUBTYPE,            STR_MPM_RFPOWER},
  {MODULE_SUBTYPE_MPM_FS_AFHDS2A, 5, true,       STR_SUBTYPE_AFHDS2A,   STR_MPM_SERVOFREQ},
  {MODULE_SUBTYPE_MPM_Q2X2,       2, false,      STR_SUBTYPE_Q2X2,      nullptr},
  {MODULE_SUBTYPE_MPM_WK_2X01,    5, false,      STR_SUBTYPE_WK2x01,    nullptr},
  {MODULE_SUBTYPE_MPM_Q303,       3, false,      STR_SUBTYPE_Q303,      nullptr},
  {MODULE_SUBTYPE_MPM_CABELL,     7, false,      STR_SUBTYPE_CABELL,    STR_MPM_OPTION},
  {MODULE_SUBTYPE_MPM_ESKY150,    1, false,      STR_SUBTYPE_ESKY150,   nullptr},
  {MODULE_SUBTYPE_MPM_H83D,       3, false,      STR_SUBTYPE_H83D,      nullptr},
  {MODULE_SUBTYPE_MPM_CORONA,     2, false,      STR_SUBTYPE_CORONA,    STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_HITEC,      2, false,      STR_SUBTYPE_HITEC,     STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_WFLY,       0, false,      STR_SUBTYPE_WFLY,      nullptr},
  {MODULE_SUBTYPE_MPM_BUGS_MINI,  1, false,      STR_SUBTYPE_BUGS_MINI, nullptr},
  {MODULE_SUBTYPE_MPM_TRAXXAS,    0, false,      STR_SUBTYPE_TRAXXAS,   nullptr},
  {MODULE_SUBTYPE_MPM_E01X,       2, false,      STR_SUBTYPE_E01X,      STR_MPM_OPTION},
  {MODULE_SUBTYPE_MPM_V911S,      1, false,      STR_SUBTYPE_V911S,     STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_GD00X,      1, false,      STR_SUBTYPE_GD00X,     STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_V761,       1, false,      STR_SUBTYPE_V761,      nullptr},
  {MODULE_SUBTYPE_MPM_KF606,      0, false,      NO_SUBTYPE,            STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_REDPINE,    1, false,      STR_SUBTYPE_REDPINE,   STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_POTENSIC,   0, false,      STR_SUBTYPE_POTENSIC,  nullptr},
  {MODULE_SUBTYPE_MPM_ZSX,        0, false,      STR_SUBTYPE_ZSX,       nullptr},
  {MODULE_SUBTYPE_MPM_HEIGHT,     1, false,      STR_SUBTYPE_HEIGHT,    nullptr},
  {MODULE_SUBTYPE_MPM_FRSKYX_RX,  1, false,      STR_SUBTYPE_FRSKYX_RX, STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_HOTT,       1, true,       STR_SUBTYPE_HOTT,      STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_FX816,      0, false,      STR_SUBTYPE_FX816,     nullptr},
  {MODULE_SUBTYPE_MPM_PELIKAN,    1, false,      STR_SUBTYPE_PELIKAN,   nullptr},
  {MODULE_SUBTYPE_MPM_XK,         1, false,      STR_SUBTYPE_XK,        STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_XN297DUMP,  4, false,      STR_SUBTYPE_XN297DUMP, nullptr},
  {MODULE_SUBTYPE_MPM_FRSKYX2,    5, true,       STR_SUBTYPE_FRSKYX2,   STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_FRSKY_R9,   6, true,       STR_SUBTYPE_FRSKYR9,   nullptr},
  {MODULE_SUBTYPE_MPM_PROPEL,     0, false,      STR_SUBTYPE_PROPEL,    nullptr},
  {MODULE_SUBTYPE_MPM_FRSKYL,     1, false,      STR_SUBTYPE_FRSKYL,    STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_SKYARTEC,   0, false,      NO_SUBTYPE,            STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_ESKY150V2,  0, false,      STR_SUBTYPE_ESKY150V2, STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_JJRC345,    1, false,      STR_SUBTYPE_JJRC345,   nullptr},
  {MODULE_SUBTYPE_MPM_Q90C,       0, false,      NO_SUBTYPE,            STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_KYOSHO,     1, false,      STR_SUBTYPE_KYOSHO,    nullptr},
  {MODULE_SUBTYPE_MPM_RLINK,      2, false,      STR_SUBTYPE_RLINK,     STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_ELRS,       0, false,      STR_SUBTYPE_ELRS,      nullptr},
  {MODULE_SUBTYPE_MPM_REALACC,    0, false,      STR_SUBTYPE_REALACC,   nullptr},
  {MODULE_SUBTYPE_MPM_OMP,        0, false,      NO_SUBTYPE,            STR_MPM_RFTUNE},
  {MODULE_SUBTYPE_MPM_WFLY2,      0, false,      STR_SUBTYPE_WFLY2,     STR_MPM_OPTION},
  {MODULE_SUBTYPE_MPM_E016HV2,    0, false,      NO_SUBTYPE,            STR_MPM_RFTUNE},
  {MM_RF_CUSTOM_SELECTED,           7, true,       STR_SUBTYPE_CUSTOM,    STR_MPM_OPTION},

  // Sentinel and default for protocols not listed above (MM_RF_CUSTOM is 0xff)
  { 0xfe,                           0, false,      NO_SUBTYPE,            nullptr}
};

int Multiprotocols::MultiProtocolDefinition::getOptionMin() const {
  if (optionsstr == STR_MPM_RFPOWER)
    return -1;
  else if (optionsstr == STR_MPM_SERVOFREQ)
    return 0;
  else
    return -128;
}

int Multiprotocols::MultiProtocolDefinition::getOptionMax() const {
  if (optionsstr == STR_MPM_RFPOWER)
    return 7;
  else if (optionsstr == STR_MPM_SERVOFREQ)
    return 70;
  else
    return 127;
}

const Multiprotocols::MultiProtocolDefinition & Multiprotocols::getProtocol(int protocol) const {
  for (const Multiprotocols::MultiProtocolDefinition & proto: protocols)
    if (proto.protocol == protocol)
      return proto;

  // Protocol not found, return the default (last) proto
  Q_ASSERT(protocols.rbegin()->protocol == 0xfe);
  return *protocols.rbegin();
}

// static
QString Multiprotocols::protocolToString(int protocol, bool custom)
{
  static const QStringList strings({
    "FlySky", "Hubsan", "FrSky", "Hisky", "V2x2", "DSM", "Devo", "YD717", "KN", "SymaX", "SLT", "CX10", "CG023",
    "Bayang", "ESky", "MT99XX", "MJXQ", "Shenqi", "FY326", "Futaba", "J6 PRO","FQ777","Assan","Hontai","Open LRS",
    "FlySky AFHDS2A", "Q2x2", "WK2x01", "Q303", "GW008", "DM002", "Cabell", "ESky 150", "H8 3D", "Corona", "CFlie",
    "Hitec", "Wfly", "Bugs", "Bugs Mini", "Traxxas", "NCC-1701-A", "E01X", "WL Heli V911S", "GD00X", "Volantex V761",
    "KFPlan KF606", "Redpine", "Potensic", "ZSX", "Height", "Scanner", "FrSky RX", "FlySky AFHDS2A RX", "HoTT", "Fx816",
    "Bayang RX", "Pelikan", "Tiger", "XK", "XN297 Dump", "FrSky X 2.1", "FrSky R9", "Propel", "FrSky L", "Skyartec",
    "ESky 150v2", "DSM RX", "JJRC345", "Q90C", "Kyosho", "RadioLink", "ExpressLRS", "Realacc", "OMP", "M-Link", "Wfly 2",
    "E016H v2"
  });

  return strings.value(protocol, CPN_STR_UNKNOWN_ITEM);
}

// static
QString Multiprotocols::subTypeToString(int protocol, unsigned subType)
{
  if (protocol > MODULE_SUBTYPE_MPM_LAST)
    return tr(qPrintable(QString::number(subType)));
  else
    return tr(qPrintable(multiProtocols.getProtocol(protocol).subTypeStrings.value(subType, CPN_STR_UNKNOWN_ITEM)));
}
