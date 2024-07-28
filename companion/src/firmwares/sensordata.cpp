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

#include "sensordata.h"
#include "modeldata.h"
#include "eeprominterface.h"
#include "compounditemmodels.h"

void SensorData::updateUnit()
{
  if (type == TELEM_TYPE_CALCULATED) {
    if (formula == TELEM_FORMULA_CONSUMPTION)
      unit = UNIT_MAH;
  }
}

QString SensorData::nameToString(int index) const
{
  return DataHelpers::getElementName(tr("TELE"), index + 1, label);
}

QString SensorData::getOrigin(const ModelData * model) const
{
  if (type != TELEM_TYPE_CUSTOM || !id)
    return QString();

  const ModuleData & module = model->moduleData[moduleIdx];
  if (module.isPxx2Module() && rxIdx <= 2 && module.access.receivers & (1 << rxIdx)) {
    return QString(module.access.receiverName[rxIdx]);
  }
  else {
    return QString();
  }
}

bool SensorData::isEmpty() const
{
  return (!isAvailable() && type == 0 && id == 0 && subid == 0 && instance == 0 && rxIdx == 0 && moduleIdx == 0 && unit == 0 && ratio == 0 && prec == 0 && offset == 0 && autoOffset == 0 && filter == 0 && onlyPositive == 0 && logs == 0);
}

QString SensorData::idToString() const
{
  return idToString(id);
}

QString SensorData::typeToString() const
{
  return typeToString(type);
}

QString SensorData::formulaToString() const
{
  return formulaToString(formula);
}

QString SensorData::cellIndexToString() const
{
  return cellIndexToString(index);
}

QString SensorData::unitToString(bool hideRaw) const
{
  return unitToString(unit, hideRaw);
}

QString SensorData::precToString() const
{
  return precToString(prec);
}

int SensorData::getMask() const
{
  int mask = 0;

  if (type == TELEM_TYPE_CALCULATED) {
    if (formula < TELEM_FORMULA_CELL)
      mask |= SENSOR_ISCONFIGURABLE | SENSOR_HAS_POSITIVE;
    if (formula == TELEM_FORMULA_DIST)
      mask |= SENSOR_HAS_GPS;
    if (formula == TELEM_FORMULA_CELL)
      mask |= SENSOR_HAS_CELLS;
    if (formula == TELEM_FORMULA_CONSUMPTION)
      mask |= SENSOR_HAS_CONSUMPTION;
    if (formula <= TELEM_FORMULA_MULTIPLY)
      mask |= SENSOR_HAS_SOURCES_12;
    if (formula < TELEM_FORMULA_MULTIPLY)
      mask |= SENSOR_HAS_SOURCES_34;
    if (formula == TELEM_FORMULA_TOTALIZE)
      mask |= SENSOR_HAS_TOTALIZE;
  }
  else {
    if (unit < UNIT_FIRST_VIRTUAL)
      mask |= (SENSOR_ISCONFIGURABLE | SENSOR_HAS_RATIO | SENSOR_HAS_POSITIVE);
  }

  if (mask & SENSOR_ISCONFIGURABLE  && unit != UNIT_FAHRENHEIT)
    mask |= SENSOR_HAS_PRECISION;

  return mask;
}

constexpr char FMT_LABEL[]        { "<b>%1:</b> " };
constexpr char FMT_VALUE[]        { "%1 " };
constexpr char FMT_LABEL_VALUE[]  { "<b>%1:</b> %2 " };

