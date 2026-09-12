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

const QVariant JsonBase::getValue(const QJsonObject & obj, const QString & name,
                                  const QVariant & dflt)
{
  bool isvalid = !obj.value(name).isUndefined();

  if (!isvalid)
    qWarning() << "Warning: name:" << name << "not found";

  return isvalid ? obj.value(name).toVariant() : dflt;
}

const bool JsonBase::getValueBool(const QJsonObject & obj, const QString & name,
                                  const bool dflt)
{
  bool isvalid = (!obj.value(name).isUndefined() && obj.value(name).isBool());

  if (!isvalid)
    qWarning() << "Warning: name:" << name << "not found and/or value type not a boolean";

  return isvalid ? obj.value(name).toBool() : dflt;
}

const int JsonBase::getValueInt(const QJsonObject & obj, const QString & name,
                                const int dflt, const int max, const int min)
{
  bool isvalid = (!obj.value(name).isUndefined() && obj.value(name).isDouble());
  int value = isvalid ? obj.value(name).toInt() : 0;

  if (!isvalid)
    qWarning() << "Warning: name:" << name << "not found and/or value type not an integer";

  if (min > max)
    qWarning() << "Warning: range check ignored for:" << name << "as min:" << min << "exceeds max:" << max;

  return (isvalid && (min < max ? value >= min && value <= max : true)) ? value : dflt;
}

const std::string JsonBase::getValueStdString(const QJsonObject & obj, const QString & name,
                                              const std::string & dflt)
{
  bool isvalid = (!obj.value(name).isUndefined() && obj.value(name).isString());

  if (!isvalid)
    qWarning() << "Warning: name:" << name << "not found and/or value type not a string";

  return isvalid ? obj.value(name).toString().toStdString() : dflt;
}

const QString JsonBase::getValueString(const QJsonObject & obj, const QString & name,
                                       const QString & dflt)
{
  bool isvalid = (!obj.value(name).isUndefined() && obj.value(name).isString());

  if (!isvalid)
    qWarning() << "Warning: name:" << name << "not found and/or value type not a string";

  return isvalid ? obj.value(name).toString() : dflt;
}

const bool JsonBase::isArray(const QJsonObject & obj, const QString & name)
{
  return !obj.value(name).isUndefined() && obj.value(name).isArray();
}

const bool JsonBase::isObject(const QJsonObject & obj, const QString & name)
{
  return !obj.value(name).isUndefined() && obj.value(name).isObject();
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
