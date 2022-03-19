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

#define STR_MULTI_SUBTYPE                    QT_TRANSLATE_NOOP("Multiprotocols", "Subtype")
#define STR_MULTI_VIDFREQ                    QT_TRANSLATE_NOOP("Multiprotocols", "Video TX frequency")
#define STR_MULTI_RFTUNE                     QT_TRANSLATE_NOOP("Multiprotocols", "CC2500 frequency fine tune")
#define STR_MULTI_RFPOWER                    QT_TRANSLATE_NOOP("Multiprotocols", "RF power")
#define STR_MULTI_TELEMETRY                  QT_TRANSLATE_NOOP("Multiprotocols", "Telemetry")
#define STR_MULTI_SERVOFREQ                  QT_TRANSLATE_NOOP("Multiprotocols", "Servo output frequency")
#define STR_MULTI_OPTION                     QT_TRANSLATE_NOOP("Multiprotocols", "Option value")
#define STR_MULTI_FIXEDID                    QT_TRANSLATE_NOOP("Multiprotocols", "Fixed ID value")
#define STR_MULTI_MAX_THROW                  QT_TRANSLATE_NOOP("Multiprotocols", "Enable Max Throw")
#define STR_MULTI_RFCHAN                     QT_TRANSLATE_NOOP("Multiprotocols", "RF Channel Number")
#define STR_MULTI_DEFAULT                    QT_TRANSLATE_NOOP("Multiprotocols", "DEFAULT")

static const QStringList STR_SUBTYPE_CUSTOM    ({
                                                  STR_MULTI_SUBTYPE " 0", STR_MULTI_SUBTYPE " 1", STR_MULTI_SUBTYPE " 2", STR_MULTI_SUBTYPE " 3",
                                                  STR_MULTI_SUBTYPE " 4", STR_MULTI_SUBTYPE " 5", STR_MULTI_SUBTYPE " 6", STR_MULTI_SUBTYPE " 7"
                                                });
static const QStringList STR_SUBTYPE_FLYSKY    {"Standard", "V9x9", "V6x6", "V912", "CX20"};
static const QStringList STR_SUBTYPE_HUBSAN    {"H107", "H301", "H501"};
static const QStringList STR_SUBTYPE_FRSKY     {"D16", "D8", "D16 8ch", "V8", "D16 EU-LBT", "D16 EU-LBT 8ch", "D8 Cloned", "D16 Cloned"};
static const QStringList STR_SUBTYPE_HISKY     {"Standard", "HK310"};
static const QStringList STR_SUBTYPE_V2X2      {"Standard", "JXD506", "MR101"};
static const QStringList STR_SUBTYPE_DSM       {"DSM2 1F", "DSM2 2F", "DSMX 1F", "DSMX 2F", "Auto", "DSMR 1F"};
static const QStringList STR_SUBTYPE_DEVO      {"8 Channel", "10 Channel", "12 Channel", "6 Channel", "7 Channel"};
static const QStringList STR_SUBTYPE_YD717     {"Standard", "Skywalker", "Syma X4", "XINXUN", "NIHUI"};
static const QStringList STR_SUBTYPE_KN        {"WLtoys", "FeiLun"};
static const QStringList STR_SUBTYPE_SYMAX     {"Standard", "Syma X5C"};
static const QStringList STR_SUBTYPE_SLT       {"V1 (6 Channel)", "V2 (8 Channel)", "Q100", "Q200", "MR100"};
static const QStringList STR_SUBTYPE_CX10      {"Green", "Blue", "DM007", "-", "JC3015a", "JC3015b", "MK33041"};
static const QStringList STR_SUBTYPE_CG023     {"Standard", "YD829"};
static const QStringList STR_SUBTYPE_BAYANG    {"Standard", "H8S3D", "X16 AH", "IRDRONE", "DHD D4", "QX100"};
static const QStringList STR_SUBTYPE_ESky      {"Standard", "ET4"};
static const QStringList STR_SUBTYPE_MT99      {"MT99", "H7", "YZ", "LS", "FY805", "A180", "Dragon""F949G"};
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
static const QStringList STR_SUBTYPE_E01X      {"E012", "E015"};
static const QStringList STR_SUBTYPE_V911S     {"Standard", "E119"};
static const QStringList STR_SUBTYPE_GD00X     {"GD V1", "GD V2"};
static const QStringList STR_SUBTYPE_V761      {"3 Channel", "4 Channel"};
static const QStringList STR_SUBTYPE_KF606     {"Standard", "MIG320"};
static const QStringList STR_SUBTYPE_REDPINE   {"Fast", "Slow"};
static const QStringList STR_SUBTYPE_POTENSIC  {"A20 Firefly"};
static const QStringList STR_SUBTYPE_ZSX       {"JJRC ZSX-280"};
static const QStringList STR_SUBTYPE_HEIGHT    {"5 Channel", "8 Channel"};
static const QStringList STR_SUBTYPE_FRSKYX_RX {"RX", "Clone TX"};
static const QStringList STR_SUBTYPE_HOTT      {"Sync", "No_Sync"};
static const QStringList STR_SUBTYPE_FX816     {"P38"};
static const QStringList STR_SUBTYPE_PELIKAN   {"Pro", "Lite", "SCX24"};
static const QStringList STR_SUBTYPE_XK        {"X450", "X420"};
static const QStringList STR_SUBTYPE_XN297DUMP {"250K", "1M", "2M", "AUTO", "NRF","CC2500"};
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
static const QStringList STR_SUBTYPE_MOULDKG   {"Analog", "Digital"};
static const QStringList STR_SUBTYPE_MT992     {"PA18"};

