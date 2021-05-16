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

#include "rawswitch.h"

#include "eeprominterface.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"

QString RawSwitch::toString(Board::Type board, const GeneralSettings * const generalSettings, const ModelData * const modelData) const
{
  if (board == Board::BOARD_UNKNOWN) {
    board = getCurrentBoard();
  }

  static const QString switches9X[] = {
    QString("THR"), QString("RUD"), QString("ELE"),
    QString("ID0"), QString("ID1"), QString("ID2"),
    QString("AIL"), QString("GEA"), QString("TRN")
  };

  static const QString trimsSwitches[] = {
    tr("RudTrim Left"), tr("RudTrim Right"),
    tr("EleTrim Down"), tr("EleTrim Up"),
    tr("ThrTrim Down"), tr("ThrTrim Up"),
    tr("AilTrim Left"), tr("AilTrim Right"),
    tr("Trim 5 Down"), tr("Trim 5 Up"),
    tr("Trim 6 Down"), tr("Trim 6 Up")
  };

  static const QString trimsSwitches2[] = {
    tr("TrmH Left"), tr("TrmH Right"),
    tr("TrmV Down"), tr("TrmV Up")
  };

  static const QString rotaryEncoders[] = {
    tr("REa"), tr("REb")
  };

  static const QString timerModes[] = {
    tr("OFF"), tr("ON"),
    tr("THs"), tr("TH%"), tr("THt")
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
    div_t qr;
    switch(type) {
      case SWITCH_TYPE_SWITCH:
        if (IS_HORUS_OR_TARANIS(board)) {
          qr = div(index-1, 3);
          if (generalSettings)
            swName = QString(generalSettings->switchName[qr.quot]);
          if (swName.isEmpty())
            swName = Boards::getSwitchInfo(board, qr.quot).name;
          return swName + directionIndicators.at(qr.rem > -1 && qr.rem < directionIndicators.size() ? qr.rem : 1);
        }
        else {
          return CHECK_IN_ARRAY(switches9X, index - 1);
        }

      case SWITCH_TYPE_VIRTUAL:
        if (modelData)
          return modelData->logicalSw[index].nameToString(index-1);
        else
          return LogicalSwitchData().nameToString(index-1);

      case SWITCH_TYPE_MULTIPOS_POT:
        if (!Boards::getCapability(board, Board::MultiposPotsPositions))
          return tr("???");
        qr = div(index - 1, Boards::getCapability(board, Board::MultiposPotsPositions));
        if (generalSettings && qr.quot < (int)DIM(generalSettings->potConfig))
          swName = QString(generalSettings->potName[qr.quot]);
        if (swName.isEmpty())
          swName = Boards::getAnalogInputName(board, qr.quot + Boards::getCapability(board, Board::Sticks));
        return swName + "_" + QString::number(qr.rem + 1);

      case SWITCH_TYPE_TRIM:
        return (Boards::getCapability(board, Board::NumTrims) == 2 ? CHECK_IN_ARRAY(trimsSwitches2, index-1) : CHECK_IN_ARRAY(trimsSwitches, index-1));

      case SWITCH_TYPE_ROTARY_ENCODER:
        return CHECK_IN_ARRAY(rotaryEncoders, index-1);

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
          return modelData->flightModeData[index-1].nameToString(index - 1);
        else
          return FlightModeData().nameToString(index - 1);

      case SWITCH_TYPE_NONE:
        return tr("----");

      case SWITCH_TYPE_TIMER_MODE:
        return CHECK_IN_ARRAY(timerModes, index);

      case SWITCH_TYPE_SENSOR:
        if (modelData)
          return modelData->sensorData[index-1].nameToString(index-1);
        else
          return SensorData().nameToString(index-1);

      case SWITCH_TYPE_TELEMETRY:
        return tr("Telemetry");

      default:
        return tr("???");
    }
  }
}

