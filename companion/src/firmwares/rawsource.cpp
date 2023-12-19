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

#include "rawsource.h"

#include "eeprominterface.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"

#include <float.h>

/*
 * RawSourceRange
 */

float RawSourceRange::getValue(int value)
{
  return float(value) * step;
}


/*
 * RawSource
 */

RawSourceRange RawSource::getRange(const ModelData * model, const GeneralSettings & settings, unsigned int flags) const
{
  RawSourceRange result;

  Firmware * firmware = Firmware::getCurrentVariant();

  switch (type) {
    case SOURCE_TYPE_TELEMETRY:
      {
        div_t qr = div(index, 3);
        const SensorData & sensor = model->sensorData[qr.quot];
        if (sensor.prec == 2)
          result.step = 0.01;
        else if (sensor.prec == 1)
          result.step = 0.1;
        else
          result.step = 1;
        result.min = -30000 * result.step;
        result.max = +30000 * result.step;
        result.decimals = sensor.prec;
        result.unit = sensor.unitToString();
        break;
      }

    case SOURCE_TYPE_LUA_OUTPUT:
      result.max = 30000;
      result.min = -result.max;
      break;

    case SOURCE_TYPE_TRIM:
      result.max = (model && model->extendedTrims ? firmware->getCapability(ExtendedTrimsRange) : firmware->getCapability(TrimsRange));
      result.min = -result.max;
      break;

    case SOURCE_TYPE_GVAR: {
      GVarData gv = model->gvarData[index];
      result.step = gv.multiplierGet();
      result.decimals = gv.prec;
      result.max = gv.getMaxPrec();
      result.min = gv.getMinPrec();
      result.unit = gv.unitToString();
      break;
    }

    case SOURCE_TYPE_SPECIAL:
      if (index == 0)  {  //Batt
        result.step = 0.1;
        result.decimals = 1;
        result.max = 25.5;
        result.unit = tr("V");
      }
      else if (index == 1) {   //Time
        result.step = 60;
        result.max = 24 * 60 * result.step - 60;  // 23:59:00 with 1-minute resolution
        result.unit = tr("s");
      }
      else if (index == 2) {   //GPS
        result.max = 30000;
        result.min = -result.max;
      }
      else {      // Timers 1 - 3
        result.step = 1;
        result.max = 9 * 60 * 60 - 1;  // 8:59:59 (to match firmware)
        result.min = -result.max;
        result.unit = tr("s");
      }
      break;

    case SOURCE_TYPE_CH:
      result.max = model->getChannelsMax(false);
      result.min = -result.max;
      break;

    default:
      result.max = 100;
      result.min = -result.max;
      break;
  }

  if (flags & RANGE_ABS_FUNCTION) {
    result.min = 0;
  }

  return result;
}

