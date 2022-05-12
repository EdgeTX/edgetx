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

#ifndef _ETX_H_
#define _ETX_H_

#include "categorized.h"

#include <QtCore>

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
#undef MINIZ_HEADER_FILE_ONLY

class EtxFormat : public CategorizedStorageFormat
{
  Q_DECLARE_TR_FUNCTIONS(EtxFormat)

  public:
    EtxFormat(const QString & filename):
      CategorizedStorageFormat(filename)
    {
    }

    virtual QString name() { return "etx"; }
    virtual bool load(RadioData & radioData);
    virtual bool write(const RadioData & radioData);

  protected:
    virtual bool loadFile(QByteArray & fileData, const QString & fileName);
    virtual bool writeFile(const QByteArray & fileData, const QString & fileName);
    virtual bool getFileList(std::list<std::string>& filelist);

    mz_zip_archive zip_archive;
};

#endif // _ETX_H_