QString SensorData::paramsToString(const ModelData * model) const
{
  if (!isAvailable())
    return "";

  QString str;
  int mask = getMask();

  if (type == TELEM_TYPE_CALCULATED) {
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Formula")).arg(formulaToString()));
  }
  else {
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Id")).arg(idToString()));
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Instance")).arg(instance));
  }

  if (mask & SENSOR_HAS_CELLS) {
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Sensor")).arg(sourceToString(model, source)));
    str.append(QString(FMT_VALUE).arg(cellIndexToString()));
  }

  if (mask & SENSOR_HAS_SOURCES_12) {
    str.append(QString(FMT_LABEL).arg(tr("Sources")));
    for (int i = 0; i < 4; i++) {
      if (i < 2 || mask & SENSOR_HAS_SOURCES_34) {
        str.append(QString(FMT_VALUE).arg(sourceToString(model, sources[i])));
      }
    }
  }

  if (mask & SENSOR_HAS_CONSUMPTION || mask & SENSOR_HAS_TOTALIZE)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Sensor")).arg(sourceToString(model, amps)));

  if (mask & SENSOR_HAS_GPS) {
    str.append(QString(FMT_LABEL_VALUE).arg(tr("GPS")).arg(sourceToString(model, gps)));
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Alt")).arg(sourceToString(model, alt)));
  }

  if (mask & SENSOR_ISCONFIGURABLE)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Unit")).arg(unitToString()));

  if (mask & SENSOR_HAS_PRECISION)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Precision")).arg(precToString()));

  if (mask & SENSOR_HAS_RATIO) {
    if (unit != UNIT_RPMS) {
      int precsn = prec == 0 ? 1 : pow(10, prec);
      str.append(QString(FMT_LABEL_VALUE).arg(tr("Ratio")).arg((float)ratio / 10));
      str.append(QString(FMT_LABEL_VALUE).arg(tr("Offset")).arg(QString::number((float)offset / precsn, 'f', prec)));
      str.append(QString(FMT_LABEL_VALUE).arg(tr("Auto Offset")).arg(DataHelpers::boolToString(autoOffset, DataHelpers::BOOL_FMT_YN)));
    }
    else {
      str.append(QString(FMT_LABEL_VALUE).arg(tr("Blades")).arg(ratio)); //  TODO refactor to dedicated RPMS field
      str.append(QString(FMT_LABEL_VALUE).arg(tr("Multiplier")).arg(offset)); //  TODO refactor to dedicated RPMS field
    }
  }

  if (mask & SENSOR_ISCONFIGURABLE)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Filter")).arg(DataHelpers::boolToString(filter, DataHelpers::BOOL_FMT_YN)));

  if (type == TELEM_TYPE_CALCULATED)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Persist")).arg(DataHelpers::boolToString(persistent, DataHelpers::BOOL_FMT_YN)));

  if (mask & SENSOR_HAS_POSITIVE)
    str.append(QString(FMT_LABEL_VALUE).arg(tr("Positive")).arg(DataHelpers::boolToString(onlyPositive, DataHelpers::BOOL_FMT_YN)));

  str.append(QString(FMT_LABEL_VALUE).arg(tr("Log")).arg(DataHelpers::boolToString(logs, DataHelpers::BOOL_FMT_YN)));

  return str;
}

FieldRange SensorData::getRatioRange() const
{
  FieldRange result;

  if (unit == SensorData::UNIT_RPMS) {
    result.decimals = 0;
    result.max = 30000;
    result.min = 1;
    result.step = 1;
  }
  else {
    result.decimals = 1;
    result.max = 30000;
    result.min = 0;
    result.step = 0.1;
  }

  return result;
}

FieldRange SensorData::getOffsetRange() const
{
  FieldRange result;

  if (unit == SensorData::UNIT_RPMS) {
    result.decimals = 0;
    result.max = 30000;
    result.min = 1;
    result.step = 1;
  }
  else {
    result.decimals = prec;
    result.max = 30000.0f / powf(10.0f, prec);
    result.min = -result.max;
    result.step = pow(0.1, prec);
  }

  return result;
}

void SensorData::formulaChanged()
{
  switch (formula) {
    case TELEM_FORMULA_CELL:
      prec = 2;
      unit = UNIT_VOLTS;
      break;
    case TELEM_FORMULA_CONSUMPTION:
      prec = 0;
      unit = UNIT_MAH;
      break;
    case TELEM_FORMULA_DIST:
      prec = 0;
      unit = UNIT_METERS;
      break;
  }
}

void SensorData::unitChanged()
{
  if (unit == UNIT_FAHRENHEIT)
    prec = 0;
}

//  static
QString SensorData::idToString(const int value)
{
  return QString::number(value, 16).toUpper();
}

