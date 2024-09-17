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

#include "rawswitch.h"

#include "eeprominterface.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"

QString RawSwitch::toString(Board::Type board, const GeneralSettings * const generalSettings, const ModelData * const modelData, bool prefixCustomName) const
{
  if (board == Board::BOARD_UNKNOWN) {
    board = getCurrentBoard();
  }

  static const QString switches9X[] = {
    QString("THR"), QString("RUD"), QString("ELE"),
    QString("ID0"), QString("ID1"), QString("ID2"),
    QString("AIL"), QString("GEA"), QString("TRN")
  };

  static const QString trimsSwitchesAir[] = {
    tr("Trim Rud-"), tr("Trim Rud+"),
    tr("Trim Ele-"), tr("Trim Ele+"),
    tr("Trim Thr-"), tr("Trim Thr+"),
    tr("Trim Ail-"), tr("Trim Ail+"),
    tr("Trim T5-"), tr("Trim T5+"),
    tr("Trim T6-"), tr("Trim T6+"),
    tr("Trim T7-"), tr("Trim T7+"),
    tr("Trim T8-"), tr("Trim T8+")
  };

  static const QString trimsSwitchesSurface[] = {
    tr("Trim ST-"), tr("Trim ST+"),
    tr("Trim TH-"), tr("Trim TH+"),
    tr("Trim T3-"), tr("Trim T3+"),
    tr("Trim T4-"), tr("Trim T4+"),
    tr("Trim T5-"), tr("Trim T5+"),
    tr("Trim T6-"), tr("Trim T6+"),
    tr("Trim T7-"), tr("Trim T7+"),
    tr("Trim T8-"), tr("Trim T8+")
  };

  static const QString trimsSwitches2[] = {
    tr("TrmH Left"), tr("TrmH Right"),
    tr("TrmV Down"), tr("TrmV Up")
  };

  static const QString timerModes[] = {
    tr("OFF"), tr("ON"),
    tr("THs"), tr("TH%"), tr("THt")
  };

  static const QString rotaryEncoders[] = {
    tr("REa"), tr("REb")
  };

  static const QStringList directionIndicators = QStringList()
      << CPN_STR_SW_INDICATOR_UP
      << CPN_STR_SW_INDICATOR_NEUT
      << CPN_STR_SW_INDICATOR_DN;

  if (index < 0) {
    return CPN_STR_SW_INDICATOR_REV % RawSwitch(type, -index).toString(board, generalSettings, modelData);
  }
  else {
    QString swName;
    QString custName;
    div_t qr;
    switch(type) {
      case SWITCH_TYPE_SWITCH:
        if (IS_HORUS_OR_TARANIS(board)) {
          qr = div(index - 1, 3);
          swName = Boards::getSwitchInfo(qr.quot, board).name.c_str();
          if (Boards::isSwitchFunc(qr.quot, board)) {
            if (modelData) {
              int fsindex = Boards::getSwitchTagNum(qr.quot, board) - 1;
              custName = QString(modelData->functionSwitchNames[fsindex]).trimmed();
            }
          }
          else {
            if (generalSettings) {
              custName = QString(generalSettings->switchConfig[qr.quot].name).trimmed();
            }
          }
          return DataHelpers::getCompositeName(swName, custName, prefixCustomName) +
                 directionIndicators.at(qr.rem > -1 && qr.rem < directionIndicators.size() ? qr.rem : 1);
        }
        else {
          return CHECK_IN_ARRAY(switches9X, index - 1);
        }

      case SWITCH_TYPE_VIRTUAL:
        if (modelData)
          return modelData->logicalSw[index].nameToString(index - 1);
        else
          return LogicalSwitchData().nameToString(index - 1);

      case SWITCH_TYPE_MULTIPOS_POT:
        if (!Boards::getCapability(board, Board::MultiposPotsPositions))
          return CPN_STR_UNKNOWN_ITEM;
        qr = div(index - 1, Boards::getCapability(board, Board::MultiposPotsPositions));
        if (generalSettings && qr.quot < (int)DIM(generalSettings->inputConfig))
          swName = QString(generalSettings->inputConfig[qr.quot].name);
        if (swName.isEmpty())
          swName = Boards::getInputName(qr.quot, board);
        return swName + "_" + QString::number(qr.rem + 1);

      case SWITCH_TYPE_TRIM:
        return (Boards::getCapability(board, Board::NumTrims) == 2 ?
                CHECK_IN_ARRAY(trimsSwitches2, index - 1) :
                (Boards::isAir(board) ? CHECK_IN_ARRAY(trimsSwitchesAir, index - 1) :
                                        CHECK_IN_ARRAY(trimsSwitchesSurface, index - 1)));

      case SWITCH_TYPE_ROTARY_ENCODER:
        return CHECK_IN_ARRAY(rotaryEncoders, index - 1);

      case SWITCH_TYPE_ON:
        return tr("ON");

      case SWITCH_TYPE_OFF:
        return tr("OFF");

      case SWITCH_TYPE_ONE:
        return tr("One");

      case SWITCH_TYPE_ACT:
        return tr("Act");

      case SWITCH_TYPE_FLIGHT_MODE:
        if (modelData)
          return modelData->flightModeData[index - 1].nameToString(index - 1);
        else
          return FlightModeData().nameToString(index - 1);

      case SWITCH_TYPE_NONE:
        return tr("----");

      case SWITCH_TYPE_TIMER_MODE:
        return CHECK_IN_ARRAY(timerModes, index);

      case SWITCH_TYPE_SENSOR:
        if (modelData)
          return modelData->sensorData[index - 1].nameToString(index - 1);
        else
          return SensorData().nameToString(index - 1);

      case SWITCH_TYPE_TELEMETRY:
        return tr("Telemetry");

      case SWITCH_TYPE_TRAINER:
        return tr("Trn");

      default:
        return CPN_STR_UNKNOWN_ITEM;
     }
  }
}

