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

#pragma once

#include "datahelpers.h"

#include <QtCore>

constexpr int CPN_MAX_SENSORS  { 60 };
constexpr int SENSOR_LABEL_LEN { 4 };

constexpr int SENSOR_ISCONFIGURABLE   { 1 << 1 };
constexpr int SENSOR_HAS_GPS          { 1 << 2 };
constexpr int SENSOR_HAS_CELLS        { 1 << 3 };
constexpr int SENSOR_HAS_CONSUMPTION  { 1 << 4 };
constexpr int SENSOR_HAS_RATIO        { 1 << 5 };
constexpr int SENSOR_HAS_TOTALIZE     { 1 << 6 };
constexpr int SENSOR_HAS_SOURCES_12   { 1 << 7 };
constexpr int SENSOR_HAS_SOURCES_34   { 1 << 8 };
constexpr int SENSOR_HAS_POSITIVE     { 1 << 9 };
constexpr int SENSOR_HAS_PRECISION    { 1 << 10 };

class ModelData;
class AbstractStaticItemModel;
class PrecisionItemModel;

class SensorData {

  Q_DECLARE_TR_FUNCTIONS(SensorData)

  public:

    enum
    {
      TELEM_TYPE_CUSTOM,
      TELEM_TYPE_CALCULATED,
      TELEM_TYPE_LAST = TELEM_TYPE_CALCULATED
    };

    enum
    {
      TELEM_FORMULA_ADD,
      TELEM_FORMULA_AVERAGE,
      TELEM_FORMULA_MIN,
      TELEM_FORMULA_MAX,
      TELEM_FORMULA_MULTIPLY,
      TELEM_FORMULA_TOTALIZE,
      TELEM_FORMULA_CELL,
      TELEM_FORMULA_CONSUMPTION,
      TELEM_FORMULA_DIST,
      TELEM_FORMULA_LAST = TELEM_FORMULA_DIST
    };

    enum {
      TELEM_CELL_INDEX_LOWEST,
      TELEM_CELL_INDEX_1,
      TELEM_CELL_INDEX_2,
      TELEM_CELL_INDEX_3,
      TELEM_CELL_INDEX_4,
      TELEM_CELL_INDEX_5,
      TELEM_CELL_INDEX_6,
      TELEM_CELL_INDEX_HIGHEST,
      TELEM_CELL_INDEX_DELTA,
      TELEM_CELL_INDEX_LAST = TELEM_CELL_INDEX_DELTA
    };

    enum
    {
      UNIT_RAW,
      UNIT_VOLTS,
      UNIT_AMPS,
      UNIT_MILLIAMPS,
      UNIT_KTS,
      UNIT_METERS_PER_SECOND,
      UNIT_FEET_PER_SECOND,
      UNIT_KMH,
      UNIT_SPEED = UNIT_KMH,
      UNIT_MPH,
      UNIT_METERS,
      UNIT_DIST = UNIT_METERS,
      UNIT_FEET,
      UNIT_CELSIUS,
      UNIT_TEMPERATURE = UNIT_CELSIUS,
      UNIT_FAHRENHEIT,
      UNIT_PERCENT,
      UNIT_MAH,
      UNIT_WATTS,
      UNIT_MILLIWATTS,
      UNIT_DB,
      UNIT_RPMS,
      UNIT_G,
      UNIT_DEGREE,
      UNIT_RADIANS,
      UNIT_MILLILITERS,
      UNIT_FLOZ,
      UNIT_MILLILITERS_PER_MINUTE,
      UNIT_HERZ,
      UNIT_MS,
      UNIT_US,
      UNIT_MAX = UNIT_US,
      UNIT_SPARE4,
      UNIT_SPARE5,
      UNIT_SPARE6,
      UNIT_SPARE7,
      UNIT_SPARE8,
      UNIT_SPARE9,
      UNIT_SPARE10,
      UNIT_HOURS,
      UNIT_MINUTES,
      UNIT_SECONDS,
      // FrSky format used for these fields, could be another format in the future
      UNIT_FIRST_VIRTUAL,
      UNIT_CELLS = UNIT_FIRST_VIRTUAL,
      UNIT_DATETIME,
      UNIT_GPS,
      UNIT_BITFIELD,
      UNIT_TEXT,
      // Internal units (not stored in sensor unit)
      UNIT_GPS_LONGITUDE,
      UNIT_GPS_LATITUDE,
      UNIT_DATETIME_YEAR,
      UNIT_DATETIME_DAY_MONTH,
      UNIT_DATETIME_HOUR_MIN,
      UNIT_DATETIME_SEC
    };

    SensorData() { clear(); }
    unsigned int type; // custom / formula
    unsigned int id;
    unsigned int subid;
    unsigned int instance;
    unsigned int rxIdx;
    unsigned int moduleIdx;
    unsigned int persistentValue;
    unsigned int formula;
    char label[SENSOR_LABEL_LEN + 1];
    unsigned int unit;
    unsigned int prec;
    bool autoOffset;
    bool filter;
    bool logs;
    bool persistent;
    bool onlyPositive;

    // for custom sensors
    unsigned int ratio;
    int offset;

    // for consumption
    unsigned int amps;

    // for cell
    unsigned int source;
    unsigned int index;

    // for calculations
    int sources[4];

    // for GPS dist
    unsigned int gps;
    unsigned int alt;

    bool isAvailable() const { return strlen(label) > 0; }
    void updateUnit();
    QString nameToString(int index) const;
    QString getOrigin(const ModelData* model) const;
    void clear() { memset(this, 0, sizeof(SensorData)); }
    bool isEmpty() const;
    QString idToString() const;
    QString typeToString() const;
    QString formulaToString() const;
    QString cellIndexToString() const;
    QString unitToString(bool hideRaw = true) const;
    QString precToString() const;
    int getMask() const;
    QString paramsToString(const ModelData * model) const;
    FieldRange getRatioRange() const;
    FieldRange getOffsetRange() const;
    void formulaChanged();
    void unitChanged();

    static QString sourceToString(const ModelData * model, const int index, const bool sign = false);
    static bool isSourceAvailable(const ModelData * model, const int index);
    static QString idToString(const int value);
    static QString typeToString(const int value);
    static QString formulaToString(const int value);
    static QString cellIndexToString(const int value);
    static QString unitToString(const int value, bool hideRaw = true);
    static QString precToString(const int value);
    static bool isRssiSensorAvailable(const ModelData * model, const int value);
    static QString rssiSensorToString(const ModelData * model, const int value);

    static AbstractStaticItemModel * typeItemModel();
    static AbstractStaticItemModel * formulaItemModel();
    static AbstractStaticItemModel * cellIndexItemModel();
    static AbstractStaticItemModel * unitItemModel();
    static PrecisionItemModel * precisionItemModel();
};
