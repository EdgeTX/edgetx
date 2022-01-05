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

#include "moduledata.h"
#include "eeprominterface.h"
#include "multiprotocols.h"
#include "afhds3.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"
#include "generalsettings.h"

#include <QPair>
#include <QVector>

void ModuleData::convert(RadioDataConversionState & cstate)
{
  Firmware *fw = getCurrentFirmware();

  cstate.setSubComp(indexToString(cstate.subCompIdx, fw));
  cstate.setItemType("Protocol", 1);
  RadioDataConversionState::EventType evt = RadioDataConversionState::EVT_NONE;
  RadioDataConversionState::LogField oldData(protocol, protocolToString(protocol));

  if (fw->getBoard() == cstate.toType) {
    if (cstate.subCompIdx < fw->getCapability(NumFirstUsableModule) || cstate.subCompIdx > fw->getCapability(NumModules) - 1) {
      if ((PulsesProtocol) protocol != PULSES_OFF)
        evt = RadioDataConversionState::EVT_INV;
    }
    else if (!fw->isAvailable((PulsesProtocol) protocol, cstate.subCompIdx)) {
      evt = RadioDataConversionState::EVT_INV;
    }
  }
  else {
    evt = RadioDataConversionState::EVT_INV;
    qDebug() << "Error - current firmware board does not match conversion to board!";
  }

  if (evt == RadioDataConversionState::EVT_INV) {
    cstate.setInvalid(oldData);
    clear();
  }
}

bool ModuleData::isPxx2Module() const
{
  switch(protocol){
    case PULSES_ACCESS_ISRM:
    case PULSES_ACCESS_R9M:
    case PULSES_ACCESS_R9M_LITE:
    case PULSES_ACCESS_R9M_LITE_PRO:
    case PULSES_XJT_LITE_X16:
      return true;
    default:
      return false;
  }
}

bool ModuleData::supportRxNum() const
{
  switch (protocol) {
    case PULSES_PXX_XJT_X16:
    case PULSES_PXX_R9M:
    case PULSES_PXX_R9M_LITE:
    case PULSES_PXX_R9M_LITE_PRO:
    case PULSES_MULTIMODULE:
    case PULSES_CROSSFIRE:
    case PULSES_ACCESS_ISRM:
    case PULSES_ACCST_ISRM_D16:
    case PULSES_ACCESS_R9M:
    case PULSES_ACCESS_R9M_LITE:
    case PULSES_ACCESS_R9M_LITE_PRO:
    case PULSES_XJT_LITE_X16:
    case PULSES_XJT_LITE_LR12:
    case PULSES_PXX_XJT_LR12:
    case PULSES_LP45:
    case PULSES_DSM2:
    case PULSES_DSMX:
      return true;
    default:
      return false;
  }
}

