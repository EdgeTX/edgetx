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

const bool Json::exists(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined();
}

const QVariant Json::value(const QJsonObject & obj, const QString & key, bool manditory,
                           const QVariant & dflt)
{
  if (!exists(obj, key)) {
    if (manditory)
      qWarning() << "Error - key:" << key << "not found. Using default value:" << dflt;

    return dflt;
  }

  return obj.value(key).toVariant();
}

const bool Json::valueBool(const QJsonObject::const_iterator & it, const bool dflt)
{
  bool isvalid = it.value().isBool();

  if (!isvalid)
    qWarning() << "Warning - key:" << it.key() << "value:" << it.value() << "is not a boolean";

  return isvalid ? it.value().toBool() : dflt;
}

const bool Json::valueBool(const QJsonObject & obj, const QString & key,
                           const bool dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isBool());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a boolean";

  return isvalid ? obj.value(key).toBool() : dflt;
}

const int Json::valueInt(const QJsonObject::const_iterator & it,
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

const int Json::valueInt(const QJsonObject & obj, const QString & key,
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

const std::string Json::valueStdString(const QJsonObject::const_iterator & it,
                                       const std::string & dflt)
{
  bool isvalid = it.value().isString();

  if (!isvalid)
    qWarning() << "Warning: key value not a string";

  return isvalid ? it.value().toString().toStdString() : dflt;
}

const std::string Json::valueStdString(const QJsonObject & obj, const QString & key,
                                       const std::string & dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isString());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a string";

  return isvalid ? obj.value(key).toString().toStdString() : dflt;
}

const QString Json::valueString(const QJsonObject::const_iterator & it,
                                const QString & dflt)
{
  bool isvalid = it.value().isString();

  if (!isvalid)
    qWarning() << "Warning: key value not a string";

  return isvalid ? it.value().toString() : dflt;
}

const QString Json::valueString(const QJsonObject & obj, const QString & key,
                                const QString & dflt)
{
  bool isvalid = (!obj.value(key).isUndefined() && obj.value(key).isString());

  if (!isvalid)
    qWarning() << "Warning: key:" << key << "not found and/or key value not a string";

  return isvalid ? obj.value(key).toString() : dflt;
}

const bool Json::isArray(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined() && obj.value(key).isArray();
}

const bool Json::isObject(const QJsonObject & obj, const QString & key)
{
  return !obj.value(key).isUndefined() && obj.value(key).isObject();
}

bool Json::load(QJsonDocument * doc, const QString & filename)
{
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly)) {
    qCritical() << "Error: Unable to open file:" << filename;
    return false;
  }

  QByteArray *buffer = new QByteArray();
  *buffer = file.readAll();
  file.close();

  if (buffer->isEmpty()) {
    qCritical() << "Error: Unable to read file:" << filename;
    return false;
  }

  QJsonParseError res;
  *doc = QJsonDocument::fromJson(*buffer, &res);
  delete buffer;

  if (res.error || doc->isNull() || !doc->isObject()) {
    qCritical() << QString("Error: %1 is not a valid json formatted file. Code: %2 Description: %3")
                           .arg(filename).arg(res.error).arg(res.errorString());
    *doc = QJsonDocument();
    return false;
  }

  return true;
}
