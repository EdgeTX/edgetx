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

#include "radiodataconversionstate.h"
#include "radiodata.h"

#include <QStandardItemModel>

RadioDataConversionState::RadioDataConversionState(Board::Type before, Board::Type after, RadioData * rdata) :
  fromType(before),
  toType(after),
  fromBoard(Boards(before)),
  toBoard(Boards(after)),
  rd(NULL),
  rdCopy(NULL),
  maxEventType(EVT_NONE),
  modelIdx(-1),
  subCompIdx(-1)
{
  setRadioData(rdata);
}

RadioDataConversionState::~RadioDataConversionState()
{
  if (rdCopy)
    delete rdCopy;
}

RadioDataConversionState & RadioDataConversionState::withModelIndex(int index)
{
  modelIdx = index;
  subCompIdx = -1;
  setLogField(FLD_ORIGIN);
  return *this;
}

RadioDataConversionState & RadioDataConversionState::withComponentIndex(int index)
{
  subCompIdx = index;
  setLogField(FLD_SUB_COMP);
  return *this;
}

RadioDataConversionState &RadioDataConversionState::withComponentField(const QString & name)
{
  setLogField(FLD_COMP_FIELD, LogField(0, name));
  return *this;
}

bool RadioDataConversionState::convert()
{
  if (rd) {
    rd->convert(*this);
    return true;
  }

  setErr(("No radio data to convert."));
  return false;
}

void RadioDataConversionState::setRadioData(RadioData * rdata)
{
  rd = rdata;
  if (rdCopy)
    delete rdCopy;
  rdCopy = new RadioData(*rdata);
  logCache = LogRecord();
}

void RadioDataConversionState::setLogField(LogFieldType type, const LogField & data, bool clearRest)
{
  if (type >= FLD_ENUM_COUNT)
    return;

  logCache.fields[type].id = data.id;
  logCache.fields[type].name = data.name;
  if (clearRest && type + 1 < FLD_ENUM_COUNT)
    setLogField(LogFieldType(type + 1));
}

void RadioDataConversionState::addLogEntry(EventType event, const QString & msg)
{
  if (event != EVT_NONE) {
    setLogField(FLD_EVT_TYPE, LogField(event, eventTypeToString(event)), false);
    setLogField(FLD_MSG, LogField(event, msg), false);
    log.append(logCache);
  }
  maxEventType = qMax(event, maxEventType);
  logCache.fields[FLD_MSG].clear();
}

void RadioDataConversionState::setInvalid(const LogField & item)
{
  setLogField(FLD_ITM_BEFORE, item);
  addLogEntry(EVT_INV, tr("is invalid"));
}

void RadioDataConversionState::setConverted(const LogField & from, const LogField & to)
{
  setLogField(FLD_ITM_BEFORE, from);
  setLogField(FLD_ITM_AFTER, to);
  addLogEntry(EVT_CVRT, tr("converted to"));
}

void RadioDataConversionState::setMoved(const LogField & from, const LogField & to)
{
  setLogField(FLD_ITM_BEFORE, from);
  setLogField(FLD_ITM_AFTER, to);
  addLogEntry(EVT_INF, tr("verify is"));
}

void RadioDataConversionState::setUnsupported(const LogField & item)
{
  setLogField(FLD_ITM_BEFORE, item);
  addLogEntry(EVT_UNSUP, tr("unsupported"));
}

const ModelData * RadioDataConversionState::toModel() const
{
  if (rd && modelIdx > -1 && modelIdx < (int)rd->models.size())
    return &rd->models[modelIdx];
  return NULL;
}

const ModelData *RadioDataConversionState::fromModel() const
{
  if (rdCopy && modelIdx > -1 && modelIdx < (int)rdCopy->models.size())
    return &rdCopy->models[modelIdx];
  return NULL;
}

const GeneralSettings * RadioDataConversionState::toGS() const
{
  if (rd)
    return &rd->generalSettings;
  return NULL;
}

