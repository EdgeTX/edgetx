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

bool Storage::fileExists()
{
  bool ret = true;
  QFile file(filename);

  if (!file.exists()) {
    setError(tr("Unable to find file %1!").arg(filename));
    ret = false;
  }

  return ret;
}

StorageFormat * Storage::getStorageFormat()
{
  foreach (StorageFactory * factory, registeredStorageFactories) {
    if (factory->probe(filename))
      return factory->instance(filename);
  }

  return nullptr;
}

bool Storage::load(RadioData & radioData)
{
  if (!fileExists())
    return false;

  bool ret = false;
  StorageFormat * format = getStorageFormat();

  if (format) {
    if (format->load(radioData)) {
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

bool Storage::load(GeneralSettings & generalSettings)
{
  if (!fileExists())
    return false;

  bool ret = false;
  StorageFormat * format = getStorageFormat();

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

bool Storage::load(ModelData & modelData)
{
  if (!fileExists())
    return false;

  bool ret = false;
  StorageFormat * format = getStorageFormat();

  if (format) {
    if (format->load(modelData)) {
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

bool Storage::write(const RadioData & radioData)
{
  bool ret = false;
  StorageFormat * format = getStorageFormat();

  if (format) {
    ret = format->write(radioData);
    delete format;
  }

  return ret;
}

bool Storage::write(const GeneralSettings & generalSettings)
{
  bool ret = false;
  StorageFormat * format = getStorageFormat();

  if (format) {
      ret = format->write(generalSettings);
      delete format;
  }

  return ret;
}

bool Storage::write(const ModelData & modelData)
{
  bool ret = false;
  StorageFormat * format = getStorageFormat();

  if (format) {
      ret = format->write(modelData);
      delete format;
  }

  return ret;
}