QString RawSource::toString(const ModelData * model, const GeneralSettings * const generalSettings, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN) {
    board = getCurrentBoard();
  }

  static const QString trims[] = {
    tr("TrmR"), tr("TrmE"), tr("TrmT"), tr("TrmA"), tr("Trm5"), tr("Trm6"), tr("Trm7"), tr("Trm8")
  };

  static const QString trims2[] = {
    tr("TrmH"), tr("TrmV")
  };

  static const QString special[] = {
    tr("Batt"), tr("Time"), tr("GPS"), tr("Reserved1"), tr("Reserved2"), tr("Reserved3"), tr("Reserved4")
  };

  static const QString rotary[]  = { tr("REa"), tr("REb") };

  if (index < 0) {
    return QString(CPN_STR_UNKNOWN_ITEM);
  }

  QString result;

  switch (type) {
    case SOURCE_TYPE_NONE:
      return QString(CPN_STR_NONE_ITEM);

    case SOURCE_TYPE_VIRTUAL_INPUT:
    {
      const char * name = nullptr;
      if (model)
        name = model->inputNames[index];
      return RadioData::getElementName(tr("I", "as in Input"), index + 1, name);
    }

    case SOURCE_TYPE_LUA_OUTPUT:
      return tr("LUA%1%2").arg(index / 16 + 1).arg(QChar('a' + index % 16));

    case SOURCE_TYPE_STICK:
      if (generalSettings)
        result = QString(generalSettings->inputConfig[index].name).trimmed();
      if (result.isEmpty())
        result = Boards::getInputName(board, index);
      return result;

    case SOURCE_TYPE_TRIM:
      return (Boards::getCapability(board, Board::NumTrims) == 2 ? CHECK_IN_ARRAY(trims2, index) : CHECK_IN_ARRAY(trims, index));

    case SOURCE_TYPE_ROTARY_ENCODER:
      return CHECK_IN_ARRAY(rotary, index);

    case SOURCE_TYPE_MIN:
      return tr("MIN");

    case SOURCE_TYPE_MAX:
      return tr("MAX");

    case SOURCE_TYPE_SWITCH:
      if (generalSettings)
        result = QString(generalSettings->switchConfig[index].name).trimmed();
      if (result.isEmpty())
        result = Boards::getSwitchInfo(board, index).name.c_str();
      return result;

    case SOURCE_TYPE_FUNCTIONSWITCH:
      if (model)
        result = QString(model->functionSwitchNames[index]).trimmed();
      if (result.isEmpty())
        result = tr("SW%1").arg(index + 1);
      return result;

    case SOURCE_TYPE_CUSTOM_SWITCH:
      return RawSwitch(SWITCH_TYPE_VIRTUAL, index + 1).toString();

    case SOURCE_TYPE_CYC:
      return tr("CYC%1").arg(index + 1);

    case SOURCE_TYPE_PPM:
      return RadioData::getElementName(tr("TR", "as in Trainer"), index + 1);

    case SOURCE_TYPE_CH:
      if (model)
        result = QString(model->limitData[index].nameToString(index)).trimmed();
      if (result.isEmpty())
        return LimitData().nameToString(index);
      return result;

    case SOURCE_TYPE_SPECIAL:
      if (index >= SOURCE_TYPE_SPECIAL_FIRST_TIMER && index <= SOURCE_TYPE_SPECIAL_LAST_TIMER) {
        if (model)
          result = QString(model->timers[index - SOURCE_TYPE_SPECIAL_FIRST_TIMER].nameToString(index - SOURCE_TYPE_SPECIAL_FIRST_TIMER)).trimmed();
        if (result.isEmpty())
          result = TimerData().nameToString(index - SOURCE_TYPE_SPECIAL_FIRST_TIMER);
      }
      else
        result = CHECK_IN_ARRAY(special, index);

      return result;

    case SOURCE_TYPE_TELEMETRY:
      {
        div_t qr = div(index, 3);
        if (model)
          result = QString(model->sensorData[qr.quot].nameToString(qr.quot)).trimmed();
        if (result.isEmpty())
          result = SensorData().nameToString(qr.quot);
        if (qr.rem)
          result += (qr.rem == 1 ? "-" : "+");
        return result;
      }

    case SOURCE_TYPE_GVAR:
      if (model)
        result = QString(model->gvarData[index].nameToString(index)).trimmed();
      if (result.isEmpty())
        result = GVarData().nameToString(index);
      return result;

    case SOURCE_TYPE_SPACEMOUSE:
      return tr("sm%1").arg(QChar('A' + index));

    default:
      return QString(CPN_STR_UNKNOWN_ITEM);
  }
}

bool RawSource::isStick(Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  if (type == SOURCE_TYPE_STICK && index < Boards::getCapability(board, Board::Sticks)) {
    return true;
  }
  return false;
}

bool RawSource::isTimeBased(Board::Type board) const
{
  return (type == SOURCE_TYPE_SPECIAL && index >= SOURCE_TYPE_SPECIAL_FIRST_TIMER && index <= SOURCE_TYPE_SPECIAL_LAST_TIMER);
}

bool RawSource::isAvailable(const ModelData * const model, const GeneralSettings * const gs, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);

  if (type == SOURCE_TYPE_STICK && index >= b.getCapability(Board::Inputs))
    return false;

  if (type == SOURCE_TYPE_SWITCH && index >= b.getCapability(Board::Switches))
    return false;

  if (type == SOURCE_TYPE_FUNCTIONSWITCH)
    if (!model || index >= b.getCapability(Board::FunctionSwitches))
      return false;

  if (type == SOURCE_TYPE_SPECIAL && index >= SOURCE_TYPE_SPECIAL_FIRST_RESERVED && index <= SOURCE_TYPE_SPECIAL_LAST_RESERVED)
      return false;

  if (model) {
    if (type == SOURCE_TYPE_SPECIAL && index >= SOURCE_TYPE_SPECIAL_FIRST_TIMER && index <= SOURCE_TYPE_SPECIAL_LAST_TIMER &&
        model->timers[index - SOURCE_TYPE_SPECIAL_FIRST_TIMER].isModeOff())
      return false;

    if (type == SOURCE_TYPE_FUNCTIONSWITCH && !model->isFunctionSwitchSourceAllowed(index))
      return false;

    if (type == SOURCE_TYPE_VIRTUAL_INPUT && !model->isInputValid(index))
      return false;

    if (type == SOURCE_TYPE_PPM && model->trainerMode == TRAINER_MODE_OFF)
      return false;

    if (type == SOURCE_TYPE_CUSTOM_SWITCH && model->logicalSw[index].isEmpty())
      return false;

    if (type == SOURCE_TYPE_TELEMETRY) {
      if (!model->sensorData[div(index, 3).quot].isAvailable()) {
        return false;
      }
    }

    if (type == SOURCE_TYPE_CH && !model->hasMixes(index))
      return false;
  }

  if (gs) {
    if (type == SOURCE_TYPE_STICK && !gs->isInputAvailable(index))
      return false;

    if (type == SOURCE_TYPE_SWITCH && IS_HORUS_OR_TARANIS(board) && !gs->switchSourceAllowedTaranis(index))
      return false;
  }

  if (type == SOURCE_TYPE_TRIM && index >= b.getCapability(Board::NumTrims))
    return false;

  if (type == SOURCE_TYPE_SPACEMOUSE &&
     (index >= CPN_MAX_SPACEMOUSE ||
     (!(gs->serialPort[GeneralSettings::SP_AUX1] == GeneralSettings::AUX_SERIAL_SPACEMOUSE ||
        gs->serialPort[GeneralSettings::SP_AUX2] == GeneralSettings::AUX_SERIAL_SPACEMOUSE))))
    return false;

  return true;
}