const GeneralSettings *RadioDataConversionState::fromGS() const
{
  if (rdCopy)
    return &rdCopy->generalSettings;
  return NULL;
}

bool RadioDataConversionState::hasLogEntries(EventType logLevel) const
{
  return (maxEventType >= logLevel);
}

QString RadioDataConversionState::eventTypeToString(int type) const
{
  // enum EventType                             { EVT_NONE,   EVT_DBG,       EVT_INF,       EVT_WRN,       EVT_CVRT,      EVT_ERR,       EVT_INV,       EVT_UNSUP
  static const QStringList evtTypes =  QStringList() << "" << tr("[DBG]") << tr("[NFO]") << tr("[WRN]") << tr("[CVT]") << tr("[ERR]") << tr("[INV]") << tr("[XSP]");
  return (type < evtTypes.size() ? evtTypes.at(type) : "");
}

QString RadioDataConversionState::eventTypeToColor(int type) const
{
  // enum EventType                                   { EVT_NONE,  EVT_DBG,     EVT_INF,   EVT_WRN,     EVT_CVRT,    EVT_ERR,  EVT_INV, EVT_UNSUP
  static const QStringList evtColors = QStringList() << "black" << "dimgrey" << "black" << "orange" << "magenta" << "red"  << "red"  << "blue";
  return (type < evtColors.size() ? evtColors.at(type) : "black");
}

QString RadioDataConversionState::getFieldName(int field) const
{
  static const QStringList dataFields = QStringList() << tr("Event") << tr("Origin") << tr("Comp") << tr("Sub-Component") << tr("Field") << tr("Type") << tr("Old") << tr("Action") << tr("New");
  return (field < dataFields.size() ? dataFields.at(field) : "");
}

QStandardItemModel * RadioDataConversionState::getLogModel(RadioDataConversionState::EventType logLevel, QObject * parent) const
{
  QStandardItemModel * stdmdl = new QStandardItemModel(parent);
  const QString tooltip = "%1: %2 (ID: %3)";
  const int sortRole = Qt::UserRole + 1;
  const int seqdigits = floorf(logf(log.size()) / logf(10.0f)) + 1;

  stdmdl->setSortRole(sortRole);

  QStringList header;
  header << tr("Seq");
  for (int i=0; i < FLD_ENUM_COUNT; ++i)
    header << getFieldName(i);
  stdmdl->setHorizontalHeaderLabels(header);

  QFont fnt("Courier");
  fnt.setStyleHint(QFont::TypeWriter);
  //fnt.setPointSize(fnt.pointSize()+1);

  int row = 0;
  foreach (const LogRecord & lr, log) {
    if (lr.fields.at(FLD_EVT_TYPE).id < logLevel)
      continue;
    QBrush fg(QColor(eventTypeToColor(lr.fields.at(FLD_EVT_TYPE).id)));
    stdmdl->setItem(row, 0, new QStandardItem(QString("%1").arg(row+1, seqdigits, 10, QChar('0'))));
    stdmdl->item(row, 0)->setData(row, sortRole);
    stdmdl->item(row, 0)->setData(fnt, Qt::FontRole);
    for (int i=0; i < FLD_ENUM_COUNT; ++i) {
      QStandardItem * itm = new QStandardItem(lr.fields.at(i).name);
      itm->setData(lr.fields.at(i).id, sortRole);
      itm->setData(fnt, Qt::FontRole);
      itm->setData(tooltip.arg(getFieldName(i)).arg(lr.fields.at(i).name).arg(lr.fields.at(i).id), Qt::ToolTipRole);
      itm->setEditable(false);
      if (i == FLD_EVT_TYPE || i == FLD_MSG)
        itm->setForeground(fg);
      stdmdl->setItem(row, i+1, itm);
    }
    ++row;
  }

  return stdmdl;
}
