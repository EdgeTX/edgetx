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
    tr("TrmR"), tr("TrmE"), tr("TrmT"), tr("TrmA"), tr("Trm5"), tr("Trm6")
  };

  static const QString trims2[] = {
    tr("TrmH"), tr("TrmV")
  };

  static const QString special[] = {
    tr("Batt"), tr("Time"), tr("Timer1"), tr("Timer2"), tr("Timer3"),
  };

  static const QString rotary[]  = { tr("REa"), tr("REb") };

  if (index<0) {
    return QString(CPN_STR_UNKNOWN_ITEM);
  }

  QString result;
  int genAryIdx = 0;
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
      if (generalSettings) {
        if (isPot(&genAryIdx))
          result = QString(generalSettings->potName[genAryIdx]);
        else if (isSlider(&genAryIdx))
          result = QString(generalSettings->sliderName[genAryIdx]);
        else if (isStick(&genAryIdx))
          result = QString(generalSettings->stickName[genAryIdx]);
      }
      if (result.isEmpty())
        result = Boards::getAnalogInputName(board, index);
      return result;

    case SOURCE_TYPE_TRIM:
      return (Boards::getCapability(board, Board::NumTrims) == 2 ? CHECK_IN_ARRAY(trims2, index) : CHECK_IN_ARRAY(trims, index));

    case SOURCE_TYPE_ROTARY_ENCODER:
      return CHECK_IN_ARRAY(rotary, index);

    case SOURCE_TYPE_MAX:
      return tr("MAX");

    case SOURCE_TYPE_SWITCH:
      if (generalSettings)
        result = QString(generalSettings->switchName[index]);
      if (result.isEmpty())
        result = Boards::getSwitchInfo(board, index).name;
      return result;

    case SOURCE_TYPE_CUSTOM_SWITCH:
      return RawSwitch(SWITCH_TYPE_VIRTUAL, index + 1).toString();

    case SOURCE_TYPE_CYC:
      return tr("CYC%1").arg(index + 1);

    case SOURCE_TYPE_PPM:
      return RadioData::getElementName(tr("TR", "as in Trainer"), index + 1);

    case SOURCE_TYPE_CH:
      if (model)
        return model->limitData[index].nameToString(index);
      else
        return LimitData().nameToString(index);

    case SOURCE_TYPE_SPECIAL:
      if (index >= SOURCE_TYPE_SPECIAL_TIMER1_IDX && index <= SOURCE_TYPE_SPECIAL_TIMER1_IDX + CPN_MAX_TIMERS - 1) {
        if (model)
          result = model->timers[index - SOURCE_TYPE_SPECIAL_TIMER1_IDX].nameToString(index - SOURCE_TYPE_SPECIAL_TIMER1_IDX);
        else
          result = TimerData().nameToString(index - SOURCE_TYPE_SPECIAL_TIMER1_IDX);
      }
      else
        result = CHECK_IN_ARRAY(special, index);

      return result;

    case SOURCE_TYPE_TELEMETRY:
      {
        div_t qr = div(index, 3);
        if (model)
          result = model->sensorData[qr.quot].nameToString(qr.quot);
        else
          result = SensorData().nameToString(qr.quot);
        if (qr.rem)
          result += (qr.rem == 1 ? "-" : "+");
        return result;
      }

    case SOURCE_TYPE_GVAR:
      if (model)
        return model->gvarData[index].nameToString(index);
      else
        return GVarData().nameToString(index);

    default:
      return QString(CPN_STR_UNKNOWN_ITEM);
  }
}

bool RawSource::isStick(int * stickIndex, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  if (type == SOURCE_TYPE_STICK && index < Boards::getCapability(board, Board::Sticks)) {
    if (stickIndex)
      *stickIndex = index;
    return true;
  }
  return false;
}

bool RawSource::isPot(int * potsIndex, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);
  if (type == SOURCE_TYPE_STICK &&
          index >= b.getCapability(Board::Sticks) &&
          index < b.getCapability(Board::Sticks) + b.getCapability(Board::Pots)) {
    if (potsIndex)
      *potsIndex = index - b.getCapability(Board::Sticks);
    return true;
  }
  return false;
}

bool RawSource::isSlider(int * sliderIndex, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);
  if (type == SOURCE_TYPE_STICK &&
          index >= b.getCapability(Board::Sticks) + b.getCapability(Board::Pots) &&
          index < b.getCapability(Board::Sticks) + b.getCapability(Board::Pots) + b.getCapability(Board::Sliders)) {
    if (sliderIndex)
      *sliderIndex = index - b.getCapability(Board::Sticks) - b.getCapability(Board::Pots);
    return true;
  }
  return false;
}

bool RawSource::isTimeBased(Board::Type board) const
{
  return (type == SOURCE_TYPE_SPECIAL && index > 0);
}

bool RawSource::isAvailable(const ModelData * const model, const GeneralSettings * const gs, Board::Type board) const
{
  if (board == Board::BOARD_UNKNOWN)
    board = getCurrentBoard();

  Boards b(board);

  if (type == SOURCE_TYPE_STICK && index >= b.getCapability(Board::MaxAnalogs))
    return false;

  if (type == SOURCE_TYPE_SWITCH && index >= b.getCapability(Board::Switches))
    return false;

  if (model) {
    if (type == SOURCE_TYPE_VIRTUAL_INPUT && !model->isInputValid(index))
      return false;

    if (type == SOURCE_TYPE_CUSTOM_SWITCH && model->logicalSw[index].isEmpty())
      return false;

    if (type == SOURCE_TYPE_TELEMETRY) {
      if (!model->sensorData[div(index, 3).quot].isAvailable()) {
        return false;
      }
    }
  }

  if (gs) {
    int gsIdx = 0;
    if (type == SOURCE_TYPE_STICK && ((isPot(&gsIdx) && !gs->isPotAvailable(gsIdx)) || (isSlider(&gsIdx) && !gs->isSliderAvailable(gsIdx))))
      return false;

    if (type == SOURCE_TYPE_SWITCH && IS_HORUS_OR_TARANIS(board) && !gs->switchSourceAllowedTaranis(index))
      return false;
  }

  if (type == SOURCE_TYPE_TRIM && index >= b.getCapability(Board::NumTrims))
    return false;

  return true;
}

RawSource RawSource::convert(RadioDataConversionState & cstate)
{
  cstate.setItemType(tr("SRC"), 1);
  RadioDataConversionState::EventType evt = RadioDataConversionState::EVT_NONE;
  RadioDataConversionState::LogField oldData(index, toString(cstate.fromModel(), cstate.fromGS(), cstate.fromType));

  if (type == SOURCE_TYPE_STICK) {
    QStringList fromStickList(getStickList(cstate.fromBoard));
    QStringList toStickList(getStickList(cstate.toBoard));
    index = toStickList.indexOf(fromStickList.at(oldData.id));
    // index set to -1 if no match found
    // perform forced mapping
  }

  if (type == SOURCE_TYPE_SWITCH) {
    QStringList fromSwitchList(getSwitchList(cstate.fromBoard));
    QStringList toSwitchList(getSwitchList(cstate.toBoard));
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

QStringList RawSource::getStickList(Boards board) const
{
  QStringList ret;

  for (int i = 0; i < board.getCapability(Board::MaxAnalogs); i++) {
    ret.append(board.getAnalogInputName(i));
  }
  return ret;
}

QStringList RawSource::getSwitchList(Boards board) const
{
  QStringList ret;

  for (int i = 0; i < board.getCapability(Board::Switches); i++) {
    ret.append(board.getSwitchInfo(i).name);
  }
  return ret;
}