static const QStringList NO_SUBTYPE            {STR_MULTI_DEFAULT};


// Table is designed to be shared with gui_common_arm.cpp

const Multiprotocols multiProtocols {
  {MODULE_SUBTYPE_MULTI_FLYSKY,     4, false,      STR_SUBTYPE_FLYSKY,    nullptr},
  {MODULE_SUBTYPE_MULTI_HUBSAN,     2, false,      STR_SUBTYPE_HUBSAN,    STR_MULTI_VIDFREQ},
  {MODULE_SUBTYPE_MULTI_FRSKY,      7, false,      STR_SUBTYPE_FRSKY,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HISKY,      1, false,      STR_SUBTYPE_HISKY,     nullptr},
  {MODULE_SUBTYPE_MULTI_V2X2,       2, false,      STR_SUBTYPE_V2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_DSM2,       5, false,      STR_SUBTYPE_DSM,       STR_MULTI_MAX_THROW},
  {MODULE_SUBTYPE_MULTI_DEVO,       4, false,      STR_SUBTYPE_DEVO,      STR_MULTI_FIXEDID},
  {MODULE_SUBTYPE_MULTI_YD717,      4, false,      STR_SUBTYPE_YD717,     nullptr},
  {MODULE_SUBTYPE_MULTI_KN,         1, false,      STR_SUBTYPE_KN,        nullptr},
  {MODULE_SUBTYPE_MULTI_SYMAX,      1, false,      STR_SUBTYPE_SYMAX,     nullptr},
  {MODULE_SUBTYPE_MULTI_SLT,        4, false,      STR_SUBTYPE_SLT,       STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_CX10,       6, false,      STR_SUBTYPE_CX10,      nullptr},
  {MODULE_SUBTYPE_MULTI_CG023,      1, false,      STR_SUBTYPE_CG023,     nullptr},
  {MODULE_SUBTYPE_MULTI_BAYANG,     5, false,      STR_SUBTYPE_BAYANG,    STR_MULTI_TELEMETRY},
  {MODULE_SUBTYPE_MULTI_ESky,       1, false,      STR_SUBTYPE_ESky,      nullptr},
  {MODULE_SUBTYPE_MULTI_MT99XX,     7, false,      STR_SUBTYPE_MT99,      nullptr},
  {MODULE_SUBTYPE_MULTI_MJXQ,       6, false,      STR_SUBTYPE_MJXQ,      nullptr},
  {MODULE_SUBTYPE_MULTI_FY326,      1, false,      STR_SUBTYPE_FY326,     nullptr},
  {MODULE_SUBTYPE_MULTI_FUTABA,     0, true,       STR_SUBTYPE_FUTABA,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_J6PRO,      0, false,      NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_HONTAI,     3, false,      STR_SUBTYPE_HONTAI,    nullptr},
  {MODULE_SUBTYPE_MULTI_OLRS,       0, false,      NO_SUBTYPE,            STR_MULTI_RFPOWER},
  {MODULE_SUBTYPE_MULTI_FS_AFHDS2A, 5, true,       STR_SUBTYPE_AFHDS2A,   STR_MULTI_SERVOFREQ},
  {MODULE_SUBTYPE_MULTI_Q2X2,       2, false,      STR_SUBTYPE_Q2X2,      nullptr},
  {MODULE_SUBTYPE_MULTI_WK_2X01,    5, false,      STR_SUBTYPE_WK2x01,    nullptr},
  {MODULE_SUBTYPE_MULTI_Q303,       3, false,      STR_SUBTYPE_Q303,      nullptr},
  {MODULE_SUBTYPE_MULTI_CABELL,     7, false,      STR_SUBTYPE_CABELL,    STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_ESKY150,    1, false,      STR_SUBTYPE_ESKY150,   nullptr},
  {MODULE_SUBTYPE_MULTI_H83D,       3, false,      STR_SUBTYPE_H83D,      nullptr},
  {MODULE_SUBTYPE_MULTI_CORONA,     2, false,      STR_SUBTYPE_CORONA,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HITEC,      2, false,      STR_SUBTYPE_HITEC,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_WFLY,       0, false,      STR_SUBTYPE_WFLY,      nullptr},
  {MODULE_SUBTYPE_MULTI_BUGS_MINI,  1, false,      STR_SUBTYPE_BUGS_MINI, nullptr},
  {MODULE_SUBTYPE_MULTI_TRAXXAS,    0, false,      STR_SUBTYPE_TRAXXAS,   nullptr},
  {MODULE_SUBTYPE_MULTI_E01X,       1, false,      STR_SUBTYPE_E01X,      nullptr},
  {MODULE_SUBTYPE_MULTI_V911S,      1, false,      STR_SUBTYPE_V911S,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_GD00X,      1, false,      STR_SUBTYPE_GD00X,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_V761,       1, false,      STR_SUBTYPE_V761,      nullptr},
  {MODULE_SUBTYPE_MULTI_KF606,      1, false,      STR_SUBTYPE_KF606,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_REDPINE,    1, false,      STR_SUBTYPE_REDPINE,   STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_POTENSIC,   0, false,      STR_SUBTYPE_POTENSIC,  nullptr},
  {MODULE_SUBTYPE_MULTI_ZSX,        0, false,      STR_SUBTYPE_ZSX,       nullptr},
  {MODULE_SUBTYPE_MULTI_HEIGHT,     1, false,      STR_SUBTYPE_HEIGHT,    nullptr},
  {MODULE_SUBTYPE_MULTI_FRSKYX_RX,  1, false,      STR_SUBTYPE_FRSKYX_RX, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_HOTT,       1, true,       STR_SUBTYPE_HOTT,      STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FX816,      0, false,      STR_SUBTYPE_FX816,     nullptr},
  {MODULE_SUBTYPE_MULTI_PELIKAN,    2, false,      STR_SUBTYPE_PELIKAN,   nullptr},
  {MODULE_SUBTYPE_MULTI_XK,         1, false,      STR_SUBTYPE_XK,        STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_XN297DUMP,  5, false,      STR_SUBTYPE_XN297DUMP, STR_MULTI_RFCHAN},
  {MODULE_SUBTYPE_MULTI_FRSKYX2,    5, true,       STR_SUBTYPE_FRSKYX2,   STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_FRSKY_R9,   6, true,       STR_SUBTYPE_FRSKYR9,   nullptr},
  {MODULE_SUBTYPE_MULTI_PROPEL,     0, false,      STR_SUBTYPE_PROPEL,    nullptr},
  {MODULE_SUBTYPE_MULTI_FRSKYL,     1, false,      STR_SUBTYPE_FRSKYL,    STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_SKYARTEC,   0, false,      NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ESKY150V2,  0, false,      STR_SUBTYPE_ESKY150V2, STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_JJRC345,    1, false,      STR_SUBTYPE_JJRC345,   nullptr},
  {MODULE_SUBTYPE_MULTI_Q90C,       0, false,      NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_KYOSHO,     1, false,      STR_SUBTYPE_KYOSHO,    nullptr},
  {MODULE_SUBTYPE_MULTI_RLINK,      2, false,      STR_SUBTYPE_RLINK,     STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_ELRS,       0, false,      STR_SUBTYPE_ELRS,      nullptr},
  {MODULE_SUBTYPE_MULTI_REALACC,    0, false,      STR_SUBTYPE_REALACC,   nullptr},
  {MODULE_SUBTYPE_MULTI_OMP,        0, false,      NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_MLINK,      0, true,       NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_WFLY2,      0, false,      STR_SUBTYPE_WFLY2,     STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_E016HV2,    0, false,      NO_SUBTYPE,            STR_MULTI_RFTUNE},
  {MODULE_SUBTYPE_MULTI_LOLI,       0, true,       NO_SUBTYPE,            nullptr},
  {MODULE_SUBTYPE_MULTI_MOULDKG,    1, false,      STR_SUBTYPE_MOULDKG,   STR_MULTI_OPTION},
  {MODULE_SUBTYPE_MULTI_MT99XX2,    0, false,      STR_SUBTYPE_MT992,     nullptr},
  {MM_RF_CUSTOM_SELECTED,           7, true,       STR_SUBTYPE_CUSTOM,    STR_MULTI_OPTION},

  // Sentinel and default for protocols not listed above (MM_RF_CUSTOM is 0xff)
  { 0xfe,                           0, false,      NO_SUBTYPE,            nullptr}
};

int Multiprotocols::MultiProtocolDefinition::getOptionMin() const {
  if (optionsstr == STR_MULTI_RFPOWER)
    return -1;
  else if (optionsstr == STR_MULTI_SERVOFREQ)
    return 0;
  else
    return -128;
}

int Multiprotocols::MultiProtocolDefinition::getOptionMax() const {
  if (optionsstr == STR_MULTI_RFPOWER)
    return 7;
  else if (optionsstr == STR_MULTI_SERVOFREQ)
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
  if (protocol > MODULE_SUBTYPE_MULTI_LAST)
    return tr(qPrintable(QString::number(subType)));
  else
    return tr(qPrintable(multiProtocols.getProtocol(protocol).subTypeStrings.value(subType, CPN_STR_UNKNOWN_ITEM)));
}
