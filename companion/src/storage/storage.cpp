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

#include "etx.h"
#include "yaml.h"
#include "sdcard.h"

#include <QFileInfo>

StorageType getStorageType(const QString & filename)
{
  QString suffix = QFileInfo(filename).suffix().toUpper();

  if (suffix == "ETX")
    return STORAGE_TYPE_ETX;
  else if (suffix == "YML")
    return STORAGE_TYPE_YML;
  else
    return STORAGE_TYPE_UNKNOWN;
}

void registerStorageFactory(StorageFactory * factory);

QList<StorageFactory *> registeredStorageFactories;

void registerStorageFactory(StorageFactory * factory)
{
  qDebug() << "register storage" << factory->name();
  registeredStorageFactories.push_back(factory);
}

void registerStorageFactories()
{
  registerStorageFactory(new DefaultStorageFactory<EtxFormat>("etx"));
  registerStorageFactory(new DefaultStorageFactory<YamlFormat>("yml"));
  registerStorageFactory(new SdcardStorageFactory());
}

void unregisterStorageFactories()
{
  foreach (StorageFactory * factory, registeredStorageFactories)
    delete factory;
}


void StorageFormat::statusMsg(const QString & text, const int & type,
                const bool richText, const bool updateLast)
{
  if (_progress)
    _progress->addMessage(text, type, richText, updateLast);

  if (type == QtCriticalMsg || type == QtFatalMsg)
    setError(text);
  else if (type == QtWarningMsg)
    setWarning(text);
}

void StorageFormat::fatalMsg(const QString & text, const bool richText)
{
  statusMsg(text, QtFatalMsg, richText);
}

void StorageFormat::progressSetInfoAndMsg(const QString & text, const int & type, const bool richText)
{
  progressSetInfo(text);
  statusMsg(text, type, richText);
}

void StorageFormat::progressSetInfo(const QString & msg)
{
  if (_progress) _progress->setInfo(msg);
}

void StorageFormat::progressSetLock(const bool lock)
{
  if (_progress) _progress->lock(lock);
}

void StorageFormat::progressSetMaximum(const int max)
{
  if (_progress) _progress->setMaximum(max);
}

void StorageFormat::progressSetValue(const int val)
{
  if (_progress) _progress->setValue(val);
}

bool Storage::load(RadioData & radioData)
{
  if (!fileExists())
    return false;

  bool ret = false;
  StorageFormat *format = getStorageFormat();

  if (format) {
    format->setProgress(_progress);

    if (format->load(radioData)) {
      board = format->getBoard();
      setWarning(format->warning());
      ret = true;
    } else {
      setError(format->error());
    }

    delete format;
  }

  return ret;
}

bool Storage::write(RadioData & radioData)
{
  bool ret = false;
  StorageFormat *format = getStorageFormat();

  if (format) {
    format->setProgress(_progress);
    ret = format->write(radioData);
    delete format;
  }

  return ret;
}

bool Storage::writeModel(const RadioData & radioData, const int modelIndex)
{
  bool ret = false;
  StorageFormat *format = getStorageFormat();

  if (format) {
    ret = format->writeModel(radioData, modelIndex);
    delete format;
  }

  return ret;
}

bool Storage::fileExists()
{
  QFile file(filename);

  if (!file.exists()) {
    setError(tr("Unable to find file %1!").arg(filename));
    return false;
  }

  return true;
}

StorageFormat * Storage::getStorageFormat()
{
  foreach (StorageFactory * factory, registeredStorageFactories) {
    if (factory->probe(filename))
      return factory->instance(filename);
  }

  return nullptr;
}

bool Storage::load(GeneralSettings & generalSettings)
{
  if (!fileExists())
    return false;

  bool ret = false;
  StorageFormat *format = getStorageFormat();

  if (format) {
    if (format->load(generalSettings)) {
      board = format->getBoard();
      setWarning(format->warning());
      ret = true;
    }
    else
      setError(format->error());

    delete format;
  }

  return ret;
}