bool RawSwitch::isAvailable(const ModelData * const model, const GeneralSettings * const gs, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);
  div_t sw;

  if (type == SWITCH_TYPE_SWITCH && abs(index) > b.getCapability(Board::SwitchesPositions))
    return false;

  if (type == SWITCH_TYPE_TRIM && abs(index) > b.getCapability(Board::NumTrimSwitches))
    return false;

  if (type == SWITCH_TYPE_SWITCH)
    sw = div(abs(index) - 1, 3);

  if (gs) {
    if (type == SWITCH_TYPE_SWITCH && !b.isSwitchFunc(sw.quot, board) && !gs->switchPositionAllowed(abs(index)))
      return false;

    if (type == SWITCH_TYPE_MULTIPOS_POT) {
      int idx = div(abs(index) - 1, b.getCapability(Board::MultiposPotsPositions)).quot;
      if (!gs->isInputAvailable(idx) || gs->inputConfig[idx].flexType != Board::FLEX_MULTIPOS)
        return false;
    }
  }

  if (model) {
    if (type == SWITCH_TYPE_SWITCH && b.isSwitchFunc(sw.quot, board)) {
      int fsindex = (((Boards::getSwitchTagNum(sw.quot, board) - 1) * 3) + sw.rem + 1) * ( index < 0 ? -1 : 1);
      return model->isFunctionSwitchPositionAvailable(fsindex);
    }
    else
      return model->isAvailable(*this);
  }

  return true;
}

RawSwitch RawSwitch::convert(RadioDataConversionState & cstate)
{
  if (!index)
    return *this;

  cstate.setItemType(tr("Switch"), 2);
  RadioDataConversionState::LogField oldData(index, toString(cstate.fromType, cstate.fromGS(), cstate.fromModel()));

  int newIdx = 0;

  if (type == SWITCH_TYPE_SWITCH) {
    div_t swtch = div(abs(index) - 1, 3);
    newIdx = Boards::getSwitchIndex(Boards::getSwitchTag(swtch.quot, cstate.fromType), Board::LVT_TAG, cstate.toType);

    if (newIdx >= 0)
      index = (newIdx * 3 + 1 + swtch.rem) * (index < 0 ? -1 : 1);
  }

  // final validation (we do not pass model to isAvailable() because we don't know what has or hasn't been converted)
  if (newIdx < 0 || !isAvailable(nullptr, cstate.toGS(), cstate.toType)) {
    cstate.setInvalid(oldData);
    clear();  // no switch is safer than an invalid one
    cstate.setConverted(oldData, RadioDataConversionState::LogField(index, tr("None")));
  }

  return *this;
}

// static
StringTagMappingTable RawSwitch::getRawSwitchTypesLookupTable()
{
  StringTagMappingTable tbl;

  tbl.insert(tbl.end(), {
                          {std::to_string(SWITCH_TYPE_NONE),            "NONE"},
                          {std::to_string(SWITCH_TYPE_SWITCH),          "Sxn"},
                          {std::to_string(SWITCH_TYPE_FUNCTIONSWITCH),  "FSn"},
                          {std::to_string(SWITCH_TYPE_VIRTUAL),         "Ln"},
                          {std::to_string(SWITCH_TYPE_MULTIPOS_POT),    "6Pn"},
                          {std::to_string(SWITCH_TYPE_TRIM),            "Trim"},
                          {std::to_string(SWITCH_TYPE_ROTARY_ENCODER),  "RE"},
                          {std::to_string(SWITCH_TYPE_ON),              "ON"},
                          {std::to_string(SWITCH_TYPE_OFF),             "OFF"},
                          {std::to_string(SWITCH_TYPE_ONE),             "ONE"},
                          {std::to_string(SWITCH_TYPE_FLIGHT_MODE),     "FMn"},
                          {std::to_string(SWITCH_TYPE_TIMER_MODE),      "Timer"},
                          {std::to_string(SWITCH_TYPE_TELEMETRY),       "TELEMETRY_STREAMING"},
                          {std::to_string(SWITCH_TYPE_SENSOR),          "Tnn"},
                          {std::to_string(SWITCH_TYPE_ACT),             "RADIO_ACTIVITY"},
                          {std::to_string(SWITCH_TYPE_TRAINER),         "TRAINER_CONNECTED"},
                          });

  return tbl;
}