QString ModuleData::rfProtocolToString() const
{
  switch (protocol) {
    case PULSES_MULTIMODULE:
      return Multiprotocols::protocolToString((int)multi.rfProtocol);
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModuleData::subTypeToString(int type) const
{
  static const char * strings[] = {
    "FCC",
    "EU",
    "868MHz",
    "915MHz"
  };

  if (type < 0)
    type = subType;

  switch (protocol) {
    case PULSES_MULTIMODULE:
      return Multiprotocols::subTypeToString((int)multi.rfProtocol, (unsigned)type);
    case PULSES_PXX_R9M:
      return CHECK_IN_ARRAY(strings, type);
    case PULSES_AFHDS3:
      return Afhds3Data::protocolToString(type);
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModuleData::powerValueToString(Firmware * fw) const
{
  const QStringList & strRef = powerValueStrings((enum PulsesProtocol)protocol, subType, fw);
  return strRef.value(protocol == PULSES_AFHDS3 ? afhds3.rfPower : pxx.power, CPN_STR_UNKNOWN_ITEM);
}

// static
QString ModuleData::indexToString(int index, Firmware * fw)
{
  if (index < 0)
    return tr("Trainer Port");

  if (fw->getCapability(NumModules) > 1) {
    if (IS_HORUS_OR_TARANIS(fw->getBoard()))
      return index == 0 ? tr("Internal Radio System") : tr("External Radio Module");
    if (index > 0)
      return tr("Extra Radio System");
  }
  return tr("Radio System");
}

// static
QString ModuleData::protocolToString(unsigned int protocol)
{
  static const char * strings[] = {
    "OFF",
    "PPM",
    "Silverlit A", "Silverlit B", "Silverlit C",
    "CTP1009",
    "LP45", "DSM2", "DSMX",
    "PPM16", "PPMsim",
    "FrSky XJT (D16)", "FrSky XJT (D8)", "FrSky XJT (LR12)", "FrSky DJT",
    "Crossfire",
    "Multi",
    "FrSky R9M",
    "FrSky R9M Lite",
    "FrSky R9M Lite Pro",
    "SBUS output at VBat",
    "FrSky ACCESS ISRM", "FrSky ACCST ISRM D16",
    "FrSky ACCESS R9M 2019",
    "FrSky ACCESS R9M Lite",
    "FrSky ACCESS R9M Lite Pro",
    "FrSky XJT lite (D16)", "FrSky XJT lite (D8)", "FrSky XJT lite (LR12)",
    "AFHDS3",
    "Ghost"
  };

  return CHECK_IN_ARRAY(strings, protocol);
}

// static
QStringList ModuleData::powerValueStrings(enum PulsesProtocol protocol, int subType, Firmware * fw)
{
  static const QStringList strings[] = {
    { tr("10mW - 16CH"), tr("100mW - 16CH"), tr("500mW - 16CH"), tr("Auto <= 1W - 16CH") },                         // full-size FCC
    { tr("25mW - 8CH"), tr("25mW - 16CH"), tr("200mW - 16CH (no telemetry)"), tr("500mW - 16CH (no telemetry)") },  // full-size EU
    { tr("100mW - 16CH") },                                                                                         // mini FCC
    { tr("25mW - 8CH"), tr("25mW - 16CH"), tr("100mW - 16CH (no telemetry)") }                                      // mini EU
  };
  static const QStringList afhds3Strings = {
    tr("25 mW"), tr("100 mW"), tr("500 mW"), tr("1 W"), tr("2 W")
  };

  switch(protocol) {
    case PULSES_AFHDS3:
      return afhds3Strings;
    default:
      int strIdx = 0;
      if (subType == MODULE_SUBTYPE_R9M_EU)
        strIdx += 1;
      if (fw->getCapability(HasModuleR9MMini))
        strIdx += 2;
      return strings[strIdx];
  }
}

bool ModuleData::hasFailsafes(Firmware * fw) const
{
  return fw->getCapability(HasFailsafe) && (
    protocol == PULSES_ACCESS_ISRM ||
    protocol == PULSES_ACCST_ISRM_D16 ||
    protocol == PULSES_PXX_XJT_X16 ||
    protocol == PULSES_PXX_R9M ||
    protocol == PULSES_ACCESS_R9M ||
    protocol == PULSES_ACCESS_R9M_LITE ||
    protocol == PULSES_ACCESS_R9M_LITE_PRO ||
    protocol == PULSES_XJT_LITE_X16 ||
    protocol == PULSES_MULTIMODULE ||
    protocol == PULSES_AFHDS3
    );
}

int ModuleData::getMaxChannelCount()
{
  switch (protocol) {
    case PULSES_ACCESS_ISRM:
      return 24;
    case PULSES_PXX_R9M:
    case PULSES_ACCESS_R9M:
    case PULSES_ACCESS_R9M_LITE:
    case PULSES_ACCESS_R9M_LITE_PRO:
    case PULSES_ACCST_ISRM_D16:
    case PULSES_XJT_LITE_X16:
    case PULSES_PXX_XJT_X16:
    case PULSES_CROSSFIRE:
    case PULSES_GHOST:
    case PULSES_SBUS:
    case PULSES_PPM:
      return 16;
    case PULSES_XJT_LITE_LR12:
    case PULSES_PXX_XJT_LR12:
      return 12;
    case PULSES_PXX_DJT:
    case PULSES_XJT_LITE_D8:
    case PULSES_PXX_XJT_D8:
      return 8;
    case PULSES_LP45:
    case PULSES_DSM2:
    case PULSES_DSMX:
      return 6;
    case PULSES_MULTIMODULE:
      if (multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
        return 12;
      else
        return 16;
      break;
    case PULSES_AFHDS3:
      return 18;
    case PULSES_OFF:
      break;
    default:
      break;
  }
  return 8;
}

/*
      If Companion did not combine the /radio/src/datastructs type and sub-type fields
      this would be so much cleaner and, modules and protocols generally easier to maintain
      Another case for refactoring ModuleData as a whole!
*/

//  static
int ModuleData::getTypeFromProtocol(unsigned int protocol)
{
  //  must be kept in sync with opentxeeprom.h ProtocolsConversionTable

  const QVector<QPair<int, int>>ProtocolTypeTable = {

                          { PULSES_OFF,                 MODULE_TYPE_NONE },
                          { PULSES_PPM,                 MODULE_TYPE_PPM },

                          { PULSES_PXX_XJT_X16,         MODULE_TYPE_XJT_PXX1 },
                          { PULSES_PXX_XJT_D8,          MODULE_TYPE_XJT_PXX1 },
                          { PULSES_PXX_XJT_LR12,        MODULE_TYPE_XJT_PXX1 },

                          { PULSES_ACCESS_ISRM,         MODULE_TYPE_ISRM_PXX2 },
                          { PULSES_ACCST_ISRM_D16,      MODULE_TYPE_ISRM_PXX2 },

                          { PULSES_LP45,                MODULE_TYPE_DSM2 },
                          { PULSES_DSM2,                MODULE_TYPE_DSM2 },
                          { PULSES_DSMX,                MODULE_TYPE_DSM2 },

                          { PULSES_CROSSFIRE,           MODULE_TYPE_CROSSFIRE },
                          { PULSES_MULTIMODULE,         MODULE_TYPE_MULTIMODULE },
                          { PULSES_PXX_R9M,             MODULE_TYPE_R9M_PXX1 },
                          { PULSES_ACCESS_R9M,          MODULE_TYPE_R9M_PXX2 },
                          { PULSES_PXX_R9M_LITE,        MODULE_TYPE_R9M_LITE_PXX1 },
                          { PULSES_ACCESS_R9M_LITE,     MODULE_TYPE_R9M_LITE_PXX2 },
                          { PULSES_GHOST,               MODULE_TYPE_GHOST },
                          { PULSES_ACCESS_R9M_LITE_PRO, MODULE_TYPE_R9M_LITE_PRO_PXX2 },
                          { PULSES_SBUS,                MODULE_TYPE_SBUS },

                          { PULSES_XJT_LITE_X16,        MODULE_TYPE_XJT_LITE_PXX2 },
                          { PULSES_XJT_LITE_D8,         MODULE_TYPE_XJT_LITE_PXX2 },
                          { PULSES_XJT_LITE_LR12,       MODULE_TYPE_XJT_LITE_PXX2 },

                          { PULSES_AFHDS3,              MODULE_TYPE_FLYSKY }
                      };

  QPair<int, int>elmt;

  foreach (elmt, ProtocolTypeTable) {
    if (elmt.first == (int)protocol)
      return elmt.second;
  }

  return -1;
}

//  static
int ModuleData::getSubTypeFromProtocol(unsigned int protocol)
{
  if (protocol == PULSES_ACCST_ISRM_D16 || protocol == PULSES_ACCESS_ISRM)
    return protocol - PULSES_ACCESS_ISRM;
  else if (protocol >= PULSES_PXX_XJT_X16 && protocol <= PULSES_PXX_XJT_LR12)
    return protocol - PULSES_PXX_XJT_X16;
  else
    return 0;
}

//  static
QString ModuleData::typeToString(int type)
{
  static const char * strings[] = {
    "OFF",
    "PPM",
    "XJT",
    "ISRM",
    "DSM2",
    "CRSF",
    "MULTI",
    "R9M",
    "R9M ACCESS",
    "R9MLite",
    "R9ML ACCESS",
    "GHST",
    "R9MLP ACCESS",
    "SBUS",
    "XJT Lite",
    "FLYSKY"
  };

  return CHECK_IN_ARRAY(strings, type);
}

AbstractStaticItemModel * ModuleData::internalModuleItemModel(int board)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("moduledata.internalmodule");

  auto modules = Boards::getSupportedInternalModules(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : (Board::Type)board);
  for(auto mod : modules) {
    mdl->appendToItemList(typeToString(mod), mod);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
bool ModuleData::isProtocolAvailable(int moduleidx, unsigned int protocol, GeneralSettings & settings)
{
  if (protocol == PULSES_OFF)
    return true;

  Firmware *fw = getCurrentFirmware();
  Board::Type board = fw->getBoard();

  if (moduleidx == 0 && Boards::getCapability(board, Board::HasInternalModuleSupport))
    return (int)settings.internalModule == getTypeFromProtocol(protocol);

  QString id = fw->getId();

  if (IS_HORUS_OR_TARANIS(board)) {
    switch (moduleidx) {
      case 1:
        switch (protocol) {
          case PULSES_OFF:
          case PULSES_PPM:
            return true;
          case PULSES_PXX_XJT_X16:
          case PULSES_PXX_XJT_D8:
          case PULSES_PXX_XJT_LR12:
            return !(IS_TARANIS_XLITES(board) || IS_TARANIS_X9LITE(board));
          case PULSES_PXX_R9M:
          case PULSES_LP45:
          case PULSES_DSM2:
          case PULSES_DSMX:
          case PULSES_SBUS:
          case PULSES_MULTIMODULE:
          case PULSES_CROSSFIRE:
          case PULSES_AFHDS3:
          case PULSES_GHOST:
            return true;
          case PULSES_ACCESS_R9M:
            return IS_ACCESS_RADIO(board, id)  || (IS_FAMILY_HORUS_OR_T16(board) && id.contains("externalaccessmod"));
          case PULSES_PXX_R9M_LITE:
          case PULSES_ACCESS_R9M_LITE:
          case PULSES_ACCESS_R9M_LITE_PRO:
          case PULSES_XJT_LITE_X16:
          case PULSES_XJT_LITE_D8:
          case PULSES_XJT_LITE_LR12:
            return (IS_TARANIS_XLITE(board) || IS_TARANIS_X9LITE(board) || IS_JUMPER_TLITE(board));
          default:
            return false;
        }

      case -1:
        switch (protocol) {
          case PULSES_PPM:
            return true;
          default:
            return false;
        }

      default:
        return false;
    }
  }
  else if (IS_SKY9X(board)) {
    switch (moduleidx) {
      case 0:
        switch (protocol) {
          case PULSES_PPM:
          case PULSES_PXX_XJT_X16:
          case PULSES_PXX_XJT_D8:
          case PULSES_PXX_XJT_LR12:
          case PULSES_PXX_R9M:
          case PULSES_LP45:
          case PULSES_DSM2:
          case PULSES_DSMX:
          case PULSES_SBUS:
          case PULSES_MULTIMODULE:
            return true;
          default:
            return false;
        }
        break;
      case 1:
        switch (protocol) {
          case PULSES_PPM:
            return true;
          default:
            return false;
        }
        break;
      default:
        return false;
    }
  }
  else {
    switch (protocol) {
      case PULSES_PPM:
      case PULSES_DSMX:
      case PULSES_LP45:
      case PULSES_DSM2:
        // case PULSES_PXX_DJT:     // Unavailable for now
      case PULSES_PPM16:
      case PULSES_PPMSIM:
        return true;
      default:
        return false;
    }
  }

  return false; //  to avoid compiler warning
}

//  static
AbstractStaticItemModel * ModuleData::protocolItemModel(GeneralSettings & settings)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("moduledata.protocol");

  for (int i = 0; i <= 1; i++) {
    for (unsigned int j = 0; j < PULSES_PROTOCOL_LAST; j++) {
      mdl->appendToItemList(protocolToString(j), j, isProtocolAvailable(i, j, settings), 0, i + 1/*flag cannot be 0*/);
    }
  }

  mdl->loadItemList();
  return mdl;
}
