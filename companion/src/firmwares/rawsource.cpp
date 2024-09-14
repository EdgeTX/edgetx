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

#include "rawsource.h"

#include "eeprominterface.h"
#include "radiodata.h"
#include "modeldata.h"
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
      if (index == 0)  {       //Batt
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
      break;

    case SOURCE_TYPE_TIMER:
      result.step = 1;
      result.max = 9 * 60 * 60 - 1;  // 8:59:59 (to match firmware)
      result.min = -result.max;
      result.unit = tr("s");
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

QString RawSource::toString(const ModelData * model, const GeneralSettings * const generalSettings, Board::Type board, bool prefixCustomName) const
{
  if (index < 0)
    return CPN_STR_SRC_INDICATOR_NEG % RawSource(type, -index).toString(model, generalSettings, board, prefixCustomName);

  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  static const QString trimsAir[] = {
    "", tr("Trim Rud"), tr("Trim Ele"), tr("Trim Thr"), tr("Trim Ail"), tr("Trim 5"), tr("Trim 6"), tr("Trim 7"), tr("Trim 8")
  };

  static const QString trimsSurface[] = {
    "", tr("Trim ST"), tr("Trim TH"), tr("Trim 3"), tr("Trim 4"), tr("Trim 5"), tr("Trim 6"), tr("Trim 7"), tr("Trim 8")
  };

  static const QString trims2[] = {
    "", tr("TrmH"), tr("TrmV")
  };

  static const QString special[] = {
    "", tr("Batt"), tr("Time"), tr("GPS"), tr("Reserved1"), tr("Reserved2"), tr("Reserved3"), tr("Reserved4")
  };

  static const QString rotary[]  = { "", tr("REa"), tr("REb") };

  QString result;
  QString dfltName;
  QString custName;

  switch (type) {
    case SOURCE_TYPE_NONE:
      return QString(CPN_STR_NONE_ITEM);

    case SOURCE_TYPE_VIRTUAL_INPUT:
    {
      const char * name = nullptr;
      if (model && index <= CPN_MAX_INPUTS)
        name = model->inputNames[index - 1];
      return RadioData::getElementName(tr("I", "as in Input"), index, name);
    }

    case SOURCE_TYPE_LUA_OUTPUT:
      {
        div_t qr = div(index - 1, 16);
        return tr("LUA%1%2").arg(qr.quot + 1).arg(QChar('a' + qr.rem));
      }

    case SOURCE_TYPE_INPUT:
      dfltName = Boards::getInputName(index - 1, board);
      if (generalSettings && index <= CPN_MAX_INPUTS)
        custName = QString(generalSettings->inputConfig[index - 1].name).trimmed();
      return DataHelpers::getCompositeName(dfltName, custName, prefixCustomName);

    case SOURCE_TYPE_TRIM:
      return (Boards::getCapability(board, Board::NumTrims) == 2 ? CHECK_IN_ARRAY(trims2, index) :
              (Boards::isAir(board) ? CHECK_IN_ARRAY(trimsAir, index) :
               CHECK_IN_ARRAY(trimsSurface, index)));

    case SOURCE_TYPE_ROTARY_ENCODER:
      return CHECK_IN_ARRAY(rotary, index);

    case SOURCE_TYPE_MIN:
      return tr("MIN");

    case SOURCE_TYPE_MAX:
      return tr("MAX");

    case SOURCE_TYPE_SWITCH:
      dfltName = Boards::getSwitchInfo(index - 1, board).name.c_str();
      if (Boards::isSwitchFunc(index - 1, board)) {
        if (model) {
          int fsindex = Boards::getSwitchTagNum(index - 1, board) - 1;
          if (fsindex >= 0 && fsindex < CPN_MAX_SWITCHES_FUNCTION)
            custName = QString(model->functionSwitchNames[fsindex]).trimmed();
        }
      }
      else {
        if (generalSettings && index <= CPN_MAX_SWITCHES) {
          custName = QString(generalSettings->switchConfig[index - 1].name).trimmed();
        }
      }

      return DataHelpers::getCompositeName(dfltName, custName, prefixCustomName);

    case SOURCE_TYPE_CUSTOM_SWITCH:
      return RawSwitch(SWITCH_TYPE_VIRTUAL, index).toString();  // RawSwitch uses 1 based index

    case SOURCE_TYPE_CYC:
      return tr("CYC%1").arg(index);

    case SOURCE_TYPE_PPM:
      return RadioData::getElementName(tr("TR", "as in Trainer"), index);

    case SOURCE_TYPE_CH:
      if (model && index <= CPN_MAX_CHNOUT)
        result = QString(model->limitData[index - 1].nameToString(index - 1)).trimmed();
      if (result.isEmpty())
        return LimitData().nameToString(index - 1);

      return result;

    case SOURCE_TYPE_SPECIAL:
      return CHECK_IN_ARRAY(special, index);

    case SOURCE_TYPE_TIMER:
      if (model && index <= CPN_MAX_TIMERS)
        result = QString(model->timers[index - 1].nameToString(index - 1)).trimmed();
      if (result.isEmpty())
        result = TimerData().nameToString(index - 1);

      return result;

    case SOURCE_TYPE_TELEMETRY:
      {
        div_t qr = div(index - 1, 3);
        if (model && qr.quot < CPN_MAX_SENSORS)
          result = QString(model->sensorData[qr.quot].nameToString(qr.quot)).trimmed();
        if (result.isEmpty())
          result = SensorData().nameToString(qr.quot);
        if (qr.rem)
          result += (qr.rem == 1 ? "-" : "+");
        return result;
      }

    case SOURCE_TYPE_GVAR:
      if (model && index <= CPN_MAX_GVARS)
        result = QString(model->gvarData[index - 1].nameToString(index - 1)).trimmed();
      if (result.isEmpty())
        result = GVarData().nameToString(index - 1);
      return result;

    case SOURCE_TYPE_SPACEMOUSE:
      return tr("sm%1").arg(QChar('A' + (index - 1)));

    case SOURCE_TYPE_FUNCTIONSWITCH_GROUP:
      return tr("GR%1").arg(index);

    default:
      return QString(CPN_STR_UNKNOWN_ITEM);
  }
}

bool RawSource::isStick(Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  if (type == SOURCE_TYPE_INPUT && index - 1 < Boards::getCapability(board, Board::Sticks)) {
    return true;
  }
  return false;
}

bool RawSource::isAvailable(const ModelData * const model, const GeneralSettings * const gs, Board::Type board) const
{
  if (type == SOURCE_TYPE_NONE && index == 0)
    return true;

  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);
  Firmware *firmware = getCurrentFirmware();

  if (type == SOURCE_TYPE_CH && abs(index) > CPN_MAX_CHNOUT)
    return false;

  if (type == SOURCE_TYPE_FUNCTIONSWITCH_GROUP && index >= b.getCapability(Board::FunctionSwitches))
    return false;

  if (type == SOURCE_TYPE_CUSTOM_SWITCH && abs(index) > CPN_MAX_LOGICAL_SWITCHES)
    return false;

  if (type == SOURCE_TYPE_LUA_OUTPUT && div(abs(index - 1), 16).quot >= CPN_MAX_SCRIPTS)
    return false;

  if (type == SOURCE_TYPE_INPUT && abs(index) > b.getCapability(Board::Inputs))
    return false;

  if (type == SOURCE_TYPE_SWITCH && abs(index) > b.getCapability(Board::Switches))
    return false;

  if (type == SOURCE_TYPE_SPECIAL && abs(index) >= SOURCE_TYPE_SPECIAL_FIRST_RESERVED)
    return false;

  if (type == SOURCE_TYPE_TIMER && abs(index) > CPN_MAX_TIMERS)
    return false;

  if (type == SOURCE_TYPE_TELEMETRY && div(abs(index), 3).quot > CPN_MAX_SENSORS)
    return false;

  if (type == SOURCE_TYPE_CYC && !firmware->getCapability(Heli))
    return false;

  if (model) {
    if (type == SOURCE_TYPE_TIMER && model->timers[abs(index) - 1].isModeOff())
      return false;

    if (type == SOURCE_TYPE_SWITCH && b.isSwitchFunc(abs(index) - 1, board) &&
        !model->isFunctionSwitchSourceAllowed(b.getSwitchTagNum(abs(index) - 1, board) - 1))
      return false;

    if (type == SOURCE_TYPE_VIRTUAL_INPUT && !model->isInputValid(abs(index) - 1))
      return false;

    if (type == SOURCE_TYPE_PPM && model->trainerMode == TRAINER_MODE_OFF)
      return false;

    if (type == SOURCE_TYPE_CUSTOM_SWITCH && model->logicalSw[abs(index) - 1].isEmpty())
      return false;

    if (type == SOURCE_TYPE_TELEMETRY) {
      if (!model->sensorData[div(abs(index) - 1, 3).quot].isAvailable()) {
        return false;
      }
    }

    if (type == SOURCE_TYPE_CH && !model->hasMixes(abs(index) - 1))
      return false;

    if (type == SOURCE_TYPE_FUNCTIONSWITCH_GROUP) {
      if (!b.getCapability(Board::FunctionSwitches))
        return false;
      else if (model->getFuncGroupSwitchCount(abs(index), CPN_MAX_SWITCHES_FUNCTION) == 0)
        return false;
    }
  }
  else {
    if (type == SOURCE_TYPE_FUNCTIONSWITCH_GROUP && b.getCapability(Board::FunctionSwitches))
      return false;
  }

  if (gs) {
    if (type == SOURCE_TYPE_INPUT) {
      if (!gs->isInputAvailable(abs(index) - 1))
        return false;
      if (gs->inputConfig[abs(index) - 1].flexType == Board::FLEX_SWITCH)
        return false;
    }

    if (type == SOURCE_TYPE_SWITCH && !b.isSwitchFunc(abs(index) - 1, board) && IS_HORUS_OR_TARANIS(board) &&
        !gs->switchSourceAllowed(abs(index) - 1))
      return false;
  }
  else {
    if (type == SOURCE_TYPE_INPUT) {
      if (!Boards::isInputAvailable(abs(index) - 1, board))
        return false;
      if (Boards::getInputInfo(abs(index) - 1, board).flexType == Board::FLEX_SWITCH)
        return false;
    }
  }

  if (type == SOURCE_TYPE_TRIM && abs(index) > b.getCapability(Board::NumTrims))
    return false;

  if (type == SOURCE_TYPE_SPACEMOUSE &&
     (abs(index) > CPN_MAX_SPACEMOUSE ||
     (!(gs->serialPort[GeneralSettings::SP_AUX1] == GeneralSettings::AUX_SERIAL_SPACEMOUSE ||
        gs->serialPort[GeneralSettings::SP_AUX2] == GeneralSettings::AUX_SERIAL_SPACEMOUSE))))
    return false;

  return true;
}

