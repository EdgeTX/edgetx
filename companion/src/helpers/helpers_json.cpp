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

#include "helpers_json.h"

#include <QByteArray>
#include <QFile>
#include <QMessageBox>

const bool JsonBase::exists(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined();
}

const QVariant JsonBase::getValue(const QJsonObject & obj, const QString & key, bool manditory,
                                  const QVariant & dflt)
{
  if (!exists(obj, key)) {
    if (manditory)
      qWarning() << "Error - key:" << key << "not found. Using default value:" << dflt;

    return dflt;
  }

  return obj.value(key).toVariant();
}

const bool JsonBase::getValueBool(const QJsonObject::const_iterator & it, const bool dflt)
{
  bool isvalid = it.value().isBool();

  if (!isvalid)
    qWarning() << "Warning: key value not a boolean";

  return isvalid ? it.value().toBool() : dflt;
}

const bool JsonBase::getValueBool(const QJsonObject & obj, const QString & key,
                                  const bool dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isBool());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a boolean";

  return isvalid ? obj.value(key).toBool() : dflt;
}

const int JsonBase::getValueInt(const QJsonObject::const_iterator & it,
                                const int dflt, const int max, const int min)
{
  bool isvalid = it.value().isDouble();
  int value = isvalid ? it.value().toInt() : 0;

  if (!isvalid)
    qWarning() << "Warning: key value not an integer";

  if (min > max)
    qWarning() << "Warning: range check ignored as min:" << min << "exceeds max:" << max;

  return (isvalid && (min < max ? value >= min && value <= max : true)) ? value : dflt;
}

const int JsonBase::getValueInt(const QJsonObject & obj, const QString & key,
                                const int dflt, const int max, const int min)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isDouble());
  int value = isvalid ? obj.value(key).toInt() : 0;

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not an integer";

  if (min > max)
    qWarning() << "Warning: range check ignored for:" << key << "as min:" << min << "exceeds max:" << max;

  return (isvalid && (min < max ? value >= min && value <= max : true)) ? value : dflt;
}

const std::string JsonBase::getValueStdString(const QJsonObject::const_iterator & it,
                                              const std::string & dflt)
{
  bool isvalid = it.value().isString();

  if (!isvalid)
    qWarning() << "Warning: key value not a string";

  return isvalid ? it.value().toString().toStdString() : dflt;
}

const std::string JsonBase::getValueStdString(const QJsonObject & obj, const QString & key,
                                              const std::string & dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isString());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a string";

  return isvalid ? obj.value(key).toString().toStdString() : dflt;
}

const QString JsonBase::getValueString(const QJsonObject::const_iterator & it,
                                       const QString & dflt)
{
  bool isvalid = it.value().isString();

  if (!isvalid)
    qWarning() << "Warning: key value not a string";

  return isvalid ? it.value().toString() : dflt;
}

const QString JsonBase::getValueString(const QJsonObject & obj, const QString & key,
                                       const QString & dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isString());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a string";

  return isvalid ? obj.value(key).toString() : dflt;
}

const bool JsonBase::isArray(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined() && obj.value(key).isArray();
}

const bool JsonBase::isObject(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined() && obj.value(key).isObject();
}

bool JsonBase::load(QJsonDocument * doc, const QString & filename)
{
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(nullptr, tr("Load Json File"),
                          tr("Error: Unable to open file %1").arg(file.fileName()));
    return false;
  }

  QByteArray *buffer = new QByteArray();
  *buffer = file.readAll();
  file.close();

  if (buffer->isEmpty()) {
    QMessageBox::critical(nullptr, tr("Load Json File"),
                          tr("Error: Unable to read file %1").arg(file.fileName()));
    return false;
  }

  QJsonParseError res;
  *doc = QJsonDocument::fromJson(*buffer, &res);
  delete buffer;

  if (res.error || doc->isNull() || !doc->isObject()) {
    QMessageBox::critical(nullptr, tr("Load Json File"),
      tr("Error: %1 is not a valid json formatted file.\nError code: %2\nError description: %3")
          .arg(file.fileName()).arg(res.error).arg(res.errorString()));
    *doc = QJsonDocument();
    return false;
  }

  return true;
}
