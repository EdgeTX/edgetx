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

#ifndef _CATEGORIZED_H_
#define _CATEGORIZED_H_

#include "storage.h"

#include <QtCore>
#include <list>
#include <string>

class CategorizedStorageFormat : public StorageFormat
{
  Q_DECLARE_TR_FUNCTIONS(CategorizedStorageFormat)

  public:
    CategorizedStorageFormat(const QString & filename):
      StorageFormat(filename)
    {
    }

    virtual bool load(RadioData & radioData);
    virtual bool write(const RadioData & radioData);

  protected:
    virtual bool loadFile(QByteArray & fileData, const QString & fileName) = 0;
    virtual bool writeFile(const QByteArray & fileData, const QString & fileName) = 0;
    virtual bool getFileList(std::list<std::string>& filelist) = 0;

    virtual bool loadBin(RadioData & radioData);
    virtual bool writeBin(const RadioData & radioData);
    virtual bool loadYaml(RadioData & radioData);
    virtual bool writeYaml(const RadioData & radioData);

    StorageType probeFormat();
};

#endif // _CATEGORIZED_H_