RawSource RawSource::convert(RadioDataConversionState & cstate)
{
  cstate.setItemType(tr("Source"), 1);
  RadioDataConversionState::LogField oldData(index, toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType));

  if (type == SOURCE_TYPE_INPUT)
    index = Boards::getInputIndex(Boards::getInputTag(oldData.id, cstate.fromType), Board::LVT_TAG, cstate.toType);
  else if (type == SOURCE_TYPE_SWITCH)
    index = Boards::getSwitchIndex(Boards::getSwitchTag(oldData.id, cstate.fromType), Board::LVT_TAG, cstate.toType);

  // final validation (we do not pass model to isAvailable() because we don't know what has or hasn't been converted)
  if (index < 0 || !isAvailable(nullptr, cstate.toGS(), cstate.toType)) {
    cstate.setInvalid(oldData);
    clear();  // no source is safer than an invalid one
    cstate.setConverted(oldData, RadioDataConversionState::LogField(index, tr("None")));
  }

  return *this;
}

// static
StringTagMappingTable RawSource::getSpecialTypesLookupTable()
{
  StringTagMappingTable tbl;

tbl.insert(tbl.end(), {
                          {std::to_string(SOURCE_TYPE_SPECIAL_NONE),       "NONE"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_BATT),    "TX_VOLTAGE"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_TIME),    "TX_TIME"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_TX_GPS),     "TX_GPS"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED1),  "RESERVED1"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED2),  "RESERVED2"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED3),  "RESERVED3"},
                          {std::to_string(SOURCE_TYPE_SPECIAL_RESERVED4),  "RESERVED4"},
                          });

  return tbl;
}