RawSource RawSource::convert(RadioDataConversionState & cstate)
{
  cstate.setItemType(tr("SRC"), 1);
  RadioDataConversionState::EventType evt = RadioDataConversionState::EVT_NONE;
  RadioDataConversionState::LogField oldData(index, toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType));

  if (type == SOURCE_TYPE_STICK) {
    QStringList fromStickList(getStickList(cstate.fromType));
    QStringList toStickList(getStickList(cstate.toType));
    if (oldData.id < fromStickList.count())
      index = toStickList.indexOf(fromStickList.at(oldData.id));
    else
      index = -1;
    // perform forced mapping
  }

  if (type == SOURCE_TYPE_SWITCH) {
    QStringList fromSwitchList(getSwitchList(cstate.fromType));
    QStringList toSwitchList(getSwitchList(cstate.toType));
    // index set to -1 if no match found
    if (oldData.id < fromSwitchList.count())
      index = toSwitchList.indexOf(fromSwitchList.at(oldData.id));
    else
      index = -1;
    // perform forced mapping
    if (index < 0) {
      if (IS_TARANIS_X7(cstate.toType) && (IS_TARANIS_X9(cstate.fromType) || IS_FAMILY_HORUS_OR_T16(cstate.fromType))) {
        // No SE and SG on X7 board
        index = toSwitchList.indexOf("SD");
        if (index >= 0)
          evt = RadioDataConversionState::EVT_CVRT;
      }
      else if (IS_FAMILY_T12(cstate.toType) && (IS_TARANIS_X9(cstate.fromType) || IS_FAMILY_HORUS_OR_T16(cstate.fromType))) {
        // No SE and SG on T12 board
        index = toSwitchList.indexOf("SD");
        if (index >= 0)
          evt = RadioDataConversionState::EVT_CVRT;
      }
    }
  }

  // final validation (we do not pass model to isAvailable() because we don't know what has or hasn't been converted)
  if (index < 0 || !isAvailable(NULL, cstate.toGS(), cstate.toType)) {
    cstate.setInvalid(oldData);
    // no source is safer than an invalid one
    clear();
  }
  else if (evt == RadioDataConversionState::EVT_CVRT) {
    cstate.setConverted(oldData, RadioDataConversionState::LogField(index, toString(cstate.toModel(), cstate.toGS(), cstate.toType)));
  }
  else if (oldData.id != index) {
    // provide info by default if anything changed
    cstate.setMoved(oldData, RadioDataConversionState::LogField(index, toString(cstate.toModel(), cstate.toGS(), cstate.toType)));
  }

  return *this;
}

QStringList RawSource::getStickList(Board::Type board) const
{
  QStringList ret;

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    ret.append(Boards::getInputInfo(board, i).name.c_str());
  }
  return ret;
}

QStringList RawSource::getSwitchList(Board::Type board) const
{
  QStringList ret;

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    ret.append(Boards::getSwitchInfo(board, i).name.c_str());
  }
  return ret;
}

// static
StringTagMappingTable RawSource::getSpecialTypesLookupTable()
{
  StringTagMappingTable tbl;

tbl.insert(tbl.end(), {
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_BATT),    "TX_VOLTAGE"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_TIME),    "TX_TIME"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_GPS),     "TX_GPS"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED1),  "RESERVED1"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED2),  "RESERVED2"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED3),  "RESERVED3"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED4),  "RESERVED4"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TIMER1),     "Tmr1"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TIMER2),     "Tmr2"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TIMER3),     "Tmr3"},
                          });

  return tbl;
}

// static
StringTagMappingTable RawSource::getCyclicLookupTable()
{
  StringTagMappingTable tbl;

tbl.insert(tbl.end(), {
                          {"0", "CYC1"},
                          {"1", "CYC2"},
                          {"2", "CYC3"},
                          });

  return tbl;
}