bool RawSwitch::isAvailable(const ModelData * const model, const GeneralSettings * const gs, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);

  if (type == SWITCH_TYPE_SWITCH && abs(index) > b.getCapability(Board::SwitchPositions))
    return false;

  if (type == SWITCH_TYPE_TRIM && abs(index) > b.getCapability(Board::NumTrimSwitches))
    return false;

  if (gs) {
    if (type == SWITCH_TYPE_SWITCH && IS_HORUS_OR_TARANIS(board) && !gs->switchPositionAllowedTaranis(abs(index)))
      return false;

    if (type == SWITCH_TYPE_MULTIPOS_POT) {
      int pot = div(abs(index) - 1, b.getCapability(Board::MultiposPotsPositions)).quot;
      if (!gs->isPotAvailable(pot) || gs->potConfig[pot] != Board::POT_MULTIPOS_SWITCH)
        return false;
    }
  }

  if (model && !model->isAvailable(*this))
    return false;

  return true;
}

RawSwitch RawSwitch::convert(RadioDataConversionState & cstate)
{
  if (!index)
    return *this;

  cstate.setItemType(tr("SW"), 2);
  RadioDataConversionState::EventType evt = RadioDataConversionState::EVT_NONE;
  RadioDataConversionState::LogField oldData(index, toString(cstate.fromType, cstate.fromGS(), cstate.fromModel()));

  int newIdx = 0;

  if (type == SWITCH_TYPE_SWITCH) {
    div_t swtch = div(abs(index) - 1, 3);  // rawsource index
    QStringList fromSwitchList(getSwitchList(cstate.fromBoard));
    QStringList toSwitchList(getSwitchList(cstate.toBoard));
    // set to -1 if no match found
    if (swtch.quot < fromSwitchList.count())
      newIdx = toSwitchList.indexOf(fromSwitchList.at(swtch.quot));
    else
      newIdx = -1;
    // perform forced mapping
    if (newIdx < 0) {
      if (IS_TARANIS_X7(cstate.toType) && (IS_TARANIS_X9(cstate.fromType) || IS_FAMILY_HORUS_OR_T16(cstate.fromType))) {
        // No SE and SG on X7 board
        newIdx = toSwitchList.indexOf("SD");
        if (newIdx >= 0)
          evt = RadioDataConversionState::EVT_CVRT;
      }
      else if (IS_FAMILY_T12(cstate.toType) && (IS_TARANIS_X9(cstate.fromType) || IS_FAMILY_HORUS_OR_T16(cstate.fromType))) {
        // No SE and SG on T12 board
        newIdx = toSwitchList.indexOf("SD");
        if (newIdx >= 0)
          evt = RadioDataConversionState::EVT_CVRT;
      }
    }

    if (newIdx >= 0)
      index = (newIdx * 3 + 1 + swtch.rem) * (index < 0 ? -1 : 1);
  }  // SWITCH_TYPE_SWITCH

  // final validation (we do not pass model to isAvailable() because we don't know what has or hasn't been converted)
  if (newIdx < 0 || !isAvailable(NULL, cstate.toGS(), cstate.toType)) {
    cstate.setInvalid(oldData);
    // no switch is safer than an invalid one
    clear();
  }
  else if (evt == RadioDataConversionState::EVT_CVRT) {
    cstate.setConverted(oldData, RadioDataConversionState::LogField(index, toString(cstate.toType, cstate.toGS(), cstate.toModel())));
  }
  else if (oldData.id != index) {
    // provide info by default if anything changed
    cstate.setMoved(oldData, RadioDataConversionState::LogField(index, toString(cstate.toType, cstate.toGS(), cstate.toModel())));
  }

  return *this;
}

QStringList RawSwitch::getSwitchList(Boards board) const
{
  QStringList ret;

  for (int i = 0; i < board.getCapability(Board::Switches); i++) {
    ret.append(board.getSwitchInfo(i).name);
  }
  return ret;
}
