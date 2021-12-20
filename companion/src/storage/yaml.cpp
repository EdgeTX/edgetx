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

#include "yaml.h"

#include <QFile>
#include <QDir>

bool YamlFormat::loadFile(QByteArray & filedata, const QString & filename)
{
  QString path = this->filename + "/" + filename;
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    setError(tr("Error opening file %1:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  filedata = file.readAll();
  qDebug() << "File" << path << "read, size:" << filedata.size();
  file.close();
  return true;
}

bool YamlFormat::writeFile(const QByteArray & filedata, const QString & filename)
{
  QString path = this->filename + "/" + filename;
  QFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    setError(tr("Error opening file %1 in write mode:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  file.write(filedata.data(), filedata.size());
  file.close();
  qDebug() << "File" << path << "written, size:" << filedata.size();
  return true;
}

bool YamlFormat::load(RadioData & radioData)
{
  QByteArray radioSettingsBuffer;
  if (!loadFile(radioSettingsBuffer, "RADIO/radio.yml")) {
    setError(tr("Can't extract RADIO/radio.yml"));
    return false;
  }

  qDebug() << "Warning: format ignored - under development";
  return false; // force failure until fully developed
}

bool YamlFormat::write(const RadioData & radioData)
{
  /*  Need this test in case of a new sdcard
  Board::Type board = getCurrentBoard();
  if (!HAS_EEPROM_YAML(board)) {
    qDebug() << "Board does not support YAML format";
    return false;
  }

  // ensure directories exist on sd card
  QDir dir(filename);
  dir.mkdir("RADIO");
  dir.mkdir("MODELS");
  */

  qDebug() << "Warning: format ignored - under development";
  return false; // force failure until fully developed
}
