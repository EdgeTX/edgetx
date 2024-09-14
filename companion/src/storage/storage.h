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

#pragma once

#include "radiodata.h"

#include <QtCore>
#include <QString>
#include <QDebug>

enum StorageType
{
  STORAGE_TYPE_UNKNOWN,
  STORAGE_TYPE_HEX,   // needed for FirmwareInterface
  STORAGE_TYPE_SDCARD,
  STORAGE_TYPE_ETX,
  STORAGE_TYPE_YML
};

StorageType getStorageType(const QString & filename);

class StorageFormat
{
  Q_DECLARE_TR_FUNCTIONS(StorageFormat)

  public:
    StorageFormat(const QString & filename, uint8_t version=0):
      filename(filename),
      version(version),
      board(Board::BOARD_UNKNOWN)
    {
    }
    virtual ~StorageFormat() {}
    virtual bool load(RadioData & radioData) = 0;
    virtual bool write(const RadioData & radioData) = 0;
    virtual bool writeModel(const RadioData & radioData, const int modelIndex) { return false; }

    QString error() {
      return _error;
    }

    QString warning() {
      return _warning;
    }

    virtual QString name() = 0;

    virtual Board::Type getBoard()
    {
      return board;
    }

  protected:
    void setError(const QString & error)
    {
      qDebug() << qPrintable(QString("[%1] error: %2").arg(name()).arg(error));
      _error = error;
    }

    void setWarning(const QString & warning)
    {
      qDebug() << qPrintable(QString("[%1] warning: %2").arg(name()).arg(warning));
      _warning = warning;
    }

    QString filename;
    uint8_t version;
    QString _error;
    QString _warning;
    Board::Type board;
};

class StorageFactory
{
  public:
    StorageFactory()
    {
    }
    virtual ~StorageFactory() {}
    virtual QString name() = 0;
    virtual bool probe(const QString & filename) = 0;
    virtual StorageFormat * instance(const QString & filename) = 0;
};

template <class T>
class DefaultStorageFactory : public StorageFactory
{
  public:
    DefaultStorageFactory(const QString & name):
      StorageFactory(),
      _name(name)
    {
    }

    virtual QString name()
    {
      return _name;
    }

    virtual bool probe(const QString & filename)
    {
      return filename.toLower().endsWith("." + _name);
    }

    virtual StorageFormat * instance(const QString & filename)
    {
      return new T(filename);
    }

    QString _name;
};

class Storage : public StorageFormat
{
  Q_DECLARE_TR_FUNCTIONS(Storage)

  public:
    Storage(const QString & filename):
      StorageFormat(filename)
    {
    }

    virtual QString name() { return "storage"; }

    void setError(const QString & error)
    {
      _error = error;
    }

    void setWarning(const QString & warning)
    {
      _warning = warning;
    }

    virtual bool load(RadioData & radioData);
    virtual bool write(const RadioData & radioData);
    virtual bool writeModel(const RadioData & radioData, const int modelIndex);
};

void registerStorageFactories();
void unregisterStorageFactories();