//  static
QString SensorData::typeToString(const int value)
{
  switch(value) {
    case TELEM_TYPE_CUSTOM:
      return tr("Custom");
    case TELEM_TYPE_CALCULATED:
      return tr("Calculated");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString SensorData::formulaToString(const int value)
{
  switch(value) {
    case TELEM_FORMULA_ADD:
      return tr("Add");
    case TELEM_FORMULA_AVERAGE:
      return tr("Average");
    case TELEM_FORMULA_MIN:
      return tr("Minimum");
    case TELEM_FORMULA_MAX:
      return tr("Maximum");
    case TELEM_FORMULA_MULTIPLY:
      return tr("Multiply");
    case TELEM_FORMULA_TOTALIZE:
      return tr("Totalize");
    case TELEM_FORMULA_CELL:
      return tr("Cell");
    case TELEM_FORMULA_CONSUMPTION:
      return tr("Consumption");
    case TELEM_FORMULA_DIST:
      return tr("Distance");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString SensorData::cellIndexToString(const int value)
{
  if (value == TELEM_CELL_INDEX_LOWEST)
    return tr("Lowest");
  if (value > TELEM_CELL_INDEX_LOWEST && value < TELEM_CELL_INDEX_HIGHEST)
    return tr("Cell %1").arg(value - TELEM_CELL_INDEX_LOWEST);
  if (value== TELEM_CELL_INDEX_HIGHEST)
    return tr("Highest");
  if (value == TELEM_CELL_INDEX_DELTA)
    return tr("Delta");
  return CPN_STR_UNKNOWN_ITEM;
}

//  static
QString SensorData::precToString(const int value)
{
  return QString("0.%1").arg(QString(value, '0'));
}

//  static
QString SensorData::sourceToString(const ModelData * model, const int index, const bool positivesign)
{
  if (model) {
    const QString prfx = index < 0 ? "-" : positivesign ? "+" : "";

    if (abs(index) > 0) {
      const SensorData &sd = model->sensorData[abs(index) - 1];
      if (sd.type == SensorData::TELEM_TYPE_CUSTOM)
        return QString("%1%2 (%3)").arg(prfx).arg(sd.label).arg(sd.getOrigin(model));
      else
        return QString("%1%2").arg(prfx).arg(sd.label);
    }
    else
      return CPN_STR_NONE_ITEM;
  }

  return "";
}

//  static
QString SensorData::unitToString(const int value, bool hideRaw)
{
  switch (value) {
    case UNIT_RAW:
      return hideRaw ? QString() : tr("Raw (-)");
    case UNIT_VOLTS:
      return tr("V");
    case UNIT_AMPS:
      return tr("A");
    case UNIT_MILLIAMPS:
      return tr("mA");
    case UNIT_KTS:
      return tr("kts");
    case UNIT_METERS_PER_SECOND:
      return tr("m/s");
    case UNIT_FEET_PER_SECOND:
      return tr("f/s");
    case UNIT_KMH:
      return tr("km/h");
    case UNIT_MPH:
      return tr("mph");
    case UNIT_METERS:
      return tr("m");
    case UNIT_FEET:
      return tr("f");
    case UNIT_CELSIUS:
      return tr("°C");
    case UNIT_FAHRENHEIT:
      return tr("°F");
    case UNIT_PERCENT:
      return tr("%");
    case UNIT_MAH:
      return tr("mAh");
    case UNIT_WATTS:
      return tr("W");
    case UNIT_MILLIWATTS:
        return tr("mW");
    case UNIT_DB:
      return tr("dB");
    case UNIT_RPMS:
      return tr("rpms");
    case UNIT_G:
      return tr("g");
    case UNIT_DEGREE:
      return tr("°");
    case UNIT_RADIANS:
      return tr("Rad");
    case UNIT_MILLILITERS:
      return tr("ml");
    case UNIT_FLOZ:
      return tr("fl.oz");
    case UNIT_MILLILITERS_PER_MINUTE:
      return tr("ml/minute");
    case UNIT_HERZ:
      return tr("Hertz");
    case UNIT_MS:
      return tr("mS");
    case UNIT_US:
      return tr("uS");
    case UNIT_HOURS:
      return tr("hours");
    case UNIT_MINUTES:
      return tr("minutes");
    case UNIT_SECONDS:
      return tr("seconds");
    case UNIT_CELLS:
      return tr("V");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
bool SensorData::isSourceAvailable(const ModelData * model, const int index)
{
  Firmware * firmware = getCurrentFirmware();
  const int sensorcnt = firmware->getCapability(Sensors);
  const int i = abs(index);

  if (model && sensorcnt > 0 && i <= sensorcnt) {
    if (i > 0)
      return model->sensorData[i - 1].isAvailable();
    else
      return true;
  }

  return false;
}

//  static
bool SensorData::isSourceVario(const ModelData * model, const int index)
{
  const int i = abs(index);

  if (i > 0) {
    if (model->sensorData[i - 1].isAvailable()) {
      if (model->sensorData[i - 1].unit == UNIT_FEET_PER_SECOND || model->sensorData[i - 1].unit == UNIT_METERS_PER_SECOND)
        return true;
    }
  }
  else
    return true;

  return false;
}

#define RSSI_ID                   0xF101

//  static
bool SensorData::isRssiSensorAvailable(const ModelData * model, const int value)
{
  if (value == 0)
    return true;
  else {
    const SensorData &sensor = model->sensorData[abs(value) - 1];
    return (sensor.isAvailable() && sensor.id == RSSI_ID);
  }
}

//  static
QString SensorData::rssiSensorToString(const ModelData * model, const int value)
{
  if (value == 0)
    return tr("(default)");
  else
    return sourceToString(model, value);
}

//  static
AbstractStaticItemModel * SensorData::typeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("sensordata.type");

  for (int i = 0; i <= TELEM_TYPE_LAST; i++) {
    mdl->appendToItemList(typeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * SensorData::formulaItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("sensordata.formula");

  for (int i = 0; i <= TELEM_FORMULA_LAST; i++) {
    mdl->appendToItemList(formulaToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * SensorData::cellIndexItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("sensordata.cellindex");

  for (int i = 0; i <= TELEM_CELL_INDEX_LAST; i++) {
    mdl->appendToItemList(cellIndexToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * SensorData::unitItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("sensordata.unit");

  for (int i = 0; i <= UNIT_MAX; i++) {
    QString str = unitToString(i, false);
    if (str != CPN_STR_UNKNOWN_ITEM)
      mdl->appendToItemList(str, i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
PrecisionItemModel * SensorData::precisionItemModel()
{
  PrecisionItemModel * mdl = new PrecisionItemModel(0, 2);
  mdl->setName("sensordata.precision");
  return mdl;
}
