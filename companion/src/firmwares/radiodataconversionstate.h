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

#ifndef RADIODATACONVERSIONSTATE_H
#define RADIODATACONVERSIONSTATE_H

#include "boards.h"
#include <QtCore>

class ModelData;
class GeneralSettings;
class RadioData;
class QStandardItemModel;

class RadioDataConversionState
{
  Q_DECLARE_TR_FUNCTIONS(RadioDataConversionState)

  public:
    enum EventType {
      EVT_NONE,  // does not get logged
      EVT_DBG,   // debug
      EVT_INF,   // misc. info
      EVT_WRN,   // misc. warning, place warning-level events after this and before EVT_ERR
      EVT_CVRT,  // something was converted (A->B)
      EVT_ERR,   // misc. error, place error-level events after this
      EVT_INV,   // invalid, control/etc not available on destination radio
      EVT_UNSUP, // unsupported, control not available on destiation radio
    };

    enum LogFieldType {
      FLD_EVT_TYPE,      // EventType
      FLD_ORIGIN,        // General Settings or Model with name
      FLD_COMP,          // Input/Mix/LS/CF/etc
      FLD_SUB_COMP,      // Mix channel/Input line/LS number/etc
      FLD_COMP_FIELD,    // optional item within subcomponent, eg. LS V1/V2/AND, etc
      FLD_ITM_TYPE,      // Source/Switch
      FLD_ITM_BEFORE,    // item before conversion
      FLD_MSG,           // conversion details
      FLD_ITM_AFTER,     // item after conversion
      FLD_ENUM_COUNT
    };

    struct LogField {
      LogField(int id = 0, const QString & name = QString()) : id(id), name(name) {}
      void clear() { id = 0; name.clear(); }
      int id;
      QString name;
    };

    struct LogRecord
    {
      LogRecord() { fields.resize(FLD_ENUM_COUNT); }
      QVector<LogField> fields;  // collected data fields indexed by LogColumn value
    };

    RadioDataConversionState(Board::Type before = Board::BOARD_UNKNOWN, Board::Type after = Board::BOARD_UNKNOWN, RadioData * rdata = NULL);
    ~RadioDataConversionState();

    RadioDataConversionState & withModelIndex(int index);
    RadioDataConversionState & withComponentIndex(int index);
    RadioDataConversionState & withComponentField(const QString & name);

    bool convert();
    void setRadioData(RadioData * rdata);
    void setLogField(LogFieldType type, const LogField & data = LogField(), bool clearRest = true);
    inline void setOrigin(const QString & name = QString())    { setLogField(FLD_ORIGIN, LogField(modelIdx, name)); }
    inline void setSubComp(const QString & name = QString())   { setLogField(FLD_SUB_COMP, LogField(subCompIdx, name)); }
    inline void setComponent(const QString & name = QString(), int id = 0) { setLogField(FLD_COMP, LogField(id, name)); }
    inline void setItemType(const QString & name = QString(), int id = 0)  { setLogField(FLD_ITM_TYPE, LogField(id, name)); }

    void addLogEntry(EventType event, const QString & msg = QString());
    void setInvalid(const LogField & item);
    void setConverted(const LogField & from, const LogField & to);
    void setMoved(const LogField & from, const LogField & to);
    void setUnsupported(const LogField & item);
    inline void setDbg(const QString & msg) { addLogEntry(EVT_DBG, msg); }
    inline void setInf(const QString & msg) { addLogEntry(EVT_INF, msg); }
    inline void setWrn(const QString & msg) { addLogEntry(EVT_WRN, msg); }
    inline void setErr(const QString & msg) { addLogEntry(EVT_ERR, msg); }

    const ModelData * toModel() const;
    const ModelData * fromModel() const;
    const GeneralSettings * toGS() const;
    const GeneralSettings * fromGS() const;
    bool hasLogEntries(EventType logLevel = EVT_INF) const;

    QString eventTypeToString(int type) const;
    QString eventTypeToColor(int type) const;
    QString getFieldName(int field) const;
    QStandardItemModel * getLogModel(EventType logLevel = EVT_INF, QObject *parent = Q_NULLPTR) const;

    Board::Type fromType;    // board type before conversion
    Board::Type toType;      // board type after
    Boards fromBoard;
    Boards toBoard;
    RadioData * rd;          // pointer to data struct being converted
    RadioData * rdCopy;      // copy of original radio data before conversion
    QVector<LogRecord> log;  // logged events
    LogRecord logCache;      // buffer for holding event data before logging
    EventType maxEventType;  // most severe event type recorded
    int modelIdx;            // model index in radio data array, -1 if none
    int subCompIdx;          // current row index within component (eg. model.mixData[componentIdx]), -1 if none
};

#endif // RADIODATACONVERSIONSTATE_H
